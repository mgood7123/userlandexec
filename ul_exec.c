// #include <libstatic/libstatic.h>
#include <errno.h>
// extern long errno;
#define NULL 0
int  linux_write(int fd, const void *data, unsigned long len);
int  linux_read(int fd, char *buffer, unsigned long bufferlen);
int  linux_open(const char *pathname, unsigned long flags, unsigned long mode);
int linux_close(int fd);
int linux_stat(const char *filename, void *buf);
void linux_exit(int code);
int  linux_print_long(int fd, unsigned long i);
int  linux_print_hex(int fd, unsigned long i);
int  linux_print_string(int fd, char *s);
int  linux_to_decimal(unsigned long x, char *p);
int  linux_to_hex(unsigned long x, char *p);
void *linux_mmap(void *start, unsigned long length, int prot, int flags, int fd, unsigned long offset);
unsigned long int linux_strtoul(const char *nptr, char **endptr, int base);
int linux_munmap(void *start, unsigned long length);
int linux_mprotect(void *addr, unsigned long len, int prot);
unsigned long linux_file_size(char *filename);
int linux_getpagesize(void);
void linux_brk(unsigned long addr);
unsigned long linux_strlen(const char *s);
char *linux_strchr(const char *s, int c);
char *linux_strstr(const char *str, char *substr);
void *linux_memcpy(void *dest, const void *src, unsigned long n);
char *linux_strcat(char *dest, const char *src);
#define PGSZ 0x1000
#include <elf.h>
#include <sys/types.h>
#include <sys/stat.h>
// #include <ulexec.h>
void *linux_map_file(char *filename);
void linux_error_msg(char *msg);
void linux_print_address(char *phrase, void *address);
void *linux_load_elf(char *mapped, int anywhere, Elf64_Ehdr **elf, Elf64_Ehdr **ldso);
void linux_unmap(char *progname);
void copy_in(char *progname, void *address);
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
#include <sys/mman.h>
#define JMP_ADDR(x) asm("\tjmp  *%0\n" :: "r" (x))
#define SET_STACK(x) asm("\tmovq %0, %%rsp\n" :: "r"(x))
#define ROUNDUP(x, y)   ((((x)+((y)-1))/(y))*(y))
#define ALIGN(k, v) (((k)+((v)-1))&(~((v)-1)))
#define ALIGNDOWN(k, v) ((unsigned long)(k)&(~((unsigned long)(v)-1)))
#define ALLOCATE(size)  \
      linux_mmap(0, (size), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)

void
linux_print_maps(void)
{
	char rbuf[1024];
	int fd, cc;

	fd = linux_open("/proc/self/maps", 0, 0);
	while (0 < (cc = linux_read(fd, rbuf, sizeof(rbuf))))
		linux_write(1, rbuf, cc);
	linux_close(fd);
}

void
ulexec(int ac, char **av, char **env)
{
    print_maps();
	char *file_to_map;
	char *file_to_unmap;
	int how_to_map = 0;
	void *mapped;
	void *entry_point;
	struct stat sb;
	Elf64_Ehdr *elf_ehdr, *ldso_ehdr;
	Elf64_Phdr *phdr;
	struct saved_block *argvb, *envb, *elfauxvb;
	int trim_args, i;
	void *stack_bottom;

	file_to_map = av[1];
	file_to_unmap = av[0];
	how_to_map = 0;
	trim_args = 1;

// 	if (file_to_unmap)
// 		linux_unmap(file_to_unmap);

    mapped = linux_map_file(av[1]);
	elf_ehdr = (Elf64_Ehdr *)mapped;

	phdr = (Elf64_Phdr *)((unsigned long)elf_ehdr + elf_ehdr->e_phoff);

	for (i = 0; i < elf_ehdr->e_phnum; ++i)
		if (phdr[i].p_type == PT_LOAD && phdr[i].p_vaddr == 0)
		{
			how_to_map = 1;  /* map it anywhere, like ld.so, or PIC code. */
			break;
		}

	entry_point = linux_load_elf(mapped, how_to_map, &elf_ehdr, &ldso_ehdr);

	linux_munmap(mapped, sb.st_size);

	argvb = save_argv(ac - trim_args, &av[trim_args]);
	envb = save_argv(0, env);
	elfauxvb = save_elfauxv(env);

	stack_bottom = stack_setup(argvb, envb, elfauxvb, elf_ehdr, ldso_ehdr);

	SET_STACK(stack_bottom);
	JMP_ADDR(entry_point);
}

void
linux_error_msg(char *msg)
{
	char buf[32];
	linux_print_string(1, msg);
	linux_print_string(1, " ");
	linux_to_decimal(errno, buf);
	linux_print_string(1, buf);
	linux_print_string(1, "\n");
}

void
linux_print_address(char *phrase, void *address)
{
	char buf[256];
	linux_to_hex((unsigned long)address, buf);
	linux_print_string(1, phrase);
	linux_print_string(1, " 0x");
	linux_print_string(1, buf);
	linux_print_string(1, "\n");
}


/*
void *
linux_memcpy(void *dest, const void *src, unsigned long n)
{
	unsigned long i;
	unsigned char *d = (unsigned char *)dest;
	unsigned char *s = (unsigned char *)src;

	for (i = 0; i < n; ++i)
		d[i] = s[i];

	return dest;
}
*/

void *memcopy(void *dest, const void *src, unsigned long n);

void *
linux_load_elf(char *mapped, int anywhere, Elf64_Ehdr **elf_ehdr, Elf64_Ehdr **ldso_ehdr)
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

		segment = linux_mmap(
			(void *)map_addr,
			rounded_len,
			PROT_WRITE, mapflags, -1, 0
		);

		if (segment == (void *) -1)
		{
			linux_print_string(1, "Failed to mmap() ");
			linux_to_hex(pdr->p_memsz, buf);
			linux_print_string(1, buf);
			linux_print_string(1, " bytes at 0x");
			linux_to_hex(map_addr, buf);
			linux_print_string(1, buf);
			linux_print_string(1, "\n");
			linux_exit(3);
		}

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

		linux_mprotect(segment, rounded_len, protflags);

		k = pdr->p_vaddr + pdr->p_memsz;
		if (k > brk_addr) brk_addr = k;
	}

	if (interp)
	{
		Elf64_Ehdr *junk_ehdr = NULL;
		entry_point = linux_load_elf(linux_map_file(&(((char *)mapped)[interp->p_offset])), 1, ldso_ehdr, &junk_ehdr);
	}

	if (!anywhere)
		linux_brk(ROUNDUP(brk_addr, 0x1000));

	return (void *)entry_point;
}

void *
memcopy(void *dest, const void *src, unsigned long n)
{
    unsigned long i;
    unsigned char *d = (unsigned char *)dest;
    unsigned char *s = (unsigned char *)src;

/*
	char buf[64];

	linux_print_string(1, "linux_load_elf() copying 0x");
	linux_to_hex(n, buf);
	linux_print_string(1, buf);
	linux_print_string(1, " bytes from 0x");
	linux_to_hex((unsigned long)src, buf);
	linux_print_string(1, buf);
	linux_print_string(1, " to 0x");
	linux_to_hex((unsigned long)dest, buf);
	linux_print_string(1, buf);
	linux_print_string(1, "\n");
	linux_read(0, buf, 1);
*/

    for (i = 0; i < n; ++i)
        d[i] = s[i];

    return dest;
}

void *
linux_map_file(char *file_linux_to_map)
{
	struct stat sb;
	void *mapped;

	if (0 > linux_stat(file_linux_to_map, &sb))
	{
		linux_error_msg("linux_map_file stat() failed ");
		linux_exit(1);
	}

	mapped = linux_mmap(NULL, sb.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

	if (mapped == (void *)-1)
	{
		linux_error_msg("linux_map_file mmap() failed ");
		linux_exit(1);
	}

	copy_in(file_linux_to_map, mapped);

	return mapped;
}

void
copy_in(char *filename, void *address)
{
	int fd, cc;
	off_t offset = 0;
	char buf[1024];

	if (0 > (fd = linux_open(filename, 0, 0)))
	{
		linux_error_msg("opening dynamically-loaded file failed");
		linux_exit(2);
	}

	while (0 < (cc = linux_read(fd, buf, sizeof(buf))))
	{
		linux_memcpy((address + offset), buf, cc);
		offset += cc;
	}

	linux_close(fd);
}

void
print_maps(void)
{
	char rbuf[1024];
	int fd, cc;

	fd = linux_open("/proc/self/maps", 0, 0);
	while (0 < (cc = linux_read(fd, rbuf, sizeof(rbuf))))
		linux_write(1, rbuf, cc);
	linux_close(fd);
}

void
linux_unmap(char *progname)
{
	char buf[1024], *p;
	char rbuf[2048];
	int cc, fd;

	fd = linux_open("/proc/self/maps", 0, 0);

	p = &buf[0];

	while (0 < (cc = linux_read(fd, rbuf, sizeof(rbuf))))
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
				 * mapped in from this running program, ld.so or libc, linux_unmap it.
				 * This will keep the exec'd program's address space a lot
				 * cleaner.  But even a 32-bit address space can hold 2 copies
				 * of glibc without ill effects, so you don't really have to
				 * mlinux_unmap() anything other than the program calling ul_exec() */
				if (linux_strstr(buf, progname) || /* linux_strstr(buf, "libdl") || linux_strstr(buf, "/usr/lib/ld-")
					|| linux_strstr(buf, "/lib64/ld-") || */linux_strstr(buf, "libc"))
				{
					char *u;
					char *first, *second;
					unsigned long low, high;

					u = linux_strchr(buf, ' ');
					*u = '\0';

					first = buf;

					second = linux_strchr(first, '-');
					*second = '\0';
					++second;

					low = linux_strtoul(first, NULL, 0x10);
					high = linux_strtoul(second, NULL, 0x10);
                    linux_print_string(1, "before unap:\n");
//                     print_maps();
					linux_munmap((void *)low, high-low);
                    linux_print_string(1, "after unap:\n");
//                     print_maps();
				}

				p = &buf[0];
			}
		}
	}

	linux_close(fd);

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
			len += linux_strlen(argv[i]) + 1;
	else {
		argc = 0;
		char **p = argv;
		while (*p)
		{
			len += linux_strlen(*p) + 1;
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
	linux_munmap((void *)args->block, args->size);
	linux_munmap((void *)args, sizeof(*args));
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
	linux_memcpy((void *)r->block, (void *)p, r->size);

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
