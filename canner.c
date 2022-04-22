#include "canner.h"
#include "config.h"
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define USAGE_ARGS "DIR [HEADERS] [-h|--help]"

static FILE *c_out;

#ifndef MAX_CALLBACKS
#define MAX_CALLBACKS 4096
#endif

canner_callback cbs[MAX_CALLBACKS];

static const char *static_cb_template[] = {
#include "static_cb.h"
  0,
};

static const char *main_template[] = {
#include "main.h"
  0,
};

// in util.c
void generate_callback_name (char *out, const char *path);
const char *guess_content_type (const char *path);
char *repl_str (const char *str, const char *from, const char *to);

int
print_callback (FILE *out, const char *cbname, const char *filename)
{
  FILE *fp = fopen (filename, "r");
  char c, *tmp1, *tmp2;
  int i, bytes_read;

  if (!fp)
    {
      fprintf (stderr, "error: couldn't open %s\n", filename);
      return 0;
    }

  fseek (fp, 0L, SEEK_END);
  bytes_read = ftell (fp);
  rewind (fp);

  fprintf (out, "// %s\n", filename);
  fprintf (out, "#define %s_CONTENT \\\n", cbname);
  fprintf (out, "\"");
  while ((c = fgetc (fp)) != EOF)
    {
      // TODO probably need more thorough escaping...
      switch (c)
        {
        case '%':
          fprintf (out, "%%%%");
          break;
        case '"':
          fprintf (out, "\\\"");
          break;
        case '\n':
          fprintf (out, "\\n\" \\\n\"");
          break;
        default:
          fprintf (out, "%c", c);
        }
    }
  fprintf (out, "\"\n");

  // TODO there are more efficient ways to do this
  tmp1 = repl_str (static_cb_template[0], "CALLBACK_NAME", cbname);
  tmp2 = repl_str (tmp1, "MIME_TYPE", guess_content_type (filename));

  fprintf (out, "%s\n", tmp2);

  free (tmp1);
  free (tmp2);

  fclose (fp);

  return 1;
}

int
generate_callbacks (FILE *out, const char *rootdir, const char *relpath)
{
  DIR *dir;
  struct dirent *entry;
  char fullpath[PATH_MAX], subpath[PATH_MAX], filename[PATH_MAX + 256];
  int i, len, callback_count = 0;

  // rootdir will always have a trailing slash;
  // relpath will never have either a leading or a trailing slash
  sprintf (fullpath, "%s%s", rootdir, relpath);

  if (!(dir = opendir (fullpath)))
    {
      fprintf (stderr, "warning: couldn't open %s\n", fullpath);
      return 0;
    }

  while ((entry = readdir (dir)) != NULL)
    {
      if (strcmp (entry->d_name, ".") == 0
          || strcmp (entry->d_name, "..") == 0)
        continue;

      if (strlen (relpath) == 0)
        {
          sprintf (subpath, "%s", entry->d_name);
          sprintf (filename, "%s%s", fullpath, entry->d_name);
        }
      else
        {
          sprintf (subpath, "%s/%s", relpath, entry->d_name);
          sprintf (filename, "%s/%s", fullpath, entry->d_name);
        }

      if (entry->d_type == DT_DIR)
        {
          callback_count += generate_callbacks (out, rootdir, subpath);
        }
      else
        {
          generate_callback_name (cbs[callback_count].name, subpath);

          // ensure a unique name by appending underscores
          for (i = 0; i < callback_count; i++)
            {
              if (strcmp (cbs[callback_count].name, cbs[i].name) == 0)
                {
                  len = strlen (cbs[callback_count].name);
                  cbs[callback_count].name[len++] = '_';
                  cbs[callback_count].name[len] = 0;
                  i = 0; // start over from the beginning
                }
            }

          if (print_callback (out, cbs[callback_count].name, filename))
            {
              sprintf (cbs[callback_count].path, "/%s", subpath);
              callback_count++;

              // special case for index.html
              if (strcmp ("index.html", entry->d_name) == 0)
                {
                  strcpy (cbs[callback_count].name,
                          cbs[callback_count - 1].name);
                  sprintf (cbs[callback_count].path, "/%s", relpath);
                  callback_count++;

                  // prefixes with a trailing slash
                  if (strlen (relpath) > 0)
                    {
                      strcpy (cbs[callback_count].name,
                              cbs[callback_count - 1].name);
                      sprintf (cbs[callback_count].path, "/%s/", relpath);
                      callback_count++;
                    }
                }
            }
        }
    }
  closedir (dir);

  return callback_count;
}

int
main (int argc, const char *argv[])
{
  char path[PATH_MAX] = { 0 }, *prefix = "";
  int i, callback_count;

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

  // TODO make this a parameter
  c_out = stdout;

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

  generate_callbacks (c_out, path, prefix);

  fprintf (c_out,
           "void canner_register_static_callbacks (struct evhttp *http) {\n");
  for (i = 0; i < callback_count; i++)
    {
      fprintf (c_out, "  evhttp_set_cb (http, \"%s\", %s, 0);\n", cbs[i].path,
               cbs[i].name);
    }
  fprintf (c_out, "}\n");

  return 0;
}
