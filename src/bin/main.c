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

/* Global variables */
gchar *db_path;

void cleanup()
{
  if (db_path)
    g_free(db_path);
}

int main(int argc, char **argv)
{
  /* Register cleanup function */
  atexit(cleanup);

  if (argc < 2)
    {
      fprintf (stderr, "Needs a command argument. Please refer to help using: \"dfym help\"\n");
      exit(EINVAL);
    }
  /* help command */
  else if (!strcmp("help", argv[1]))
    {
      printf("Usage: dfym [command] [flags] [arguments...]\n"
             "\n"
             "Commands:\n"
             "tag [tag] [file]          add tag to file or directory\n"
             "untag [tag] [file]        remove tag from file or directory\n"
             "show [file]               show the tags of a file directory\n"
             "tags                      show all defined tags\n"
             "tagged                    show tagged resources\n"
             "search [tag]              search for files or directories that match this tag\n"
             "                            flags:\n"
             "                            -f show only files\n"
             "                            -d show only directories\n"
             "                            -nX show only the first X occurences of the query\n"
             "                            -r randomize order of results\n"
             "discover                  list files or directories in a directory that are haven't been tagged\n"
             "                            flags:\n"
             "                              -f show only files\n"
             "                              -d show only directories\n"
             "                              -nX show only the first X occurences of the query\n"
             "                              -r randomize order of results\n"
             "rename [file] [file]      rename files or directories\n"
             "rename-tag [tag]          rename a tag\n"
             "\n"
            );
      exit(0);
    }

  /* Database preparation */
  struct passwd *pw = getpwuid(getuid());
  char *homedir = pw->pw_dir;
  db_path = g_strconcat(homedir, "/.dfym.db", NULL);

  sqlite3 *db = NULL;

  db = dfym_open_or_create_database (db_path);

  /* TAG command */
  if (!strcmp("tag", argv[1]))
    {
      if (argc != 4)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit(EINVAL);
        }
      else
        {
          const char *tag = argv[2];
          const char *argument_path = argv[3];
          char path[PATH_MAX];
          if (realpath(argument_path, path))
            {
              if (dfym_add_tag (db, tag, path) != DFYM_OK)
                {
                  fprintf (stderr, "Database error\n");
                  exit(1);
                }
            }
          else
            {
              if (errno == ENOENT)
                {
                  fprintf (stderr, "File doesn't exist\n");
                  exit(ENOENT);
                }
              else
                {
                  fprintf (stderr, "Unknown error\n");
                  exit(1);
                }
            }
        }
    }
  /* UNTAG command */
  else if (!strcmp("untag", argv[1]))
    {
      if (argc != 4)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit(EINVAL);
        }
      else
        {
          const char *tag = argv[2];
          const char *argument_path = argv[3];
          char path[PATH_MAX];
          if (realpath(argument_path, path))
            {
              if (dfym_remove_tag (db, tag, path) != DFYM_OK)
                {
                  fprintf (stderr, "Database error\n");
                  exit(1);
                }
            }
          else
            {
              if (errno == ENOENT)
                {
                  fprintf (stderr, "File doesn't exist\n");
                  exit(ENOENT);
                }
              else
                {
                  fprintf (stderr, "Unknown error\n");
                  exit(1);
                }
            }
        }
    }
  /* SHOW command */
  else if (!strcmp("show", argv[1]))
    {
      if (argc != 3)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit(EINVAL);
        }
      else
        {
          const char *argument_path = argv[2];
          char path[PATH_MAX];
          if (realpath(argument_path, path))
            {
              switch ( dfym_show_file_tags (db, path))
                {
                case DFYM_OK:
                  break;
                case DFYM_NOT_EXISTS:
                  exit(1);
                default:
                  fprintf (stderr, "Database error\n");
                  exit(192);
                }
            }
          else
            {
              switch (errno)
                {
                case ENOENT:
                  fprintf (stderr, "File doesn't exist\n");
                  exit(ENOENT);
                default:
                  fprintf (stderr, "Unknown error\n");
                  exit(1);
                }
            }
        }
    }
  /* TAGS command */
  else if (!strcmp("tags", argv[1]))
    {
      /* dfym_all_tags(db); */
    }
  /* TAGGED command */
  else if (!strcmp("tagged", argv[1]))
    {
      printf("TAGGED\n");
    }
  /* SEARCH command */
  else if (!strcmp("search", argv[1]))
    {
      /* printf("SEARCH\n");
      int opt;
      char flags = 0;
      char *number_value_flag = NULL;
      |+ Command flags +|
      while ((opt = getopt(argc-1, argv+1, "rn:fd")) != -1)
        {
          switch (opt)
            {
            case 'r':
              flags |= OPT_RANDOM;
              printf("R\n");
              break;
            case 'n':
              number_value_flag = optarg;
              printf("N\n");
              break;
            case 'f':
              flags |= OPT_FILES;
              printf("F\n");
              break;
            case 'd':
              flags |= OPT_DIRECTORIES;
              printf("D\n");
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
              exit(EINVAL);
              break;
            default:
              abort();
            }
        }
      if ((argc - optind - 1) != 1)
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit(EINVAL);
        }
      else
        {
          unsigned long int number_flag = 0;
          if (number_value_flag)
            number_flag = atoi(number_value_flag);
          dfym_search_with_tag(db, argv[optind], number_flag, flags);
        } */
    }
  /* discover command */
  else if (!strcmp("discover", argv[1]))
    {
      printf("DISCOVER\n");
    }
  /* rename command */
  else if (!strcmp("rename", argv[1]))
    {
      printf("RENAME\n");
    }
  /* rename-tag command */
  else if (!strcmp("rename-tag", argv[1]))
    {
      printf("RENAME-TAG\n");
    }
  else
    {
      printf("Wrong command. Please try \"dfym help\"\n");
      exit(EINVAL);
    }

  return 0;
}

