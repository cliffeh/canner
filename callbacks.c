#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

struct callback_entry
{
  char name[PATH_MAX];
  char path[PATH_MAX];
  TAILQ_ENTRY (callback_entry) entries;
};

TAILQ_HEAD (, callback_entry) callbacks_head;

static const char *static_cb_template[] = {
#include "static_cb_template.h"
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

void
generate_callbacks (FILE *out, const char *rootdir, const char *relpath)
{
  DIR *dir;
  struct dirent *direntry;
  char fullpath[PATH_MAX], subpath[PATH_MAX], filename[PATH_MAX + 256];
  int i, len, matches;

  // rootdir will always have a trailing slash;
  // relpath will never have either a leading or a trailing slash
  sprintf (fullpath, "%s%s", rootdir, relpath);

  if (!(dir = opendir (fullpath)))
    {
      fprintf (stderr, "warning: couldn't open %s\n", fullpath);
      return;
    }

  while ((direntry = readdir (dir)) != NULL)
    {
      if (strcmp (direntry->d_name, ".") == 0
          || strcmp (direntry->d_name, "..") == 0)
        continue;

      if (strlen (relpath) == 0)
        {
          sprintf (subpath, "%s", direntry->d_name);
          sprintf (filename, "%s%s", fullpath, direntry->d_name);
        }
      else
        {
          sprintf (subpath, "%s/%s", relpath, direntry->d_name);
          sprintf (filename, "%s/%s", fullpath, direntry->d_name);
        }

      if (direntry->d_type == DT_DIR)
        {
          generate_callbacks (out, rootdir, subpath);
        }
      else
        {
          struct callback_entry *cb
              = calloc (1, sizeof (struct callback_entry)),
              *tmp;
          generate_callback_name (cb->name, subpath);

          do
            {
              matches = 0;
              // ensure a unique name by appending underscores
              TAILQ_FOREACH (tmp, &callbacks_head, entries)
              {
                if (strcmp (tmp->name, cb->name) == 0)
                  {
                    len = strlen (cb->name);
                    cb->name[len++] = '_';
                    cb->name[len] = 0;
                    // need to walk the list again and ensure our new name
                    // isn't duplicated
                    matches = 1;
                  }
              }
            }
          while (matches != 0);

          if (print_callback (out, cb->name, filename))
            {
              sprintf (cb->path, "/%s", subpath);
              TAILQ_INSERT_TAIL (&callbacks_head, cb, entries);

              // special case for index.html
              if (strcmp ("index.html", direntry->d_name) == 0)
                {
                  tmp = calloc (1, sizeof (struct callback_entry));
                  strcpy (tmp->name, cb->name);
                  sprintf (tmp->path, "/%s", relpath);
                  TAILQ_INSERT_TAIL (&callbacks_head, tmp, entries);

                  // prefixes with a trailing slash
                  if (strlen (relpath) > 0)
                    {
                      tmp = calloc (1, sizeof (struct callback_entry));
                      strcpy (tmp->name, cb->name);
                      sprintf (tmp->path, "/%s/", relpath);
                      TAILQ_INSERT_TAIL (&callbacks_head, tmp, entries);
                    }
                }
            }
        }
    }
  closedir (dir);
}