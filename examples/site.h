#ifndef _CANNER_EXAMPLE_SITE_H
#define _CANNER_EXAMPLE_SITE_H 1

#include <event2/http.h>

int roll_dice (int result[], int rolls, int sides);
void api_roll_dice (struct evhttp_request *req, void *arg);

#endif
