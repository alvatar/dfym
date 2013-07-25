#include <stdio.h>
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

int main(int argc, char **argv)
{
  /* Read directory files */
  /*
  GDir *dir;
  GError *error;
  const gchar *filename;

  dir = g_dir_open(argv[1], 0, &error);
  while ( (filename = g_dir_read_name(dir)) )
    {
      printf("%s\n", filename);
    }
    */

  /* Commands */
  if (!strcmp("tag", argv[1]))
    {
      printf("ADD TAG\n");
    }
  else if (!strcmp("search", argv[1]))
    {
      printf("SEARCH WITH TAGS\n");
    }
  else if (!strcmp("show", argv[1]))
    {
      printf("SHOW TAGS ASSIGNED TO A FILE OR DIRECTORY\n");
    }
  else if (!strcmp("discover", argv[1]))
    {
      printf("GET UNTAGGED CONTENTS OF A DIRECTORY\n");
    }
  else if (!strcmp("rename", argv[1]))
    {
      printf("RENAME FILE OR DIRECTORY\n");
    }
  else
    {
      printf("Please provide a command\n");
    }

  /* Argument parsing */

  int opt;
  int random_flag = 0;
  char *number_value = NULL;
  int filter_files_flag = 0;
  int filter_directories_flag = 0;

  while ( (opt = getopt(argc, argv, "rn:fd")) != -1)
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

  printf("Flags: r=%i, n=%s, f=%i, d=%i\n",
         random_flag,
         number_value,
         filter_files_flag,
         filter_directories_flag);

  /* Home dir */
  struct passwd *pw = getpwuid(getuid());
  char *homedir = pw->pw_dir;

  /* Database */
  sqlite3 *db;
  char *sql;
  sqlite3_stmt *stmt;
  char *exec_error_msg = NULL;

  gchar *db_path = g_strconcat(homedir, "/.dfym.db", NULL);
  CALL_SQLITE(open(db_path, &db));

  /* Create tables if not found */
  sql = "SELECT name FROM sqlite_master WHERE type='table' AND name=?";
  CALL_SQLITE( prepare_v2(db, sql, strlen (sql) + 1, &stmt, NULL) );
  CALL_SQLITE (bind_text (stmt, 1, "tags", strlen("tags"), SQLITE_STATIC));
  if (!(sqlite3_step(stmt)==SQLITE_ROW))
    {
      /* No tags table found: create */
      /* Table: tags
       * name: text
       * */
      sql = "create table tags(name text);";
      CALL_SQLITE_EXPECT( exec(db, sql, NULL, 0, &exec_error_msg), OK);
    }
  else
    {
      const unsigned char *text = sqlite3_column_text(stmt, 0);
      printf("table name: %s\n", text);
    }

  //CALL_SQLITE_EXPECT (step (stmt), DONE);


  free(db_path);
  return 0;
}

