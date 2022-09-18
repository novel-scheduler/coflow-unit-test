#include <stdio.h>
#include <stdlib.h>
#include "./Test/TestUtilities.h"
#include "Util/testing.h"

void util_test()
{
  // initialize expect object
  expectObj *eo = malloc(sizeof(struct expectObj));

  // assign functions to function pointers
  eo->toContain = toContain;
  eo->printDataTypeName = printDataTypeName;

  dummy *d = malloc(sizeof(struct dummy));

  printf("FLAG 1\n");
  expect(eo, (void *)d)->toContain(eo, 0);
  printf("FLAG 2\n");
  eo->dataObjType = LinkedList;
  printf("FLAG 3\n");

  eo->printDataTypeName(eo);
}

void Test_Test()
{
}

int main()
{
  printf("---------- <Unit Testing Session> ----------\n\n");

  util_test();

  printf("\n---------- <END Of Session> ----------\n");
  return 0;
}