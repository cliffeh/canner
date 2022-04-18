#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// 1000d6
#define ROLLS 1000
#define SIDES 6

int roll_dice (int result[], int rolls, int sides);

int
main ()
{
  int result[ROLLS], dist[SIDES], i;

  roll_dice (result, ROLLS, SIDES);

  for (i = 0; i < ROLLS; i++)
    {
      assert (result[i] >= 1);
      assert (result[i] <= SIDES);
      dist[result[i] - 1]++;
    }

  // TODO assert something about fairness?
  for (i = 0; i < SIDES; i++)
    {
      printf ("%i: %i\n", i + 1, dist[i]);
    }
}
