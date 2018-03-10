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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
extern char **environ;
// "look deep into yourself, Clarice"  -- Hanibal Lector
char findyourself_save_pwd[PATH_MAX];
char findyourself_save_argv0[PATH_MAX];
char findyourself_save_path[PATH_MAX];
char findyourself_path_separator='/';
char findyourself_path_separator_as_string[2]="/";
char findyourself_path_list_separator[8]=":";  // could be ":; "
char findyourself_debug=0;

int findyourself_initialized=0;

void findyourself_init(char *argv0)
{

  getcwd(findyourself_save_pwd, sizeof(findyourself_save_pwd));

  strncpy(findyourself_save_argv0, argv0, sizeof(findyourself_save_argv0));
  findyourself_save_argv0[sizeof(findyourself_save_argv0)-1]=0;

  strncpy(findyourself_save_path, getenv("PATH"), sizeof(findyourself_save_path));
  findyourself_save_path[sizeof(findyourself_save_path)-1]=0;
  findyourself_initialized=1;
}


int find_yourself(char *result, size_t size_of_result)
{
  char newpath[PATH_MAX+256];
  char newpath2[PATH_MAX+256];

  assert(findyourself_initialized);
  result[0]=0;

  if(findyourself_save_argv0[0]==findyourself_path_separator) {
    if(findyourself_debug) printf("  absolute path\n");
     realpath(findyourself_save_argv0, newpath);
     if(findyourself_debug) printf("  newpath=\"%s\"\n", newpath);
     if(!access(newpath, F_OK)) {
        strncpy(result, newpath, size_of_result);
        result[size_of_result-1]=0;
        return(0);
     } else {
    perror("access failed 1");
      }
  } else if( strchr(findyourself_save_argv0, findyourself_path_separator )) {
    if(findyourself_debug) printf("  relative path to pwd\n");
    strncpy(newpath2, findyourself_save_pwd, sizeof(newpath2));
    newpath2[sizeof(newpath2)-1]=0;
    strncat(newpath2, findyourself_path_separator_as_string, sizeof(newpath2));
    newpath2[sizeof(newpath2)-1]=0;
    strncat(newpath2, findyourself_save_argv0, sizeof(newpath2));
    newpath2[sizeof(newpath2)-1]=0;
    realpath(newpath2, newpath);
    if(findyourself_debug) printf("  newpath=\"%s\"\n", newpath);
    if(!access(newpath, F_OK)) {
        strncpy(result, newpath, size_of_result);
        result[size_of_result-1]=0;
        return(0);
     } else {
    perror("access failed 2");
      }
  } else {
    if(findyourself_debug) printf("  searching $PATH\n");
    char *saveptr;
    char *pathitem;
    for(pathitem=strtok_r(findyourself_save_path, findyourself_path_list_separator,  &saveptr); pathitem; pathitem=strtok_r(NULL, findyourself_path_list_separator, &saveptr) ) {
       if(findyourself_debug>=2) printf("pathitem=\"%s\"\n", pathitem);
       strncpy(newpath2, pathitem, sizeof(newpath2));
       newpath2[sizeof(newpath2)-1]=0;
       strncat(newpath2, findyourself_path_separator_as_string, sizeof(newpath2));
       newpath2[sizeof(newpath2)-1]=0;
       strncat(newpath2, findyourself_save_argv0, sizeof(newpath2));
       newpath2[sizeof(newpath2)-1]=0;
       realpath(newpath2, newpath);
       if(findyourself_debug) printf("  newpath=\"%s\"\n", newpath);
      if(!access(newpath, F_OK)) {
          strncpy(result, newpath, size_of_result);
          result[size_of_result-1]=0;
          return(0);
      } 
    } // end for
    perror("access failed 3");

  } // end else
  // if we get here, we have tried all three methods on argv[0] and still haven't succeeded.   Include fallback methods here.
  return(1);
}
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

char *strjoinb(const char *_a, const char *_b) {
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

// not used but kept incase needed, a version of lseek_string that has an offset multiplier as so this does not need to be specified multiple times, eg if offset is 64 and multiplier is 2 the offset is then 128, this is intended for loops and related
void lseek_stringb(char **src, int len, int offset, int offsetT) {
    char *p = malloc(len);
    int off;
    off=((len*offsetT));
    memcpy(p, *src+offset+off, len);
    *src = p;
}

char *strjoin(const char *_a, const char *_b, int _a_len, int len) {
    size_t na = _a_len;
    size_t nb = len;
    char *p = malloc(na + nb + 1);
    memcpy(p, _a, na);
    memcpy(p + na, _b, nb);
    p[na + nb] = 0;
    return p;
}

int stream__(char *file, char **p, int *q, int LINES_TO_READ) {
            const char *filename = file;
            int fd = open(filename, O_RDONLY);
            if (fd < 0) {
                printf("cannot open \"%s\", returned %i\n", filename, fd);
                return -1;
            }
            char * array;
            char ch;
            size_t lines = 1;
            // Read the file byte by byte
            int bytes=1;
            int count=1;
            array = malloc(sizeof(char) * 2048);
            char *array_tmp;
            while (read(fd, &ch, 1) == 1) {
            printf("\rbytes read: %'i", bytes);
                if (count == 1024) { array_tmp = realloc(array, bytes+1024);
                    if (array_tmp == NULL) {
                        printf("failed to allocate array to new size");
                        free(array);
                        exit(1);
                    } else {
                        array = array_tmp;
                    }
                    count=1;
                }
                array[bytes-1] = ch;
                if (ch == '\n') {
                    if (lines == LINES_TO_READ) {
                        break;
                    }
                    lines++;
                }
                count++;
                bytes++;
            }
            bytes--;
            array_tmp = realloc(array, bytes);
            if (array_tmp == NULL) {
                printf("failed to allocate array to new size");
                free(array);
                exit(1);
            } else {
                array = array_tmp;
            }
            printf("\rbytes read: %'i\n", bytes);
    *p = array;
    *q = bytes;
    return bytes;
}

// not used but kept incase needed, a version of stream__ that only outputs the last line read
int stream__o(char *file, char **p, int *q, int LINES_TO_READ) {
            const char *filename = file;
            int fd = open(filename, O_RDONLY);
            if (fd < 0) {
                printf("cannot open \"%s\", returned %i\n", filename, fd);
                return -1;
            }
            char * array;
            char * array_tmp;
            char * array_lines;
            char * array_lines_tmp;
            char ch;
            size_t lines = 1;
            // Read the file byte by byte
            int bytes=1;
            int count=1;
            array = malloc(sizeof(char) * 2048);
            while (read(fd, &ch, 1) == 1) {
            printf("\rbytes read: %'i", bytes);
                if (count == 1024) { array_tmp = realloc(array, bytes+1024);
                    if (array_tmp == NULL) {
                        printf("failed to allocate array to new size");
                        free(array);
                        exit(1);
                    } else {
                        array = array_tmp;
                    }
                    count=1;
                }
                array[bytes-1] = ch;
                if (ch == '\n') {
                    printf("attempting to reset array\n");
                    if (lines == LINES_TO_READ) {
                        break;
                    } else {
                        // reset array to as if we just executed this function
                        int y;
                        for (y=0; y<bytes; y++) {
                            array[y] = 0;
                        }
                        free(array);
                        array = malloc(sizeof(char) * 2048);
                        bytes=1;
                        count=1;
                    }
                    lines++;
                }
//                 count++;
                bytes++;
            }
            bytes--;
            array_tmp = realloc(array, bytes);
            if (array_tmp == NULL) {
                printf("failed to allocate array to new size");
                free(array);
                exit(1);
            } else {
                array = array_tmp;
            }
            printf("\rbytes read: %'i\n", bytes);
    *p = array;
    *q = bytes;
    return bytes;
}

// reads a entire file
int read__(char *file, char **p, size_t *q) {
    int fd;
    size_t len = 0;
    char *o;
    if (!(fd = open(file, O_RDONLY)))
    {
        fprintf(stderr, "open() failure\n");
        return (1);
    }
    len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, 0);
    if (!(o = malloc(len))) {
        fprintf(stderr, "failure to malloc()\n");
    }
    if ((read(fd, o, len)) == -1) {
        fprintf(stderr, "failure to read()\n");
    }
    int cl = close(fd);
    if (cl < 0) {
        printf("cannot close \"%s\", returned %i\n", file, cl);
        return -1;
    }
    *p = o;
    *q = len;
    return len;
}

void *
stack_setupb(
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

#include <sys/auxv.h>

void die(const char *s)
{
	perror(s);
	exit(errno);
}
void set_auxv(char * value)
{
	char buf[1024];
	int fd = -1;
	ssize_t r = 0;
	Elf64_auxv_t *auxv = NULL;

	snprintf(buf, sizeof(buf), "/proc/self/auxv");

	if ((fd = open(buf, O_RDONLY)) < 0)
		die("[-] open");

	if ((r = read(fd, buf, sizeof(buf))) < 0)
		die("[-] read");
	close(fd);

	for (auxv = (Elf64_auxv_t *)buf; auxv->a_type != AT_NULL && (char *)auxv < buf + r; ++auxv) {
		switch (auxv->a_type) {
		case AT_EXECFN:
			printf("old AT_EXECFN:\t%s\n", (void *)auxv->a_un.a_val);
            auxv->a_un.a_val = value;
			printf("new AT_EXECFN:\t%s\n", (void *)auxv->a_un.a_val);
			break;
		default:
            break;
		}
	}
}

char * get_full_path() {
    findyourself_init((char *)getauxval(AT_EXECFN));
    char * auxAT = (char *)getauxval(AT_EXECFN);
    char newpath[PATH_MAX];
    find_yourself(newpath, sizeof(newpath));
    if(1 || strcmp((char *)getauxval(AT_EXECFN),newpath)) { }
    char *fullpath  = strdup( newpath );
    char *directorypath = dirname( strdup( newpath ) );
    printf("current = %s\nfullpath = %s\ndirname = %s\n", auxAT, fullpath, directorypath);
    return fullpath;
}
#define _dl_printf printf
#define strchrb strchr
#define strcpyb strcpy
#define strncpyb strncpy
#define strncatb strncat
#define strstrb strstr
int *
resolve(char * path)
  {
   _dl_printf("called resolve()\n");
      if(!access(path, F_OK)) {
        } else {
        return -1;
        }
    char * pathb = (char *)malloc(strlen(path) + 1);
    char * strcpyb(char *dest, const char *src);
    strcpyb(pathb,path);
    char save_pwd[PATH_MAX];
    getcwd(save_pwd, sizeof(save_pwd));
    char path_separator='/';
    char relpathdot_separator[4]="/./";
    char relpathdotdor_separator[5]="/../";
    char newpathb[PATH_MAX+256];
    char newpathc[PATH_MAX+256];
    char linkb[PATH_MAX+256];
    char linkd[PATH_MAX+256];
    char tmp_pwd[PATH_MAX];
    char current_pwd[PATH_MAX];
    getcwd(current_pwd, sizeof(current_pwd));
        #include <sys/types.h>
        #include <sys/stat.h>
    char* resolvedir(const char * pathb)
    {
        _dl_printf("chdir(%s)\n", pathb);
        chdir(pathb);
        _dl_printf("getcwd(%s, sizeof(%s))\n", tmp_pwd, tmp_pwd);
        getcwd(tmp_pwd, sizeof(tmp_pwd));
        _dl_printf("%s points to %s\n\n", pathb, tmp_pwd);
        _dl_printf("chdir(%s)\n", current_pwd);
        chdir(current_pwd);
        _dl_printf("return %s\n", tmp_pwd);
        return tmp_pwd;
    }
    char* resolvefile(char * pathb)
    {
        _dl_printf("strncpyb(%s, %s, sizeof(%s)\n", linkb, pathb, linkb);
        strncpyb(linkb, pathb, sizeof(linkb));
        _dl_printf("linkb[sizeof(%s)-1]=0\n", linkb);
        linkb[sizeof(linkb)-1]=0;
        _dl_printf("strncpyb(%s, %s, sizeof(%s)\n", linkd, pathb, linkb);
        strncpyb(linkd, pathb, sizeof(linkb));
        _dl_printf("linkb[sizeof(%s)-1]=0\n", linkb);
        linkb[sizeof(linkb)-1]=0;
        _dl_printf("dirname(%s)\n", linkd);
        dirname(linkd);
        _dl_printf("strncatb(%s, \"/\", sizeof(%s));\n", linkd, linkd);
        strncatb(linkd, "/", sizeof(linkd));
        _dl_printf("linkd[sizeof(%s)-1]=0\n", linkd);
        linkd[sizeof(linkd)-1]=0;
        _dl_printf("chdir(%s)\n", linkd);
        chdir(linkd);
        _dl_printf("getcwd(%s, sizeof(%s))\n", tmp_pwd, tmp_pwd);
        getcwd(tmp_pwd, sizeof(tmp_pwd));
        _dl_printf("strncatb(%s, \"/\", sizeof(%s));\n", tmp_pwd, tmp_pwd);
        strncatb(tmp_pwd, "/", sizeof(tmp_pwd));
        _dl_printf("tmp_pwd[sizeof(%s)-1]=0\n", tmp_pwd);
        tmp_pwd[sizeof(tmp_pwd)-1]=0;
        _dl_printf("strncpyb(%s, %s, sizeof(%s));\n", linkb, basename(pathb), linkb);
        strncpyb(linkb, basename(pathb), sizeof(linkb));
        _dl_printf("linkb[sizeof(%s)-1]=0\n", linkb);
        linkb[sizeof(linkb)-1]=0;
        _dl_printf("strncatb(%s, %s, sizeof(%s));\n", tmp_pwd, linkb, tmp_pwd);
        strncatb(tmp_pwd, linkb, sizeof(tmp_pwd));
        _dl_printf("tmp_pwd[sizeof(%s)-1]=0\n", tmp_pwd);
        tmp_pwd[sizeof(tmp_pwd)-1]=0;
        _dl_printf("%s points to %s\n\n", pathb, tmp_pwd);
        _dl_printf("chdir(%s)\n", current_pwd);
        chdir(current_pwd);
        _dl_printf("return %s\n", tmp_pwd);
        return tmp_pwd;
    }
    #include <sys/types.h>
    #include <sys/stat.h>
    char * getlink(const char * link)
    {
        struct stat p_statbuf;
        if (lstat(link,&p_statbuf)==0) {
            _dl_printf("%s type is <int>\n",link, S_ISLNK(p_statbuf.st_mode));
            if (S_ISLNK(p_statbuf.st_mode)==1)
            {
            _dl_printf("%s is symbolic link \n", link);
            } else
            {
            _dl_printf("%s is not symbolic link \n", link);
                return 0;
            }
        }
        struct stat sb;
        char *linkname;
        ssize_t r;

        if (lstat(link, &sb) == -1)
        {
                _exit(EXIT_FAILURE);
        }

        linkname = malloc(sb.st_size + 1);
            if (linkname == NULL)
            {
                _exit(EXIT_FAILURE);
            }

        r = readlink(link, linkname, sb.st_size + 1);

        if (r < 0)
        {
                _exit(EXIT_FAILURE);
        }

        if (r > sb.st_size)
        {
                _exit(EXIT_FAILURE);
        }

        linkname[sb.st_size] = '\0';

       _dl_printf("\"%s\" points to '%s'\n", link, linkname);

        path = linkname;
        char * checkifsymlink(const char * tlink)
        {
            struct stat p_statbuf;
            if (lstat(tlink,&p_statbuf)==0)
            {
                _dl_printf("%s type is <int>\n",tlink, S_ISLNK(p_statbuf.st_mode));
                if (S_ISLNK(p_statbuf.st_mode)==1)
                {
                _dl_printf("%s is symbolic link \n", tlink);
                _dl_printf("called getlink()\n");
                    getlink(tlink);
                } else
                {
                _dl_printf("%s is not symbolic link \n", tlink);
                    return 0;
                }
            }
        return 0;
        }
       _dl_printf("called checkifsymlink()\n");
        checkifsymlink(path);
        return 0;
    }
   _dl_printf("called getlink()\n");
    getlink(path);
    char * testtype(const char * patha)
    {
        int is_regular_file(const char *patha)
        {
            struct stat path_stat;
            stat(patha, &path_stat);
            return S_ISREG(path_stat.st_mode);
        }
        int isDirectory(const char *patha)
        {
            struct stat statbuf;
            if (stat(patha, &statbuf) != 0)
                return 0;
            return S_ISDIR(statbuf.st_mode);
        }
        if (is_regular_file(patha)==1)
        {
           _dl_printf("%s is file \n", patha);
            if (path[0]==path_separator)
            {
                if ( strstrb(path, relpathdot_separator ))
                {
                   _dl_printf("%s is an absolute path which contains a dot relative path\n", path);
                   _dl_printf("called Rresolvefile()\n");
                    return resolvefile(path);
                } else if ( strstrb(path, relpathdotdor_separator ))
                {
                   _dl_printf("%s is an absolute path which contains a dot dot relative path\n", path);
                   _dl_printf("called resolvefile()\n");
                   return resolvefile(path);
                } else
                {
                   _dl_printf("%s is an absolute path with no relative paths\n", path);
                    return path;
                }
            } else if ( strchrb(path, path_separator ))
            {
               _dl_printf("%s is a relative path\n", path);
                strncpyb(newpathb, current_pwd, sizeof(newpathb));
                newpathb[sizeof(newpathb)-1]=0;
                strncatb(newpathb, "/", sizeof(newpathb));
                newpathb[sizeof(newpathb)-1]=0;
                strncatb(newpathb, path, sizeof(newpathb));
                newpathb[sizeof(newpathb)-1]=0;
               _dl_printf("called resolvefile()\n");
                printf("need to re execute\n");
                char * new_aux = resolvefile(newpathb);
                printf("executing with %s\n\n\n\n", new_aux);
                int ret = execv(new_aux, NULL);
                printf("ret = %d\n\n", ret);
                return "ERROR";
            } else
            {
               _dl_printf("could not determine path type of %s\n", path);
                return "NULL";
            }
        } else if (isDirectory(patha)==1)
        {
           _dl_printf("%s is a directory \n", patha);
            if (path[0]==path_separator)
            {
                if ( strstrb(path, relpathdot_separator ))
                {
                   _dl_printf("%s is an absolute path which contains a dot relative path\n", path);
                   _dl_printf("called resolvedir()\n");
                    resolvedir(path);
                } else if ( strstrb(path, relpathdotdor_separator ))
                {
                   _dl_printf("%s is an absolute path which contains a dot dot relative path\n", path);
                   _dl_printf("called resolvedir()\n");
                    resolvedir(path);
                } else
                {
                   _dl_printf("%s is an absolute path with no relative paths\n", path);
                    return path;
                }
            } else if ( strchrb(path, path_separator ))
            {
               _dl_printf("%s is a relative path\n", path);
               _dl_printf("strncpyb(%s, %s, sizeof(%s));\n", newpathc, current_pwd, newpathc);
                strncpyb(newpathc, current_pwd, sizeof(newpathc));
               _dl_printf("newpath2[sizeof(%s)-1]=0;\n", newpathc);
                newpathc[sizeof(newpathc)-1]=0;
               _dl_printf("strncatb(%s, %s, sizeof(%s));\n", newpathc, "/", newpathc);
                strncatb(newpathc, "/", sizeof(newpathc));
               _dl_printf("newpathc[sizeof(%s)-1]=0;\n", newpathc);
                newpathc[sizeof(newpathc)-1]=0;
               _dl_printf("strncatb(%s, %s, sizeof(%s));\n", newpathc, path, newpathc);
                strncatb(newpathc, path, sizeof(newpathc));
               _dl_printf("newpathc[sizeof(%s)-1]=0;\n", newpathc);
                newpathc[sizeof(newpathc)-1]=0;
               _dl_printf("called resolvedir()\n");
                return resolvedir(newpathc);
            } else
            {
               _dl_printf("could not determine path type of %s\n", path);
                return "NULL";
            }
        }
        return "FAILED";
    }
   _dl_printf("called testtype()\n");
    return testtype(path);
  }

void * setaux(int *argc, char ** argv, void * value, unsigned long type) {
//     printf("type = %d\n", type);
    #define AUX_CNT 32
// AT_SYSINFO_EHDR: 0x7fffdb375000
// AT_HWCAP:        178bfbff
// AT_PAGESZ:       4096
// AT_CLKTCK:       100
// AT_PHDR:         0x400040
// AT_PHENT:        56
// AT_PHNUM:        10
// AT_BASE:         0x7ff5e3c1c000
// AT_FLAGS:        0x0
// AT_ENTRY:        0x4032e0
// AT_UID:          1000
// AT_EUID:         1000
// AT_GID:          1001
// AT_EGID:         1001
// AT_SECURE:       0
// AT_RANDOM:       0x7fffdb34a379
// AT_HWCAP2:       0x0
// AT_EXECFN:       /usr/bin/gcc
// AT_PLATFORM:     x86_64
	size_t i;
    int num = -1;
    void * AUXV_TYPE;
    char * AUXV_NAME;
    for (i=argc+1; argv[i]; i++);
    for (int ii=0; ii<=(void *)(argv+i+1)[0]+(2*20); ii+=2) { // 20 extra incase auxv does not have the same vectors for every machine (could have more than +4)
        size_t tmp = (void *)(argv+i+1+ii)[0];
        switch(tmp)
        {
                case 0:
                    AUXV_NAME = "AT_NULL";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 0;
                    break;
                case 1:
                    AUXV_NAME = "AT_IGNORE";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 1;
                    break;
                case 2:
                    AUXV_NAME = "AT_EXECFD";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 2;
                    break;
                case 3:
                    AUXV_NAME = "AT_PHDR";
                    AUXV_TYPE = "ADDRESS";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 3;
                    break;
                case 4:
                    AUXV_NAME = "AT_PHENT";
                    AUXV_TYPE = "INT";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 4;
                    break;
                case 5:
                    AUXV_NAME = "AT_PHNUM";
                    AUXV_TYPE = "INT";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 5;
                    break;
                case 6:
                    AUXV_NAME = "AT_PAGESZ";
                    AUXV_TYPE = "INT";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 6;
                    break;
                case 7:
                    AUXV_NAME = "AT_BASE";
                    AUXV_TYPE = "ADDRESS";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 7;
                    break;
                case 8:
                    AUXV_NAME = "AT_FLAGS";
                    AUXV_TYPE = "ADDRESS";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 8;
                    break;
                case 9:
                    AUXV_NAME = "AT_ENTRY";
                    AUXV_TYPE = "ADDRESS";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 9;
                    break;
                case 10:
                    AUXV_NAME = "AT_NOTELF";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 10;
                    break;
                case 11:
                    AUXV_NAME = "AT_UID";
                    AUXV_TYPE = "INT";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 11;
                    break;
                case 12:
                    AUXV_NAME = "AT_EUID";
                    AUXV_TYPE = "INT";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 12;
                    break;
                case 13:
                    AUXV_NAME = "AT_GID";
                    AUXV_TYPE = "INT";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 13;
                    break;
                case 14:
                    AUXV_NAME = "AT_EGID";
                    AUXV_TYPE = "INT";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 14;
                    break;
                case 15:
                    AUXV_NAME = "AT_PLATFORM";
                    AUXV_TYPE = "CHAR*";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 15;
                    break;
                case 16:
                    AUXV_NAME = "AT_HWCAP";
                    AUXV_TYPE = "ADDRESS";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 16;
                    break;
                case 17:
                    AUXV_NAME = "AT_CLKTCK";
                    AUXV_TYPE = "INT";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 17;
                    break;
                case 18:
                    AUXV_NAME = "AT_FPUCW";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 18;
                    break;
                case 19:
                    AUXV_NAME = "AT_DCACHEBSIZE";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 19;
                    break;
                case 20:
                    AUXV_NAME = "AT_ICACHEBSIZE";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 20;
                    break;
                case 21:
                    AUXV_NAME = "AT_UCACHEBSIZE";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 21;
                    break;
                case 22:
                    AUXV_NAME = "AT_IGNOREPPC";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 22;
                    break;
                case 23:
                    AUXV_NAME = "AT_SECURE";
                    AUXV_TYPE = "INT";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 23;
                    break;
                case 24:
                    AUXV_NAME = "AT_BASE_PLATFORM";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 24;
                    break;
                case 25:
                    AUXV_NAME = "AT_RANDOM";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 25;
                    break;
                case 26:
                    AUXV_NAME = "AT_HWCAP2";
                    AUXV_TYPE = "ADDRESS";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 26;
                    break;
                case 31:
                    AUXV_NAME = "AT_EXECFN";
                    AUXV_TYPE = "CHAR*";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 31;
                    break;
                case 32:
                    AUXV_NAME = "AT_SYSINFO";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 32;
                    break;
                case 33:
                    AUXV_NAME = "AT_SYSINFO_EHDR";
                    AUXV_TYPE = "ADDRESS";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 33;
                    break;
                case 34:
                    AUXV_NAME = "AT_L1I_CACHESHAPE";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 34;
                    break;
                case 35:
                    AUXV_NAME = "AT_L1D_CACHESHAPE";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 35;
                    break;
                case 36:
                    AUXV_NAME = "AT_L2_CACHESHAPE";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 36;
                    break;
                case 37:
                    AUXV_NAME = "AT_L3_CACHESHAPE";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = 37;
                    break;
                default:
                    AUXV_NAME = "UNDEFINED";
                    AUXV_TYPE = "";
                    // printf("AUXV_NAME = %s, AUXV_TYPE = %s\n", AUXV_NAME, AUXV_TYPE);
                    num = -1;
                    break;
        }
        if (num == type) {
//             printf("changing %s (type %s)\n", AUXV_NAME, AUXV_TYPE);
//             printf("address of (void *)(argv+%d+1+%d)[1] = %p\n", i, ii, (void *)&(argv+i+1+ii)[1]);
        #include <sys/auxv.h>
        if (AUXV_TYPE == "CHAR*") {

            printf("calling getauxval: ");
                printf("%s = %s\n", AUXV_NAME, (char *)getauxval(type));
            char * string = value;
            char *j = (void *)(argv+i+1+ii)[1];
            int len = strlen((void *)(argv+i+1+ii)[1]);
            int len_ = strlen(string);
//             printf("j = %s\n(void *)(argv+i+1+%d)[1] = %s\n", j, ii, (void *)(argv+i+1+ii)[1]);
//             printf("attempting to modify %s\n", AUXV_NAME);
            for (int g = 0; g<=len_; g++) {
                *j = string[g];
                j+=1;
            }
            for (int g = 0; g<=len-len_; g++) { // NULL the rest of the string
                *j = '\0';
                j+=1;
            }
//             printf("j = %s\n(void *)(argv+i+1+%d)[1] = %s\n", j, ii, (void *)(argv+i+1+ii)[1]);
            printf("calling getauxval: ");
                printf("%s = %s\n", AUXV_NAME, (char *)getauxval(type));

            
        } else if (AUXV_TYPE == "INT") {

            printf("calling getauxval: ");
                printf("%s = %d\n", AUXV_NAME, (char *)getauxval(type));
//             printf("(void *)(argv+i+1+%d)[1] (auxv) =\n", ii);
//             print_quoted_string_catraw((void *)(argv+i+1+ii)[1], 1, QUOTE_OMIT_TRAILING_0|QUOTE_OMIT_LEADING_TRAILING_QUOTES);
//             printf("\n");
//             printf("(void *)(argv+i+1+%d)[1] = %p\n", ii, (void *)(argv+i+1+ii)[1]);
//             printf("attempting to modify %s\n", AUXV_NAME);
            (argv+i+1+ii)[1] = value;
//             printf("(void *)(argv+i+1+%d)[1] = %p\n", ii, (void *)(argv+i+1+ii)[1]);
            printf("calling getauxval: ");
                printf("%s = %d\n", AUXV_NAME, (char *)getauxval(type));

            
        } else if (AUXV_TYPE == "ADDRESS") {

            printf("calling getauxval: ");
                printf("%s = %p\n", AUXV_NAME, (char *)getauxval(type));
//             printf("(void *)(argv+i+1+%d)[1] = %p\n", ii, (void *)(argv+i+1+ii)[1]);
//             printf("attempting to modify %s\n", AUXV_NAME);
            (argv+i+1+ii)[1] = value;
//             printf("(void *)(argv+i+1+%d)[1] = %p\n", ii, (void *)(argv+i+1+ii)[1]);
            printf("calling getauxval: ");
                printf("%s = %p\n", AUXV_NAME, (char *)getauxval(type));

            
        }

            
        }
    }
    type = -1;
}

// execve implimentation
void
ulexec(char * pro, char * args, char **env)
{
    print_maps();
    char * current_aux = (char *)getauxval(AT_EXECFN);
//     printf("returned %s\n", resolve(current_aux));
    
    printf("AT_EXECFN = %s\n", current_aux);
    printf ("current_aux[0] = %c\n", current_aux[0]);
    printf("need to re execute\n");
    char * new_aux = get_full_path();
    printf("executing with %s\n\n", new_aux);
//     setaux(argc, argv, "", AT_EXECFN);
//         int ret = execv(new_aux, NULL);
//         printf("ret = %d\n\n", ret);
    printf("safe to continue\n\n");
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
    char * s = strjoinb(pro, " ");
    s = strjoinb(s, args);
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
    
//     print_maps();
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
// 		unmap(file_to_unmap);#include <sys/mman.h>
    void * mapped_interp;
    const char * filename = program_to_execute[0];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("cannot open \"%s\", returned %i\n", filename, fd);
    }
    size_t len = 0;
    len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, 0);
    void * mapped = mmap (NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        printf ("map failed\n");
        exit;
    } else {
        printf ("map (%s) succeded with address: 0x%08x\n", filename, mapped);
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
                printf ("map ("); print_quoted_string(tmp99, phdr[i].p_memsz, QUOTE_OMIT_TRAILING_0|QUOTE_OMIT_LEADING_TRAILING_QUOTES, "print"); printf(") succeded with address: 0x%08x\n", mapped_i);
            }
        }
    }


    printf("loading\n");
	entry_point = load_elf(mapped, how_to_map, &elf_ehdr, &ldso_ehdr, mapped_i);

    printf("unmapping\n");
	munmap(mapped, sb.st_size);

    printf("argvb = save_argv(%d, %s);\n", c, program_program_arguments[0]);
	argvb = save_argv(c, &program_program_arguments[0]);
    printf("saving envb\n");
	envb = save_argv(0, env);
    printf("saving elfauxvb\n");
	elfauxvb = save_elfauxv(env);

    printf("stack_setup()\n");
	stack_bottom = stack_setup(argvb, envb, elfauxvb, elf_ehdr, ldso_ehdr);

    printf("SET_STACK()\n");
	SET_STACK(stack_bottom);
    printf("printing maps before executing\n");
    print_maps();
    printf("jumping to %p\n", entry_point);
	JMP_ADDR(entry_point);
}

// executes an exeutable using the packaged interpreter
void
ulexec_array(void * mapped, void * mapped_interpreter, char * args, char **env)
{
    print_maps();
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
    void * mapped_i;
    if (mapped_interpreter == NULL) {
        printf("mapped = null\n");
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
                int fd = open(filename, O_RDONLY); // opens the system's ld.so and uses it if an interpreter is not provided in the 2nd argument of ulexec_array
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
                    printf ("map ("); print_quoted_string(tmp99, phdr[i].p_memsz, QUOTE_OMIT_TRAILING_0|QUOTE_OMIT_LEADING_TRAILING_QUOTES, "print"); printf(") succeded with address: 0x%08x\n", mapped_i);
                }
            }
        }
    } else {
        mapped_i = mapped_interpreter;
    }

    printf("loading %p\n", mapped_i);
	entry_point = load_elf(mapped, how_to_map, &elf_ehdr, &ldso_ehdr, mapped_i);

    printf("unmapping\n");
	munmap(mapped, sb.st_size);

    printf("argvb = save_argv(%d, %s);\n", c+1, program_arguments[0]);
	argvb = save_argv(c+1, &program_arguments[0]);
	envb = save_argv(0, env);
	elfauxvb = save_elfauxv(env);

	stack_bottom = stack_setup(argvb, envb, elfauxvb, elf_ehdr, ldso_ehdr);

	SET_STACK(stack_bottom);
//     printf("printing maps before executing\n");
//     print_maps();
    printf("jumping to %p\n", entry_point);
	JMP_ADDR(entry_point);
}

// executes using system's interpreter
int ulexecb(void * array, char ** env)
{
    print_maps();
    int i = 0;
        print_quoted_string(array, 16, QUOTE_FORCE_HEX, "print");
    printf("hai\n");      
        # include <elf.h>
            printf(" )\n");
            Elf64_Ehdr * _elf_header = (Elf64_Ehdr *) array;
    printf("hai again\n"); 
            printf("ELF Identifier\t %s (", _elf_header->e_ident);
            print_quoted_string(_elf_header->e_ident, sizeof(_elf_header->e_ident), QUOTE_FORCE_HEX|QUOTE_OMIT_LEADING_TRAILING_QUOTES, "print");
            printf(" )\n");
        if(!strncmp((char*)_elf_header->e_ident, "\177ELF", 4)) {
//                 ELF Header:
//                 Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
//                 Class:                             ELF64
//                 Data:                              2's complement, little endian
//                 Version:                           1 (current)
//                 OS/ABI:                            UNIX - System V
//                 ABI Version:                       0
//                 Type:                              EXEC (Executable file)
//                 Machine:                           Advanced Micro Devices X86-64
//                 Version:                           0x1
//                 Entry point address:               0x400820
//                 Start of program headers:          64 (bytes into file)
//                 Start of section headers:          11408 (bytes into file)
//                 Flags:                             0x0
//                 Size of this header:               64 (bytes)
//                 Size of program headers:           56 (bytes)
//                 Number of program headers:         9
//                 Size of section headers:           64 (bytes)
//                 Number of section headers:         30
//                 Section header string table index: 29
//
            printf("ELF Identifier\t %s (", _elf_header->e_ident);
            print_quoted_string(_elf_header->e_ident, sizeof(_elf_header->e_ident), QUOTE_FORCE_HEX|QUOTE_OMIT_LEADING_TRAILING_QUOTES, "print");
            printf(" )\n");

            printf("Architecture\t ");
            switch(_elf_header->e_ident[EI_CLASS])
            {
                case ELFCLASSNONE:
                    printf("None\n");
                    break;

                case ELFCLASS32:
                    printf("32-bit\n");
                    break;

                case ELFCLASS64:
                    printf("64-bit\n");
                    break;
                    
                case ELFCLASSNUM:
                    printf("NUM ( unspecified )\n");
                    break;

                default:
                    printf("Unknown CLASS\n");
                    break;
            }

            printf("Data Type\t ");
            switch(_elf_header->e_ident[EI_DATA])
            {
                case ELFDATANONE:
                    printf("None\n");
                    break;

                case ELFDATA2LSB:
                    printf("2's complement, little endian\n");
                    break;

                case ELFDATA2MSB:
                    printf("2's complement, big endian\n");
                    break;
                    
                case ELFDATANUM:
                    printf("NUM ( unspecified )\n");
                    break;

                default:
                    printf("Unknown \n");
                    break;
            }

            printf("Version\t\t ");
            switch(_elf_header->e_ident[EI_VERSION])
            {
                case EV_NONE:
                    printf("None\n");
                    break;

                case EV_CURRENT:
                    printf("Current\n");
                    break;

                case EV_NUM:
                    printf("NUM ( Unspecified )\n");
                    break;

                default:
                    printf("Unknown \n");
                    break;
            }

            printf("OS ABI\t\t ");
            switch(_elf_header->e_ident[EI_OSABI])
            {
                case ELFOSABI_NONE:
                    printf("UNIX System V ABI\n");
                    break;

//                     case ELFOSABI_SYSV:
//                         printf("SYSV\n");
//                         break;
// 
                case ELFOSABI_HPUX:
                    printf("HP-UX\n");
                    break;

                case ELFOSABI_NETBSD:
                    printf("NetBSD\n");
                    break;

                case ELFOSABI_GNU:
                    printf("GNU\n");
                    break;

//                     case ELFOSABI_LINUX:
//                         printf("Linux\n");
//                         break;
// 
                case ELFOSABI_SOLARIS:
                    printf("Sun Solaris\n");
                    break;

                case ELFOSABI_AIX:
                    printf("ABM AIX\n");
                    break;

                case ELFOSABI_FREEBSD:
                    printf("FreeBSD\n");
                    break;

                case ELFOSABI_TRU64:
                    printf("Compaq Tru64\n");
                    break;

                case ELFOSABI_MODESTO:
                    printf("Novell Modesto\n");
                    break;

                case ELFOSABI_OPENBSD:
                    printf("OpenBSD\n");
                    break;

//                 case ELFOSABI_ARM_AEABI:
//                     printf("ARM EABI\n");
//                     break;

                case ELFOSABI_ARM:
                    printf("ARM\n");
                    break;

                case ELFOSABI_STANDALONE:
                    printf("Standalone (embedded) application\n");
                    break;

                default:
                    printf("Unknown \n");
                    break;
            }

            printf("File Type\t ");
            switch(_elf_header->e_type)
            {
                case ET_NONE:
                    printf("None\n");
                    break;

                case ET_REL:
                    printf("Relocatable file\n");
                    break;

                case ET_EXEC:
                    printf("Executable file\n");
                    break;

                case ET_DYN:
                    printf("Shared object file\n");
                    break;

                case ET_CORE:
                    printf("Core file\n");
                    break;

                case ET_NUM:
                    printf("Number of defined types\n");
                    break;

                case ET_LOOS:
                    printf("OS-specific range start\n");
                    break;

                case ET_HIOS:
                    printf("OS-specific range end\n");
                    break;

                case ET_LOPROC:
                    printf("Processor-specific range start\n");
                    break;

                case ET_HIPROC:
                    printf("Processor-specific range end\n");
                    break;

                default:
                    printf("Unknown \n");
                    break;
            }

            printf("Machine\t\t ");
            switch(_elf_header->e_machine)
            {
                case EM_NONE:
                    printf("None\n");
                    break;

                case EM_386:
                        printf("INTEL x86\n");
                        break;

                case EM_X86_64:
                        printf("AMD x86-64 architecture\n");
                        break;

                case EM_ARM:
                        printf("ARM\n");
                        break;
                default:
                        printf("Unknown\n");
                break;
            }
            
            /* Entry point */
            int entry=_elf_header->e_entry;
            printf("Entry point\t 0x%08x Step 3. and jump to e_entry\n", _elf_header->e_entry);
            

            /* ELF header size in bytes */
            printf("ELF header size\t 0x%08x\n", _elf_header->e_ehsize);

            /* Program Header */
            printf("Program Header\t 0x%08x (%d entries with a total of %d bytes)\n",
            _elf_header->e_phoff,
            _elf_header->e_phnum,
            _elf_header->e_phentsize
            );
            // for static, obtain the following: everything in the structure Elf64_Phdr, then `, PT_LOAD, PT_DYNAMIC, then e_entry

            Elf64_Phdr *elf_program_header = (Elf64_Phdr *)((unsigned long)_elf_header + _elf_header->e_phoff);
            char * exe = "";
            int lenexe = 0;
            int lentotal = 0;
            for (i = 0; i < _elf_header->e_phnum; ++i) {

//                 printf("dl_iterate_phdr =\n");
//                 dl_iterate_phdr(callback, NULL);
                printf("p_type;\t\t\t/* Segment type */\t\t= ");
                switch(elf_program_header[i].p_type)
                {
                    case PT_NULL:
                        printf("PT_NULL\t\t/* Program header table entry unused */\n");
                        break;
                    case PT_LOAD:
                        printf("PT_LOAD\t\t/* Loadable program segment */ Step 2. then parse and process all PT_LOAD segments\n");
                        break;
                    case PT_DYNAMIC:
                        printf("PT_DYNAMIC\t\t/* Dynamic linking information */ Step 2.5 keep in mind that static-PIE binaries are dynamic ELF objects though, so to load those you need to parse the PT_DYNAMIC stuff\n");
                        break;
                    case PT_INTERP:
                        printf("PT_INTERP\t\t/* Program interpreter */\n");
                        break;
                    case PT_NOTE:
                        printf("PT_NOTE\t\t/* Auxiliary information */\n");
                        break;
                    case PT_SHLIB:
                        printf("PT_SHLIB\t\t/* Reserved */\n");
                        break;
                    case PT_PHDR:
                        printf("PT_PHDR\t\t/* Entry for header table itself */ Step 1. if the first entry is a PT_PHDR, use that as the program header table\n");
                        break;
                    case PT_TLS:
                        printf("PT_TLS\t\t/* Thread-local storage segment */\n");
                        break;
                    case PT_NUM:
                        printf("PT_NUM\t\t/* Number of defined types */\n");
                        break;
                    case PT_LOOS:
                        printf("PT_LOOS\t\t/* Start of OS-specific */\n");
                        break;
                    case PT_GNU_EH_FRAME:
                        printf("PT_GNU_EH_FRAME\t/* GCC .eh_frame_hdr segment */\n");
                        break;
                    case PT_GNU_STACK:
                        printf("PT_GNU_STACK\t\t/* Indicates stack executability */\n");
                        break;
                    case PT_GNU_RELRO:
                        printf("PT_GNU_RELRO\t\t/* Read-only after relocation */\n");
                        break;
                    case PT_SUNWBSS:
                        printf("PT_SUNWBSS\t\t/* Sun Specific segment */\n");
                        break;
                    case PT_SUNWSTACK:
                        printf("PT_SUNWSTACK\t\t/* Stack segment */\n");
                        break;
                    case PT_HIOS:
                        printf("PT_HIOS\t\t/* End of OS-specific */\n");
                        break;
                    case PT_LOPROC:
                        printf("PT_LOPROC\t\t/* Start of processor-specific */\n");
                        break;
                    case PT_HIPROC:
                        printf("PT_HIPROC\t\t/* End of processor-specific */\n");
                        break;
                    default:
                            printf("Unknown\n");
                    break;
                }
// read_pload (dst address in memory, how many bytes to read, offset in the file) read_pload(ph->p_paddr, ph->p_memsz, ph->p_offset);
                    char * tmp99;
                if (elf_program_header[i].p_type == PT_LOAD) {
                    lentotal = lentotal + elf_program_header[i].p_memsz;
                    exe = strjoin(exe, tmp99, lenexe, lentotal);
//                     print_quoted_string(exe, lentotal, 0, "print");
                    printf("\n");
                    lenexe = lentotal;
                } else {
                    printf("ATTEMPING TO READ\n");
                    read_(array, &tmp99, (elf_program_header[i].p_memsz + elf_program_header[i].p_offset));
                    lseek_string(&tmp99, elf_program_header[i].p_memsz, elf_program_header[i].p_offset);
                    print_quoted_string(tmp99, elf_program_header[i].p_memsz, 0, "print");
                    printf("\nREAD\n");
                }
                

//                 could this [ // read_pload (dst address in memory, how many bytes to read, offset in the file) read_pload(ph->p_paddr, ph->p_memsz, ph->p_offset); ] be shortened to [ read_pload(mapped_file, ph->p_memsz, (ph->p_paddr + ph->p_offset); ] ? https://stackoverflow.com/a/29326748/8680581                

                printf("p_flags;\t\t/* Segment flags */\t\t= 0x%08x\np_offset;\t\t/* Segment file offset */\t= 0x%08x\np_vaddr;\t\t/* Segment virtual address */\t= 0x%08x\np_paddr;\t\t/* Segment physical address */\t= 0x%08x\np_filesz;\t\t/* Segment size in file */\t= 0x%08x\np_memsz;\t\t/* Segment size in memory */\t= 0x%08x\np_align;\t\t/* Segment alignment */\t\t= 0x%08x\n\n\n", elf_program_header[i].p_flags, elf_program_header[i].p_offset, elf_program_header[i].p_vaddr, elf_program_header[i].p_paddr, elf_program_header[i].p_filesz, elf_program_header[i].p_memsz, elf_program_header[i].p_align);
                printf("p_flags = 0x%08x, p_offset = 0x%08x, p_vaddr = 0x%08x, p_paddr = 0x%08x, p_filesz = 0x%08x, p_memsz = 0x%08x, p_align = 0x%08x\n\n\n", elf_program_header[i].p_flags, elf_program_header[i].p_offset, elf_program_header[i].p_vaddr, elf_program_header[i].p_paddr, elf_program_header[i].p_filesz, elf_program_header[i].p_memsz, elf_program_header[i].p_align);
            }
            
            // rest MAY be irrelivant for static executable execution
//             printf("Section Header\t \
_elf_header->e_shstrndx 0x%08x (\
_elf_header->e_shnum = %d entries with a total of \
_elf_header->e_shentsize = %d (should match %d) bytes, offset is \
_elf_header->e_shoff = 0x%08x)\n",\
            _elf_header->e_shstrndx,\
            _elf_header->e_shnum,\
            _elf_header->e_shentsize,\
            sizeof(Elf64_Shdr),\
            _elf_header->e_shoff,\
            (char *)array + _elf_header->e_shoff\
            );

            Elf64_Shdr *_symbol_table;
            // read section header table
            void read_section_header_table_(const char * arrayb, Elf64_Ehdr * eh, Elf64_Shdr * sh_table[])
            {
                *sh_table = (Elf64_Shdr *)(arrayb + eh->e_shoff);
                if(!_symbol_table) {
                    printf("Failed to read table\n");
                }
            }
            
            char * read_section_(char * ar, Elf64_Shdr sh) {
                char * buff = (char *)(ar + sh.sh_offset);
                return buff ;
            }

            char * print_section_headers_(char * sourcePtr, Elf64_Ehdr * eh, Elf64_Shdr sh_table[]) {
                printf ("\n");
                printf("eh->e_shstrndx = 0x%x\n", eh->e_shstrndx);
                char * sh_str;
                sh_str = read_section_(sourcePtr, sh_table[eh->e_shstrndx]); // will fail untill section header table can be read
                printf("\t========================================");
                printf("========================================\n");
                printf("\tidx offset     load-addr  size       algn type       flags      section\n");
                printf("\t========================================");
                printf("========================================\n");

                for(i=0; i<eh->e_shnum; i++) { // will fail untill section header table can be read
                    printf("\t%03d ", i);
                    printf("0x%08x ", _symbol_table[i].sh_offset); // p_offset
                    printf("0x%08x ", _symbol_table[i].sh_addr); // p_paddr or p_vaddr
                    printf("0x%08x ", _symbol_table[i].sh_size); // p_filesz or p_memsz
                    printf("%4d ", _symbol_table[i].sh_addralign); // p_align
                    // for some reason sh_flags ans sh_type are swiched around
                    printf("0x%08x ", _symbol_table[i].sh_flags); // p_flags
                    printf("0x%08x ", _symbol_table[i].sh_type); // Unknown
                    printf("%s\t", (sh_str + sh_table[i].sh_name));
                    printf("\n");
                }
                printf("\t========================================");
                printf("========================================\n");
                printf("\n");
            }

            void print_symbol_table(char * arrayc, Elf64_Ehdr eh, Elf64_Shdr sh_table[], uint64_t symbol_table)
            {

                char *str_tbl;
                Elf64_Sym* sym_tbl;
                uint64_t i, symbol_count;

                sym_tbl = (Elf64_Sym*)read_section_(arrayc, sh_table[symbol_table]);

                /* Read linked string-table
                * Section containing the string table having names of
                * symbols of this section
                */
                uint64_t str_tbl_ndx = sh_table[symbol_table].sh_link;
                printf("str_tbl_ndx = 0x%x\n", str_tbl_ndx);
                str_tbl = read_section_(arrayc, sh_table[str_tbl_ndx]);

                symbol_count = (sh_table[symbol_table].sh_size/sizeof(Elf64_Sym));
                printf("%d symbols\n", symbol_count);

                for(i=0; i< symbol_count; i++) {
                    printf("PART0 sym_tbl[i].st_value = 0x%08x\n", sym_tbl[i].st_value);
                    printf("PART1 ELF64_ST_BIND(sym_tbl[%d].st_info) = 0x%02x\n", i, ELF64_ST_BIND(sym_tbl[i].st_info));
                    printf("PART2 ELF64_ST_TYPE(sym_tbl[%d].st_info) = 0x%02x\n", i, ELF64_ST_TYPE(sym_tbl[i].st_info));
                    printf("PART3 (str_tbl + sym_tbl[%d].st_name) = %s\n\n", i, (str_tbl + sym_tbl[i].st_name));
                }
            }
            void print_symbols(char * arrayd, Elf64_Ehdr * eh, Elf64_Shdr sh_table[])
            {
                for(i=0; i<eh->e_shnum; i++) {
                    if ((sh_table[i].sh_type==SHT_SYMTAB)
                    || (sh_table[i].sh_type==SHT_DYNSYM)) {
                        printf("\n[Section %03d]", i);
                        print_symbol_table(arrayd, *eh, sh_table, i);
                    }
                }
            }
// 

// ## dynamic
// p_flags;                /* Segment flags */             = 0x00000006
// p_offset;               /* Segment file offset */       = 0x00003e20
// p_vaddr;                /* Segment virtual address */   = 0x00603e20
// p_paddr;                /* Segment physical address */  = 0x00603e20
// p_filesz;               /* Segment size in file */      = 0x000001d0
// p_memsz;                /* Segment size in memory */    = 0x000001d0
// p_align;                /* Segment alignment */         = 0x00000008
// 
// 021 0x00003e20 0x00603e20 0x000001d0    8 0x00000006 0x00000003 .dynamic

            read_section_header_table_(array, _elf_header, &_symbol_table);
            print_section_headers_(array, _elf_header, _symbol_table);
            /* Symbol tables :
            * _symbol_table[i].sh_type
            * |`- SHT_SYMTAB
            *  `- SHT_DYNSYM
            */
            print_symbols(array, _elf_header, _symbol_table);
// 	fd2 = open(pwd, O_RDWR|O_SYNC|O_CREAT);
            
            print_maps();
            ulexec_array(array, NULL, "-l", env);
            return 1;
        } else {
            printf("ELFMAGIC mismatch!\n");
            /* Not ELF file */
            return 0;
        }
        printf("\n");
        // OBJECT END
        exit(0);
}
