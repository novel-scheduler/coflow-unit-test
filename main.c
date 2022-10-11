#include "./Lib/build_bug.h"
#include "./Lib/hash.h"
#include <stdio.h>
#include <stdlib.h>
#include "./Util/testing.h"

void before(char *testSuiteName)
{
  // initialize expect object
  eo = (struct expectObj *)malloc(sizeof(struct expectObj));

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
  struct fq_flow *flow = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  flow->socket_hash = 123;
  flow->next = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  flow->next->socket_hash = 456;
  flow->next->next = (struct fq_flow *)malloc(sizeof(struct fq_flow));
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
  struct fq_flow *dummyCoflow = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  struct fq_flow *dummyFlow = (struct fq_flow *)malloc(sizeof(struct fq_flow));

  struct fq_sched_data *q = (struct fq_sched_data *)malloc(sizeof(struct fq_sched_data));

  struct fq_flow *f = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  struct fq_flow *g = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  struct fq_flow *h = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  struct fq_flow *i = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  struct fq_flow *j = (struct fq_flow *)malloc(sizeof(struct fq_flow));

  struct fq_flow_head *oldFlowshead = (struct fq_flow_head *)malloc(sizeof(struct fq_flow_head));
  struct fq_flow_head *newFlowshead = (struct fq_flow_head *)malloc(sizeof(struct fq_flow_head));
  struct fq_flow_head *coFlowshead = (struct fq_flow_head *)malloc(sizeof(struct fq_flow_head));

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
  struct fq_flow_head *coFlowsheadDummy = (struct fq_flow_head *)malloc(sizeof(struct fq_flow_head));
  struct fq_flow *dummyFlowF = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  dummyFlowF->sk = 1;
  dummyFlowF->socket_hash = (u32)dummyFlowF->sk + 10;
  struct fq_flow *dummyFlowG = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  dummyFlowG->sk = 2;
  dummyFlowG->socket_hash = (u32)dummyFlowG->sk + 10;
  struct fq_flow *dummyFlowH = (struct fq_flow *)malloc(sizeof(struct fq_flow));
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

  expect((void *)res1)->toEqualNumberValueOf(2);
  expect((void *)res2)->toEqualNumberValueOf(-1);
}

void Test_fq_flow_add_tail()
{
  // Some dummy data configurations
  struct fq_flow_head *flowList1Head = (struct fq_flow_head *)malloc(sizeof(struct fq_flow_head));
  struct fq_flow *flow1 = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  flow1->socket_hash = 123;
  flowList1Head->first = flow1;
  flowList1Head->last = flow1;

  eo->dataObjType = FlowList;

  struct fq_flow *flow1Add = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  flow1Add->socket_hash = 456;

  struct fq_flow *flow2 = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  flow2->socket_hash = 123;
  flow2->next = malloc(sizeof(struct fq_flow));
  flow2->next->socket_hash = 456;

  fq_flow_add_tail(flowList1Head, flow1Add);

  // Tests
  expect((void *)flowList1Head->first)->toHaveLengthOf(2);
  expect((void *)flowList1Head->first)->toEqualFlowListOf(flow2);

  // printEntireLinkedList(flow1);
  // printEntireLinkedList(flow2);

  struct fq_flow_head *flowList3Head = (struct fq_flow_head *)malloc(sizeof(struct fq_flow_head));
  struct fq_flow *flow3Add = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  flow3Add->socket_hash = 123;

  struct fq_flow *flow4 = (struct fq_flow *)malloc(sizeof(struct fq_flow));
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
  // dummy packet 1
  struct sk_buff *skb1 = (struct sk_buff *)malloc(sizeof(struct sk_buff));
  struct sock *sk_dummy1 = malloc(sizeof(struct sock));
  sk_dummy1->sk_hash = 3;
  skb1->sk = sk_dummy1;
  skb1->tstamp = 3;

  // dummy packet 2
  struct sk_buff *skb2 = (struct sk_buff *)malloc(sizeof(struct sk_buff));
  struct sock *sk_dummy2 = malloc(sizeof(struct sock));
  sk_dummy2->sk_hash = 3; // same hash to target same rbtree as dummy packet 1 & 3
  skb2->sk = sk_dummy2;
  skb2->tstamp = 4;

  // dummy packet 3
  struct sk_buff *skb3 = (struct sk_buff *)malloc(sizeof(struct sk_buff));
  struct sock *sk_dummy3 = malloc(sizeof(struct sock));
  skb3->sk = sk_dummy1; // same sock, meaning packet sbk3 is in the same flow as skb1
  skb3->tstamp = 5;

  struct Qdisc *sch = (struct Qdisc *)malloc(sizeof(struct Qdisc));

  struct sk_buff *to_free = (struct sk_buff *)malloc(sizeof(struct sk_buff));

  struct fq_sched_data *q = fq_init();

  // ENQUEUE
  fq_enqueue(q, skb1, sch, &to_free);
  fq_enqueue(q, skb2, sch, &to_free);
  fq_enqueue(q, skb3, sch, &to_free);

  printf("* AFTER fq_enqueue\n");

  printf("\n----- AFTER Enqueue -----\n\n");

  // printf("skb->rb_node color: %u\n", skb->rbnode.__rb_parent_color);
  printf("new flows list: first exists: %d\n", q->new_flows.first != NULL);
  printf("new flows list: first flow: socket_hash: %u\n", q->new_flows.first->socket_hash);
  printf("first flow - first pkt tstamp: %d\n", q->new_flows.first->head->tstamp);
  printf("first flow - second pkt tstamp: %d\n", q->new_flows.first->head->next->tstamp);
  // * the above shows that pkts in same flow gets to the same per-flow queue (LL) and the later pkt gets appended to the sk_buff LL of the flow. (as can be seen in tstamp)
  // * since dummy pkts 1 & 3 belong to the same flow, there will only be a single addition to the new flows LL.(if you process dummy pkt 2, there will be two flows in the new flows LL at the end)

  printf("\nnew flows list: second exists: %d\n", q->new_flows.first->next != NULL);
  printf("new flows list: second flow: socket_hash: %u\n", q->new_flows.first->next->socket_hash);
  printf("second flow - first pkt tstamp: %d\n", q->new_flows.first->next->head->tstamp);

  // printf("old flows list: %d\n", q->old_flows.first != NULL);
  // printf("co flows list: %d\n", q->co_flows.first != NULL);

  struct rb_root *root = &q->fq_root[3];
  struct rb_node **p = &root->rb_node;
  struct fq_flow *f = rb_entry(*p, struct fq_flow, fq_node);
  printf("\nroot flow in fq_root[3]: sk_hash: %u\n", f->sk->sk_hash);
}

void Test2_fq_enqueue()
{
  /*
  <scenario>: (6 total pkts)
  flow f, g, h, k
  f: 2 packets: [
    {
      sk_hash: 3,
      tstamp: 1,
    },
    {
      sk_hash: 3,
      tstamp: 4,
    }
  ]
  g: 1 packet: [
    {
      sk_hash: 3,
      tstamp: 2,
    }
  ]
  h: 2 packets: [
    {
      sk_hash: 3,
      tstamp: 3,
    },
    {
      sk_hash: 3,
      tstamp: 5,
    }
  ]
  k: 1 packet: [
    {
      sk_hash: 5,
      tstamp: 6,
    }
  ]

  <purpose>:
  - testing fq_enqueue() where 3 flows (f, g, h) are in the same bucket of hash 3, and 1 flow (k) in another bucket of hash 5

  <expected result>:
  - 3 rb_nodes in rbtree in bucket of hash 3
  - 1 rb_node in rbtree in bucket of hash 5
  - 4 total flows in new flows list (LL)
  */

  // dummy packet 1
  struct sk_buff *skb1 = (struct sk_buff *)malloc(sizeof(struct sk_buff));
  struct sock *sk_dummy1 = malloc(sizeof(struct sock));
  sk_dummy1->sk_hash = 3;
  skb1->sk = sk_dummy1;
  skb1->tstamp = 3;

  // dummy packet 2
  struct sk_buff *skb2 = (struct sk_buff *)malloc(sizeof(struct sk_buff));
  struct sock *sk_dummy2 = malloc(sizeof(struct sock));
  sk_dummy2->sk_hash = 3;
  skb2->sk = sk_dummy2;
  skb2->tstamp = 4;

  // dummy packet 3
  struct sk_buff *skb3 = (struct sk_buff *)malloc(sizeof(struct sk_buff));
  struct sock *sk_dummy3 = malloc(sizeof(struct sock));
  skb3->sk = sk_dummy1;
  skb3->tstamp = 5;

  struct Qdisc *sch = (struct Qdisc *)malloc(sizeof(struct Qdisc));

  struct sk_buff *to_free = (struct sk_buff *)malloc(sizeof(struct sk_buff));

  struct fq_sched_data *q = fq_init();

  // ENQUEUE
  fq_enqueue(q, skb1, sch, &to_free);
  fq_enqueue(q, skb2, sch, &to_free);
  fq_enqueue(q, skb3, sch, &to_free);

  printf("\n----- AFTER Enqueue -----\n\n");
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
