/* Low-level I/O routines for BFDs.

   Copyright (C) 1990-2018 Free Software Foundation, Inc.

   Written by Cygnus Support.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include <limits.h>
#include "bfd.h"
#include "libbfd.h"
#include <sys/mman.h>
#include <linux/memfd.h>

int tmp_write;
#ifndef S_IRUSR
#define S_IRUSR 0400    /* Read by owner.  */
#endif
#ifndef S_IRGRP
#define S_IRGRP 0040    /* Read by group.  */
#endif
#ifndef S_IROTH
#define S_IROTH 0004    /* Read by others.  */
#endif
#ifndef S_IWUSR
#define S_IWUSR 0200    /* Write by owner.  */
#endif
#ifndef S_IWGRP
#define S_IWGRP 0020    /* Write by group.  */
#endif
#ifndef S_IWOTH
#define S_IWOTH 0002    /* Write by others.  */
#endif
#ifndef S_IXUSR
#define S_IXUSR 0100    /* Execute by owner.  */
#endif
#ifndef S_IXGRP
#define S_IXGRP 0010    /* Execute by group.  */
#endif
#ifndef S_IXOTH
#define S_IXOTH 0001    /* Execute by others.  */
#endif

#ifndef FD_CLOEXEC
#define FD_CLOEXEC 1
#endif
file_ptr
_bfd_real_ftell (FILE *file)
{
// printf("f-telling file\n");
#if defined (HAVE_FTELLO64)
  return ftello64 (file);
#elif defined (HAVE_FTELLO)
  return ftello (file);
#else
  return ftell (file);
#endif
}

int
_bfd_real_fseek (FILE *file, file_ptr offset, int whence)
{
// printf("\nf-seeking file with fd %d\n\n", fileno(file));
#if defined (HAVE_FSEEKO64)
  return fseeko64 (file, offset, whence);
#elif defined (HAVE_FSEEKO)
  return fseeko (file, offset, whence);
#else
  return fseek (file, offset, whence);
#endif
}

/* Mark FILE as close-on-exec.  Return FILE.  FILE may be NULL, in
   which case nothing is done.  */
static FILE *
close_on_exec (FILE *file)
{
// printf("1: attempting to mark file as close-on-exec\n");
#if defined (HAVE_FILENO) && defined (F_GETFD)
// printf("2: attempting to mark file as close-on-exec\n");
  if (file)
    {
// printf("file is non-zero: attempting to mark file as close-on-exec\n");
      int fd = fileno (file);
      int old = fcntl (fd, F_GETFD, 0);
      if (old >= 0)
	fcntl (fd, F_SETFD, old | FD_CLOEXEC);
    }
#endif
// printf("returning file\n");
  return file;
}

int read_(const char *src, char **dest, int len) {
    char *p = malloc(len + 1);
    memcpy(p, src, len);
    p[len] = 0;
    *dest = p;
    return len;
}

FILE *
map_file_TMP(const char * filename, const char * mode, int method)
{
//     fprintf(stderr, "   START OF FUNCTION: method = %d\n", method);
    int fd;
    fprintf(stderr, "file = %s, mode = %s\n", filename, mode);
    if ( strcmp(mode, "r") == 0 ) {
        if (method == 0) {
        fprintf(stderr, "using fopen\n");
#if defined (HAVE_FOPEN64)
    return fopen64(filename, mode);
#else
    return fopen(filename, mode);
#endif
        } else if ( method == 1 ) {
        fprintf(stderr, "using fmemopen\n");
        fd = open(filename, O_RDONLY);
        if (fd < 0) {
            printf("    cannot open \"%s\", returned %i\n", filename, fd);
            exit (-1);
        }
        size_t len = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, 0);
        void* mapped = mmap (NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
        if (mapped == MAP_FAILED) {
            printf ("map failed\n");
            exit;
        } else {
            printf ("map (%s) succeded with address: %p\n", filename, &mapped);
        }
            void *k;
        read_(mapped, &k, 2);
        printf("closing fd\n");
        munmap(mapped, len);;
        close(fd);
        FILE* TMP = fmemopen(k, len, mode);
        if (TMP == NULL) {
            printf("    FAILED\n");
        }
            return TMP;
        }
    } else  if ( strcmp(mode, "w+") == 0 ) {
        tmp_write=1;
        // suppresses the file creation untill write in wich we re create with real size, for now we satisfy the request
        
        /* since creat() returns an fd and we do not need to do anything with it (as we need to re open the file as read only or read/write accordingly) we can do this
        
        equivilant of
        
        $ touch $filename
        
        */

        if (method == 0) {
        fprintf(stderr, "using fopen\n");
#if defined (HAVE_FOPEN64)
    return fopen64(filename, mode);
#else
    return fopen(filename, mode);
#endif
        } else if ( method == 1 ) {
        fprintf(stderr, "using fmemopen\n");
        tmp_write=1;
        // suppresses the file creation untill write in wich we re create with real size, for now we satisfy the request with a FILE*
        
        /* since creat() returns an fd and we do not need to do anything with it (as we need to re open the file as read only or read/write accordingly) we can do this
        
        equivilant of
        
        $ touch $filename
        
        */
        printf("map opening %s read-write\n", filename);

//      first open/close
        close(creat(filename, O_RDWR));

//      set file to -rw-rw-rw-
        chmod(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

//      2nd open/close
        int fdt = open(filename, O_RDWR);
//         printf("write(%s, \"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\", 65);\n", filename);
//         write(fdt, "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE", 65);
        write(fdt, "E", 2);
        close(fdt);

//      final open, does not write anything during mapping
        int fd = open(filename, O_RDWR);
        if (fd < 0) {
            printf("cannot open \"%s\", returned %i\n", filename, fd);
            exit (-1);
        }
        size_t len = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, 0);
        void* mapped = mmap (NULL, 128*128, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, fd, 0);
        if (mapped == MAP_FAILED) {
            printf ("map failed\n");
            exit;
        } else {
            printf ("\nmap (%s) succeded with address: %p\n", filename, &mapped);
        }
        void * k;
        read_(mapped, &k, 2);
        printf("closing fd\n");
        munmap(mapped, 128*128);;
        close(fd);
        FILE* TMP = fmemopen(k, 128*128, mode);
        if (TMP == NULL) {
            printf("    FAILED\n");
        }
            return TMP;
        }
    } else {
        printf("    unknown mode, recieved \n%s\n, testing against \"r\", got %d, testing against \"w+\", got %d\n", mode, strcmp(mode, "r"), strcmp(mode, "w+"));
    }
}


FILE *
_bfd_real_fopen_TMP (const char *filename, const char *modes, int method)
{
#ifdef VMS
  char *vms_attr;

  /* On VMS, fopen allows file attributes as optional arguments.
     We need to use them but we'd better to use the common prototype.
     In fopen-vms.h, they are separated from the mode with a comma.
     Split here.  */
  vms_attr = strchr (modes, ',');
  if (vms_attr == NULL)
    {
      /* No attributes.  */
      printf("    _bfd_real_fopen_TMP: *VMS  1 /* On VMS, fopen allows file attributes as optional arguments. We need to use them but we'd better to use the common prototype. In fopen-vms.h, they are separated from the mode with a comma. Split here.  */* : opening \n%s\nwith modes:\n%s\n", filename, modes);
      return close_on_exec (map_file (filename, modes, method));
    }
  else
    {
      /* Attributes found.  Split.  */
      size_t modes_len = strlen (modes) + 1;
      char attrs[modes_len + 1];
      char *at[3];
      int i;

      memcpy (attrs, modes, modes_len);
      at[0] = attrs;
      for (i = 0; i < 2; i++)
	{
	  at[i + 1] = strchr (at[i], ',');
	  BFD_ASSERT (at[i + 1] != NULL);
	  *(at[i + 1]++) = 0; /* Replace ',' with a nul, and skip it.  */
	}
      printf("    _bfd_real_fopen_TMP: *VMS  2 /* On VMS, fopen allows file attributes as optional arguments. We need to use them but we'd better to use the common prototype. In fopen-vms.h, they are separated from the mode with a comma. Split here.  */* : opening \n%s\nwith modes:\n%s\n", filename, modes);
      return close_on_exec (fopen (filename, at[0], at[1], at[2]));
    }
#else /* !VMS */
#if defined (HAVE_FOPEN64)
// printf("    _bfd_real_fopen_TMP: opening \n%s\nwith modes:\n%s\n", filename, modes);
return close_on_exec (map_file_TMP (filename, modes, method));
#else
// printf("    _bfd_real_fopen_TMP: opening \n%s\nwith modes:\n%s\n", filename, modes);
return close_on_exec (map_file_TMP (filename, modes, method));
#endif
#endif /* !VMS */
}


bfd *
bfd_fopen_ (const char *filename, const char *target, const char *mode, int fd, int method)
{
  bfd *nbfd;
  const bfd_target *target_vec;

  nbfd = _bfd_new_bfd ();
  if (nbfd == NULL)
    {
      if (fd != -1)
	close (fd);
      return NULL;
    }

  target_vec = bfd_find_target (target, nbfd);
  if (target_vec == NULL)
    {
      if (fd != -1)
	close (fd);
      _bfd_delete_bfd (nbfd);
      return NULL;
    }

    nbfd->iostream = _bfd_real_fopen_TMP (filename, mode, method);
  if (nbfd->iostream == NULL)
    {
      bfd_set_error (bfd_error_system_call);
      _bfd_delete_bfd (nbfd);
      return NULL;
    }

  /* OK, put everything where it belongs.  */

  /* PR 11983: Do not cache the original filename, but
     rather make a copy - the original might go away.  */
  nbfd->filename = xstrdup (filename);

  /* Figure out whether the user is opening the file for reading,
     writing, or both, by looking at the MODE argument.  */
  if ((mode[0] == 'r' || mode[0] == 'w' || mode[0] == 'a')
      && mode[1] == '+')
    nbfd->direction = both_direction;
  else if (mode[0] == 'r')
    nbfd->direction = read_direction;
  else
    nbfd->direction = write_direction;

  if (! bfd_cache_init (nbfd))
    {
      _bfd_delete_bfd (nbfd);
      return NULL;
    }
  nbfd->opened_once = TRUE;

  /* If we opened the file by name, mark it cacheable; we can close it
     and reopen it later.  However, if a file descriptor was provided,
     then it may have been opened with special flags that make it
     unsafe to close and reopen the file.  */
  if (fd == -1)
    (void) bfd_set_cacheable (nbfd, TRUE);

  return nbfd;
}

bfd *
bfd_fopen_tmp (const char *filename, const char *target, const char *mode, int fd, int method)
{
  bfd *nbfd;
  const bfd_target *target_vec;

  nbfd = _bfd_new_bfd ();
  if (nbfd == NULL)
    {
      if (fd != -1)
	close (fd);
      return NULL;
    }

  target_vec = bfd_find_target (target, nbfd);
  if (target_vec == NULL)
    {
      if (fd != -1)
	close (fd);
      _bfd_delete_bfd (nbfd);
      return NULL;
    }

    nbfd->iostream = _bfd_real_fopen_TMP (filename, mode, method);
  if (nbfd->iostream == NULL)
    {
      bfd_set_error (bfd_error_system_call);
      _bfd_delete_bfd (nbfd);
      return NULL;
    }

  /* OK, put everything where it belongs.  */

  /* PR 11983: Do not cache the original filename, but
     rather make a copy - the original might go away.  */
  nbfd->filename = xstrdup (filename);

  /* Figure out whether the user is opening the file for reading,
     writing, or both, by looking at the MODE argument.  */
  if ((mode[0] == 'r' || mode[0] == 'w' || mode[0] == 'a')
      && mode[1] == '+')
    nbfd->direction = both_direction;
  else if (mode[0] == 'r')
    nbfd->direction = read_direction;
  else
    nbfd->direction = write_direction;

  if (! bfd_cache_init (nbfd))
    {
      _bfd_delete_bfd (nbfd);
      return NULL;
    }
  nbfd->opened_once = TRUE;

  /* If we opened the file by name, mark it cacheable; we can close it
     and reopen it later.  However, if a file descriptor was provided,
     then it may have been opened with special flags that make it
     unsafe to close and reopen the file.  */
  if (fd == -1)
    (void) bfd_set_cacheable (nbfd, TRUE);

  return nbfd;
}


extern bfd * testA(const char * filename, const char *target, const char * mode, int fd, int method, int * ret) {
    // 0 = open with fopen
    // 1 = open with fmemopen
//     char * me;
//     if (method == 0) { me = "fopen/fopen64"; } else if ( method == 1 ) { me = "fmemopen"; }
    bfd * entryA;
    bfd *check;
    char **matching;
    entryA = bfd_fopen_tmp (filename, target, mode, fd, method);
    if (!bfd_check_format (entryA, bfd_archive)
    && !bfd_check_format_matches (entryA, bfd_object, &matching))
    {
//         fprintf(stderr, "    entryA (%s): no match (must match)\n", me);
        ret = -1;
    } else {
//         fprintf(stderr, "    entryA (%s): match (must match)\n", me);
        ret = 0;
    }
    return entryA;
}

// extern bfd * testA_(const char * filename, const char * mode, int method, int * ret) {
//     // 0 = open with fopen
//     // 1 = open with fmemopen
// //     char * me;
// //     if (method == 0) { me = "fopen/fopen64"; } else if ( method == 1 ) { me = "fmemopen"; }
//     bfd * entryA;
//     bfd *check;
//     char **matching;
//     entryA = bfd_fopen_ (filename, target, mode, fd, method);
//     if (!bfd_check_format (entryA, bfd_archive)
//     && !bfd_check_format_matches (entryA, bfd_object, &matching))
//     {
// //         fprintf(stderr, "    entryA (%s): no match (must match)\n", me);
//         ret = -1;
//     } else {
// //         fprintf(stderr, "    entryA (%s): match (must match)\n", me);
//         ret = 0;
//     }
//     return entryA;
// }

extern bfd * testB(const char * filename, const char *target, const char * mode, int fd, int method, int * ret) {
    // 0 = open with fopen
    // 1 = open with fmemopen
//     char * me;
//     if (method == 0) { me = "fopen/fopen64"; } else if ( method == 1 ) { me = "fmemopen"; }
    bfd *entryB;
    bfd *check;
    char **matching;
    entryB = bfd_fopen_tmp (filename, target, mode, fd, method);
    if (!bfd_check_format (entryB, bfd_archive)
    && !bfd_check_format_matches (entryB, bfd_object, &matching))
    {
//         fprintf(stderr, "    entryB (%s): no match (must match)\n", me);
        ret = -1;
    } else {
//         fprintf(stderr, "    entryB (%s): match (must match)\n", me);
        ret = 0;
    }
    return entryB;
}

FILE *
map_file(const char * filename, const char * mode)
{
//     return map_file_TMP(filename, mode, 1); // need to figure out how to use fmemopen on a newly created file
//  bfd____format.c : bfd_check_format_matches
//  lang_process (void)
//  /* Finalize dynamic list.  */
//  if (link_info.dynamic_list)
//    lang_finalize_version_expr_head (&link_info.dynamic_list->head);
// 
//  current_target = default_target;
// 
//  /* Open the output file.  */
//  lang_for_each_statement (ldlang_open_output);
//  init_opb ();
// 
//  ldemul_create_output_section_statements ();
// 
//  /* Add to the hash table all undefineds on the command line.  */
//  lang_place_undefineds ();
// 
//  if (!bfd_section_already_linked_table_init ())
//    einfo (_("%P%F: Failed to create hash table\n"));
// 
//  /* Create a bfd for each input file.  */
//  current_target = default_target;
//  lang_statement_iteration++;
//  open_input_bfds (statement_list.head, OPEN_BFD_NORMAL);
//  open_input_bfds (lang_statement_union_type *s, enum open_bfd_mode mode)
//  for (; s != NULL; s = s->header.next)
//  case lang_input_statement_enum:
//  if (!load_symbols (&s->input_statement, &add))
//  load_symbols (lang_input_statement_type *entry, lang_statement_list_type *place)
//  ldfile_open_file (entry);
//  ldfile_open_file (lang_input_statement_type *entry)
//  if (ldfile_try_open_bfd (entry->filename, entry))
//  ldfile_try_open_bfd (const char *attempt, lang_input_statement_type *entry)
//  entry->the_bfd = bfd_openr (attempt, entry->target);
//  ldfile_try_open_bfd (ld____ldfile.c):   entry->the_bfd = bfd_openr (attempt, entry->target);
//  >
//  ( bfd_openr > bfd_fopen) bfd____opncls.c > _bfd_real_fopen > map_file
//  IMPORTANT: initial bdf format (abfd->format) cannot be bdf_unknown
//  find where abfd->format is determined, note abfd>xvex is x86_64_elf64_vec on a complete FILE
//  format is set in function bfd_set_format in bfd____format.c
    printf("map_file: attempting to open %s with modes %s\n", filename, mode);
    int fd;
    if ( strcmp(mode, "r") == 0 ) {
        printf("opening read only\n");
        fd = open(filename, O_RDONLY);
        if (fd < 0) {
            printf("cannot open \"%s\", returned %i\n", filename, fd);
            exit (-1);
        }
        size_t len = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, 0);
        void* mapped = mmap (NULL, 1, PROT_READ, MAP_PRIVATE, fd, 0);
        if (mapped == MAP_FAILED) {
            printf ("map failed\n");
            exit;
        } else {
            printf ("\nmap (%s) succeded with address: %p\n", filename, &mapped);
        }
        printf("attempting to fopen\n");
#if defined (HAVE_FOPEN64)
    return fopen64(filename, mode);
#else
    return fopen(filename, mode);
#endif
        printf("attempting to fmemopen\n");
        FILE* TMP = fmemopen(mapped, len, mode);
        if (TMP == NULL) {
            printf("FAILED\n");
            exit (-1);
        } else
        {
            printf("success (fd %d)\n", fileno (TMP));
        }
        return TMP;
    } else  if ( strcmp(mode, "w+") == 0 ) {
        printf("opening read-write\n");
        tmp_write=1;
        // suppresses the file creation untill write in wich we re create with real size, for now we satisfy the request with a FILE*
        
        /* since creat() returns an fd and we do not need to do anything with it (as we need to re open the file as read only or read/write accordingly) we can do this
        
        equivilant of
        
        $ touch $filename
        
        */
        close(creat(filename, NULL));
        // set file to -rw-rw-rw-
        chmod(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        printf("attempting to fopen\n");
#if defined (HAVE_FOPEN64)
    return fopen64(filename, mode);
#else
    return fopen(filename, mode);
#endif
        fd = open(filename, O_RDWR);
        if (fd < 0) {
            printf("cannot open \"%s\", returned %i\n", filename, fd);
            exit (-1);
        }
        write(fd, "PLACEHOLDER", 11);
        size_t len = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, 0);
        void* mapped = mmap (NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
        if (mapped == MAP_FAILED) {
            printf ("map failed\n");
            exit;
        } else {
            printf ("\nmap (%s) succeded with address: %p\n", filename, &mapped);
        }
        printf("attempting to fmemopen\n");
        FILE* TMP = fmemopen(mapped, len, mode);
        if (TMP == NULL) {
            printf("FAILED\n");
            exit (-1);
        } else
        {
            printf("success (fd %d)\n", fileno (TMP));
        }
        return TMP;
    } else {
        printf("unknown mode, recieved \n%s\n, testing against \"r\", got %d, testing against \"w+\", got %d\n", mode, strcmp(mode, "r"), strcmp(mode, "w+"));
//         exit (-1);
    }
}

FILE *
_bfd_real_fopen (const char *filename, const char *modes)
{
    char * target = "binary";
    int fd = -1;
    bfd *a;
    bfd *b;
    bfd *c;
    bfd *d;
    char * matchinga;
    char * matchingb;
    char * matchingc;
    char * matchingd;
    int reta;
    int retb;
    int retc;
    int retd;
    a = testA(filename, target, modes, fd, 0, reta);
    b = testA(filename, target, modes, fd, 1, retb);
    c = testB(filename, target, modes, fd, 0, retc);
    d = testB(filename, target, modes, fd, 1, retd);
        if (!bfd_check_format (a, bfd_archive)
    && !bfd_check_format_matches (a, bfd_object, &matchinga))
    {
        fprintf(stderr, "    a: (fopen) no match (must match for file \"%s\" to be mapped into memory)\n", filename);
        reta = -1;
    } else {
        fprintf(stderr, "    a: (fopen) match (must match for file \"%s\" to be mapped into memory)\n", filename);
        reta = 5;
    }
    if (!bfd_check_format (b, bfd_archive)
    && !bfd_check_format_matches (b, bfd_object, &matchingb))
    {
        fprintf(stderr, "    b: (map) no match (must match for file \"%s\" to be mapped into memory)\n", filename);
        retb = -1;
    } else {
        fprintf(stderr, "    b: (map) match (must match for file \"%s\" to be mapped into memory)\n", filename);
        retb = 5;
    }
    if (!bfd_check_format (c, bfd_archive)
    && !bfd_check_format_matches (c, bfd_object, &matchingc))
    {
        fprintf(stderr, "    c: (fopen) no match (must match for file \"%s\" to be mapped into memory)\n", filename);
        retc = -1;
    } else {
        fprintf(stderr, "    c: (fopen) match (must match for file \"%s\" to be mapped into memory)\n", filename);
        retc = 5;
    }
    if (!bfd_check_format (d, bfd_archive)
    && !bfd_check_format_matches (d, bfd_object, &matchingd))
    {
        fprintf(stderr, "    d: (map) no match (must match for file \"%s\" to be mapped into memory)\n", filename);
        retc = -1;
    } else {
        fprintf(stderr, "    d: (map) match (must match for file \"%s\" to be mapped into memory)\n", filename);
        retd = 5;
    }
    if ( reta == 5 && retb == 5 && retc == 5 && retd == 5) {
        fprintf(stderr, "using mmap\n");
        return map_file_TMP(filename, modes, 1);
    } else {
        fprintf(stderr, "cannot map %s, using fopen instead\n", filename);
        return map_file_TMP(filename, modes, 0);
  }
#ifdef VMS
  char *vms_attr;

  /* On VMS, fopen allows file attributes as optional arguments.
     We need to use them but we'd better to use the common prototype.
     In fopen-vms.h, they are separated from the mode with a comma.
     Split here.  */
  vms_attr = strchr (modes, ',');
  if (vms_attr == NULL)
    {
      /* No attributes.  */
      printf("_bfd_real_fopen: *VMS  1 /* On VMS, fopen allows file attributes as optional arguments. We need to use them but we'd better to use the common prototype. In fopen-vms.h, they are separated from the mode with a comma. Split here.  */* : opening \n%s\nwith modes:\n%s\n", filename, modes);
      return close_on_exec (map_file (filename, modes));
    }
  else
    {
      /* Attributes found.  Split.  */
      size_t modes_len = strlen (modes) + 1;
      char attrs[modes_len + 1];
      char *at[3];
      int i;

      memcpy (attrs, modes, modes_len);
      at[0] = attrs;
      for (i = 0; i < 2; i++)
	{
	  at[i + 1] = strchr (at[i], ',');
	  BFD_ASSERT (at[i + 1] != NULL);
	  *(at[i + 1]++) = 0; /* Replace ',' with a nul, and skip it.  */
	}
      printf("_bfd_real_fopen: *VMS  2 /* On VMS, fopen allows file attributes as optional arguments. We need to use them but we'd better to use the common prototype. In fopen-vms.h, they are separated from the mode with a comma. Split here.  */* : opening \n%s\nwith modes:\n%s\n", filename, modes);
      return close_on_exec (fopen (filename, at[0], at[1], at[2]));
    }
#else /* !VMS */
#if defined (HAVE_FOPEN64)
printf("_bfd_real_fopen: opening \n%s\nwith modes:\n%s\n", filename, modes);
return close_on_exec (map_file (filename, modes));
#else
printf("_bfd_real_fopen: opening \n%s\nwith modes:\n%s\n", filename, modes);
return close_on_exec (map_file (filename, modes));
#endif
#endif /* !VMS */
}

/*
INTERNAL_DEFINITION
	struct bfd_iovec

DESCRIPTION

	The <<struct bfd_iovec>> contains the internal file I/O class.
	Each <<BFD>> has an instance of this class and all file I/O is
	routed through it (it is assumed that the instance implements
	all methods listed below).

.struct bfd_iovec
.{
.  {* To avoid problems with macros, a "b" rather than "f"
.     prefix is prepended to each method name.  *}
.  {* Attempt to read/write NBYTES on ABFD's IOSTREAM storing/fetching
.     bytes starting at PTR.  Return the number of bytes actually
.     transfered (a read past end-of-file returns less than NBYTES),
.     or -1 (setting <<bfd_error>>) if an error occurs.  *}
.  file_ptr (*bread) (struct bfd *abfd, void *ptr, file_ptr nbytes);
.  file_ptr (*bwrite) (struct bfd *abfd, const void *ptr,
.		       file_ptr nbytes);
.  {* Return the current IOSTREAM file offset, or -1 (setting <<bfd_error>>
.     if an error occurs.  *}
.  file_ptr (*btell) (struct bfd *abfd);
.  {* For the following, on successful completion a value of 0 is returned.
.     Otherwise, a value of -1 is returned (and <<bfd_error>> is set).  *}
.  int (*bseek) (struct bfd *abfd, file_ptr offset, int whence);
.  int (*bclose) (struct bfd *abfd);
.  int (*bflush) (struct bfd *abfd);
.  int (*bstat) (struct bfd *abfd, struct stat *sb);
.  {* Mmap a part of the files. ADDR, LEN, PROT, FLAGS and OFFSET are the usual
.     mmap parameter, except that LEN and OFFSET do not need to be page
.     aligned.  Returns (void *)-1 on failure, mmapped address on success.
.     Also write in MAP_ADDR the address of the page aligned buffer and in
.     MAP_LEN the size mapped (a page multiple).  Use unmap with MAP_ADDR and
.     MAP_LEN to unmap.  *}
.  void *(*bmmap) (struct bfd *abfd, void *addr, bfd_size_type len,
.		   int prot, int flags, file_ptr offset,
.		   void **map_addr, bfd_size_type *map_len);
.};

.extern const struct bfd_iovec _bfd_memory_iovec;

*/


/* Return value is amount read.  */

bfd_size_type
bfd_bread (void *ptr, bfd_size_type size, bfd *abfd)
{
//     printf("bfd_bread: attempting to read\n");
  size_t nread;

  /* If this is an archive element, don't read past the end of
     this element.  */
  if (abfd->arelt_data != NULL)
    {
      bfd_size_type maxbytes = arelt_size (abfd);

      if (abfd->where + size > maxbytes)
	{
	  if (abfd->where >= maxbytes)
	    return 0;
	  size = maxbytes - abfd->where;
	}
    }

  if (abfd->iovec)
    nread = abfd->iovec->bread (abfd, ptr, size);
  else
    nread = 0;
  if (nread != (size_t) -1)
    abfd->where += nread;

  return nread;
}

bfd_size_type
bfd_bwrite (const void *ptr, bfd_size_type size, bfd *abfd)
{
//     printf("bfd_bwrite: attempting to write %zu (%d) bytes\n", size);
  size_t nwrote;

  if (abfd->iovec)
    nwrote = abfd->iovec->bwrite (abfd, ptr, size);
  else
    nwrote = 0;

  if (nwrote != (size_t) -1)
    abfd->where += nwrote;
  if (nwrote != size)
    {
#ifdef ENOSPC
      errno = ENOSPC;
#endif
      bfd_set_error (bfd_error_system_call);
    }
  return nwrote;
}

file_ptr
bfd_tell (bfd *abfd)
{
  file_ptr ptr;

  if (abfd->iovec)
    {
      bfd *parent_bfd = abfd;
      ptr = abfd->iovec->btell (abfd);

      while (parent_bfd->my_archive != NULL
	     && !bfd_is_thin_archive (parent_bfd->my_archive))
	{
	  ptr -= parent_bfd->origin;
	  parent_bfd = parent_bfd->my_archive;
	}
    }
  else
    ptr = 0;

  abfd->where = ptr;
  return ptr;
}

int
bfd_flush (bfd *abfd)
{
  if (abfd->iovec)
    return abfd->iovec->bflush (abfd);
  return 0;
}

/* Returns 0 for success, negative value for failure (in which case
   bfd_get_error can retrieve the error code).  */
int
bfd_stat (bfd *abfd, struct stat *statbuf)
{
  int result;

  if (abfd->iovec)
    result = abfd->iovec->bstat (abfd, statbuf);
  else
    result = -1;

  if (result < 0)
    bfd_set_error (bfd_error_system_call);
  return result;
}

/* Returns 0 for success, nonzero for failure (in which case bfd_get_error
   can retrieve the error code).  */

int
bfd_seek (bfd *abfd, file_ptr position, int direction)
{
//     printf ("seeking to %d with mode %d\n", position, direction);
  int result;
  file_ptr file_position;
  /* For the time being, a BFD may not seek to it's end.  The problem
     is that we don't easily have a way to recognize the end of an
     element in an archive.  */

  BFD_ASSERT (direction == SEEK_SET || direction == SEEK_CUR);

  if (direction == SEEK_CUR && position == 0)
    return 0;

  if (abfd->my_archive == NULL || bfd_is_thin_archive (abfd->my_archive))
    {
      if (direction == SEEK_SET && (bfd_vma) position == abfd->where)
	return 0;
    }
  else
    {
      /* We need something smarter to optimize access to archives.
	 Currently, anything inside an archive is read via the file
	 handle for the archive.  Which means that a bfd_seek on one
	 component affects the `current position' in the archive, as
	 well as in any other component.

	 It might be sufficient to put a spike through the cache
	 abstraction, and look to the archive for the file position,
	 but I think we should try for something cleaner.

	 In the meantime, no optimization for archives.  */
    }

  file_position = position;
  if (direction == SEEK_SET)
    {
      bfd *parent_bfd = abfd;

      while (parent_bfd->my_archive != NULL
	     && !bfd_is_thin_archive (parent_bfd->my_archive))
	{
	  file_position += parent_bfd->origin;
	  parent_bfd = parent_bfd->my_archive;
	}
    }
  if (abfd->iovec) { // all should be empty
//     printf("checking abfd->iovec->bseek\n"); //should be 0
//     printf("abfd->iovec->bseek = %d\n", abfd->iovec->bseek); //should be 0
    result = abfd->iovec->bseek (abfd, file_position, direction); // errors
} else {
    result = -1;
}

  if (result != 0)
    {
//       printf ("abfd->iovec->bseek is INVALID\n");
      pause();
      int hold_errno = errno;

      /* Force redetermination of `where' field.  */
      bfd_tell (abfd);

      /* An EINVAL error probably means that the file offset was
	 absurd.  */
      if (hold_errno == EINVAL)
	bfd_set_error (bfd_error_file_truncated);
      else
	{
	  bfd_set_error (bfd_error_system_call);
	  errno = hold_errno;
	}
    }
  else
    {
//       printf ("abfd->iovec->bseek is VALID\n");
      /* Adjust `where' field.  */
      if (direction == SEEK_SET)
	abfd->where = position;
      else
	abfd->where += position;
    }
  return result;
}

/*
FUNCTION
	bfd_get_mtime

SYNOPSIS
	long bfd_get_mtime (bfd *abfd);

DESCRIPTION
	Return the file modification time (as read from the file system, or
	from the archive header for archive members).

*/

long
bfd_get_mtime (bfd *abfd)
{
  struct stat buf;

  if (abfd->mtime_set)
    return abfd->mtime;

  if (abfd->iovec == NULL)
    return 0;

  if (abfd->iovec->bstat (abfd, &buf) != 0)
    return 0;

  abfd->mtime = buf.st_mtime;		/* Save value in case anyone wants it */
  return buf.st_mtime;
}

/*
FUNCTION
	bfd_get_size

SYNOPSIS
	ufile_ptr bfd_get_size (bfd *abfd);

DESCRIPTION
	Return the file size (as read from file system) for the file
	associated with BFD @var{abfd}.

	The initial motivation for, and use of, this routine is not
	so we can get the exact size of the object the BFD applies to, since
	that might not be generally possible (archive members for example).
	It would be ideal if someone could eventually modify
	it so that such results were guaranteed.

	Instead, we want to ask questions like "is this NNN byte sized
	object I'm about to try read from file offset YYY reasonable?"
	As as example of where we might do this, some object formats
	use string tables for which the first <<sizeof (long)>> bytes of the
	table contain the size of the table itself, including the size bytes.
	If an application tries to read what it thinks is one of these
	string tables, without some way to validate the size, and for
	some reason the size is wrong (byte swapping error, wrong location
	for the string table, etc.), the only clue is likely to be a read
	error when it tries to read the table, or a "virtual memory
	exhausted" error when it tries to allocate 15 bazillon bytes
	of space for the 15 bazillon byte table it is about to read.
	This function at least allows us to answer the question, "is the
	size reasonable?".
*/

ufile_ptr
bfd_get_size (bfd *abfd)
{
  struct stat buf;

  if (abfd->iovec == NULL)
    return 0;

  if (abfd->iovec->bstat (abfd, &buf) != 0)
    return 0;

  return buf.st_size;
}

/*
FUNCTION
	bfd_get_file_size

SYNOPSIS
	ufile_ptr bfd_get_file_size (bfd *abfd);

DESCRIPTION
	Return the file size (as read from file system) for the file
	associated with BFD @var{abfd}.  It supports both normal files
	and archive elements.

*/

ufile_ptr
bfd_get_file_size (bfd *abfd)
{
  if (abfd->my_archive != NULL
      && !bfd_is_thin_archive (abfd->my_archive))
    return arelt_size (abfd);

  return bfd_get_size (abfd);
}

/*
FUNCTION
	bfd_mmap

SYNOPSIS
	void *bfd_mmap (bfd *abfd, void *addr, bfd_size_type len,
			int prot, int flags, file_ptr offset,
			void **map_addr, bfd_size_type *map_len);

DESCRIPTION
	Return mmap()ed region of the file, if possible and implemented.
	LEN and OFFSET do not need to be page aligned.  The page aligned
	address and length are written to MAP_ADDR and MAP_LEN.

*/

void *
bfd_mmap (bfd *abfd, void *addr, bfd_size_type len,
	  int prot, int flags, file_ptr offset,
	  void **map_addr, bfd_size_type *map_len)
{
  printf("bfd_mmap\n");
  void *ret = (void *)-1;

  if (abfd->iovec == NULL)
    return ret;

  return abfd->iovec->bmmap (abfd, addr, len, prot, flags, offset,
			     map_addr, map_len);
}

/* Memory file I/O operations.  */

static file_ptr
memory_bread (bfd *abfd, void *ptr, file_ptr size)
{
  printf("memory_bread\n");
  struct bfd_in_memory *bim;
  bfd_size_type get;

  bim = (struct bfd_in_memory *) abfd->iostream;
  get = size;
  if (abfd->where + get > bim->size)
    {
      if (bim->size < (bfd_size_type) abfd->where)
	get = 0;
      else
	get = bim->size - abfd->where;
      bfd_set_error (bfd_error_file_truncated);
    }
  memcpy (ptr, bim->buffer + abfd->where, (size_t) get);
  return get;
}

static file_ptr
memory_bwrite (bfd *abfd, const void *ptr, file_ptr size)
{
  printf("memory_bwrite\n");
  struct bfd_in_memory *bim = (struct bfd_in_memory *) abfd->iostream;

  if (abfd->where + size > bim->size)
    {
      bfd_size_type newsize, oldsize;

      oldsize = (bim->size + 127) & ~(bfd_size_type) 127;
      bim->size = abfd->where + size;
      /* Round up to cut down on memory fragmentation */
      newsize = (bim->size + 127) & ~(bfd_size_type) 127;
      if (newsize > oldsize)
	{
	  bim->buffer = (bfd_byte *) bfd_realloc_or_free (bim->buffer, newsize);
	  if (bim->buffer == NULL)
	    {
	      bim->size = 0;
	      return 0;
	    }
	  if (newsize > bim->size)
	    memset (bim->buffer + bim->size, 0, newsize - bim->size);
	}
    }
  memcpy (bim->buffer + abfd->where, ptr, (size_t) size);
  return size;
}

static file_ptr
memory_btell (bfd *abfd)
{
  printf("memory_btell\n");
  return abfd->where;
}

static int
memory_bseek (bfd *abfd, file_ptr position, int direction)
{
  printf("memory_bseek\n");
  file_ptr nwhere;
  struct bfd_in_memory *bim;

  bim = (struct bfd_in_memory *) abfd->iostream;

  if (direction == SEEK_SET)
    nwhere = position;
  else
    nwhere = abfd->where + position;

  if (nwhere < 0)
    {
      abfd->where = 0;
      errno = EINVAL;
      return -1;
    }

  if ((bfd_size_type)nwhere > bim->size)
    {
      if (abfd->direction == write_direction
	  || abfd->direction == both_direction)
	{
	  bfd_size_type newsize, oldsize;

	  oldsize = (bim->size + 127) & ~(bfd_size_type) 127;
	  bim->size = nwhere;
	  /* Round up to cut down on memory fragmentation */
	  newsize = (bim->size + 127) & ~(bfd_size_type) 127;
	  if (newsize > oldsize)
	    {
	      bim->buffer = (bfd_byte *) bfd_realloc_or_free (bim->buffer, newsize);
	      if (bim->buffer == NULL)
		{
		  errno = EINVAL;
		  bim->size = 0;
		  return -1;
		}
	      memset (bim->buffer + oldsize, 0, newsize - oldsize);
	    }
	}
      else
	{
	  abfd->where = bim->size;
	  errno = EINVAL;
	  bfd_set_error (bfd_error_file_truncated);
	  return -1;
	}
    }
  return 0;
}

static int
memory_bclose (struct bfd *abfd)
{
  printf("memory_bclose\n");
  struct bfd_in_memory *bim = (struct bfd_in_memory *) abfd->iostream;

  if (bim->buffer != NULL)
    free (bim->buffer);
  free (bim);
  abfd->iostream = NULL;

  return 0;
}

static int
memory_bflush (bfd *abfd ATTRIBUTE_UNUSED)
{
  printf("memory_bflush\n");
  return 0;
}

static int
memory_bstat (bfd *abfd, struct stat *statbuf)
{
  printf("memory_bstat\n");
  struct bfd_in_memory *bim = (struct bfd_in_memory *) abfd->iostream;

  memset (statbuf, 0, sizeof (*statbuf));
  statbuf->st_size = bim->size;

  return 0;
}

static void *
memory_bmmap (bfd *abfd ATTRIBUTE_UNUSED, void *addr ATTRIBUTE_UNUSED,
	      bfd_size_type len ATTRIBUTE_UNUSED, int prot ATTRIBUTE_UNUSED,
	      int flags ATTRIBUTE_UNUSED, file_ptr offset ATTRIBUTE_UNUSED,
	      void **map_addr ATTRIBUTE_UNUSED,
	      bfd_size_type *map_len ATTRIBUTE_UNUSED)
{
  printf("memory_bmmap\n");
  return (void *)-1;
}

const struct bfd_iovec _bfd_memory_iovec =
{
  &memory_bread, &memory_bwrite, &memory_btell, &memory_bseek,
  &memory_bclose, &memory_bflush, &memory_bstat, &memory_bmmap
};
