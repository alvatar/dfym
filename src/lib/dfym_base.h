/** \file
  * dfym: Library functions using a SQLite3 backend */

#define CALL_SQLITE(f)                                          \
    {                                                           \
        int i;                                                  \
        i = sqlite3_ ## f;                                      \
        if (i != SQLITE_OK) {                                   \
            fprintf (stderr, "%s failed with status %d: %s\n",  \
                     #f, i, sqlite3_errmsg (db));               \
            exit (1);                                           \
        }                                                       \
    }                                                           \
 
#define CALL_SQLITE_EXPECT(f,x)                                 \
    {                                                           \
        int i;                                                  \
        i = sqlite3_ ## f;                                      \
        if (i != SQLITE_ ## x) {                                \
            fprintf (stderr, "%s failed with status %d: %s\n",  \
                     #f, i, sqlite3_errmsg (db));               \
            exit (1);                                           \
        }                                                       \
    }                                                           \
 
/** Return status used for database queries */
typedef enum
{
  DFYM_OK,                 /**< Everything OK */
  DFYM_NOT_EXISTS,         /**< Database doesn't find any result */
  DFYM_DATABASE_ERROR      /**< Database error */
} dfym_status_t;

/** Option codes for database quering */
typedef enum
{
  OPT_FILES = 1 << 0,          /**< Select files */
  OPT_DIRECTORIES = 1 << 1,    /**< Select directories */
  OPT_RANDOM = 1 << 2          /**< Return results in random order */
} query_flag_t;

sqlite3 *dfym_open_or_create_database(char *const);

int dfym_add_tag(sqlite3 *, char const *const, char const *const);

int dfym_untag(sqlite3 *, char const *const, char const *const);

int dfym_show_file_tags(sqlite3 *, char const *const);

int dfym_all_tags(sqlite3 *);

int dfym_all_files(sqlite3 *);

int dfym_search_with_tag(sqlite3 *, char const *const, unsigned long int, unsigned char);

int dfym_discover_untagged(sqlite3 *, char const *const, unsigned long int, unsigned char);

int dfym_rename_file(sqlite3 *db, char const *const, char const *const);

int dfym_rename_tag(sqlite3 *db, char const *const, char const *const);

int dfym_delete_file(sqlite3 *db, char const *const);

int dfym_delete_tag(sqlite3 *db, char const *const);

