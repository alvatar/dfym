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

typedef enum {TAG, UNTAG, SHOW, SEARCH, DISCOVER, RENAME, RENAME_TAG } command_t;

sqlite3 *dfym_open_or_create_database(char *db_path)
{
  sqlite3 *db = NULL;
  char *exec_error_msg = NULL;

  CALL_SQLITE (open(db_path, &db));

  CALL_SQLITE_EXPECT (exec(db,
                           "create table if not exists tags("
                           "id       integer primary key,"
                           "name     text               not null"
                           ")",
                           NULL, 0, &exec_error_msg), OK);
  CALL_SQLITE_EXPECT (exec(db,
                           "create table if not exists files("
                           "id       integer primary key,"
                           "name     text               not null"
                           ")",
                           NULL, 0, &exec_error_msg), OK);
  CALL_SQLITE_EXPECT (exec(db,
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

// TODO: const
int dfym_add_tag(sqlite3 *db, char *tag, char *file)
{
  char *sql = NULL;
  /* char *exec_error_msg = NULL; */
  sqlite3_stmt *stmt = NULL;

  /* Insert tag */
  sql = "INSERT INTO tags ( name ) VALUES ( ? )";

  CALL_SQLITE (prepare_v2(db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, tag, strlen (tag), 0));
  CALL_SQLITE_EXPECT (step(stmt), DONE);
  sqlite3_int64 tag_id = sqlite3_last_insert_rowid(db);

  /* Insert file */
  sql = "INSERT INTO files ( name ) VALUES ( ? )";

  CALL_SQLITE (prepare_v2(db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, file, strlen (file), 0));
  CALL_SQLITE_EXPECT (step(stmt), DONE);
  sqlite3_int64 file_id = sqlite3_last_insert_rowid(db);

  /* Insert tagging relation */
  sql = "INSERT INTO taggings ( tag_id, file_id ) VALUES ( ?1, ?2 )";

  CALL_SQLITE (prepare_v2(db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_int (stmt, 1, tag_id));
  CALL_SQLITE (bind_int (stmt, 2, file_id));
  CALL_SQLITE_EXPECT (step(stmt), DONE);

  return 0;
}

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
      exit(0);
    }
  else
    {
      fprintf (stderr, "Wrong command. Please refer to help using: \"dfym help\"\n");
      exit(EINVAL);
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


  /* Database */
  struct passwd *pw = getpwuid(getuid());
  char *homedir = pw->pw_dir;
  gchar *db_path = g_strconcat(homedir, "/.dfym.db", NULL);

  sqlite3 *db = NULL;
  /*
  char *sql = NULL;
  char *exec_error_msg = NULL;
  sqlite3_stmt *stmt = NULL;
  */

  /* Locate first argument */
  do
    optind++;
  while (strncmp(argv[optind], "-", 1) == 0);

  /* Command logic */
  switch (command)
    {
    case TAG:
      db = dfym_open_or_create_database(db_path);
      dfym_add_tag( db, argv[optind], argv[optind+1] );
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

