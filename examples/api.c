#include "site.h"

#include <time.h>
#include <stdio.h> /* remove me */

#define DICE_MAX_ROLLS 20

void log_access (const char *client_ip, time_t *rawtime, const char *method,
                 const char *uri, int status, long size);

void
api_roll_dice (struct evhttp_request *req, void *arg)
{
  const char *uri = evhttp_request_get_uri (req);
  fprintf (stderr, "%s\n", uri);
  evhttp_send_reply (req, 200, "OK", 0);

  // int roll_dice (int result[], int rolls, int sides);
}

void canner_preregister_callbacks (struct evhttp *http)
{
  evhttp_set_cb (http, "/api/dice", api_roll_dice, 0);
}
