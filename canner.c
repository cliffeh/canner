#include "config.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <unistd.h>

#define USAGE_ARGS "DIR [HEADERS] [-h|--help]"

static const char *main_template[] = {
#include "main.h"
  0,
};

// in callbacks.c
struct callback_entry
{
  char name[PATH_MAX];
  char path[PATH_MAX];
  TAILQ_ENTRY (callback_entry) entries;
};
extern TAILQ_HEAD (, callback_entry) callbacks_head;
void generate_callbacks (FILE *out, const char *rootdir, const char *relpath);

int
main (int argc, const char *argv[])
{
  struct callback_entry *cb;
  char path[PATH_MAX] = { 0 }, *prefix = "";
  int i;
  // TODO make this a parameter
  FILE *c_out = stdout;

  if (argc < 2)
    {
      fprintf (stderr, "usage: %s " USAGE_ARGS "\n", argv[0]);
      exit (1);
    }

  for (i = 1; i < argc; i++)
    {
      if (strcmp ("--help", argv[i]) == 0 || strcmp ("-h", argv[i]) == 0)
        {
          fprintf (stdout, "usage: %s " USAGE_ARGS "\n", argv[0]);
          exit (0);
        }
    }

  sprintf (path, "%s", argv[1]);
  // ensure a single trailing slash
  while (path[strlen (path) - 1] == '/')
    {
      path[strlen (path) - 1] = 0;
    }
  path[strlen (path)] = '/';

  // include any extra headers
  for (i = 2; i < argc; i++)
    {
      switch (argv[i][0])
        {
        case '<':
        case '"':
          fprintf (c_out, "#include %s\n", argv[i]);
          break;
        default:
          fprintf (c_out, "#include \"%s\"\n", argv[i]);
        }
    }

  fprintf (c_out, "%s\n", main_template[0]);

  TAILQ_INIT (&callbacks_head);
  generate_callbacks (c_out, path, prefix);

  fprintf (c_out,
           "void canner_register_static_callbacks (struct evhttp *http) {\n");

  while ((cb = TAILQ_FIRST (&callbacks_head)))
    {
      TAILQ_REMOVE (&callbacks_head, cb, entries);
      fprintf (c_out, "  evhttp_set_cb (http, \"%s\", %s, 0);\n", cb->path,
               cb->name);
      free (cb);
    }
  fprintf (c_out, "}\n");

  return 0;
}
