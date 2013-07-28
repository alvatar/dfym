#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
// SQLite
#include <sqlite3.h>
// Glib
#include <glib.h>
#include <glib/gstdio.h>

#include "dfym_base.h"

typedef enum
{
  TAG,
  UNTAG,
  SHOW,
  SEARCH,
  DISCOVER,
  RENAME,
  RENAME_TAG
} command_t;

int main(int argc, char **argv)
{
  /* Commands */
  command_t command = 0;
  if (argc < 2)
    {
      fprintf (stderr, "Needs a command argument. Please refer to help using: \"dfym help\"\n");
      exit(EINVAL);
    }
  else if (!strcmp("tag", argv[1]))
    command = TAG;
  else if (!strcmp("untag", argv[1]))
    command = UNTAG;
  else if (!strcmp("show", argv[1]))
    command = SHOW;
  else if (!strcmp("search", argv[1]))
    command = SEARCH;
  else if (!strcmp("discover", argv[1]))
    command = DISCOVER;
  else if (!strcmp("rename", argv[1]))
    command = RENAME;
  else if (!strcmp("rename-tag", argv[1]))
    command = RENAME_TAG;
  else if (!strcmp("help", argv[1]))
    {
      printf("Usage: dfym [command] [flags] [arguments...]\n"
             "\n"
             "Commands:\n"
             "tag [tag] [file]      -- add tag to file or directory\n"
             "untag                 -- remove tag from file or directory\n"
             "show                  -- show the tags of a file directory\n"
             "search                -- search for files or directories that match this tag\n"
             "discover              -- list files or directories in a directory that are haven't been tagged\n"
             "rename                -- rename files or directories\n"
             "rename-tag            -- rename a tag\n"
             "\n"
             "Flags:\n"
             "-f                    -- show only files\n"
             "-d                    -- show only directories\n"
             "-nX                   -- show only the first X occurences of the query\n"
             "-r                    -- randomize order of query\n"
            );
      exit(0);
    }
  else
    {
      fprintf (stderr, "Wrong command. Please refer to help using: \"dfym help\"\n");
      exit(EINVAL);
    }

  /* Arguments */
  int opt;
  int random_flag = 0;
  char *number_value = NULL;
  int filter_files_flag = 0;
  int filter_directories_flag = 0;

  /* Flags:
   * r
   * n has a required argument
   * f
   * d
   * */
  while ((opt = getopt(argc, argv, "rn:fd")) != -1)
    {
      switch (opt)
        {
        case 'r':
          random_flag = 1;
          break;
        case 'n':
          number_value = optarg;
          break;
        case 'f':
          filter_files_flag = 1;
          break;
        case 'd':
          filter_directories_flag = 1;
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
          break;
        default:
          abort();
        }
    }

  printf("Flags: r=%i, n=%s, f=%i, d=%i\n\n",
         random_flag,
         number_value,
         filter_files_flag,
         filter_directories_flag);

  /* Database preparation */
  struct passwd *pw = getpwuid(getuid());
  char *homedir = pw->pw_dir;
  gchar *db_path = g_strconcat(homedir, "/.dfym.db", NULL);

  sqlite3 *db = NULL;

  /* Locate arguments */
  do
    optind++;
  while (strncmp(argv[optind], "-", 1) == 0);

  /* Command logic */
  switch (command)
    {
    case TAG:
      if (argc-optind == 2)
        {
          db = dfym_open_or_create_database (db_path);
          const char *tag = argv[optind];
          const char *argument_path = argv[optind + 1];
          char path[PATH_MAX];
          if (realpath(argument_path, path))
            {
              printf("PATH: %s\n", path);
              if (dfym_add_tag (db, tag, path) != DFYM_OK)
                {
                  fprintf (stderr, "Database error\n");
                  exit(-1);
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
                  exit(-1);
                }
            }
        }
      else
        {
          fprintf (stderr, "Wrong number of arguments. Please refer to help using: \"dfym help\"\n");
          exit(EINVAL);
        }
      break;
    case UNTAG:
      break;
    case SHOW:
      break;
    case SEARCH:
      break;
    case DISCOVER:
      break;
    case RENAME:
      break;
    case RENAME_TAG:
      break;
    default:
      exit(EINVAL);
    }

  g_free(db_path);
  return 0;
}

