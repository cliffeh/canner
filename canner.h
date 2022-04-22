#ifndef _CANNER_H
#define _CANNER_H 1

#include <limits.h>

typedef struct canner_callback
{
  char name[PATH_MAX];
  char path[PATH_MAX];
} canner_callback;

#endif
