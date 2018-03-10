// #include <libstatic/libstatic.h>
#include <errno.h>
// extern long errno;
#define PGSZ 0x1000
#include <elf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
// #include <ulexec.h>
unsigned long
file_size(char *filename)
{
	char sbuf[144];
	unsigned long ret;

	if (0 > (long)(ret = stat(filename, (void *)&sbuf)))
	{
		printf("stat problem: %l\n", errno);
	} else {
		ret = *(unsigned long *)(sbuf+48);
	}

	return ret;
}


void brk_(unsigned long addr)
{
	asm volatile ("syscall" : : "a" (__NR_brk), "D" (addr));
}

struct saved_block {
	int size;
	int cnt;
	char *block;
};
void release_args(struct saved_block *args);
struct saved_block *save_elfauxv(char **envp);
struct saved_block *save_argv(int argc, char **argv);
void *stack_setup(
    struct saved_block *args,
    struct saved_block *envp,
    struct saved_block *auxvp,
    Elf64_Ehdr *ehdr,
    Elf64_Ehdr *ldso
);
#define JMP_ADDR(x) asm("\tjmp  *%0\n" :: "r" (x))
#define SET_STACK(x) asm("\tmovq %0, %%rsp\n" :: "r"(x))
#define ROUNDUP(x, y)   ((((x)+((y)-1))/(y))*(y))
#define ALIGN(k, v) (((k)+((v)-1))&(~((v)-1)))
#define ALIGNDOWN(k, v) ((unsigned long)(k)&(~((unsigned long)(v)-1)))
#define ALLOCATE(size)  \
      mmap(0, (size), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)

void
print_maps(void)
{
	char rbuf[1024];
	int fd, cc;

	fd = open("/proc/self/maps", 0, 0);
	while (0 < (cc = read(fd, rbuf, sizeof(rbuf))))
		write(1, rbuf, cc);
	close(fd);
}

void
error_msg(char *msg)
{
	char buf[32];
	printf("%s, %d\n", msg, errno);
}

void
print_address(char *phrase, void *address)
{
	printf("%s, 0x%08x\n", phrase, address);
}


void *
memcopy(void *dest, const void *src, unsigned long n)
{
    unsigned long i;
    unsigned char *d = (unsigned char *)dest;
    unsigned char *s = (unsigned char *)src;

    for (i = 0; i < n; ++i)
        d[i] = s[i];

    return dest;
}

void
copy_in(char *filename, void *address)
{
	int fd, cc;
	off_t offset = 0;
	char buf[1024];

	if (0 > (fd = open(filename, 0, 0)))
	{
		error_msg("opening dynamically-loaded file failed");
		exit(2);
	}

	while (0 < (cc = read(fd, buf, sizeof(buf))))
	{
		memcpy((address + offset), buf, cc);
		offset += cc;
	}

	close(fd);
}

void *
map_file(char *file_to_map)
{
	struct stat sb;
	void *mapped;

	if (0 > stat(file_to_map, &sb))
	{
		error_msg("map_file stat() failed ");
		exit(1);
	}

	mapped = mmap(NULL, sb.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

	if (mapped == (void *)-1)
	{
		error_msg("map_file mmap() failed ");
		exit(1);
	}

	copy_in(file_to_map, mapped);

	return mapped;
}

void
unmap(char *progname)
{
	char buf[1024], *p;
	char rbuf[2048];
	int cc, fd;

	fd = open("/proc/self/maps", 0, 0);

	p = &buf[0];

	while (0 < (cc = read(fd, rbuf, sizeof(rbuf))))
	{
		int i;

		for (i = 0; i < cc; ++i)
		{
			int c = rbuf[i];

			if ('\n' != c)
				*p++ = c;
			else {
				*p = '\0';
				/* When a line from /proc/self/maps shows up as having been
				 * mapped in from this running program, ld.so or libc, unmap it.
				 * This will keep the exec'd program's address space a lot
				 * cleaner.  But even a 32-bit address space can hold 2 copies
				 * of glibc without ill effects, so you don't really have to
				 * munmap() anything other than the program calling ul_exec() */
				if (strstr(buf, progname) || /* strstr(buf, "libdl") || strstr(buf, "/usr/lib/ld-")
					|| strstr(buf, "/lib64/ld-") || */strstr(buf, "libc"))
				{
					char *u;
					char *first, *second;
					unsigned long low, high;

					u = strchr(buf, ' ');
					*u = '\0';

					first = buf;

					second = strchr(first, '-');
					*second = '\0';
					++second;

					low = strtoul(first, NULL, 0x10);
					high = strtoul(second, NULL, 0x10);
                    printf("before unap:\n");
//                     print_maps();
					munmap((void *)low, high-low);
                    printf("after unap:\n");
//                     print_maps();
				}

				p = &buf[0];
			}
		}
	}

	close(fd);

}

/* call with argc as positive value for main's argv,
 * call with argc == 0 for env. */
struct saved_block *
save_argv(int argc, char **argv)
{
	struct saved_block *r = NULL;
	int i, len;
	char *str;

	if (argc > 0)
		for (i = 0, len = 0; i < argc; ++i)
			len += strlen(argv[i]) + 1;
	else {
		argc = 0;
		char **p = argv;
		while (*p)
		{
			len += strlen(*p) + 1;
			++p;  /* move past ASCII Nul */
			++argc;
		}
	}

	r = ALLOCATE(sizeof(*r));
	r->size = len;
	r->cnt = argc;
	r->block = ALLOCATE(len);

	/* Do it this way because the values of argv[] may not actually
	 * exist as contiguous strings.  We will make them contiguous. */
	for (i = 0, str = r->block; i < argc; i++)
	{
		int j;
		for (j = 0; argv[i][j]; ++j)
			str[j] = argv[i][j];
		str[j] = '\0';
		str += (j + 1);
	}

	return r;
}

void
release_args(struct saved_block *args)
{
	munmap((void *)args->block, args->size);
	munmap((void *)args, sizeof(*args));
}


struct saved_block *
save_elfauxv(char **envp)
{
	struct saved_block *r;
	unsigned long *p;
	int cnt;
	Elf64_auxv_t *q;

	p = (unsigned long *)envp;
	while (*p != 0)
		++p;

	++p; /* skip null word after env */

	for (cnt = 0, q = (Elf64_auxv_t *)p; q->a_type != AT_NULL; ++q)
		++cnt;

	++cnt; /* The AT_NULL final entry */

	r = ALLOCATE(sizeof(*r));
	r->size = sizeof(*q) * cnt;
	r->cnt = cnt;
	r->block = ALLOCATE(r->size);
	memcpy((void *)r->block, (void *)p, r->size);

	return r;
}

/* Returns value for %rsp, the new "bottom of the stack */
void *
stack_setup(
	struct saved_block *args,
	struct saved_block *envp,
    struct saved_block *auxvp,
	Elf64_Ehdr *ehdr,
	Elf64_Ehdr *ldso
)
{
	Elf64_auxv_t	*aux, *excfn = NULL;
	char **av, **ev;
	char	*addr, *str, *rsp;
	unsigned long *ptr;
	int	  i, j;
	char newstack[16384];

	/* Align new stack. */
	rsp = (char *)ALIGN(((unsigned long)&newstack[150]), 16);

	/* 
	 * After returning from
	 * stack_setup(), don't do anything that uses the call stack: that
	 * will roach this newly-constructed stack.
	 */

	ptr = (unsigned long *)rsp;

	*ptr++ = args->cnt; /* set argc */
	av = (char **)ptr;
	ptr += args->cnt;  /* skip over argv[] */
	*ptr++ = 0;

	ev = (char **)ptr;
	ptr += envp->cnt;  /* skip over envp[] */
	*ptr++ = 0;

	aux = (Elf64_auxv_t *)ptr;

	ptr = (unsigned long *)ROUNDUP((unsigned long)ptr + auxvp->size, sizeof(unsigned long));
	
	/* copy ELF auxilliary vector table */
	addr =  (char *)aux;
	for (j = 0; j < auxvp->size; ++j)
		addr[j] = auxvp->block[j];

	/* Fix up a few entries: kernel will have set up the AUXV
	 * for the user-land exec program, mapped in at a low address.
	 * need to fix up a few AUXV entries for the "real" program. */
	for (i = 0; i < auxvp->cnt; ++i)
	{
		switch (aux[i].a_type)
		{
		case AT_PHDR:  aux[i].a_un.a_val = (unsigned long)((char *)ehdr + ehdr->e_phoff); break;
		case AT_PHNUM: aux[i].a_un.a_val = ehdr->e_phnum; break;
		case AT_BASE:  aux[i].a_un.a_val = (unsigned long)ldso; break;
		case AT_ENTRY: aux[i].a_un.a_val = (unsigned long)ehdr->e_entry; break;
#ifdef AT_EXECFN
		case AT_EXECFN: excfn = &(aux[i]); break;
#endif
		}
	}

	*ptr++ = 0;

	/* Copy argv strings onto stack */
	addr =  (char *)ptr;
	str = args->block;

	for (i = 0; i < args->cnt; ++i)
	{
		av[i] = addr;
		for (j = 0; *str; ++j)
			*addr++ = *str++;
		*addr++ = *str++;  /* ASCII Nul */
	}

	ptr = (unsigned long *)ROUNDUP((unsigned long)addr, sizeof(unsigned long));
	*ptr = 0;

	/* Copy envp strings onto stack */
	addr =  (char *)ptr;
	str = envp->block;

	for (i = 0; i < envp->cnt; ++i)
	{
		ev[i] = addr;
		for (j = 0; *str; ++j)
			*addr++ = *str++;
		*addr++ = *str++;  /* ASCII Nul */
	}

	ptr = (unsigned long *)ROUNDUP((unsigned long)addr, sizeof(unsigned long));
	*ptr = 0;

	/* Executable name at top of stack */
	if (excfn)
	{
		addr =  (char *)ptr;
		str = args->block;
		excfn->a_un.a_val = (unsigned long)addr;
		for (j = 0; *str; ++j)
			*addr++ = *str++;
		*addr++ = *str++;  /* ASCII Nul */

		ptr = (unsigned long *)ROUNDUP((unsigned long)addr, sizeof(unsigned long));
	}

	release_args(args);
	release_args(envp);
	release_args(auxvp);

	return ((void *)rsp);
}

int read_(const char *src, char **dest, int len) {
    char *p = malloc(len + 1);
    memcpy(p, src, len);
    p[len] = 0;
    *dest = p;
    return len;
}
void lseek_string(char **src, int len, int offset) {
    char *p = malloc(len);
    memcpy(p, *src+offset, len);
    *src = p;
}
#define QUOTE_0_TERMINATED			0x01
#define QUOTE_OMIT_LEADING_TRAILING_QUOTES	0x02
#define QUOTE_OMIT_TRAILING_0			0x08
#define QUOTE_FORCE_HEX				0x10
#define QUOTE_FORCE_LEN				9999
int
string_quote(const char *instr, char *outstr, const unsigned int size, const unsigned int style)
{
    const unsigned char *ustr = (const unsigned char *) instr;
    char *s = outstr;
    unsigned int i;
    int usehex, uselen, c;

    int xflag = 0;
    usehex = 0;
    uselen = 0;
    if ((style == 9999)) {
        uselen = 1;
    } else if ((xflag > 1) || (style & QUOTE_FORCE_HEX)) {
        usehex = 1;
    } else if (xflag) {
        /* Check for presence of symbol which require
        to hex-quote the whole string. */
        for (i = 0; i < size; ++i) {
            c = ustr[i];
            /* Check for NUL-terminated string. */
            if (c == 0x100)
                break;

            /* Force hex unless c is printable or whitespace */
            if (c > 0x7e) {
                usehex = 1;
                break;
            }
            /* In ASCII isspace is only these chars: "\t\n\v\f\r".
            * They happen to have ASCII codes 9,10,11,12,13.
            */
            if (c < ' ' && (unsigned)(c - 9) >= 5) {
                usehex = 1;
                break;
            }
        }
    }

    if (!(style & QUOTE_OMIT_LEADING_TRAILING_QUOTES))
        *s++ = '\"';

    if (usehex == 1) {
        /* Hex-quote the whole string. */
        for (i = 0; i < size; ++i) {
            c = ustr[i];
            /* Check for NUL-terminated string. */
            if (c == 0x100)
                goto asciz_ended;
            // print hex in " 00 00" format instead of "\x00\x00" format
//             *s++ = '\\';
            *s++ = ' ';
            *s++ = "0123456789abcdef"[c >> 4];
            *s++ = "0123456789abcdef"[c & 0xf];
        }
    } else if (uselen == 1) {
        /* Hex-quote the whole string. */
        for (i = 0; i < size; ++i) {
            c = ustr[i];
            /* Check for NUL-terminated string. */
            if (c == 0x100)
                goto asciz_ended;
            *s++ = '1';
        }
    } else {
        for (i = 0; i < size; ++i) {
            c = ustr[i];
            /* Check for NUL-terminated string. */
            if (c == 0x100)
                goto asciz_ended;
            if ((i == (size - 1)) &&
                (style & QUOTE_OMIT_TRAILING_0) && (c == '\0'))
                goto asciz_ended;
                int pass_one = 0;
                int pass_two = 0;
                int pass_three = 0;
                int pass_four = 0;
                if (c == '\f') {
                    *s++ = '\\';
                    *s++ = 'f';
                    pass_one = 1;
                    pass_three = 1;
                    pass_four= 1;
                }
                if (pass_one == 0) {
                    if (c == '%'/*FOR PRINTF*/) {
                        *s++ = '%';
                        *s++ = '%';
                        pass_two = 1;
                        pass_three = 1;
                        pass_four= 1;
                    } else {
                        pass_two = 1;
                    }
                }
                if (pass_two == 0) {
                    if (c == '\"') {
                        /*FOR PRINTF/SHELL*/
                        *s++ = '\\';
                        *s++ = '\"';
                        pass_three = 1;
                        pass_four= 1;
                    } else if (c == '\\') {
                        /*FOR PRINTF/SHELL*/
                        *s++ = '\\';
                        *s++ = '\\';
                        pass_three = 1;
                        pass_four= 1;
                    } else if (c == '`'/*FOR PRINTF*/|| c == '$'/*FOR BASH*/) {
//                             *s++ = '\\';
                        *s++ = c;
                        pass_three = 1;
                        pass_four= 1;
                    } else if (c == '\''/*FOR PRINTF*/) {
//                             *s++ = '\\';
//                             *s++ = 'x';
//                             *s++ = '2';
                        *s++ = c;
                        pass_three = 1;
                        pass_four= 1;
                    } else if (c == '!'/*FOR BASH*/ || c ==  '-'/*FOR PRINTF*/) {
//                             *s++ = '"';
//                             *s++ = '\'';
                        *s++ = c;
//                             *s++ = '\'';
//                             *s++ = '"';
                        pass_three = 1;
                        pass_four= 1;
                    } else if (c == '%'/*FOR PRINTF*/) {
                        *s++ = '%';
                        *s++ = '%';
                        *s++ = '%';
                        *s++ = '%';
                        pass_three = 1;
                        pass_four= 1;
                    }
                }
                if (pass_three == 0) {
                    if (c == '\n') {
                        *s++ = '\\';
                        *s++ = 'n';
                        pass_four = 1;
                    } else if (c == '\r') {
                        *s++ = '\\';
                        *s++ = 'r';
                        pass_four = 1;
                    } else if (c == '\t') {
                        *s++ = '\\';
                        *s++ = 't';
                        pass_four = 1;
                    } else if (c == '\v') {
                        *s++ = '\\';
                        *s++ = 'v';
                        pass_four = 1;
                    }
                }
                if (pass_four == 0) {
                    if (c >= ' ' && c <= 0x7e)
                        *s++ = c;
                    else {
                        /* Print \octal */
                        *s++ = '\\';
                        if (i + 1 < size
                            && ustr[i + 1] >= '0'
                            && ustr[i + 1] <= '9'
                        ) {
                            /* Print \ooo */
                            *s++ = '0' + (c >> 6);
                            *s++ = '0' + ((c >> 3) & 0x7);
                        } else {
                            /* Print \[[o]o]o */
                            if ((c >> 3) != 0) {
                                if ((c >> 6) != 0)
                                    *s++ = '0' + (c >> 6);
                                *s++ = '0' + ((c >> 3) & 0x7);
                            }
                        }
                        *s++ = '0' + (c & 0x7);
                    }
            }
        }
    }

    if (!(style & QUOTE_OMIT_LEADING_TRAILING_QUOTES))
        *s++ = '\"';
    *s = '\0';

    /* Return zero if we printed entire ASCIZ string (didn't truncate it) */
    if (style & QUOTE_0_TERMINATED && ustr[i] == '\0') {
        /* We didn't see NUL yet (otherwise we'd jump to 'asciz_ended')
        * but next char is NUL.
        */
        return 0;
    }

    return 1;

asciz_ended:
    if (!(style & QUOTE_OMIT_LEADING_TRAILING_QUOTES))
        *s++ = '\"';
    *s = '\0';
    /* Return zero: we printed entire ASCIZ string (didn't truncate it) */
    return 0;
}

#ifndef ALLOCA_CUTOFF
# define ALLOCA_CUTOFF	4032
#endif
#define use_alloca(n) ((n) <= ALLOCA_CUTOFF)

/*
* Quote string `str' of length `size' and print the result.
*
* If QUOTE_0_TERMINATED `style' flag is set,
* treat `str' as a NUL-terminated string and
* quote at most (`size' - 1) bytes.
*
* If QUOTE_OMIT_LEADING_TRAILING_QUOTES `style' flag is set,
* do not add leading and trailing quoting symbols.
*
* Returns 0 if QUOTE_0_TERMINATED is set and NUL was seen, 1 otherwise.
* Note that if QUOTE_0_TERMINATED is not set, always returns 1.
*/
char *
print_quoted_string(const char *str, unsigned int size, const unsigned int style, const char * return_type)
{
    char *buf;
    char *outstr;
    unsigned int alloc_size;
    int rc;

    if (size && style & QUOTE_0_TERMINATED)
        --size;

    alloc_size = 4 * size;
    if (alloc_size / 4 != size) {
        error_msg("Out of memory");
        printf("???");
        return "-1";
    }
    alloc_size += 1 + (style & QUOTE_OMIT_LEADING_TRAILING_QUOTES ? 0 : 2);

    if (use_alloca(alloc_size)) {
        outstr = alloca(alloc_size);
        buf = NULL;
    } else {
        outstr = buf = malloc(alloc_size);
        if (!buf) {
            error_msg("Out of memory");
            printf("???");
            return "-1";
        }
    }

//         rc = string_quote(str, outstr, size, style);
    string_quote(str, outstr, size, style);
    if ( return_type == "return") {
        return outstr;
    } else if ( return_type == "print") {
        printf(outstr);
    }

    free(buf);
//         return rc;
}

void *
load_elf(char *mapped, int anywhere, Elf64_Ehdr **elf_ehdr, Elf64_Ehdr **ldso_ehdr, void * mapped_b)
{
	Elf64_Ehdr *hdr;
	Elf64_Phdr *pdr, *interp = NULL;
	int i;
	void *text_segment = NULL;
	void *entry_point = NULL;
	unsigned long initial_vaddr = 0;
	unsigned long brk_addr = 0;
	char buf[128];
	unsigned int mapflags = MAP_PRIVATE|MAP_ANONYMOUS;

	if (!anywhere)
		mapflags |= MAP_FIXED;

	/* Just addresses in mapped-in file. */
	hdr = (Elf64_Ehdr *)mapped;
	pdr = (Elf64_Phdr *)(mapped + hdr->e_phoff);

	entry_point = (void *)hdr->e_entry;

	for (i = 0; i < hdr->e_phnum; ++i, ++pdr)
	{
		unsigned int protflags = 0;
		unsigned long map_addr = 0, rounded_len, k;
		unsigned long unaligned_map_addr = 0;
		void *segment;

		if (pdr->p_type == 0x03)  /* PT_INTERP */
		{
			interp = pdr;
			continue;
		}

		if (pdr->p_type != PT_LOAD)  /* Segment not "loadable" */
			continue;

		if (text_segment != 0 && anywhere)
		{
			unaligned_map_addr
				= (unsigned long)text_segment
				+ ((unsigned long)pdr->p_vaddr - (unsigned long)initial_vaddr)
			;
			map_addr = ALIGNDOWN((unsigned long)unaligned_map_addr, 0x1000);
			mapflags |= MAP_FIXED;
		} else if (!anywhere) {
			map_addr = ALIGNDOWN(pdr->p_vaddr, 0x1000);
		} else {
			map_addr = 0UL;
		}

		if (!anywhere && initial_vaddr == 0)
			initial_vaddr = pdr->p_vaddr;

		/* mmap() freaks out if you give it a non-multiple of pagesize */
		rounded_len = (unsigned long)pdr->p_memsz + ((unsigned long)pdr->p_vaddr % 0x1000);
		rounded_len = ROUNDUP(rounded_len, 0x1000);

		segment = mmap(
			(void *)map_addr,
			rounded_len,
			PROT_WRITE, mapflags, -1, 0
		);

		if (segment == (void *) -1)
		{
			printf("Failed to mmap()");
			exit(3);
		}

		printf("anywhere = %d\n", anywhere);
        print_maps();
		memcopy(
			!anywhere? (void *)pdr->p_vaddr:
			(void *)((unsigned long)segment + ((unsigned long)pdr->p_vaddr % 0x1000)),
			mapped + pdr->p_offset,
			pdr->p_filesz
		);

		if (!text_segment)
		{
			*elf_ehdr = segment;
			text_segment = segment;
			initial_vaddr = pdr->p_vaddr;
			if (anywhere)
				entry_point = (void *)((unsigned long)entry_point
					- (unsigned long)pdr->p_vaddr
					+ (unsigned long)text_segment);
		}


		if (pdr->p_flags & PF_R)
			protflags |= PROT_READ;
		if (pdr->p_flags & PF_W)
			protflags |= PROT_WRITE;
		if (pdr->p_flags & PF_X)
			protflags |= PROT_EXEC;

		mprotect(segment, rounded_len, protflags);

		k = pdr->p_vaddr + pdr->p_memsz;
		if (k > brk_addr) brk_addr = k;
	}

	if (interp)
	{
        Elf64_Ehdr *junk_ehdr = NULL;
        printf("LOAD_ELF mapping %p\n", mapped_b);
		entry_point = load_elf(mapped_b, 1, ldso_ehdr, &junk_ehdr, NULL);
	}

	if (!anywhere)
		brk_(ROUNDUP(brk_addr, 0x1000));

	return (void *)entry_point;
}

char *strjoin(const char *_a, const char *_b) {
    size_t na = strlen(_a);
    size_t nb = strlen(_b);
    char *p = malloc(na + nb + 1);
    memcpy(p, _a, na);
    memcpy(p + na, _b, nb);
    p[na + nb] = 0;
    return p;
}

int shift_split(char * argv[], char * program[], char * args[], int * ac) { // shift function modified for this purpose
    char ** args_tmp = malloc(1 * sizeof(*args_tmp));
    for(int i=0; i<99; i++) {
        if (argv[i] == NULL) {
            printf("end of argument list\n");
            break;
        }
        if (i == 0) {
        } else if (i == 1) {
            printf("program[%d] = %s\n", *ac, argv[i]);
            program[0] = argv[i];
            *ac = *ac+1;
        } else {
            printf("args[%d] = %s\n", *ac-1, argv[i]);
            args[i-2] = argv[i];
            *ac = *ac+1;
        }
    }
    return 0;
}

int split (char *str, char c, char ***arr)
{
    int count = 1;
    int token_len = 1;
    int i = 0;
    char *p;
    char *t;

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
            count++;
        p++;
    }

    *arr = (char**) malloc(sizeof(char*) * count);
    if (*arr == NULL)
        exit(1);

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
        {
            (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
            if ((*arr)[i] == NULL)
                exit(1);

            token_len = 0;
            i++;
        }
        p++;
        token_len++;
    }
    (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
    if ((*arr)[i] == NULL)
        exit(1);

    i = 0;
    p = str;
    t = ((*arr)[i]);
    while (*p != '\0')
    {
        if (*p != c && *p != '\0')
        {
            *t = *p;
            t++;
        }
        else
        {
            *t = '\0';
            i++;
            t = ((*arr)[i]);
        }
        p++;
    }

    return count;
}

void
ulexec(char * pro, char * args, char **env)
{
    int arg_c = 0;
    char ** program_to_execute = malloc(1 * sizeof(*program_to_execute));
    printf("allocating %d\n", 1 * sizeof(*program_to_execute));
    program_to_execute[0] = "placeholder";
    char ** program_arguments = malloc(1 * sizeof(*program_arguments));
    printf("allocating %d\n", 1 * sizeof(*program_arguments));
    program_arguments[0] = "placeholder";
    char ** program_program_arguments = malloc(1 * sizeof(*program_program_arguments));
    program_program_arguments[0] = "placeholder";
//     shift_split(av, program_to_execute, program_arguments, program_program_arguments, &arg_c);
    char * s = strjoin(pro, " ");
    s = strjoin(s, args);
    int c = 0;
    char **arr = NULL;

    c = split(s, ' ', &arr);

    printf("found %d tokens.\n", c-1);

    
    for (int i = 0; i < c; i++) {
        program_program_arguments[i] = arr[i];
        printf("program_program_arguments[%d] = %s\n", i, program_program_arguments[i]);
    }

    program_to_execute[0] = program_program_arguments[0];

    printf("program_to_execute[%d] = %s\n", 0, program_to_execute[0]);

    for (int i = 1; i < c; i++) {
        program_arguments[i-1] = arr[i];
        printf("program_arguments[%d] = %s\n", i-1, program_arguments[i-1]);
    }
    
    printf("number of arguments: \n%d\nprogram: \n%s\n", c, program_to_execute[0]);
    
    for (int i = 0; i<=arg_c-2; i++)
        printf("program args: %d = \n%s\n", i, program_arguments[i]);
    
    print_maps();
	int how_to_map = 0;
	void *mapped;
	void *entry_point;
	struct stat sb;
	Elf64_Ehdr *ldso_ehdr;
	struct saved_block *argvb, *envb, *elfauxvb;
	int trim_args, i;
	void *stack_bottom;

	how_to_map = 0;
	trim_args = 1;

// 	if (file_to_unmap)
// 		unmap(file_to_unmap);
    void * mapped_interp;
    const char * filename = program_to_execute[0];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("cannot open \"%s\", returned %i\n", filename, fd);
    }
    size_t len = 0;
    len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, 0);
    mapped = mmap (NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        printf ("map failed\n");
        exit;
    } else {
        printf ("map (%s) succeded with address: %p\n", filename, &mapped);
    }
//     mapped = map_file(av[1]);
// 	elf_ehdr = (Elf64_Ehdr *)mapped;
    printf("aquiring header\n");
    Elf64_Ehdr * elf_ehdr = (Elf64_Ehdr *) mapped;    
// 	phdr = (Elf64_Phdr *)((unsigned long)elf_ehdr + elf_ehdr->e_phoff);
    printf("aquiring program header\n");
    Elf64_Phdr *phdr = (Elf64_Phdr *)((unsigned long)elf_ehdr + elf_ehdr->e_phoff);
    printf("searching for PT_LOAD and PT_INTERP\n");
    void * mapped_i;
	for (i = 0; i < elf_ehdr->e_phnum; ++i) {
		if (phdr[i].p_type == PT_LOAD && phdr[i].p_vaddr == 0)
		{
			how_to_map = 1;  /* map it anywhere, like ld.so, or PIC code. */
            printf("mapping anywhere\n");
			break;
		}
		if (phdr[i].p_type == PT_INTERP)
        {
            printf("ATTEMPING TO READ\n");
            char * tmp99;
            read_(mapped, &tmp99, (phdr[i].p_memsz + phdr[i].p_offset));
            lseek_string(&tmp99, phdr[i].p_memsz, phdr[i].p_offset);
            print_quoted_string(tmp99, phdr[i].p_memsz, QUOTE_OMIT_TRAILING_0|QUOTE_OMIT_LEADING_TRAILING_QUOTES, "print");
            printf("\nREAD\n");
            const char * filename = print_quoted_string(tmp99, phdr[i].p_memsz, QUOTE_OMIT_TRAILING_0|QUOTE_OMIT_LEADING_TRAILING_QUOTES, "return");
            int fd = open(filename, O_RDONLY); // usually /lib64/ld-linux-x86-64.so.2
            if (fd < 0) {
                printf ("cannot open \""); print_quoted_string(tmp99, phdr[i].p_memsz, QUOTE_OMIT_TRAILING_0|QUOTE_OMIT_LEADING_TRAILING_QUOTES, "print"); printf("\", returned %i\n", fd);
            }
            size_t len = 0;
            len = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, 0);
            mapped_i = mmap (NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
            if (mapped_i == MAP_FAILED) {
                printf ("map ("); print_quoted_string(tmp99, phdr[i].p_memsz, QUOTE_OMIT_TRAILING_0|QUOTE_OMIT_LEADING_TRAILING_QUOTES, "print"); printf(") failed\n");
                exit;
            } else {
                printf ("map ("); print_quoted_string(tmp99, phdr[i].p_memsz, QUOTE_OMIT_TRAILING_0|QUOTE_OMIT_LEADING_TRAILING_QUOTES, "print"); printf(") succeded with address: %p\n", &mapped_i);
            }
        }
    }


    printf("loading\n");
	entry_point = load_elf(mapped, how_to_map, &elf_ehdr, &ldso_ehdr, mapped_i);

    printf("unmapping\n");
	munmap(mapped, sb.st_size);

    printf("argvb = save_argv(%d, %s);\n", c, program_program_arguments[0]);
	argvb = save_argv(c, &program_program_arguments[0]);
// 	argvb = save_argv(ac - trim_args, &av[trim_args]);
	envb = save_argv(0, env);
	elfauxvb = save_elfauxv(env);

	stack_bottom = stack_setup(argvb, envb, elfauxvb, elf_ehdr, ldso_ehdr);

	SET_STACK(stack_bottom);
    printf("printing maps before executing\n");
    print_maps();
    printf("jumping to %p\n", entry_point);
	JMP_ADDR(entry_point);
}

void
ulexec_array(void * mapped, void * mapped_interpreter, char * args, char **env)
{
    int arg_c = 0;
    char ** program_arguments = malloc(1 * sizeof(*program_arguments));
    printf("allocating %d\n", 1 * sizeof(*program_arguments));
    program_arguments[0] = "placeholder";
    int c = 0;
    char **arr = NULL;

    c = split(args, ' ', &arr);

    printf("found %d tokens.\n", c);

    
    for (int i = 0; i < c; i++) {
        program_arguments[i+1] = arr[i];
        printf("program_arguments[%d] = %s\n", i+1, program_arguments[i+1]);
    }
    
    
    for (int i = 0; i<=arg_c-2; i++)
        printf("program args: %d = \n%s\n", i, program_arguments[i]);
    
    print_maps();
	int how_to_map = 0;
	void *entry_point;
	struct stat sb;
	Elf64_Ehdr *ldso_ehdr;
	struct saved_block *argvb, *envb, *elfauxvb;
	int trim_args, i;
	void *stack_bottom;

	how_to_map = 0;
	trim_args = 1;

// 	if (file_to_unmap)
// 		unmap(file_to_unmap);
    void * mapped_interp;
//     mapped = map_file(av[1]);
// 	elf_ehdr = (Elf64_Ehdr *)mapped;
    printf("aquiring header\n");
    Elf64_Ehdr * elf_ehdr = (Elf64_Ehdr *) mapped;    
// 	phdr = (Elf64_Phdr *)((unsigned long)elf_ehdr + elf_ehdr->e_phoff);
    printf("aquiring program header\n");
    Elf64_Phdr *phdr = (Elf64_Phdr *)((unsigned long)elf_ehdr + elf_ehdr->e_phoff);
    printf("searching for PT_LOAD and PT_INTERP\n");
    for (i = 0; i < elf_ehdr->e_phnum; ++i) {
        if (phdr[i].p_type == PT_LOAD && phdr[i].p_vaddr == 0)
        {
            how_to_map = 1;  /* map it anywhere, like ld.so, or PIC code. */
            printf("mapping anywhere\n");
            break;
        }
    }


    printf("loading\n");
	entry_point = load_elf(mapped, how_to_map, &elf_ehdr, &ldso_ehdr, mapped_interpreter);

    printf("unmapping\n");
	munmap(mapped, sb.st_size);

    printf("argvb = save_argv(%d, %s);\n", c+1, program_arguments[0]);
	argvb = save_argv(c+1, &program_arguments[0]);
	envb = save_argv(0, env);
	elfauxvb = save_elfauxv(env);

	stack_bottom = stack_setup(argvb, envb, elfauxvb, elf_ehdr, ldso_ehdr);

	SET_STACK(stack_bottom);
    printf("printing maps before executing\n");
    print_maps();
    printf("jumping to %p\n", entry_point);
	JMP_ADDR(entry_point);
}
