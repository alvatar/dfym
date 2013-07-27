#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//SQLite
#include <sqlite3.h>

#include "dfym_base.h"

int sql_if_row(sqlite3 *db, sqlite3_stmt **in_stmt, char *sql_test, char *sql_if_true, char *sql_if_false)
{
  sqlite3_stmt *stmt = NULL;
  /* char *exec_error_msg = NULL; */
  int return_step = 0;

  CALL_SQLITE( prepare_v2(db, sql_test, strlen (sql_test) + 1, &stmt, NULL) );
  if ((sqlite3_step(stmt)==SQLITE_ROW))
    {
      /* CALL_SQLITE_EXPECT( exec(db, sql_if_true, NULL, 0, &exec_error_msg), OK); */
      if (sql_if_true)
      {
        CALL_SQLITE( prepare_v2(db, sql_if_true, strlen (sql_if_true) + 1, &stmt, NULL) );
        return_step = sqlite3_step(stmt);
#ifdef SQL_VERBOSE
        printf("** SQL **\n%s\n", sql_if_true);
#endif
      }
    }
  else
    {
      /* CALL_SQLITE_EXPECT( exec(db, sql_if_false, NULL, 0, &exec_error_msg), OK); */
      if (sql_if_false)
      {
        CALL_SQLITE( prepare_v2(db, sql_if_false, strlen (sql_if_false) + 1, &stmt, NULL) );
        return_step = sqlite3_step(stmt);
#ifdef SQL_VERBOSE
        printf("** SQL **\n%s\n", sql_if_false);
#endif
      }
    }
  *in_stmt = stmt;
  return return_step;
}

