#include <stdio.h>
#include <getopt.h>
#include <string.h>

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

  /* String compare */
  if (!strcmp("tag", argv[1]))
    {
      printf("TAG\n");
    }
  else if (!strcmp("search", argv[1]))
    {
      printf("SEARCH WITH TAGS\n");
    }
  else if (!strcmp("show", argv[1]))
    {
      printf("SHOW TAGS\n");
    }
  else
    {
      printf("Please provide a command\n");
    }

  /* Argument parsing */

  int opt;
  int aflag = 0;

  while ( (opt = getopt(argc, argv, "abc:")) != -1)
    {
      switch (opt)
        {
        case 'a':
          aflag = 1;
          break;
        default:
          break;
        }
    }

  printf("Flags: a=%i\n", aflag);

  return dfym_fun();
}

