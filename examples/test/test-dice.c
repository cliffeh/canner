#include <assert.h>
#include <stdio.h>

#define NROLLS 4096
#define NSIDES 6

int roll_dice (int result[], int rolls, int sides);

int
main ()
{
  int rolls[NROLLS] = { 0 }, i, results[NSIDES] = { 0 };
  // NROLLSd6
  roll_dice (rolls, NROLLS, NSIDES);
  for (i = 0; i < NROLLS; i++)
    {
      assert (1 <= rolls[i]);
      assert (rolls[i] <= NSIDES);
      results[rolls[i] - 1]++;
    }

  for (i = 0; i < NSIDES; i++)
    {
      // TODO check for fairness
      assert (rolls[i] > 0);
      printf ("%i: %i\n", i + 1, results[i]);
    }
}
