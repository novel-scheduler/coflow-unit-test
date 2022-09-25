#include <stdio.h>
#include <stdlib.h>
// #include "./Test/TestUtilities.h"
#include "Util/testing.h"

void before()
{
  // set up eo with malloc and attach function pointers
}

void after()
{
  // free eo memory
}

void expect_tests()
{
  // initialize expect object
  eo = malloc(sizeof(struct expectObj));

  // assign functions to function pointers
  eo->toContain = toContain;
  eo->printDataTypeString = printDataTypeString;
  eo->toBeOfDataType = toBeOfDataType;
  eo->toHaveLengthOf = toHaveLengthOf;

  // Some configurations
  struct fq_flow *flow = malloc(sizeof(struct fq_flow));
  flow->socket_hash = 123;
  flow->next = malloc(sizeof(struct fq_flow));
  flow->next->socket_hash = 456;
  flow->next->next = malloc(sizeof(struct fq_flow));
  flow->next->next->socket_hash = 789;
  eo->dataObj = (void *)flow;

  // Set data type
  eo->dataObjType = FlowList;

  // Set tests number
  totalTestsNumber = 3;
  
  // Tests
  expect((void *)flow)->toContain(0);
  expect((void *)flow)->toBeOfDataType(FlowList);
  expect((void *)flow)->toHaveLengthOf(3);

  // print entire LL
  printEntireLinkedList(flow);

  // Free alocated objects
  free(eo);
  freeFlowList(flow);
}

void Test_Test()
{
}

int main()
{
  printTestingSessionStartText();

  expect_tests();

  printTestingSessionEndText();
  return 0;
}