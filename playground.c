#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#include <math.h>

// #define _POSIX_C_SOURCE 199309L

int main()
{

  printf("Timestamp: %ld\n", (uint64_t)time(NULL));
  printf("Timestamp: %d\n", (int)time(NULL));

  return 0;
}