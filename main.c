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

static const char *site_h_template[] = {
#include "site.h"
  0,
};

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
extern void generate_callbacks (FILE *out, const char *rootdir,
                                const char *relpath);

#ifndef DEFAULT_HEADER_NAME
#define DEFAULT_HEADER_NAME "site.h"
#endif

static char path[PATH_MAX] = { 0 };
static char *c_out_filename = "-", *h_out_filename;
static FILE *c_out, *h_out;

static int
parse_options (int argc, const char *argv[])
{
  int rc;
  const char *dirname;

  poptContext optCon;
  struct poptOption options[]
      = { /* longName, shortName, argInfo, arg, val, descrip, argDescript */
          { "outfile", 'o', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT,
            &c_out_filename, 'o', "specify output file name", "FILE" },
          { "header", 'H', POPT_ARG_STRING | POPT_ARGFLAG_OPTIONAL,
            &h_out_filename, 'H',
            "also generate a C header file (filename defaults to "
            "\"" DEFAULT_HEADER_NAME "\" if FILE not specified)",
            "FILE" },
          POPT_AUTOHELP POPT_TABLEEND
        };

  optCon = poptGetContext (0, argc, argv, options, 0);
  poptSetOtherOptionHelp (optCon, "[OPTION...] DIR\nOptions:");

  while ((rc = poptGetNextOpt (optCon)) > 0)
    {
      switch (rc)
        {
        case 'H':
          {
            if (!h_out_filename)
              h_out_filename = DEFAULT_HEADER_NAME;
          }
          break;
        }
    }

  if (rc != -1)
    {
      fprintf (stderr, "error: %s: %s\n",
               poptBadOption (optCon, POPT_BADOPTION_NOALIAS),
               poptStrerror (rc));
      poptPrintHelp (optCon, stderr, 0);
      poptFreeContext (optCon);
      return 1;
    }

  dirname = poptGetArg (optCon);
  if (!dirname || poptGetArg (optCon))
    {
      // TODO allow multiple directories to be passed in?
      fprintf (stderr, "error: exactly one directory name must be provided\n");
      poptPrintHelp (optCon, stderr, 0);
      poptFreeContext (optCon);
      return 1;
    }

  snprintf (path, sizeof (path), "%s", dirname);
  // ensure a single trailing slash
  while (path[strlen (path) - 1] == '/')
    {
      path[strlen (path) - 1] = 0;
    }
  path[strlen (path)] = '/';

  c_out = (!c_out_filename || (strcmp (c_out_filename, "-") == 0))
              ? stdout
              : fopen (c_out_filename, "w");
  if (!c_out)
    {
      fprintf (stderr, "error: unable to open output file: %s (%s)\n",
               c_out_filename, strerror (errno));
      return 1;
    }

  if (h_out_filename)
    {
      if (!(h_out = fopen (h_out_filename, "w")))
        {
          fprintf (stderr, "error: unable to open header file: %s (%s)\n",
                   h_out_filename, strerror (errno));
          return 1;
        }
    }

  return 0;
}

int
main (int argc, const char *argv[])
{
  struct callback_entry *cb;
  char *prefix = "";
  const char *dir;
  int i, rc;

  if (parse_options (argc, argv) != 0)
    exit (1);

  if (h_out) // we're meant to be writing a separate header file
    {
      fprintf (h_out, "%s\n", site_h_template[0]);
      fprintf (c_out, "#include \"%s\"\n\n", h_out_filename);
    }
  else // we'll just put all the header stuff directly into our C file
    {
      fprintf (c_out, "%s\n", site_h_template[0]);
    }

  fprintf (c_out, "%s\n", main_template[0]);

  TAILQ_INIT (&callbacks_head);
  generate_callbacks (c_out, path, prefix);

  fprintf (c_out,
           "void canner_register_static_callbacks (struct evhttp *http)\n{\n");

  while ((cb = TAILQ_FIRST (&callbacks_head)))
    {
      TAILQ_REMOVE (&callbacks_head, cb, entries);
      fprintf (c_out, "  evhttp_set_cb (http, \"%s\", %s, 0);\n", cb->path,
               cb->name);
      free (cb);
    }
  fprintf (c_out, "}\n");

  // cleanup
  fclose (c_out);
  if (h_out)
    fclose (h_out);

  return 0;
}
