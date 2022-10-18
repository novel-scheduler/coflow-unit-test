#include "./Lib/build_bug.h"
#include "./Lib/hash.h"
#include <stdio.h>
#include <stdlib.h>
#include "./Util/testing.h"

void init(struct fq_sched_data **q, struct Qdisc **sch)
{
  struct fq_sched_data *q_tobe = fq_init();

  struct Qdisc *sch_tobe = (struct Qdisc *)malloc(sizeof(struct Qdisc));
  sch_tobe->dev_queue = (struct netdev_queue *)malloc(sizeof(struct netdev_queue));
  sch_tobe->dev_queue->dev = (struct net_device *)malloc(sizeof(struct net_device));

  (*q) = q_tobe;
  (*sch) = sch_tobe;
}

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

void Test2_fq_enqueue(struct Qdisc *sch, struct fq_sched_data *q)
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

  // * flow F
  // socket of flow
  struct sock *sk_f = malloc(sizeof(struct sock));
  sk_f->sk_hash = 3;

  // first & second packet
  struct sk_buff *skb_f_1 = (struct sk_buff *)malloc(sizeof(struct sk_buff));
  struct sk_buff *skb_f_2 = (struct sk_buff *)malloc(sizeof(struct sk_buff));

  skb_f_1->sk = sk_f;
  skb_f_1->tstamp = 1;
  skb_f_2->sk = sk_f;
  skb_f_2->tstamp = 4;

  // set custom packet sizes
  skb_f_1->plen = 100000;
  skb_f_2->plen = 50000;

  // * flow G
  // socket of flow
  struct sock *sk_g = malloc(sizeof(struct sock));
  sk_g->sk_hash = 3;

  // first & second packet
  struct sk_buff *skb_g_1 = (struct sk_buff *)malloc(sizeof(struct sk_buff));

  skb_g_1->sk = sk_g;
  skb_g_1->tstamp = 2;

  skb_g_1->plen = 50000;

  // * flow H
  // socket of flow
  struct sock *sk_h = malloc(sizeof(struct sock));
  sk_h->sk_hash = 3;

  // first & second packet
  struct sk_buff *skb_h_1 = (struct sk_buff *)malloc(sizeof(struct sk_buff));
  struct sk_buff *skb_h_2 = (struct sk_buff *)malloc(sizeof(struct sk_buff));

  skb_h_1->sk = sk_h;
  skb_h_1->tstamp = 3;
  skb_h_2->sk = sk_h;
  skb_h_2->tstamp = 5;

  skb_h_1->plen = 50000;
  skb_h_2->plen = 50000;

  // * flow K
  // socket of flow
  struct sock *sk_k = malloc(sizeof(struct sock));
  sk_k->sk_hash = 5;

  // first & second packet
  struct sk_buff *skb_k_1 = (struct sk_buff *)malloc(sizeof(struct sk_buff));

  skb_k_1->sk = sk_k;
  skb_k_1->tstamp = 6;

  skb_k_1->plen = 50000;

  // initial config
  struct sk_buff *to_free = (struct sk_buff *)malloc(sizeof(struct sk_buff));

  // * ENQUEUE
  // enqueue them in the order of the tstamps
  fq_enqueue(q, skb_f_1, sch, &to_free);
  fq_enqueue(q, skb_g_1, sch, &to_free);
  fq_enqueue(q, skb_h_1, sch, &to_free);
  fq_enqueue(q, skb_f_2, sch, &to_free);
  fq_enqueue(q, skb_h_2, sch, &to_free);
  fq_enqueue(q, skb_k_1, sch, &to_free);

  printf("\n----- AFTER Enqueue -----\n\n");

  printFlowsList("NEW", q->new_flows.first);
  printFlowsList("OLD", q->old_flows.first);

  printf("\n\n");

  // Check New Flows List Content
  struct fq_flow *firstFlow = q->new_flows.first;
  printf("NFL: first flow: socket_hash => %u | tstamp => %ld\n", firstFlow->socket_hash, firstFlow->head->tstamp);

  struct fq_flow *secondFlow = q->new_flows.first->next;
  printf("NFL: second flow: socket_hash => %u | tstamp => %ld\n", secondFlow->socket_hash, secondFlow->head->tstamp);

  struct fq_flow *thirdFlow = q->new_flows.first->next->next;
  printf("NFL: third flow: socket_hash => %u | tstamp => %ld\n", thirdFlow->socket_hash, thirdFlow->head->tstamp);

  struct fq_flow *fourthFlow = q->new_flows.first->next->next->next;
  printf("NFL: fourth flow: socket_hash => %u | tstamp => %ld\n", fourthFlow->socket_hash, fourthFlow->head->tstamp);

  printf("\nNFL: fifth flow: exists => %d\n", q->new_flows.first->next->next->next->next != NULL);

  // Check Packet LL for each flow with more than single packet (F & H)
  // Flow F
  printf("\nNFL: flow F: first packet: tstamp => %ld\n", firstFlow->head->tstamp);
  printf("NFL: flow F: second packet: tstamp => %ld\n", firstFlow->head->next->tstamp);

  // Flow H
  printf("\nNFL: flow H: first packet: tstamp => %ld\n", thirdFlow->head->tstamp);
  printf("NFL: flow H: second packet: tstamp => %ld\n", thirdFlow->head->next->tstamp);

  // ! ********** DEBUG **********
  // printf("\n----- EXTRA STUFF -----\n\n");
  // struct rb_root *root = &q->fq_root[3];

  // int rb_tree_height = getRbTreeHeight(root->rb_node);
  // printf("tree height: %d\n\n", rb_tree_height);
  // int numNodesInTree = getRbTreeSize(root->rb_node);
  // printf("num nodes in tree: %d\n", numNodesInTree);

  // struct rb_node **p = &root->rb_node;
  // struct fq_flow *f = rb_entry(*p, struct fq_flow, fq_node);
  // printf("\nroot flow in fq_root[3]: sk_hash: %u\n", f->sk->sk_hash);
  // printf("f credits: %d\n", f->credit);

  // // checking whether t_root of a flow refers to the rb-tree the flow is in or sth else
  // root = &f->t_root;
  // p = &root->rb_node;
  // struct fq_flow *f_likely = rb_entry(*p, struct fq_flow, fq_node);
  // printf("root flow in fq_root[3]: sk_hash: %u\n", f->sk->sk_hash);
  // numNodesInTree = getRbTreeSize(root->rb_node);
  // printf("num nodes in tree: %d\n", numNodesInTree);
  // // ! num nodes comes out as 0 and height as 0
  // // ? does this mean the t_root is sth else?
}

void Test2_fq_dequeue(struct Qdisc *sch, struct fq_sched_data *q)
{
  fq_dequeue(sch, q);
  fq_dequeue(sch, q);
  fq_dequeue(sch, q);
  fq_dequeue(sch, q);
  fq_dequeue(sch, q);
  fq_dequeue(sch, q);

  printf("\n----- AFTER Dequeue -----\n\n");

  printFlowsList("NEW", q->new_flows.first);
  printFlowsList("OLD", q->old_flows.first);
  printFlowsList("CO", q->co_flows.first);

  printf("\n\n");

  // * If flow got moved to OFL due to credit exhaustion, it should have no packets. It packets run out before credits are used up, the flow will still have some packets left to transmit
  // printf("q->old_flows.first->head != NULL: %d\n", q->old_flows.first->head != NULL);
  // * only call this if there is at least one packet in the flow
  // printf("q->old_flows.first->head->tstamp: %u\n", q->old_flows.first->head->tstamp);

  // if credit <= 0 for flow -> fq_dequeue will
}

void Test_playground()
{
  struct fq_flow *flow1 = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  flow1->flowName = "F";
  flow1->socket_hash = 1;

  struct fq_flow *flow2 = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  flow2->flowName = "G";
  flow2->socket_hash = 2;
  flow1->next = flow2;

  struct fq_flow *flow3 = (struct fq_flow *)malloc(sizeof(struct fq_flow));
  flow3->flowName = "H";
  flow3->socket_hash = 3;
  flow2->next = flow3;

  struct fq_flow *flowsListHead = flow1;

  printFlowsList("NEW", flowsListHead);
}

/**
 * Main Testing Program.
 */
int main()
{
  // initial config
  struct fq_sched_data *q;
  struct Qdisc *sch;
  init(&q, &sch);

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

  // before("Test_fq_enqueue");
  // Test_fq_enqueue();
  // after();

  before("Test2_fq_enqueue");
  Test2_fq_enqueue(sch, q);
  after();

  before("Test2_fq_dequeue");
  Test2_fq_dequeue(sch, q);
  after();

  printTestingSessionResult();
  printTestingSessionEndText();

  return 0;
}
