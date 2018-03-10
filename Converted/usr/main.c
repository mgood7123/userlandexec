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

#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

int
make_named_socket (const char *filename)
{
    struct sockaddr_un name;
    int sock;
    size_t size;

    /* Create the socket. */
    sock = socket (PF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0)
        {
            perror ("socket");
            exit (EXIT_FAILURE);
        }

    /* Bind a name to the socket. */
    name.sun_family = AF_LOCAL;
    strncpy (name.sun_path, filename, sizeof (name.sun_path));
    name.sun_path[sizeof (name.sun_path) - 1] = '\0';

    /* The size of the address is
        the offset of the start of the filename,
        plus its length (not including the terminating null byte).
        Alternatively you can just do:
        size = SUN_LEN (&name);
    */
    size = (offsetof (struct sockaddr_un, sun_path)
            + strlen (name.sun_path));

    if (bind (sock, (struct sockaddr *) &name, size) < 0)
        {
            perror ("bind");
            exit (EXIT_FAILURE);
        }

    return sock;
}

int main(int argc, char **argv, char ** env)
{
  findyourself_init(argv[0]);

  char newpath[PATH_MAX];
  find_yourself(newpath, sizeof(newpath));
//   if(1 || strcmp(argv[0],newpath)) { }
//   char *dummy  = strdup( newpath );
//   char *dname = dirname( dummy );

  // OBJECT START
        extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_bin_ls_start;
        extern size_t _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_bin_ls_size;
        extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_bin_ls_end;
        char * p = &_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_bin_ls_start;
        char ch;
        int bytes=1;
        printf("size is %zu bytes\n", (size_t)&_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_bin_ls_size);
  
        char * array = malloc((size_t)&_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_bin_ls_size);
        while ( p != &_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_bin_ls_end ) {
           ch = *p++;
            array[bytes-1] = ch;
            bytes++;
        }
        bytes--;

        extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib64_ld_linux_x86_64_so_2_start;
        extern size_t _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib64_ld_linux_x86_64_so_2_size;
        extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib64_ld_linux_x86_64_so_2_end;
        char * pi = &_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib64_ld_linux_x86_64_so_2_start;
        char chi;
        int bytesi=1;
        printf("size is %zu bytes\n", (size_t)&_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib64_ld_linux_x86_64_so_2_size);
  
        char * arrayi = malloc((size_t)&_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib64_ld_linux_x86_64_so_2_size);
        while ( pi != &_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib64_ld_linux_x86_64_so_2_end ) {
           chi = *pi++;
            arrayi[bytesi-1] = chi;
            bytesi++;
        }
        bytesi--;

        printf("passing program and interperater to ul.so:\n");

        ulexec_array(array, arrayi, "-l -h", env);

        return (0);
  printf("Extracting...\n");

//         // OBJECT START
//         extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_bin_ls_start;
//         extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_bin_ls_size; // we keep this just in case we ever need it
//         extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_bin_ls_end;
//         char * a0p = &_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_bin_ls_start;
//         FILE *a0fp;
//         chdir(dname);
//         
//         mkdir("usr", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("usr"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         mkdir("bin", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("bin"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         
//         a0fp = fopen("./ls", ("w+"));
//         while ( a0p != &_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_bin_ls_end ) fputc(*a0p++, a0fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
//         fclose(a0fp);
//         chmod("./ls", 00755);
//         chdir(dname);
//         
//         // OBJECT END
//         
//         // OBJECT START
//         extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib64_ld_linux_x86_64_so_2_start;
//         extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib64_ld_linux_x86_64_so_2_size; // we keep this just in case we ever need it
//         extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib64_ld_linux_x86_64_so_2_end;
//         char * a1p = &_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib64_ld_linux_x86_64_so_2_start;
//         FILE *a1fp;
//         chdir(dname);
//         
//         mkdir("usr", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("usr"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         mkdir("lib64", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("lib64"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         
//         a1fp = fopen("./ld-linux-x86-64.so.2", ("w+"));
//         while ( a1p != &_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib64_ld_linux_x86_64_so_2_end ) fputc(*a1p++, a1fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
//         fclose(a1fp);
//         chmod("./ld-linux-x86-64.so.2", 00755);
//         chdir(dname);
//         
//         // OBJECT END
//         
//         // OBJECT START
//         extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib_libcap_so_2_start;
//         extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib_libcap_so_2_size; // we keep this just in case we ever need it
//         extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib_libcap_so_2_end;
//         char * a2p = &_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib_libcap_so_2_start;
//         FILE *a2fp;
//         chdir(dname);
//         
//         mkdir("usr", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("usr"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         mkdir("lib", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("lib"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         
//         a2fp = fopen("./libcap.so.2", ("w+"));
//         while ( a2p != &_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib_libcap_so_2_end ) fputc(*a2p++, a2fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
//         fclose(a2fp);
//         chmod("./libcap.so.2", 00644);
//         chdir(dname);
//         
//         // OBJECT END
//         
//         // OBJECT START
//         extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib_libc_so_6_start;
//         extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib_libc_so_6_size; // we keep this just in case we ever need it
//         extern char _binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib_libc_so_6_end;
//         char * a3p = &_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib_libc_so_6_start;
//         FILE *a3fp;
//         chdir(dname);
//         
//         mkdir("usr", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("usr"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         mkdir("lib", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("lib"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         
//         a3fp = fopen("./libc.so.6", ("w+"));
//         while ( a3p != &_binary__home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr_lib_libc_so_6_end ) fputc(*a3p++, a3fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
//         fclose(a3fp);
//         chmod("./libc.so.6", 00755);
//         chdir(dname);
//         
//         // OBJECT END
//         
//         // MKDIRS START
//         chdir(dname);
//         
//         mkdir("usr", 0777);
//          // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("usr"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         
//         chdir(dname);
//         
//         // MKDIRS END
//         
//         // MKDIRS START
//         chdir(dname);
//         
//         mkdir("usr", 0777);
//          // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("usr"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         mkdir("bin", 0777);
//          // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("bin"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         
//         chdir(dname);
//         
//         // MKDIRS END
//         
//         // MKDIRS START
//         chdir(dname);
//         
//         mkdir("usr", 0777);
//          // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("usr"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         mkdir("lib64", 0777);
//          // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("lib64"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         
//         chdir(dname);
//         
//         // MKDIRS END
//         
//         // MKDIRS START
//         chdir(dname);
//         
//         mkdir("usr", 0777);
//          // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("usr"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         mkdir("lib", 0777);
//          // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         chdir("lib"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
//         
//         chdir(dname);
//         
//         // MKDIRS END
//         
//         // CHMODDIR START
//         chdir(dname);
//         chmod("usr", 00755);
//         chdir(dname);
//         
//         // CHMODDIR END
//         
//         // CHMODDIR START
//         chdir(dname);
//         chmod("usr/bin", 00755);
//         chdir(dname);
//         
//         // CHMODDIR END
//         
//         // CHMODDIR START
//         chdir(dname);
//         chmod("usr/lib64", 00755);
//         chdir(dname);
//         
//         // CHMODDIR END
//         
//         // CHMODDIR START
//         chdir(dname);
//         chmod("usr/lib", 00755);
//         chdir(dname);
//         
//         // CHMODDIR END
//         
//   free( dummy );
//   printf("Extracted\n");
  return 0;
    }

