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

#define INCLUDE_COUNT 3
const char *includes[INCLUDE_COUNT]
    = { "event2/buffer.h", "event2/event.h", "event2/http.h" };

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
  out[i] = 0;

  // ensure it's unique
  for (j = 0; j < callback_count; j++)
    {
      if (strcmp (out, cbs[j].name) == 0)
        {
          out[i++] = '_';
          j = 0; // start over from the beginning
        }
    }
}

void
print_callback (const char *filename, const char *path)
{
  FILE *fp = fopen (path, "r");
  char c;
  if (!fp)
    {
      fprintf (stderr, "error: couldn't open %s\n", filename);
      return;
    }

  generate_callback_name (cbs[callback_count].name, filename);

  printf ("\nvoid %s_cb (struct evhttp_request *req, void *arg)\n"
          "{\n"
          "  struct evbuffer *buf = evbuffer_new ();\n"
          "  const char *html = (const char *)arg;\n"
          "  evbuffer_add_printf (buf,\n",
          cbs[callback_count].name);

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

  printf (");\n"
          "  evhttp_send_reply (req, 200, \"OK\", buf);\n"
          "  evbuffer_free(buf);\n"
          "}\n");

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

  for (i = 0; i < INCLUDE_COUNT; i++)
    {
      printf ("#include <%s>\n", includes[i]);
    }
  printf ("\n");

  generate_callbacks (argv[1]);
  printf ("\n");

  printf ("void register_html_callbacks (struct evhttp *http) {\n");
  for (i = 0; i < callback_count; i++)
    {
      printf ("  evhttp_set_cb (http, \"%s\", %s_cb, 0);\n", cbs[i].path,
              cbs[i].name);
    }
  printf ("}\n");

  return 0;
}