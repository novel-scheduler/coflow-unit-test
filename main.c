#include <stdio.h>
#include <stdlib.h>
// #include "./Test/TestUtilities.h"
#include "Util/testing.h"

void before()
{
  // initialize expect object
  eo = malloc(sizeof(struct expectObj));

  // assign functions to function pointers
  eo->toBeDataTypeOf = toBeDataTypeOf;
  eo->toHaveSocketHashAtIndexOf = toHaveSocketHashAtIndexOf;
  eo->toContainSocketHashOf = toContainSocketHashOf;
  eo->toHaveLengthOf = toHaveLengthOf;
  eo->toEqualFlowListOf = toEqualFlowListOf;
}

void after()
{
  // free eo memory
  free(eo);
}

/**
 * Sample test suite.
 */
void Test_Sample()
{
  // Some dummy data configurations
  struct fq_flow *flow = malloc(sizeof(struct fq_flow));
  flow->socket_hash = 123;
  flow->next = malloc(sizeof(struct fq_flow));
  flow->next->socket_hash = 456;
  flow->next->next = malloc(sizeof(struct fq_flow));
  flow->next->next->socket_hash = 789;
  eo->dataObj = (void *)flow;
  eo->dataObjType = FlowList;

  // Set tests number
  totalTestsNumber = 5;

  // Tests
  expect((void *)flow)->toBeDataTypeOf(FlowList);
  expect((void *)flow)->toHaveSocketHashAtIndexOf(123, 1);
  expect((void *)flow)->toContainSocketHashOf(123);
  expect((void *)flow)->toHaveLengthOf(3);
  expect((void *)flow)->toEqualFlowListOf(flow);

  // print entire LL
  // printEntireLinkedList(flow);
  // printEntireLinkedList(flow->next);

  // Free alocated objects
  freeFlowList(flow);
}

void Test_Promotecoflows()
{
  // TODO:
}

void Test_fq_flow_add_tail()
{
  // Some dummy data configurations
  struct fq_flow_head *flowList1Head = malloc(sizeof(struct fq_flow_head));
  struct fq_flow *flow1 = malloc(sizeof(struct fq_flow));
  flow1->socket_hash = 123;
  flowList1Head->first = flow1;
  flowList1Head->last = flow1;

  eo->dataObjType = FlowList;

  struct fq_flow *flow1Add = malloc(sizeof(struct fq_flow));
  flow1Add->socket_hash = 456;

  struct fq_flow *flow2 = malloc(sizeof(struct fq_flow));
  flow2->socket_hash = 123;
  flow2->next = malloc(sizeof(struct fq_flow));
  flow2->next->socket_hash = 456;

  // Set tests number
  totalTestsNumber = 4;

  fq_flow_add_tail(flowList1Head, flow1Add);

  // Tests
  expect((void *)flowList1Head->first)->toHaveLengthOf(2);
  expect((void *)flowList1Head->first)->toEqualFlowListOf(flow2);

  // printEntireLinkedList(flow1);
  // printEntireLinkedList(flow2);
  
  struct fq_flow_head *flowList3Head = malloc(sizeof(struct fq_flow_head));
  struct fq_flow *flow3Add = malloc(sizeof(struct fq_flow));
  flow3Add->socket_hash = 123;
  
  struct fq_flow *flow4 = malloc(sizeof(struct fq_flow));
  flow4->socket_hash = 123;
  
  fq_flow_add_tail(flowList3Head, flow3Add);
  
  // Tests
  expect((void *)flowList3Head->first)->toHaveLengthOf(1);
  expect((void *)flowList3Head->first)->toEqualFlowListOf(flow4);
}

/**
 * Main Testing Program.
 */
int main()
{
  printTestingSessionStartText();

  // Run test suite
  // before();
  // Test_Sample();
  // after();

  before();
  Test_fq_flow_add_tail();
  after();

  printTestingSessionResult();
  printTestingSessionEndText();

  return 0;
}
