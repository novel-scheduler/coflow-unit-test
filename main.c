#include <stdio.h>
#include <stdlib.h>
// #include "./Test/TestUtilities.h"
#include "Util/testing.h"

void before(char *testSuiteName)
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

  printTestingSuiteStartText(testSuiteName);
}

void after()
{
  // free eo memory
  free(eo);

  printTestingSuiteEndText();
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

// TODO: use toHaveSocketHashAtIndexOf()
void Test_Promotecoflows()
{
  // dummy parameters
  struct fq_flow *dummyCoflow = malloc(sizeof(struct fq_flow));
  struct fq_flow *dummyFlow = malloc(sizeof(struct fq_flow));

  struct fq_sched_data *q = malloc(sizeof(struct fq_sched_data));

  struct fq_flow *f = malloc(sizeof(struct fq_flow));
  struct fq_flow *g = malloc(sizeof(struct fq_flow));
  struct fq_flow *h = malloc(sizeof(struct fq_flow));
  struct fq_flow *i = malloc(sizeof(struct fq_flow));
  struct fq_flow *j = malloc(sizeof(struct fq_flow));

  struct fq_flow_head *oldFlowshead = malloc(sizeof(struct fq_flow_head));
  struct fq_flow_head *newFlowshead = malloc(sizeof(struct fq_flow_head));
  struct fq_flow_head *coFlowshead = malloc(sizeof(struct fq_flow_head));

  // socket hash is a file descriptor
  f->sk = 1;
  g->sk = 2;
  h->sk = 3;
  i->sk = 4;
  j->sk = 5;

  // random unqiue numbers
  f->socket_hash = (u32)f->sk + 10;
  g->socket_hash = (u32)g->sk + 10;
  i->socket_hash = (u32)i->sk + 10;
  h->socket_hash = (u32)h->sk + 10;
  j->socket_hash = (u32)j->sk + 10;

  // populate new flows list
  fq_flow_add_tail(newFlowshead, f);
  fq_flow_add_tail(newFlowshead, g);
  fq_flow_add_tail(newFlowshead, j);

  // populate old flows list
  fq_flow_add_tail(oldFlowshead, h);
  fq_flow_add_tail(oldFlowshead, i);

  // create dummy data to match against resulting coflows list
  struct fq_flow_head *coFlowsheadDummy = malloc(sizeof(struct fq_flow_head));
  struct fq_flow *dummyFlowF = malloc(sizeof(struct fq_flow));
  dummyFlowF->sk = 1;
  dummyFlowF->socket_hash = (u32)dummyFlowF->sk + 10;
  struct fq_flow *dummyFlowG = malloc(sizeof(struct fq_flow));
  dummyFlowG->sk = 2;
  dummyFlowG->socket_hash = (u32)dummyFlowG->sk + 10;
  struct fq_flow *dummyFlowH = malloc(sizeof(struct fq_flow));
  dummyFlowH->sk = 3;
  dummyFlowH->socket_hash = (u32)dummyFlowH->sk + 10;
  fq_flow_add_tail(coFlowsheadDummy, dummyFlowF);
  fq_flow_add_tail(coFlowsheadDummy, dummyFlowG);
  fq_flow_add_tail(coFlowsheadDummy, dummyFlowH);

  // initialize arr: 2 from new + 1 from old
  // used for identifying whether a flow belongs to a coflow set
  unsigned arr[] = {f->socket_hash, g->socket_hash, h->socket_hash};
  int lengthOfarray = 3;

  // 1 on success, 0 on fail
  int successCode = Promotecoflows(oldFlowshead, newFlowshead, coFlowshead, dummyFlow, dummyCoflow, arr, lengthOfarray);

  // check lengths of flow lists
  expect((void *)coFlowshead->first)->toHaveLengthOf(3);
  expect((void *)newFlowshead->first)->toHaveLengthOf(1);
  expect((void *)oldFlowshead->first)->toHaveLengthOf(1);

  // check equality of lists using socket hash
  expect((void *)coFlowshead->first)->toEqualFlowListOf(coFlowsheadDummy->first);

  // check success code returned from function
  expect((void *)successCode)->toEqualNumberValueOf(1);
}

void Test_valuePresentInArray()
{
  u32 val1 = 3;
  u32 val2 = 7;
  u32 arr[] = {1, 2, 3, 4, 5};
  int res1 = valuePresentInArray(val1, arr, 5);
  int res2 = valuePresentInArray(val2, arr, 5);

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

void Test_fq_enqueue()
{
  struct sk_buff *skb = malloc(sizeof(struct sk_buff));
  struct Qdisc *sch = malloc(sizeof(struct Qdisc));
  struct sk_buff *to_free = malloc(sizeof(struct sk_buff));

  // TODO:HEON - ERROR on rb_entry macro function
  fq_enqueue(skb, sch, to_free);
}

/**
 * Main Testing Program.
 */
int main()
{
  printTestingSessionStartText();

  // *** TEST SUITES ***

  // before("Test_Sample");
  // Test_Sample();
  // after();

  // before("Test_fq_flow_add_tail");
  // Test_fq_flow_add_tail();
  // after();

  // before("Test_valuePresentInArray");
  // Test_valuePresentInArray();
  // after();

  // before("Test_Promotecoflows");
  // Test_Promotecoflows();
  // after();

  before("Test_fq_enqueue");
  Test_fq_enqueue();
  after();

  printTestingSessionResult();
  printTestingSessionEndText();

  return 0;
}
