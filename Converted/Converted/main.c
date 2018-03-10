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
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_ul_exec_libc_so_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_ul_exec_libc_so_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_ul_exec_libc_so_end;
        char * a0p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_ul_exec_libc_so_start;
        FILE *a0fp;
        chdir(dname);
        
        mkdir("Converted", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("Converted"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        a0fp = fopen("./ul_exec_libc.so", ("w+"));
        while ( a0p != &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_ul_exec_libc_so_end ) fputc(*a0p++, a0fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
        fclose(a0fp);
        chmod("./ul_exec_libc.so", 00755);
        chdir(dname);
        
        // OBJECT END
        // objectconvertMulti()
        // OBJECT START
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted__gdb_history_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted__gdb_history_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted__gdb_history_end;
        char * a1p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted__gdb_history_start;
        FILE *a1fp;
        chdir(dname);
        
        mkdir("Converted", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("Converted"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        a1fp = fopen("./.gdb_history", ("w+"));
        while ( a1p != &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted__gdb_history_end ) fputc(*a1p++, a1fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
        fclose(a1fp);
        chmod("./.gdb_history", 00600);
        chdir(dname);
        
        // OBJECT END
        // objectconvertMulti()
        // OBJECT START
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_2_o_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_2_o_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_2_o_end;
        char * a2p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_2_o_start;
        FILE *a2fp;
        chdir(dname);
        
        mkdir("Converted", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("Converted"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("usr2", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        a2fp = fopen("./2.o", ("w+"));
        while ( a2p != &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_2_o_end ) fputc(*a2p++, a2fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
        fclose(a2fp);
        chmod("./2.o", 00644);
        chdir(dname);
        
        // OBJECT END
        // objectconvertMulti()
        // OBJECT START
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_a_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_a_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_a_end;
        char * a3p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_a_start;
        FILE *a3fp;
        chdir(dname);
        
        mkdir("Converted", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("Converted"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("usr2", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        a3fp = fopen("./a", ("w+"));
        while ( a3p != &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_a_end ) fputc(*a3p++, a3fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
        fclose(a3fp);
        chmod("./a", 00755);
        chdir(dname);
        
        // OBJECT END
        // objectconvertMulti()
        // OBJECT START
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_1_o_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_1_o_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_1_o_end;
        char * a4p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_1_o_start;
        FILE *a4fp;
        chdir(dname);
        
        mkdir("Converted", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("Converted"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("usr2", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        a4fp = fopen("./1.o", ("w+"));
        while ( a4p != &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_1_o_end ) fputc(*a4p++, a4fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
        fclose(a4fp);
        chmod("./1.o", 00644);
        chdir(dname);
        
        // OBJECT END
        // objectconvertMulti()
        // OBJECT START
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_main_c_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_main_c_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_main_c_end;
        char * a5p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_main_c_start;
        FILE *a5fp;
        chdir(dname);
        
        mkdir("Converted", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("Converted"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("usr2", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        a5fp = fopen("./main.c", ("w+"));
        while ( a5p != &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_main_c_end ) fputc(*a5p++, a5fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
        fclose(a5fp);
        chmod("./main.c", 00644);
        chdir(dname);
        
        // OBJECT END
        // objectconvertMulti()
        // OBJECT START
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_3_o_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_3_o_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_3_o_end;
        char * a6p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_3_o_start;
        FILE *a6fp;
        chdir(dname);
        
        mkdir("Converted", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("Converted"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("usr2", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        a6fp = fopen("./3.o", ("w+"));
        while ( a6p != &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_3_o_end ) fputc(*a6p++, a6fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
        fclose(a6fp);
        chmod("./3.o", 00644);
        chdir(dname);
        
        // OBJECT END
        // objectconvertMulti()
        // OBJECT START
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_0_o_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_0_o_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_0_o_end;
        char * a7p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_0_o_start;
        FILE *a7fp;
        chdir(dname);
        
        mkdir("Converted", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("Converted"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("usr2", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        a7fp = fopen("./0.o", ("w+"));
        while ( a7p != &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_0_o_end ) fputc(*a7p++, a7fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
        fclose(a7fp);
        chmod("./0.o", 00644);
        chdir(dname);
        
        // OBJECT END
        // objectconvertMulti()
        // OBJECT START
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_a_out_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_a_out_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_a_out_end;
        char * a8p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_a_out_start;
        FILE *a8fp;
        chdir(dname);
        
        mkdir("Converted", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("Converted"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        mkdir("usr2", 0777); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        chdir("usr2"); // Simulates the mkdir -p ( mkdir --help: -p, --parents ) argument of the mkdir binary found in /bin/mkdir or /usr/bin/mkdir
        
        a8fp = fopen("./a.out", ("w+"));
        while ( a8p != &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_a_out_end ) fputc(*a8p++, a8fp); // credit for fputc(*p++, fp): flawless_snowflake from kik messenger
        fclose(a8fp);
        chmod("./a.out", 00755);
        chdir(dname);
        
        // OBJECT END
        // objectconvertMulti()
        // OBJECT START
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_4_o_start;
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_4_o_size; // we keep this just in case we ever need it
        extern char _binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_4_o_end;
        char * a9p = &_binary__chakra_home_universalpackagemanager_UPM_asm_elf_parser_not_compiled_Converted_usr2_4_o_start;
        FILE *a9fp;
        chdir(dname);
        
        