/** \file
  * dfym: Library functions using a SQLite3 backend */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//SQLite
#include <sqlite3.h>
// Glib
#include <glib.h>
#include <glib/gstdio.h>

#include "dfym_base.h"

void shuffle (void **array, size_t n)
{
  srand (time (NULL));
  if (n > 1)
    {
      size_t i;
      for (i = 0; i < n - 1; i++)
        {
          size_t j = i + rand () / (RAND_MAX / (n - i) + 1);
          void *t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }
}

/**
 * Callback for sqlite3_exec that will will print all results
 */
static int callback_print_sqlite (void *NotUsed, int argc, char **argv, char **azColName)
{
  for (int i=0; i<argc; i++)
    {
      /* printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL"); */
      printf ("%s\n", argv[i] ? argv[i] : "NULL");
    }
  return 0;
}

/**
 * \addtogroup database Database query functions
 */
/**@{*/

/** Open the database if it exists, create it otherwise.
 *
 * The database will be placed in ~/.dfum.db by default. Currently, no means of
 * changing this default are provided.
 * \param db The SQLite3 database.
 * \return Error code \ref dfym_status_t.
 */
sqlite3 *dfym_open_or_create_database (char *const db_path)
{
  sqlite3 *db = NULL;
  char *sql = NULL;
  char *exec_error_msg = NULL;
  CALL_SQLITE (open (db_path, &db));
  sql =
    "CREATE TABLE IF NOT EXISTS tags("
    "id          INTEGER PRIMARY KEY, "
    "name        TEXT UNIQUE NOT NULL"
    ")";
    CALL_SQLITE_EXPECT (exec (db, sql, NULL, 0, &exec_error_msg), OK);
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  sql =
    "CREATE TABLE IF NOT EXISTS files("
    "id          INTEGER PRIMARY KEY, "
    "name        TEXT UNIQUE NOT NULL"
    ")";
  CALL_SQLITE_EXPECT (exec (db, sql, NULL, 0, &exec_error_msg), OK);
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  sql =
    "CREATE TABLE IF NOT EXISTS taggings("
    "id          INTEGER PRIMARY KEY, "
    "tag_id      INTEGER NOT NULL, "
    "file_id     INTEGER NOT NULL, "
    "CONSTRAINT UniqueTagging UNIQUE(tag_id, file_id), "
    "FOREIGN KEY(tag_id) REFERENCES tags(id), "
    "FOREIGN KEY(file_id) REFERENCES files(id)"
    ")";
  CALL_SQLITE_EXPECT (exec (db, sql, NULL, 0, &exec_error_msg), OK);
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  if (exec_error_msg)
    sqlite3_free (exec_error_msg);

  return db;
}

/** Add a tag to a file.
 * This will add the file to the database if it didn't exist.
 *
 * \param db The SQLite3 database.
 * \param tag The name of the tag.
 * \param file The full (normalized) path to the file to tag.
 * \return Error code \ref dfym_status_t.
 */
int dfym_add_tag (sqlite3 *db,
                  char const *const tag,
                  char const *const file)
{
  char *sql = NULL;
  sqlite3_stmt *stmt = NULL;
  int step;
  sqlite3_int64 tag_id = 0, file_id = 0;

  /* Insert tag if doesn't exist */
  sql = "INSERT OR IGNORE INTO tags ( name ) VALUES ( ? )";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, tag, strlen (tag), 0));
  CALL_SQLITE_EXPECT (step (stmt), DONE);
  sql = "SELECT id FROM tags WHERE name=?";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, tag, strlen (tag), 0));
  do
    {
      step = sqlite3_step (stmt);
      if (step == SQLITE_ROW)
        tag_id = sqlite3_column_int (stmt,0);
    }
  while (step != SQLITE_DONE);

  /* Insert file if doesn't exist */
  sql = "INSERT OR IGNORE INTO files ( name ) VALUES ( ? )";
#ifdef SQL_VERBOSE
  printf ("SQL: %s", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, file, strlen (file), 0));
  CALL_SQLITE_EXPECT (step (stmt), DONE);
  sql = "SELECT id FROM files WHERE name=?";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, file, strlen (file), 0));
  do
    {
      step = sqlite3_step (stmt);
      if (step == SQLITE_ROW)
        file_id = sqlite3_column_int (stmt,0);
    }
  while (step != SQLITE_DONE);

  if (!tag_id || !file_id)
    return DFYM_DATABASE_ERROR;

  /* Insert tagging relation if doesn't exist */
  sql = "INSERT OR IGNORE INTO taggings ( tag_id, file_id ) VALUES ( ?1, ?2 )";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_int (stmt, 1, tag_id));
  CALL_SQLITE (bind_int (stmt, 2, file_id));
  CALL_SQLITE_EXPECT (step (stmt), DONE);

  return DFYM_OK;
}

/** Remove a tag from a file.
 * This will remove the file if it is left without any tag.
 *
 * \param db The SQLite3 database.
 * \param tag The name of the tag.
 * \param file The full (normalized) path to a file to tag.
 * \return Error code \ref dfym_status_t.
 */
int dfym_untag (sqlite3 *db,
                char const *const tag,
                char const *const file)
{
  char *sql = NULL;
  sqlite3_stmt *stmt = NULL;
  char *exec_error_msg = NULL;

  /* Check wether the file exists in the database */
  sql = "SELECT id FROM files WHERE files.name = ?";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, file, strlen (file), 0));
  if (sqlite3_step (stmt) == SQLITE_DONE)
    return DFYM_NOT_EXISTS;

  sql =
    "DELETE "
    "FROM taggings "
    "WHERE file_id IN (SELECT files.id FROM files WHERE files.name = ?1) "
    "AND tag_id IN (SELECT tags.id FROM tags WHERE tags.name = ?2)";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, file, strlen (file), 0));
  CALL_SQLITE (bind_text (stmt, 2, tag, strlen (tag), 0));
  CALL_SQLITE_EXPECT (step (stmt), DONE);

  /* Delete any file that has no tag at all */
  sql =
    "DELETE FROM files "
    "WHERE files.id "
    "IN ("
    "  SELECT files.id "
    "  FROM files "
    "  OUTER LEFT JOIN taggings "
    "  ON (taggings.file_id = files.id) "
    "  WHERE taggings.id IS NULL)";
  CALL_SQLITE_EXPECT (exec (db, sql, NULL, 0, &exec_error_msg), OK);
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif

  return DFYM_OK;
}

/** Print all tags associated to this file.
 * \param db The SQLite3 database.
 * \param file The full (normalized) path to a file to tag.
 * \return Error code \ref dfym_status_t.
 */
int dfym_show_file_tags (sqlite3 *db,
                         char const *const file)
{
  char *sql = NULL;
  sqlite3_stmt *stmt = NULL;
  int step;

  sql = "SELECT id FROM files WHERE files.name = ?";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, file, strlen (file), 0));
  if (sqlite3_step (stmt) == SQLITE_DONE)
    return DFYM_NOT_EXISTS;

  sql =
    "SELECT t.name "
    "FROM files f "
    "JOIN taggings tgs ON (tgs.file_id = f.id) "
    "JOIN tags t ON (tgs.tag_id = t.id) "
    "WHERE f.name = ?";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, file, strlen (file), 0));
  do
    {
      step = sqlite3_step (stmt);
      if (step == SQLITE_ROW)
        printf ("%s\n", sqlite3_column_text (stmt,0));
    }
  while (step != SQLITE_DONE);

  return DFYM_OK;
}

/** Print all tags in the database.
 * \param db The SQLite3 database.
 * \return Error code \ref dfym_status_t.
 */
int dfym_all_tags (sqlite3 *db)
{
  char *exec_error_msg = NULL;
  char *sql = "SELECT name FROM tags";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE_EXPECT ( exec (db, sql, callback_print_sqlite, 0, &exec_error_msg), OK);

  return DFYM_OK;
}

/** Print all files in the database.
 *
 * \param db The SQLite3 database.
 * \return Error code \ref dfym_status_t.
 */
int dfym_all_files (sqlite3 *db)
{
  char *exec_error_msg = NULL;
  char *sql = "SELECT name FROM files";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE_EXPECT ( exec (db, sql, callback_print_sqlite, 0, &exec_error_msg), OK);

  return DFYM_OK;
}

/** Print all files that have been tagged with the given tag.
 *
 * \param db The SQLite3 database.
 * \param tag The name of the tag.
 * \param number_results Maximum number of files to print.
 * \param options An OR'ed set of flags from \ref query_flag_t.
 * \return Error code \ref dfym_status_t.
 */
int dfym_search_with_tag (sqlite3 *db,
                          char const *const tag,
                          unsigned long int number_results,
                          unsigned char options)
{
  sqlite3_stmt *stmt = NULL;
  int step;

  char *sql = NULL;
  sql = g_strconcat (
          "SELECT f.name "
          "FROM files f "
          "JOIN taggings tgs ON (tgs.file_id = f.id) "
          "JOIN tags t ON (tgs.tag_id = t.id) "
          "WHERE t.name = ?1",
          (options & OPT_RANDOM) ? " ORDER BY RANDOM()" : "",
          number_results ? " LIMIT ?2" : "",
          NULL);
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif

  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, tag, strlen (tag), 0));
  if (number_results)
    CALL_SQLITE (bind_int (stmt, 2, number_results));
  do
    {
      step = sqlite3_step (stmt);
      if (step == SQLITE_ROW)
        {
          const unsigned char *element = sqlite3_column_text (stmt,0);
          if (  ! (options & (OPT_FILES | OPT_DIRECTORIES))
                || ((options & OPT_FILES) && g_file_test ((const char *)element, G_FILE_TEST_IS_REGULAR))
                || ((options & OPT_DIRECTORIES) && g_file_test ((const char *)element, G_FILE_TEST_IS_DIR)))
            printf ("%s\n", element);
        }
    }
  while (step != SQLITE_DONE);

  g_free (sql);
  return DFYM_OK;
}

/** Print files found in the given path that haven't been tagged.
 *
 * \param db The SQLite3 database.
 * \param directory The directory to look into.
 * \param options An OR'ed set of flags from \ref query_flag_t.
 * \return Error code \ref dfym_status_t.
 */
int dfym_discover_untagged (sqlite3 *db,
                            char const *const directory,
                            unsigned long int number_results,
                            unsigned char options)
{
  GDir *dir;
  GError *error;
  const gchar *filename;
  char *sql = NULL;
  sqlite3_stmt *stmt = NULL;
  int step;
  int limit = 0;
  char path[PATH_MAX];

  sql = "SELECT name FROM files WHERE files.name = ?";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif

  dir = g_dir_open (directory, 0, &error);
  /* If the user requests randomized results, we need to get all directory contents */
  if (options & OPT_RANDOM)
    {
      GPtrArray *dir_contents = g_ptr_array_new ();
      while ((filename = g_dir_read_name (dir)))
        {
          g_ptr_array_add (dir_contents, (gpointer)filename);
        }
      shuffle (dir_contents->pdata, dir_contents->len);

      for (int i=0; i<dir_contents->len && limit<number_results; i++)
        {
          CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
          realpath (dir_contents->pdata[i], path);
          CALL_SQLITE (bind_text (stmt, 1, path, strlen (path), 0));
          step = sqlite3_step (stmt);
          if (step == SQLITE_DONE
              && (! (options & (OPT_FILES | OPT_DIRECTORIES))
                  || ((options & OPT_FILES) && g_file_test ((const char *)path, G_FILE_TEST_IS_REGULAR))
                  || ((options & OPT_DIRECTORIES) && g_file_test ((const char *)path, G_FILE_TEST_IS_DIR))))
            {
              printf ("%s\n", path);
              limit++;
            }
        }
      g_ptr_array_free (dir_contents, TRUE);
    }
  /* Otherwise, we can deal directly with SQL */
  else
    {
      while ((filename = g_dir_read_name (dir))
             && (!number_results || limit < number_results))
        {
          CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
          realpath (filename, path);
          CALL_SQLITE (bind_text (stmt, 1, path, strlen (path), 0));
          step = sqlite3_step (stmt);
          if (step == SQLITE_DONE
              && (! (options & (OPT_FILES | OPT_DIRECTORIES))
                  || ((options & OPT_FILES) && g_file_test ((const char *)path, G_FILE_TEST_IS_REGULAR))
                  || ((options & OPT_DIRECTORIES) && g_file_test ((const char *)path, G_FILE_TEST_IS_DIR))))
            {
              printf ("%s\n", path);
              limit++;
            }
        }
    }
  g_dir_close (dir);
  return DFYM_OK;
}

/** Rename a file in the database.
 *
 * \param db The SQLite3 database.
 * \param file_from The path of the file to rename.
 * \param file_to The new path of the file.
 * \return Error code \ref dfym_status_t.
 */
int dfym_rename_file (sqlite3 *db,
                      char const *const file_from,
                      char const *const file_to)
{
  char *sql = NULL;
  sqlite3_stmt *stmt = NULL;

  sql = "SELECT id FROM files WHERE files.name = ?";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, file_from, strlen (file_from), 0));
  if (sqlite3_step (stmt) == SQLITE_DONE)
    return DFYM_NOT_EXISTS;

  sql =
    "UPDATE files "
    "SET name = ?1 "
    "WHERE name = ?2";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, file_to, strlen (file_to), 0));
  CALL_SQLITE (bind_text (stmt, 2, file_from, strlen (file_from), 0));
  CALL_SQLITE_EXPECT (step (stmt), DONE);

  return DFYM_OK;
}

/** Rename a tag in the database.
 * Returns DFYM_NOT_EXISTS if the tag is not found in the database.
 *
 * \param db The SQLite3 database.
 * \param tag_from The original name of the tag.
 * \param tag_to The new name of the tag.
 * \return Error code \ref dfym_status_t.
 */
int dfym_rename_tag (sqlite3 *db,
                     char const *const tag_from,
                     char const *const tag_to)
{
  char *sql = NULL;
  sqlite3_stmt *stmt = NULL;

  sql = "SELECT id FROM tags WHERE tags.name = ?";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, tag_from, strlen (tag_from), 0));
  if (sqlite3_step (stmt) == SQLITE_DONE)
    return DFYM_NOT_EXISTS;

  sql =
    "UPDATE tags "
    "SET name = ?1 "
    "WHERE name = ?2";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, tag_to, strlen (tag_to), 0));
  CALL_SQLITE (bind_text (stmt, 2, tag_from, strlen (tag_from), 0));
  CALL_SQLITE_EXPECT (step (stmt), DONE);

  return DFYM_OK;
}

/** Remove a file from the database.
 * Returns DFYM_NOT_EXISTS if the file is not found in the database.
 *
 * \param db The SQLite3 database.
 * \param file The full (normalized) path of the file to remove.
 * \return Error code \ref dfym_status_t.
 */
int dfym_delete_file (sqlite3 *db,
                      char const *const file)
{
  char *sql = NULL;
  sqlite3_stmt *stmt = NULL;

  /* Check if file exists in the database */
  sql = "SELECT id FROM files WHERE name = ?";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, file, strlen (file), 0));
  if (sqlite3_step (stmt) == SQLITE_DONE)
    return DFYM_NOT_EXISTS;

  /* Delete any tagging including this file */
  sql =
    "DELETE "
    "FROM taggings "
    "WHERE file_id IN (SELECT files.id FROM files WHERE files.name = ?1) ";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, file, strlen (file), 0));
  CALL_SQLITE_EXPECT (step (stmt), DONE);

  /* Delete file */
  sql = "DELETE FROM files WHERE name = ?1";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, file, strlen (file), 0));
  CALL_SQLITE_EXPECT (step (stmt), DONE);

  return DFYM_OK;
}

/** Remove a tag from the database.
 * This will remove the file if it is left without any tag.
 *
 * \param db The SQLite3 database.
 * \param tag The name of the tag to remove.
 * \return Error code \ref dfym_status_t.
 */
int dfym_delete_tag (sqlite3 *db,
                     char const *const tag)
{
  char *sql = NULL;
  sqlite3_stmt *stmt = NULL;
  char *exec_error_msg = NULL;

  /* Check if tag exists in the database */
  sql = "SELECT id FROM tags WHERE name = ?";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, tag, strlen (tag), 0));
  if (sqlite3_step (stmt) == SQLITE_DONE)
    return DFYM_NOT_EXISTS;

  /* Delete any tagging including this tag */
  sql =
    "DELETE "
    "FROM taggings "
    "WHERE tag_id IN (SELECT tags.id FROM tags WHERE tags.name = ?1) ";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, tag, strlen (tag), 0));
  CALL_SQLITE_EXPECT (step (stmt), DONE);

  /* Delete any file that has no tag at all */
  sql =
    "DELETE FROM files "
    "WHERE files.id "
    "IN ("
    "  SELECT files.id "
    "  FROM files "
    "  OUTER LEFT JOIN taggings "
    "  ON (taggings.file_id = files.id) "
    "  WHERE taggings.id IS NULL)";
  CALL_SQLITE_EXPECT (exec (db, sql, NULL, 0, &exec_error_msg), OK);
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif

  /* Delete tag */
  sql = "DELETE FROM tags WHERE name = ?1";
#ifdef SQL_VERBOSE
  printf ("** SQL **\n%s\n", sql);
#endif
  CALL_SQLITE (prepare_v2 (db, sql, strlen (sql) + 1, &stmt, NULL));
  CALL_SQLITE (bind_text (stmt, 1, tag, strlen (tag), 0));
  CALL_SQLITE_EXPECT (step (stmt), DONE);

  return DFYM_OK;
}

/**@}*/

