#include "config.h"
#include <errno.h>
#include <limits.h>
#include <popt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <unistd.h>

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
  const char *dir;
  int i, rc;
  FILE *c_out = stdout;

  char *c_out_name = "-";

  poptContext optCon;
  struct poptOption options[]
      = { /* longName, shortName, argInfo, arg, val, descrip, argDescript */
          { "outfile", 'o', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT,
            &c_out_name, 'o', "specify output file name", "FILE" },
          POPT_AUTOHELP POPT_TABLEEND
        };

  optCon = poptGetContext (0, argc, argv, options, 0);
  poptSetOtherOptionHelp (optCon, "[OPTION...] DIR\nOptions:");

  while ((rc = poptGetNextOpt (optCon)) > 0)
    {
    }

  if (rc != -1)
    {
      fprintf (stderr, "error: %s: %s\n",
               poptBadOption (optCon, POPT_BADOPTION_NOALIAS),
               poptStrerror (rc));
      poptPrintHelp (optCon, stderr, 0);
      poptFreeContext (optCon);
      exit (1);
    }

  dir = poptGetArg (optCon);
  if (!dir || poptGetArg (optCon))
    {
      // TODO allow multiple directories to be passed in?
      fprintf (stderr, "error: exactly one directory name must be provided\n");
      poptPrintHelp (optCon, stderr, 0);
      poptFreeContext (optCon);
      exit (1);
    }

  sprintf (path, "%s", dir);
  // ensure a single trailing slash
  while (path[strlen (path) - 1] == '/')
    {
      path[strlen (path) - 1] = 0;
    }
  path[strlen (path)] = '/';

  c_out = (strcmp (c_out_name, "-") == 0) ? stdout : fopen (c_out_name, "w");
  if (!c_out)
    {
      fprintf (stderr, "error: unable to open output file: %s (%s)\n",
               c_out_name, strerror (errno));
      exit (1);
    }

  fprintf (c_out, "%s\n", main_template[0]);

  TAILQ_INIT (&callbacks_head);
  generate_callbacks (c_out, path, prefix);

  fprintf (c_out,
           "void canner_register_static_callbacks (struct evhttp *http)\n{");

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
