#include <stdio.h>

#include <glib.h>
#include <glib/gstdio.h>

#include "dfym_base.h"

int main(int argc, char **argv)
{
  printf("Welcome to DFYM\n");

  /* Read directory files */
  GDir *dir;
  GError *error;
  const gchar *filename;

  dir = g_dir_open(argv[1], 0, &error);
  while ( (filename = g_dir_read_name(dir)) )
    {
      printf("%s\n", filename);
    }

  return dfym_fun();
}

