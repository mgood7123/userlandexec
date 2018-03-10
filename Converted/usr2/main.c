// preinitmulti()
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

int main(int argc, char **argv)
{
    int extract_files(void) {
        findyourself_init(argv[0]);

        char newpath[PATH_MAX];
        find_yourself(newpath, sizeof(newpath));
        if(1 || strcmp(argv[0],newpath)) { }
        char *dummy  = strdup( newpath );
        char *dname = dirname( dummy );
        printf("Extracting...\n");
// objectconvertMulti()
        // OBJECT START
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_bin_cat_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_bin_cat_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_bin_cat_end;
        char * a0p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_bin_cat_start;
        FILE *a0fp;
        chdir(dname);
        
        mkdir("usr2", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("bin", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("bin"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        a0fp = fopen("./cat", ("w+"));
        while ( a0p != &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_bin_cat_end ) fputc(*a0p++, a0fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
        fclose(a0fp);
        chmod("./cat", 00755);
        chdir(dname);
        
        // OBJECT END
        // objectconvertMulti()
        // OBJECT START
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_bin_ls_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_bin_ls_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_bin_ls_end;
        char * a1p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_bin_ls_start;
        FILE *a1fp;
        chdir(dname);
        
        mkdir("usr2", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("bin", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("bin"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        a1fp = fopen("./ls", ("w+"));
        while ( a1p != &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_bin_ls_end ) fputc(*a1p++, a1fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
        fclose(a1fp);
        chmod("./ls", 00755);
        chdir(dname);
        
        // OBJECT END
        // objectconvertMulti()
        // OBJECT START
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib64_ld_linux_x86_64_so_2_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib64_ld_linux_x86_64_so_2_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib64_ld_linux_x86_64_so_2_end;
        char * a2p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib64_ld_linux_x86_64_so_2_start;
        FILE *a2fp;
        chdir(dname);
        
        mkdir("usr2", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("lib64", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("lib64"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        a2fp = fopen("./ld-linux-x86-64.so.2", ("w+"));
        while ( a2p != &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib64_ld_linux_x86_64_so_2_end ) fputc(*a2p++, a2fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
        fclose(a2fp);
        chmod("./ld-linux-x86-64.so.2", 00755);
        chdir(dname);
        
        // OBJECT END
        // objectconvertMulti()
        // OBJECT START
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib_libcap_so_2_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib_libcap_so_2_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib_libcap_so_2_end;
        char * a3p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib_libcap_so_2_start;
        FILE *a3fp;
        chdir(dname);
        
        mkdir("usr2", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("lib", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("lib"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        a3fp = fopen("./libcap.so.2", ("w+"));
        while ( a3p != &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib_libcap_so_2_end ) fputc(*a3p++, a3fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
        fclose(a3fp);
        chmod("./libcap.so.2", 00644);
        chdir(dname);
        
        // OBJECT END
        // objectconvertMulti()
        // OBJECT START
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib_libc_so_6_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib_libc_so_6_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib_libc_so_6_end;
        char * a4p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib_libc_so_6_start;
        FILE *a4fp;
        chdir(dname);
        
        mkdir("usr2", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("lib", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("lib"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        a4fp = fopen("./libc.so.6", ("w+"));
        while ( a4p != &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_usr2_lib_libc_so_6_end ) fputc(*a4p++, a4fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
        fclose(a4fp);
        chmod("./libc.so.6", 00755);
        chdir(dname);
        
        // OBJECT END
        // mkdirs()
        // MKDIRS START
        chdir(dname);
        
        mkdir("usr2", 0777);
         // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        chdir(dname);
        
        // MKDIRS END
        // mkdirs()
        // MKDIRS START
        chdir(dname);
        
        mkdir("usr2", 0777);
         // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("bin", 0777);
         // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("bin"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        chdir(dname);
        
        // MKDIRS END
        // mkdirs()
        // MKDIRS START
        chdir(dname);
        
        mkdir("usr2", 0777);
         // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("lib64", 0777);
         // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("lib64"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        chdir(dname);
        
        // MKDIRS END
        // mkdirs()
        // MKDIRS START
        chdir(dname);
        
        mkdir("usr2", 0777);
         // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("lib", 0777);
         // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("lib"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        chdir(dname);
        
        // MKDIRS END
        // chmoddir()
        // CHMODDIR START
        chdir(dname);
        chmod("usr2", 00755);
        chdir(dname);
        
        // CHMODDIR END
        // chmoddir()
        // CHMODDIR START
        chdir(dname);
        chmod("usr2/bin", 00755);
        chdir(dname);
        
        // CHMODDIR END
        // chmoddir()
        // CHMODDIR START
        chdir(dname);
        chmod("usr2/lib64", 00755);
        chdir(dname);
        
        // CHMODDIR END
        // chmoddir()
        // CHMODDIR START
        chdir(dname);
        chmod("usr2/lib", 00755);
        chdir(dname);
        
        // CHMODDIR END
        // finmulti()
        free( dummy );
        printf("Extracted\n");
        return 0;
    }
    int list() {
        printf("listing\n");
        printf("\
./usr2 \n\
./usr2/bin \n\
./usr2/bin/cat \n\
./usr2/bin/ls \n\
./usr2/lib64 \n\
./usr2/lib64/ld-linux-x86-64.so.2 \n\
./usr2/lib \n\
./usr2/lib/libcap.so.2 \n\
./usr2/lib/libc.so.6 \n\
"
        );
        return 0;
    }
    int shift(int times) {
        #ifdef SHIFT_VERBOSE
            int shift_mode_verbose = 1;
        #else
            int shift_mode_verbose = 0;
        #endif
        if (shift_mode_verbose == 1) {
            printf("The following arguments were passed (before) to main(%i):\n", argc);
            for(int i=1; i<argc; i++) {
                printf("arg0: %s\n", i, argv[i]);
            }
            printf("\n");
        }
        if (shift_mode_verbose == 1) {printf("shifting %i times\n", times);}
        for(int t=1; t<times+1; ++t) { // we set this to 1 and respectively increase by 1, to avoid shifting argv[0] as it is holds the program name
            for(int i=1; i<argc; ++i) { // we set this to 1 to avoid shifting argv[0] as it is holds the program name
                if (shift_mode_verbose == 1) {printf("shift %i: arg%i: %s >", t, i, argv[i]);}
                    argv[i]  = argv[i+1];
                if (shift_mode_verbose == 1) {printf(" %s\n", argv[i]);}
            }
            --argc;
        }
        if (shift_mode_verbose == 1) {
            printf("The following arguments were passed (after) to main(%i):\n", argc);
            for(int i=1; i<argc; i++) {
                printf("arg%i: %s\n", i, argv[i]);
            }
            printf("\n");
    }
        return 0;
    }

    int fshift(int times) { // a potentially faster version of shift()
        times = argc < times ? argc : times;
        argc = (argc) - times;
        (argv) += times;
        return 0;
    }
    char *mode;
    if (argc < 2) {
         mode = "NULL";
    } else {
        mode  = argv[1];
    }
    if (strcmp(mode,"list") == 0) {
        list();
    } else if (strcmp(mode,"NULL") == 0) {
        extract_files();
    } else {
        printf("invalid\n");
    }
}
