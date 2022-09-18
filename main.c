#include <stdio.h>
#include <stdlib.h>
// #include "./Test/TestUtilities.h"
#include "Util/testing.h"

void expect_tests()
{
  // initialize expect object
  eo = malloc(sizeof(struct expectObj));

  // assign functions to function pointers
  eo->toContain = toContain;
  eo->printDataTypeString = printDataTypeString;
  eo->toBeOfDateType = toBeOfDateType;
  eo->toHaveLengthOf = toHaveLengthOf;

  struct fq_flow *flow = malloc(sizeof(struct fq_flow));
  flow->socket_hash = 123;
  flow->next = malloc(sizeof(struct fq_flow));
  flow->next->socket_hash = 456;
  flow->next->next = malloc(sizeof(struct fq_flow));
  flow->next->next->socket_hash = 789;
  eo->dataObj = (void *)flow;

  eo->dataObjType = FlowList;

  expect((void *)flow)->toContain(0);

  expect((void *)flow)->toBeOfDateType(FlowList);

  expect((void *)flow)->toHaveLengthOf(3);

  // Free alocated objects
  free(eo);
  freeFlowList(flow);
}

void Test_Test()
{
}

int main()
{
  printf("---------- <Unit Testing Session> ----------\n\n");

  expect_tests();

  printf("\n---------- <END Of Session> ----------\n");
  return 0;
}