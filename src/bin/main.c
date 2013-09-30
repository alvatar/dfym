/** \file
  * dfym: Main program. Handles commands, arguments and interaction. */

#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
/* SQLite */
#include <sqlite3.h>
/* Glib */
#include <glib.h>
#include <glib/gstdio.h>

#include "dfym_base.h"

/** \page compilation Compiling the program

Dfym uses Autotools for compilation, following GNU's standards:

\code
./configure
make
make install
\endcode

If configure fails, try running:
\code
autoreconf -iv
\endcode
*/

/* Global variables */
gchar *db_path = NULL;
sqlite3 *db = NULL;


void cleanup ()
{
  if (db_path)
    g_free (db_path);
  if (db)
    sqlite3_close (db);
}

int main (int argc, char **argv)
{
  /* Register cleanup function */
  atexit (cleanup);

  if (argc < 2)
    {
      fprintf (stderr, "Needs a command argument. Please refer to help using: \"dfym help\"\n");
      exit (EXIT_FAILURE);
    }
  /* help command */
  else if (!strcmp ("help", argv[1]))
    {
      printf ("Usage: dfym [command] [flags] [arguments...]\n"
              "\n"
              "Commands:\n"
              "tag [tags...] [file]          add tag to file or directory\n"
              "untag [tags...] [file]        remove tag from file or directory\n"
              "show [file]               show the tags of a file directory\n"
              "tags                      show all defined tags\n"
              "tagged                    show tagged files\n"
              "search [tag]              search for files or directories that match this tag\n"
              "                            flags:\n"
              "                              -f show only files\n"
              "                              -d show only directories\n"
              "                              -nX show only the first X occurences of the query\n"
              "                              -r randomize order of results\n"
              "discover [directory]      list untagged files within a given directory\n"
              "                            flags:\n"
              "                              -f show only files\n"
              "                              -d show only directories\n"
              "                              -nX show only the first X occurences of the query\n"
              "                              -r randomize order of results\n"
              "rename [file] [file]      rename files or directories\n"
              "rename-tag [tag] [tag]    rename a tag\n"
              "delete [file] [file]      delete files or directories\n"
              "delete-tag [tag] [tag]    delete a tag\n"
             );
      exit (EXIT_SUCCESS);
    }

  /* Database preparation */
  struct passwd *pw = getpwuid (getuid ());
  char *homedir = pw->pw_dir;
  db_path = g_strconcat (homedir, "/.dfym.db", NULL);

  db = dfym_open_or_create_database (db_path);

  /* TAG command */
  if (!strcmp ("tag", argv[1]))
    {
      if (argc < 4)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit (EXIT_FAILURE);
        }
      else
        {
          int i;
          for (i=0; i< (argc-3); i++)
            {
              const char *tag = argv[2+i];
              const char *argument_path = argv[argc-1];
              char path[PATH_MAX];
              if (realpath (argument_path, path))
                switch (dfym_add_tag (db, tag, path))
                  {
                  case DFYM_OK:
                    break;
                  default:
                    fprintf (stderr, "Database error\n");
                    exit (EXIT_FAILURE);
                  }
              else
                switch (errno)
                  {
                  case ENOENT:
                    fprintf (stderr, "File doesn't exist\n");
                    exit (EXIT_FAILURE);
                  default:
                    fprintf (stderr, "Unknown error\n");
                    exit (EXIT_FAILURE);
                  }
            }
        }
    }
  /* UNTAG command */
  else if (!strcmp ("untag", argv[1]))
    {
      if (argc < 4)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit (EXIT_FAILURE);
        }
      else
        {
          int i;
          for (i=0; i< (argc-3); i++)
            {
              const char *tag = argv[2+i];
              const char *argument_path = argv[argc-1];
              char path[PATH_MAX];
              if (realpath (argument_path, path))
                switch (dfym_untag (db, tag, path))
                  {
                  case DFYM_OK:
                    break;
                  case DFYM_NOT_EXISTS:
                    fprintf (stderr, "File not found in the database\n");
                    exit (EXIT_FAILURE);
                  default:
                    fprintf (stderr, "Database error\n");
                    exit (EXIT_FAILURE);
                  }
              else
                switch (errno)
                  {
                  case ENOENT:
                    fprintf (stderr, "File doesn't exist\n");
                    exit (EXIT_FAILURE);
                  default:
                    fprintf (stderr, "Unknown error\n");
                    exit (EXIT_FAILURE);
                  }
            }
        }
    }
  /* SHOW command */
  else if (!strcmp ("show", argv[1]))
    {
      if (argc != 3)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit (EXIT_FAILURE);
        }
      else
        {
          const char *argument_path = argv[2];
          char path[PATH_MAX];
          if (realpath (argument_path, path))
            switch ( dfym_show_file_tags (db, path))
              {
              case DFYM_OK:
                break;
              case DFYM_NOT_EXISTS:
                fprintf (stderr, "File not found in the database\n");
                exit (EXIT_FAILURE);
              default:
                fprintf (stderr, "Database error\n");
                exit (EXIT_FAILURE);
              }
          else
            switch (errno)
              {
              case ENOENT:
                fprintf (stderr, "File doesn't exist\n");
                exit (EXIT_FAILURE);
              default:
                fprintf (stderr, "Unknown error\n");
                exit (EXIT_FAILURE);
              }
        }
    }
  /* TAGS command */
  else if (!strcmp ("tags", argv[1]))
    {
      if (argc != 2)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit (EXIT_FAILURE);
        }
      else
        switch (dfym_all_tags (db))
          {
          case DFYM_OK:
            break;
          default:
            fprintf (stderr, "Database error\n");
            exit (EXIT_FAILURE);
          }
    }
  /* TAGGED command */
  else if (!strcmp ("tagged", argv[1]))
    {
      if (argc != 2)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit (EXIT_FAILURE);
        }
      switch (dfym_all_files (db))
        {
        case DFYM_OK:
          break;
        default:
          fprintf (stderr, "Database error\n");
          exit (EXIT_FAILURE);
        }
    }
  /* SEARCH command */
  else if (!strcmp ("search", argv[1]))
    {
      int opt;
      unsigned char flags = 0;
      char *number_value_flag = NULL;
      /* Command flags */
      while ((opt = getopt (argc-1, argv+1, "rn:fd")) != -1)
        {
          switch (opt)
            {
            case 'r':
              flags |= OPT_RANDOM;
              break;
            case 'n':
              number_value_flag = optarg;
              break;
            case 'f':
              flags |= OPT_FILES;
              break;
            case 'd':
              flags |= OPT_DIRECTORIES;
              break;
            case '?':
              if (optopt == 'n')
                fprintf (stderr, "Option -n requires an argument.\n");
              else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
              else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
              exit (EXIT_FAILURE);
              break;
            default:
              abort ();
            }
        }
      optind++; /* we are looking into the command, not the executable */
      if ((argc - optind) != 1)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit (EXIT_FAILURE);
        }
      else
        {
          unsigned long int number_flag = 0;
          if (number_value_flag) number_flag = atoi (number_value_flag);
          switch (dfym_search_with_tag (db, argv[optind], number_flag, flags))
            {
            case DFYM_OK:
              break;
            default:
              fprintf (stderr, "Database error\n");
              exit (EXIT_FAILURE);
            }
        }
    }
  /* DISCOVER command */
  else if (!strcmp ("discover", argv[1]))
    {
      int opt;
      unsigned char flags = 0;
      char *number_value_flag = NULL;
      /* Command flags */
      while ((opt = getopt (argc-1, argv+1, "rn:fd")) != -1)
        {
          switch (opt)
            {
            case 'r':
              flags |= OPT_RANDOM;
              break;
            case 'n':
              number_value_flag = optarg;
              break;
            case 'f':
              flags |= OPT_FILES;
              break;
            case 'd':
              flags |= OPT_DIRECTORIES;
              break;
            case '?':
              if (optopt == 'n')
                fprintf (stderr, "Option -n requires an argument.\n");
              else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
              else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
              exit (EXIT_FAILURE);
              break;
            default:
              abort ();
            }
        }
      optind++; /* we are looking into the command, not the executable */
      if ((argc - optind) != 1)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit (EXIT_FAILURE);
        }
      else
        {
          const char *target_dir = argv[optind];
          unsigned long int number_flag = 0;
          if (number_value_flag) number_flag = atoi (number_value_flag);
          if (g_file_test (target_dir, G_FILE_TEST_IS_DIR))
            dfym_discover_untagged (db, target_dir, number_flag, flags);
          else
            {
              fprintf (stderr, "Argument is not a directory\n");
              exit (EXIT_FAILURE);
            }
        }
    }
  /* rename command */
  else if (!strcmp ("rename", argv[1]))
    {
      if (argc != 4)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit (EXIT_FAILURE);
        }
      else
        {
          const char *path_from_arg = argv[2];
          const char *path_to_arg = argv[3];
          char path_to[PATH_MAX];
          /* Path from doesn't get checked for existence, path_to does */
          if (realpath (path_to_arg, path_to))
            switch (dfym_rename_file (db, path_from_arg, path_to))
              {
              case DFYM_OK:
                break;
              case DFYM_NOT_EXISTS:
                fprintf (stderr, "File not found in the database\n");
                exit (EXIT_FAILURE);
              default:
                fprintf (stderr, "Database error\n");
                exit (EXIT_FAILURE);
              }
          else
            switch (errno)
              {
              case ENOENT:
                fprintf (stderr, "File you are trying to rename to doesn't exist\n");
                exit (EXIT_FAILURE);
              default:
                fprintf (stderr, "Unknown error\n");
                exit (EXIT_FAILURE);
              }
        }
    }
  /* rename-tag command */
  else if (!strcmp ("rename-tag", argv[1]))
    {
      if (argc != 4)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit (EXIT_FAILURE);
        }
      else
        switch (dfym_rename_tag (db, argv[2], argv[3]))
          {
          case DFYM_OK:
            break;
          case DFYM_NOT_EXISTS:
            fprintf (stderr, "Tag not found in the database\n");
            exit (EXIT_FAILURE);
          default:
            fprintf (stderr, "Database error\n");
            exit (EXIT_FAILURE);
          }
    }
  /* delete command */
  else if (!strcmp ("delete", argv[1]))
    {
      if (argc != 3)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit (EXIT_FAILURE);
        }
      else
        switch (dfym_delete_file (db, argv[2]))
          {
          case DFYM_OK:
            break;
          case DFYM_NOT_EXISTS:
            fprintf (stderr, "File not found in the database\n");
            exit (EXIT_FAILURE);
          default:
            fprintf (stderr, "Database error\n");
            exit (EXIT_FAILURE);
          }
    }
  /* delete-tag command */
  else if (!strcmp ("delete-tag", argv[1]))
    {
      if (argc != 3)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit (EXIT_FAILURE);
        }
      else
        switch (dfym_delete_tag (db, argv[2]))
          {
          case DFYM_OK:
            break;
          case DFYM_NOT_EXISTS:
            fprintf (stderr, "Tag not found in the database\n");
            exit (EXIT_FAILURE);
          default:
            fprintf (stderr, "Database error\n");
            exit (EXIT_FAILURE);
          }
    }
  else
    {
      fprintf (stderr, "Wrong command. Please try \"dfym help\"\n");
      exit (EXIT_FAILURE);
    }

  return EXIT_SUCCESS;
}

