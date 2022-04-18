#include <stdlib.h>

int
roll_dice (int result[], int rolls, int sides)
{
  int i;
  for (i = 0; i < rolls; i++)
    {
      result[i] = rand () % sides + 1;
    }
  return i;
}
