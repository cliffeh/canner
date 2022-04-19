#include <dirent.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#if (__STDC_VERSION__ >= 199901L)
#include <stdint.h>
#endif

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

// https://creativeandcritical.net/str-replace-c
static char *
repl_str (const char *str, const char *from, const char *to)
{

  /* Adjust each of the below values to suit your needs. */

  /* Increment positions cache size initially by this number. */
  size_t cache_sz_inc = 16;
  /* Thereafter, each time capacity needs to be increased,
   * multiply the increment by this factor. */
  const size_t cache_sz_inc_factor = 3;
  /* But never increment capacity by more than this number. */
  const size_t cache_sz_inc_max = 1048576;

  char *pret, *ret = NULL;
  const char *pstr2, *pstr = str;
  size_t i, count = 0;
#if (__STDC_VERSION__ >= 199901L)
  uintptr_t *pos_cache_tmp, *pos_cache = NULL;
#else
  ptrdiff_t *pos_cache_tmp, *pos_cache = NULL;
#endif
  size_t cache_sz = 0;
  size_t cpylen, orglen, retlen, tolen, fromlen = strlen (from);

  /* Find all matches and cache their positions. */
  while ((pstr2 = strstr (pstr, from)) != NULL)
    {
      count++;

      /* Increase the cache size when necessary. */
      if (cache_sz < count)
        {
          cache_sz += cache_sz_inc;
          pos_cache_tmp = realloc (pos_cache, sizeof (*pos_cache) * cache_sz);
          if (pos_cache_tmp == NULL)
            {
              goto end_repl_str;
            }
          else
            pos_cache = pos_cache_tmp;
          cache_sz_inc *= cache_sz_inc_factor;
          if (cache_sz_inc > cache_sz_inc_max)
            {
              cache_sz_inc = cache_sz_inc_max;
            }
        }

      pos_cache[count - 1] = pstr2 - str;
      pstr = pstr2 + fromlen;
    }

  orglen = pstr - str + strlen (pstr);

  /* Allocate memory for the post-replacement string. */
  if (count > 0)
    {
      tolen = strlen (to);
      retlen = orglen + (tolen - fromlen) * count;
    }
  else
    retlen = orglen;
  ret = malloc (retlen + 1);
  if (ret == NULL)
    {
      goto end_repl_str;
    }

  if (count == 0)
    {
      /* If no matches, then just duplicate the string. */
      strcpy (ret, str);
    }
  else
    {
      /* Otherwise, duplicate the string whilst performing
       * the replacements using the position cache. */
      pret = ret;
      memcpy (pret, str, pos_cache[0]);
      pret += pos_cache[0];
      for (i = 0; i < count; i++)
        {
          memcpy (pret, to, tolen);
          pret += tolen;
          pstr = str + pos_cache[i] + fromlen;
          cpylen = (i == count - 1 ? orglen : pos_cache[i + 1]) - pos_cache[i]
                   - fromlen;
          memcpy (pret, pstr, cpylen);
          pret += cpylen;
        }
      ret[retlen] = '\0';
    }

end_repl_str:
  /* Free the cache and return the post-replacement string,
   * which will be NULL in the event of an error. */
  free (pos_cache);
  return ret;
}

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

  printf ("// %s\n", filename);
  printf ("#define %s_CONTENT \\\n", cbname);
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
          printf ("\\n\" \\\n\"");
          break;
        default:
          printf ("%c", c);
        }
    }
  printf ("\"\n");

  // TODO there are more efficient ways to do this
  tmp1 = repl_str (http_cb_template[0], "CALLBACK_NAME", cbname);
  tmp2 = repl_str (tmp1, "MIME_TYPE", guess_content_type (filename));

  printf ("%s\n", tmp2);

  free (tmp1);
  free (tmp2);

  fclose (fp);

  return 1;
}

static void
generate_callbacks (const char *rootdir, const char *relpath)
{
  DIR *dir;
  struct dirent *entry;
  char fullpath[PATH_MAX], subpath[PATH_MAX], filename[PATH_MAX + 256];

  // rootdir will always have a trailing slash;
  // relpath will never have either a leading or a trailing slash
  sprintf (fullpath, "%s%s", rootdir, relpath);

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
          generate_callbacks (rootdir, subpath);
        }
      else
        {
          generate_callback_name (cbs[callback_count].name, subpath);
          if (print_callback (cbs[callback_count].name, filename))
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
}

int
main (int argc, char *argv[])
{
  char static_path[PATH_MAX], *prefix = "", site_h_path[PATH_MAX];
  FILE *fp;
  int i;
  if (argc != 2)
    {
      fprintf (stderr, "usage: %s DIR\n", argv[0]);
      exit (1);
    }

  sprintf (site_h_path, "%s/site.h", argv[1]);

  if (access (site_h_path, R_OK) == 0)
    {
      fprintf (stderr, "%s exists; including it...\n", site_h_path);
      printf("#include \"site.h\"\n");
    }
  else
    {
      fprintf (stderr, "%s doesn't exist or isn't readable; skipping it...\n",
               site_h_path);
    }

  sprintf (static_path, "%s", argv[1]);
  strcat (static_path, "/static/");

  printf ("%s\n", main_template[0]);

  generate_callbacks (static_path, prefix);

  printf ("void canner_register_callbacks (struct evhttp *http) {\n");
  for (i = 0; i < callback_count; i++)
    {
      printf ("  evhttp_set_cb (http, \"%s\", %s, 0);\n", cbs[i].path,
              cbs[i].name);
    }
  printf ("}\n");

  return 0;
}