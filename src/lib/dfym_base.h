
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
 
typedef enum
{
  DFYM_OK,
  DFYM_NOT_EXISTS,
  DFYM_DATABASE_ERROR
} dfym_status_t;

typedef enum
{
  OPT_FILES = 1 << 0,
  OPT_DIRECTORIES = 1 << 1,
  OPT_RANDOM = 1 << 2
} query_flag_t;

sqlite3 *dfym_open_or_create_database(char *const);

int dfym_add_tag(sqlite3 *, char const *const, char const *const);

int dfym_untag(sqlite3 *, char const *const, char const *const);

int dfym_show_resource_tags(sqlite3 *, char const *const);

int dfym_all_tags(sqlite3 *);

int dfym_all_tagged(sqlite3 *);

int dfym_search_with_tag(sqlite3 *, char const *const, unsigned long int, unsigned char);

int dfym_discover_untagged(sqlite3 *, char const *const, unsigned long int, unsigned char);

int dfym_rename_resource(sqlite3 *db, char const *const, char const *const);

int dfym_rename_tag(sqlite3 *db, char const *const, char const *const);

int dfym_delete_resource(sqlite3 *db, char const *const);

int dfym_delete_tag(sqlite3 *db, char const *const);

int dfym_sql_if_row(sqlite3 *, sqlite3_stmt **, char const *const, char const *const, char const *const);

