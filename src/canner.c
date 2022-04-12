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

// TODO maybe preamble/postamble headers?
const char *includes[] = { "event2/buffer.h",
                           "event2/event.h",
                           "event2/http.h",
                           "stdlib.h" /* for exit() */,
                           "string.h" /* for memset() */,
                           "unistd.h" /* for getopt() */,
                           0 };

const char *html_cb_template[] = {
#include "html_cb.h"
  0,
};

const char *main_template[] = {
#include "main.h"
  0,
};

struct callback
{
  char name[PATH_MAX];
  char path[PATH_MAX];
};

struct callback cbs[MAX_CALLBACKS];
int callback_count = 0;

void
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

void // TODO should probably return a value indicating success/failure
print_callback (const char *filename, const char *path)
{
  FILE *fp = fopen (path, "r");
  char c, *tmp;
  int i;

  if (!fp)
    {
      fprintf (stderr, "error: couldn't open %s\n", filename);
      return;
    }

  generate_callback_name (cbs[callback_count].name, filename);

  for (i = 0; html_cb_template[i]; i++)
    {
      if ((tmp = strstr (html_cb_template[i], "CALLBACK_NAME")))
        {
          printf ("%s%.*s", cbs[callback_count].name,
                  (int)(tmp - html_cb_template[i]), html_cb_template[i]);
          tmp += strlen ("CALLBACK_NAME");
          printf ("%s\n", tmp);
        }
      else if ((tmp = strstr (html_cb_template[i], "CALLBACK_CONTENT")))
        {
          printf ("%.*s", (int)(tmp - html_cb_template[i]),
                  html_cb_template[i]);
          printf ("\"");
          while ((c = fgetc (fp)) != EOF)
            {
              switch (c)
                {
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
          tmp += strlen ("CALLBACK_CONTENT");
          printf ("%s\n", tmp);
        }
      else
        {
          printf ("%s\n", html_cb_template[i]);
        }
    }

  fclose (fp);
}

void
generate_callbacks (const char *path)
{
  DIR *dir;
  struct dirent *entry;

  if (!(dir = opendir (path)))
    return;

  while ((entry = readdir (dir)) != NULL)
    {
      if (strcmp (entry->d_name, ".") == 0
          || strcmp (entry->d_name, "..") == 0)
        continue;

      char subpath[PATH_MAX];
      snprintf (subpath, sizeof (subpath), "%s/%s", path, entry->d_name);
      if (entry->d_type == DT_DIR)
        {
          generate_callbacks (subpath);
        }
      else
        {
          print_callback (subpath, subpath);
          strncpy (cbs[callback_count].path,
                   strchr (subpath, '/'), // strip root directory
                   sizeof (cbs[callback_count].path));
          callback_count++;
          // special case for index.html
          if (strcmp ("index.html", entry->d_name) == 0)
            {
              strncpy (cbs[callback_count].name, cbs[callback_count - 1].name,
                       sizeof (cbs[callback_count].name));
              snprintf (subpath, sizeof (subpath), "%s/", path);
              strncpy (cbs[callback_count].path,
                       strchr (subpath, '/'), // strip root directory
                       sizeof (cbs[callback_count].path));
              callback_count++;
            }
        }
    }
  closedir (dir);
}

int
main (int argc, char *argv[])
{
  int i;
  if (argc != 2)
    {
      fprintf (stderr, "usage: %s DIR\n", argv[0]);
      exit (1);
    }

  for (i = 0; includes[i]; i++)
    {
      printf ("#include <%s>\n", includes[i]);
    }
  printf ("\n");

  generate_callbacks (argv[1]);
  printf ("\n");

  printf ("void register_html_callbacks (struct evhttp *http) {\n");
  for (i = 0; i < callback_count; i++)
    {
      printf ("  evhttp_set_cb (http, \"%s\", %s, 0);\n", cbs[i].path,
              cbs[i].name);
    }
  printf ("}\n");

  for (i = 0; main_template[i]; i++)
    {
      printf ("%s\n", main_template[i]);
    }

  return 0;
}