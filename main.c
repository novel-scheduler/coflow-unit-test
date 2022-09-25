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
  eo->toEqualNumberValueOf = toEqualNumberValueOf;
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

// TODO:
void Test_Promotecoflows()
{
  // TODO: initialize fq_sched_data + arr[] 
  // ? what is unsigned arr[] used for? what array is this? 
  // ? I need to know the arguments of Promotecoflows() to test it
  
  // struct fq_flow f;
  // struct fq_flow g;
  // struct fq_flow h;
  // struct fq_flow i;
  // struct fq_flow j;
  // struct fq_flow coflow;
  // struct fq_flow_head oldFlowshead;
  // struct fq_flow_head newFlowshead;
  // struct fq_flow_head coFlowshead;

  // f.sk = 1;
  // g.sk = 2;
  // h.sk = 3;
  // i.sk = 4;
  // j.sk = 5;
  // coflow.sk = 6;
  // coFlowshead = q->co_flows;
  // oldFlowshead = q->old_flows;
  // newFlowshead = q->new_flows;

  // struct fq_flow_head *coFlowsheadptr = &(q->co_flows);
  // struct fq_flow_head *newFlowsheadptr = &(q->new_flows);
  // struct fq_flow_head *oldFlowsheadptr = &(q->co_flows);

  // f.socket_hash = f.sk + 10;
  // g.socket_hash = g.sk + 10;
  // i.socket_hash = i.sk + 10;
  // h.socket_hash = h.sk + 10;
  // j.socket_hash = j.sk + 10;

  // coflow.socket_hash = coflow.sk + 10;

  // fq_flow_add_tail(&q->new_flows, &f);
  // fq_flow_add_tail(&q->new_flows, &g);
  // fq_flow_add_tail(&q->new_flows, &j);
  // fq_flow_add_tail(&q->old_flows, &h);
  // fq_flow_add_tail(&q->old_flows, &i);
}

void Test_valuePresentInArray()
{
  u32 val1 = 3;
  u32 val2 = 7;
  u32 arr[] = {1, 2, 3, 4, 5};
  int res1 = valuePresentInArray(val1, arr, 5);
  int res2 = valuePresentInArray(val2, arr, 5);

  totalTestsNumber = 2;

  expect(res1)->toEqualNumberValueOf(2);
  expect(res2)->toEqualNumberValueOf(-1);
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

  // Free Memory
  freeFlowList(flowList1Head->first);
  free(flow2);
  free(flowList1Head);
  freeFlowList(flowList3Head->first);
  free(flow4);
  free(flowList3Head);
}

/**
 * Main Testing Program.
 */
int main()
{
  printTestingSessionStartText();

  // *** TEST SUITES ***

  // before();
  // Test_Sample();
  // after();

  // before();
  // Test_fq_flow_add_tail();
  // after();

  // before();
  // Test_valuePresentInArray();
  // after();

  printTestingSessionResult();
  printTestingSessionEndText();

  return 0;
}
