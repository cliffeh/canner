#include "site-with-custom.h"

#include <string.h>
#include <time.h>

// in dice.c
int roll_dice (int result[], int rolls, int sides);

void
api_roll_dice (struct evhttp_request *req, void *arg)
{
  time_t rawtime = time (0);

  // default response: empty array ([] = 2 bytes)
  int rolls = 0, sides = 0, size = 2, i;

  struct evbuffer *buf = evbuffer_new ();
  evhttp_add_header (evhttp_request_get_output_headers (req), "Content-Type",
                     "application/json");

  const char *uri = evhttp_request_get_uri (req);
  const char *qs = strchr (uri, '?'), *p;

  // TODO check for malformed querystring
  // example: ?4d6 (roll a 6-sided die 4 times)
  if (qs)
    {
      rolls = atoi (qs + 1);
      p = strchr (qs, 'd');
      sides = atoi (p + 1);
    } // otherwise we'll just roll 0 dice
  size += evbuffer_add_printf (buf, "[");

  // TODO enforce a policy on the max # of rolls?
  if (rolls)
    {
      int *result = malloc(rolls*sizeof(int));
      roll_dice(result, rolls, sides);

      size += evbuffer_add_printf(buf, "%i", result[0]);
      for(i = 1; i < rolls; i++) {
        size += evbuffer_add_printf(buf, ",%i", result[i]);
      }
    }
  
  size += evbuffer_add_printf (buf, "]");

  evhttp_send_reply (req, 200, "OK", buf);

  char *client_ip;
  u_short client_port;
  evhttp_connection_get_peer (evhttp_request_get_connection (req), &client_ip,
                              &client_port);

  log_access (client_ip, &rawtime, "GET", evhttp_request_get_uri (req), 200,
              size);
  evbuffer_free (buf);
}

void
canner_register_custom_callbacks (struct evhttp *http)
{
  evhttp_set_cb (http, "/api/dice", api_roll_dice, 0);
}
