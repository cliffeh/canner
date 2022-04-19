#include <assert.h>
#define NROLLS 1024
#define NSIDES 6

int roll_dice (int result[], int rolls, int sides);

int
main ()
{
  int results[NROLLS], i;
  // NROLLSd6
  roll_dice (results, NROLLS, NSIDES);
  for (i = 0; i < NROLLS; i++)
    {
        assert(1 <= results[i]);
        assert(results[i] <= NSIDES);
        // TODO test for fair distribution
    }
}
