#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/auxv.h>
void ulexec(char * pro, char * args, char **env);
void ulexec_array(void * mapped, void * mapped_interpreter, char * args, char **env);
main(int argc, char *argv[], char ** env)
{
    printf("%s\n", (char *)getauxval(AT_EXECFN));
//     ulexec(argc, argv, env, NULL);
    ulexec("/bin/cat", "/proc/self/maps", env);

    const char * filename = "/bin/ls";
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("cannot open \"%s\", returned %i\n", filename, fd);
        return -1;
    }
    size_t len = 0;
    len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, 0);
    void* mapped = mmap (NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        printf ("map failed\n");
        exit;
    } else {
        printf ("map (%s) succeded with address: %p\n", filename, &mapped);
    }

    const char * interp = "/lib64/ld-linux-x86-64.so.2";
    int fdI = open(interp, O_RDONLY);
    if (fdI < 0) {
        printf("cannot open \"%s\", returned %i\n", interp, fdI);
        return -1;
    }
    size_t lenI = 0;
    lenI = lseek(fdI, 0, SEEK_END);
    lseek(fdI, 0, 0);
    void* mappedI = mmap (NULL, len, PROT_READ, MAP_PRIVATE, fdI, 0);
    if (mappedI == MAP_FAILED) {
        printf ("map failed\n");
        exit;
    } else {
        printf ("map (%s) succeded with address: %p\n", interp, &mappedI);
    }

//     ulexec_array(mapped, mappedI, "-l", env);

    printf("\n\nfinished\n\n");
	return 0;
}
