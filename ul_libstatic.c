#include <sys/syscall.h>
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

int
linux_close(int fd)
{

	long ret;
	asm volatile ("syscall" : "=a" (ret) : "a" (__NR_close),
		      "D" (fd):
		      "cc", "memory", "rcx",
		      "r8", "r9", "r10", "r11" );
	if (ret < 0)
	{
		errno = -ret;
		ret = -1;
	}
	return (int)ret;
}

void linux_exit(int code)
{
	asm volatile ("syscall" : : "a" (__NR_exit), "D" (code));
}

int
linux_open(const char *pathname, unsigned long flags, unsigned long mode)
{

	long ret;
	asm volatile ("syscall" : "=a" (ret) : "a" (__NR_open),
		      "D" (pathname), "S" (flags), "d" (mode) :
		      "cc", "memory", "rcx",
		      "r8", "r9", "r10", "r11" );
	if (ret < 0)
	{
		errno = -ret;
		ret = -1;
	}
	return (int) ret;
}

int
linux_read(int fd, char *buffer, unsigned long bufferlen)
{

	long ret;
	asm volatile ("syscall" : "=a" (ret) : "a" (__NR_read),
		      "D" (fd), "S" (buffer), "d" (bufferlen) :
		      "cc", "memory", "rcx",
		      "r8", "r9", "r10", "r11" );
	if (ret < 0)
	{
		errno = -ret;
		ret = -1;
	}
	return (int)ret;
}

int
linux_write(int fd, const void *data, unsigned long len)
{

	long ret;
	asm volatile ("syscall" : "=a" (ret) : "a" (__NR_write),
		      "D" (fd), "S" (data), "d" (len) :
		      "cc", "memory", "rcx",
		      "r8", "r9", "r10", "r11" );
	if (ret < 0)
	{
		errno = -ret;
		ret = -1;
	}
	return (int)ret;
}

/*
void *
linux_mmap(
	void *start,
	unsigned long length,
	int prot,
	int flags,
	int fd,
	unsigned long offset
)
*/
asm (
    ".text\n"
    ".global linux_mmap\n"
        ".type linux_mmap,@function\n"
        "linux_mmap:\n\t"
		"mov    %rcx,%r10\n\t"
		"mov    $0x9,%eax\n\t"
		"syscall \n\t"
		"cmp    $0xfffffffffffff001,%rax\n\t"
		"jae    .Lx1\n\t"
		".Lx2:\n\t"
		"retq   \n\t"
		".Lx1:\n\t"
		"mov    2623486(%rip),%rcx\n\t"
		"xor    %edx,%edx\n\t"
		"sub    %rax,%rdx\n\t"
		"mov    %edx,%fs:(%rcx)\n\t"
		"or     $0xffffffffffffffff,%rax\n\t"
		"jmp    .Lx2\n"
);

int
linux_munmap(void *start, unsigned long length)
{

	long ret;
	asm volatile ("syscall" : "=a" (ret) : "a" (__NR_munmap),
		      "D" (start), "S" (length) :
		      "cc", "memory", "rcx",
		      "r8", "r9", "r10", "r11" );
	if (ret < 0)
	{
		errno = -ret;
		ret = -1;
	}
	return (int)ret;
}

//       int stat(const char *path, struct stat *buf);

int
linux_stat(const char *path, void *buf)
{
	long ret;
	asm volatile ("syscall" :
		"=a" (ret) :
		"a" (4), "D" (path), "S" (buf) :
		"memory"
	);
	if (ret < 0)
	{
		errno = -ret;
		ret = -1;
	}
	return (int)ret;
}

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


int
linux_print_long(int fd, unsigned long i)
{
	char i_buff[32];
	int l, r;
	linux_to_decimal(i, i_buff);
	for (l = 0; i_buff[l]; ++l);
	r = linux_write(fd, i_buff, l);
	return r;
}

int
linux_print_hex(int fd, unsigned long i)
{
	char i_buff[64];
	int l, r;
	linux_to_hex(i, i_buff);
	for (l = 0; i_buff[l]; ++l);
	r = linux_write(fd, i_buff, l);
	return r;
}

int
linux_print_string(int fd, char *s)
{
	int i, r;
	for (i = 0; s[i]; ++i);
	r = linux_write(fd, s, i);
	return r;
}

int
linux_to_decimal(unsigned long x, char *p)
{
	int count = 0;

	if (x == 0)
		*p++ ='0';
	else {
		unsigned long q, r, b;
		int f = 0;
		b = 10000000000000000000U;

		do {
			q = x/b;
			if (q || f)
			{
				*p++ = ('0' + q);
				++count;
				f = 1;
				x = x%b;
			}
			b /= 10;
		} while (b > 0);

	}

	*p = '\0';

	return count;
}

int
linux_to_hex(unsigned long n, char *p)
{
	int i;
	int count = 0;
	for (i = 0; i < 16; ++i)
	{
		char x = ((n >> 60)  & 0xf);
		if (x < (char)10)
			*p++ = x + '0';
		else
			*p++ = (x - 10) + 'a';
		++count;
		n <<= 4;
	}
	*p = '\0';
	return count;
}

// long errno = 0;

unsigned long int
linux_strtoul(const char *nptr, char **endptr, int base)
{
	unsigned long ret = 0;
	int i;

	for (i = 0; nptr[i]; ++i)
	{
		char digit = nptr[i];
		unsigned int value;
		if (digit <= '9')
		{
			value = '0';
		} else if (digit <= 'Z') {
			value = 'A' - 10;
		} else if (digit <= 'z') {
			value = 'a' - 10;
		}
		ret *= base;
		ret += (digit - value);
		if (endptr) *endptr = &(nptr[i]);
	}

	return ret;
}

unsigned long
linux_file_size(char *filename)
{
	char sbuf[144];
	unsigned long ret;

	if (0 > (long)(ret = linux_stat(filename, (void *)&sbuf)))
	{
		linux_print_string(2, "stat problem: ");
		linux_print_long(2, errno);
		linux_print_string(2, "\n");
	} else {
		ret = *(unsigned long *)(sbuf+48);
	}

	return ret;
}

unsigned long
linux_strlen(const char *s)
{
	unsigned long r = 0;
	for (; s && *s; ++s, ++r);
	return r;
}

char *linux_strchr(const char *s, int c)
{
	char *r = NULL;

	for (; s && *s; ++s)
	{
		if (*s == c)
		{
			r = (char *)s;
			break;
		}
	}
	return r;
}

char *
linux_strcat(char *dest, const char *src)
{
	if (dest && src)
	{
		char *p = dest;
		while (*p)
			++p;

		for (; *src; ++p, ++src)
			*p = *src;
	}

	return dest;
}
char *linux_strstr(const char *str, char *substr)
{
	char *r = NULL;
	int substrl = linux_strlen(substr);
	int linux_strl = linux_strlen(str);

	if (substrl < linux_strl)
	{
		int i;

		for (i = 0; i <= linux_strl - substrl; ++i)
		{
			char *p = (char *)&str[i];
			int j;

			for (j = 0; j < substrl; ++j)
			{
				if (p[j] != substr[j])
					break;
			}

			if (j == substrl)
			{
				r = p;
				break;
			}
		}
	} else if (substrl == linux_strl) {
		int i;
		char *p = (char *)&str[0];
		for (i = 0; i < substrl; ++i)
		{
			if (p[i] != substr[i])
				break;
		}
		if (i == substrl)
			r = p;
	}

	return r;
}

int
linux_mprotect(void *addr, unsigned long len, int prot)
{

	long ret;
	asm volatile ("syscall" : "=a" (ret) : "a" (__NR_mprotect),
		      "D" (addr), "S" (len), "d" (prot) :
		      "cc", "memory", "rcx",
		      "r8", "r9", "r10", "r11" );
	if (ret < 0)
	{
		errno = -ret;
		ret = -1;
	}
	return (int) ret;
}

void linux_brk(unsigned long addr)
{
	asm volatile ("syscall" : : "a" (__NR_brk), "D" (addr));
}
