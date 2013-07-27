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

sqlite3* open_or_create_database(char *db_path)
{
  sqlite3 *db = NULL;
  char *exec_error_msg = NULL;
  char *sql = NULL;
  sqlite3_stmt *stmt = NULL;

  CALL_SQLITE(open(db_path, &db));

  CALL_SQLITE_EXPECT( exec(db,
                           "create table if not exists tags("
                           "id       integer primary key,"
                           "name     text               not null"
                           ")",
                           NULL, 0, &exec_error_msg), OK);
  CALL_SQLITE_EXPECT( exec(db,
                           "create table if not exists files("
                           "id       integer primary key,"
                           "name     text               not null"
                           ")",
                           NULL, 0, &exec_error_msg), OK);
  CALL_SQLITE_EXPECT( exec(db,
                           "create table if not exists taggings("
                           "id          integer primary key,"
                           "tag_id      integer not null,"
                           "file_id     integer not null,"
                           "FOREIGN KEY(tag_id) REFERENCES tags(id),"
                           "FOREIGN KEY(file_id) REFERENCES files(id)"
                           ")",
                           NULL, 0, &exec_error_msg), OK);
  return db;
}

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

  if (argc < 2)
    {
      fprintf (stderr, "Needs a command argument. Please refer to help using: \"dfym help\"\n");
      exit(1);
    }

  /* Commands */
  if (!strcmp("tag", argv[1]))
    {
      printf("ADD TAG\n");
    }
  else if (!strcmp("untag", argv[1]))
    {
      printf("REMOVE A TAG FROM A FILE OR DIRECTORY\n");
    }
  else if (!strcmp("show", argv[1]))
    {
      printf("SHOW TAGS ASSIGNED TO A FILE OR DIRECTORY\n");
    }
  else if (!strcmp("search", argv[1]))
    {
      printf("SEARCH WITH TAGS\n");
    }
  else if (!strcmp("discover", argv[1]))
    {
      printf("GET UNTAGGED CONTENTS OF A DIRECTORY\n");
    }
  else if (!strcmp("rename", argv[1]))
    {
      printf("RENAME FILE OR DIRECTORY\n");
    }
  else if (!strcmp("rename", argv[1]))
    {
      printf("RENAME FILE OR DIRECTORY\n");
    }
  else if (!strcmp("help", argv[1]))
    {
      printf("Usage: dfym [command] [flags] [arguments...]\n"
             "\n"
             "Commands:\n"
             "tag         -- add tag to file or directory\n"
             "untag       -- remove tag from file or directory\n"
             "show        -- show the tags of a file directory\n"
             "search      -- search for files or directories that match this tag\n"
             "discover    -- list files or directories in a directory that are haven't been tagged\n"
             "rename      -- rename files or directories\n"
             "rename-tag  -- rename a tag\n"
             "\n"
             "Flags:\n"
             "-f          -- show only files\n"
             "-d          -- show only directories\n"
             "-nX         -- show only the first X occurences of the query\n"
             "-r          -- randomize order of query\n"
            );
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

  /* Arguments:
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

  printf("Flags: r=%i, n=%s, f=%i, d=%i\n",
         random_flag,
         number_value,
         filter_files_flag,
         filter_directories_flag);

  /* Database */
  struct passwd *pw = getpwuid(getuid());
  char *homedir = pw->pw_dir;
  gchar *db_path = g_strconcat(homedir, "/.dfym.db", NULL);

  sqlite3 *db = open_or_create_database(db_path);

  char *exec_error_msg = NULL;
  char *sql = NULL;
  sqlite3_stmt *stmt = NULL;
  sql = "INSERT INTO files ( name ) VALUES ( ? )";

  CALL_SQLITE( prepare_v2(db, sql, strlen (sql) + 1, &stmt, NULL) );
  CALL_SQLITE (bind_text (stmt,
                          1,
                          "Dvorak",
                          strlen ("Dvorak"),
                          0
                         ));
  CALL_SQLITE_EXPECT (step(stmt), DONE);
  sqlite3_int64 last_file_id = sqlite3_last_insert_rowid(db);
  printf("LAST ID: %i\n", last_file_id);


  g_free(db_path);
  return 0;
}

