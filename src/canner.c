#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef MAX_CALLBACKS
#define MAX_CALLBACKS 4096
#endif

#define DEFAULT_MIME_TYPE "application/octet-stream"
static const struct table_entry
{
  const char *extension;
  const char *content_type;
} mime_types[] = { { "css", "text/css" },
                   { "htm", "text/htm" },
                   { "html", "text/html" },
                   { "js", "text/javascript" },
                   { "json", "application/javascript" },
                   { "txt", "text/plain" },
                   0 };

static const char *http_cb_template[] = {
#include "http_cb.h"
  0,
};

static const char *main_template[] = {
#include "main.h"
  0,
};

static int callback_count = 0;
static struct callback
{
  char name[PATH_MAX];
  char path[PATH_MAX];
} cbs[MAX_CALLBACKS];

static const char *
guess_content_type (const char *path)
{
  char *p;
  int i;
  p = strrchr (path, '.');
  if (!p)
    return DEFAULT_MIME_TYPE;
  for (i = 0; mime_types[i].extension; i++)
    {
      if (strcmp (p + 1, mime_types[i].extension) == 0)
        return mime_types[i].content_type;
    }
  return DEFAULT_MIME_TYPE;
}

static void
generate_callback_name (char *out, const char *path)
{
  int i, j, len;
  char c;

  len = strlen (path);
  for (i = 0; i < len; i++)
    {
      switch (c = path[i])
        {
        case '.':
        case '/':
          c = '_';
        default:
          out[i] = c;
        }
    }

  // ensure it's unique
  for (j = 0; j < callback_count; j++)
    {
      if (strncmp (out, cbs[j].name, i) == 0)
        {
          out[i++] = '_';
          j = 0; // start over from the beginning
        }
    }

  // null-terminate!
  out[i] = 0;
}

static int
print_callback (const char *cbname, const char *filename)
{
  FILE *fp = fopen (filename, "r");
  char c, *tmp;
  int i, bytes_read;

  if (!fp)
    {
      fprintf (stderr, "error: couldn't open %s\n", filename);
      return 0;
    }

  fseek (fp, 0L, SEEK_END);
  bytes_read = ftell (fp);
  rewind (fp);

  printf ("// from filename: %s\n", filename);

  for (i = 0; http_cb_template[i]; i++)
    {
      // TODO there's probably a better way to handle string replacement
      if ((tmp = strstr (http_cb_template[i], "CALLBACK_NAME")))
        {
          printf ("%s%.*s", cbname, (int)(tmp - http_cb_template[i]),
                  http_cb_template[i]);
          tmp += strlen ("CALLBACK_NAME");
          printf ("%s\n", tmp);
        }
      else if ((tmp = strstr (http_cb_template[i], "\"CALLBACK_CONTENT\"")))
        {
          printf ("%.*s", (int)(tmp - http_cb_template[i]),
                  http_cb_template[i]);
          printf ("\"");
          while ((c = fgetc (fp)) != EOF)
            {
              // TODO probably need more thorough escaping...
              switch (c)
                {
                case '%':
                  printf ("%%%%");
                  break;
                case '"':
                  printf ("\\\"");
                  break;
                case '\n':
                  printf ("\\n\"\n\"");
                  break;
                default:
                  printf ("%c", c);
                }
            }
          printf ("\"");
          tmp += strlen ("\"CALLBACK_CONTENT\"");
          printf ("%s\n", tmp);
        }
      else if ((tmp = strstr (http_cb_template[i], "\"MIME_TYPE\"")))
        {
          printf ("%.*s", (int)(tmp - http_cb_template[i]),
                  http_cb_template[i]);
          printf ("\"%s\"", guess_content_type (filename));
          tmp += strlen ("\"MIME_TYPE\"");
          printf ("%s\n", tmp);
        }
      else if ((tmp = strstr (http_cb_template[i], "RESPONSE_SIZE")))
        {
          printf ("%.*s", (int)(tmp - http_cb_template[i]),
                  http_cb_template[i]);
          printf ("%i", bytes_read);
          tmp += strlen ("RESPONSE_SIZE");
          printf ("%s\n", tmp);
        }
      else
        {
          printf ("%s\n", http_cb_template[i]);
        }
    }

  fclose (fp);

  return 1;
}

static void
generate_callbacks (const char *rootdir, const char *relpath)
{
  DIR *dir;
  struct dirent *entry;
  char fullpath[PATH_MAX], subpath[PATH_MAX], filename[PATH_MAX + 256];

  sprintf (fullpath, "%s/%s", rootdir, relpath);

  if (!(dir = opendir (fullpath)))
    {
      fprintf (stderr, "warning: couldn't open %s\n", fullpath);
      return;
    }

  while ((entry = readdir (dir)) != NULL)
    {
      if (strcmp (entry->d_name, ".") == 0
          || strcmp (entry->d_name, "..") == 0)
        continue;

      sprintf (subpath, "%s/%s", relpath, entry->d_name);

      if (entry->d_type == DT_DIR)
        {
          generate_callbacks (rootdir, subpath);
        }
      else
        {
          sprintf (filename, "%s/%s", fullpath, entry->d_name);
          generate_callback_name (cbs[callback_count].name, subpath);
          if (print_callback (cbs[callback_count].name, filename))
            {
              strcpy (cbs[callback_count].path, subpath);
              callback_count++;

              // special case for index.html
              if (strcmp ("index.html", entry->d_name) == 0)
                {
                  strcpy (cbs[callback_count].name,
                          cbs[callback_count - 1].name);
                  sprintf (cbs[callback_count].path, "%s/", relpath);
                  callback_count++;

                  // prefixes without a trailing slash
                  if (strlen (relpath) > 0)
                    {
                      strcpy (cbs[callback_count].name,
                              cbs[callback_count - 1].name);
                      strcpy (cbs[callback_count].path, relpath);
                      callback_count++;
                    }
                }
            }
        }
    }
  closedir (dir);
}

int
main (int argc, char *argv[])
{
  char path[PATH_MAX], *prefix = "";
  int i;
  if (argc != 2)
    {
      fprintf (stderr, "usage: %s DIR\n", argv[0]);
      exit (1);
    }
  sprintf (path, "%s", argv[1]);

  // strip trailing slashes
  while (path[strlen (path) - 1] == '/')
    {
      path[strlen (path) - 1] = 0;
    }

  for (i = 0; main_template[i]; i++)
    {
      printf ("%s\n", main_template[i]);
    }
  printf ("\n");

  generate_callbacks (path, prefix);
  printf ("\n");

  printf ("void register_static_http_callbacks (struct evhttp *http) {\n");
  for (i = 0; i < callback_count; i++)
    {
      printf ("  evhttp_set_cb (http, \"%s\", %s, 0);\n", cbs[i].path,
              cbs[i].name);
    }
  printf ("}\n");

  return 0;
}