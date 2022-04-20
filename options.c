#include "config.h"
#include <popt.h>
#include <stdlib.h> /* for exit() */

int
parse_options (int argc, const char *argv[])
{
  poptContext optCon;

  struct poptOption optionsTable[]
      = { /* longName, shortName, argInfo, arg, val, descrip, argDescript */
          { "version", 'V', POPT_ARG_NONE, /* .arg = */ 0, 'V',
            "show version information and exit", /* .argDescrip = */ 0 },
          POPT_AUTOHELP POPT_TABLEEND
        };

  optCon = poptGetContext (NULL, argc, argv, optionsTable, 0);

  int rc;
  while ((rc = poptGetNextOpt (optCon)) > 0)
    {
      switch (rc)
        {
        case 'V':
          printf (PACKAGE_STRING "\n");
          exit (0);
        }
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

  return 0;
}