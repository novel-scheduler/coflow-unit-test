#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include "./temp_types.h"
#include "../Lib/rbtree.c"

#ifndef max
#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })
#endif

#ifndef min
#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })
#endif

#define max_t(type, x, y) max((type)x, (type)y)

#define time_after(a, b) \
  (((long)((b) - (a)) < 0))

#define NSEC_PER_SEC 1000L

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define div64_ul(x, y) div64_u64((x), (y))

static inline u64 div64_u64(u64 dividend, u64 divisor)
{
  return dividend / divisor;
}

typedef s64 ktime_t;

typedef unsigned int __u32;

struct qdisc_skb_head
{
  __u32 qlen;
};

struct net_device
{
};

struct netdev_queue
{
  struct net_device *dev;
};

struct Qdisc
{
  struct qdisc_skb_head q;
  struct netdev_queue *dev_queue;
};

struct sock
{
  unsigned int sk_hash;
  unsigned long sk_pacing_rate; /* bytes per second */
  // TODO: incomplete
};

typedef unsigned int sk_buff_data_t;

struct sk_buff
{
  struct sk_buff *next;
  struct sk_buff *prev;
  struct sock *sk;
  ktime_t tstamp;
  __u32 priority;
  struct rb_node rbnode;
  char cb[48];
  sk_buff_data_t end;
  struct net_device *dev;

  // * custom fields
  int plen;     // user can manually set packet size
  char *pkt_id; // user can manually set unique packet id
  int flow_id;  // ex) 0, 1, 2, etc.

  // TODO: incomplete
};

struct fq_skb_cb
{
  u64 time_to_send;
};

struct qdisc_skb_cb
{
  struct
  {
    unsigned int pkt_len;
    u16 slave_dev_queue_mapping;
    u16 tc_classid;
  };
#define QDISC_CB_PRIV_LEN 20
  unsigned char data[QDISC_CB_PRIV_LEN];
};

static inline struct qdisc_skb_cb *qdisc_skb_cb(const struct sk_buff *skb)
{
  return (struct qdisc_skb_cb *)skb->cb;
}

// ! THIS NEEDS TO BE CHECKED
// ! FOR NOW, assume fq_skb_cb() works
static inline struct fq_skb_cb *fq_skb_cb(struct sk_buff *skb)
{
  // qdisc_cb_private_validate(skb, sizeof(struct fq_skb_cb));
  return (struct fq_skb_cb *)qdisc_skb_cb(skb)->data;
}

static inline struct net_device *qdisc_dev(const struct Qdisc *qdisc)
{
  return qdisc->dev_queue->dev;
}

struct qdisc_watchdog
{
};

#define nFlows 4

#define barrierNumber 10000

#define timeInterval 10000

#define plimit 17

int pCount_enq = 0;

int pCount_deq = 0;

int pCount = 0;

int barriercounter_flow[2] = {0};

int dcounter = 0;

u32 pFlowid[2] = {-1, -1};
// u32 pFlowid[2] = {5, 1111}; // * changed by Heon
// u32 pFlowid[2] = {111, 333}; // * changed by Heon

int firstflag = 0;

int barrier[barrierNumber] = {0};

struct fq_flow *flowmapper[2];

char *bitmap[nFlows];

unsigned long pCoflow[nFlows];

unsigned long time_first, time_nw, time_elapsed;

/*
 * Per flow structure, dynamically allocated.
 * If packets have monotically increasing time_to_send, they are placed in O(1)
 * in linear list (head,tail), otherwise are placed in a rbtree (t_root).
 */
struct fq_flow
{
  // HEON:ADD - custom member
  char *flowName;

  /* First cache line : used in fq_gc(), fq_enqueue(), fq_dequeue() */
  struct rb_root t_root;
  struct sk_buff *head; /* list of skbs for this flow : first skb */
  union
  {
    struct sk_buff *tail; /* last skb in the list */
    unsigned long age;    /* (jiffies | 1UL) when flow was emptied, for gc */
  };
  struct rb_node fq_node; /* anchor in fq_root[] trees */
  struct sock *sk;
  u32 socket_hash; /* sk_hash */
  int qlen;        /* number of packets in flow queue */

  // * dequeue uses credits & lists -> so we can manually mock them up *
  /* Second cache line, used in fq_dequeue() */
  int credit;
  /* 32bit hole on 64bit arches */

  struct fq_flow *next; /* next pointer in RR lists */

  struct rb_node rate_node; /* anchor in q->delayed tree */
  u64 time_next_packet;
} ____cacheline_aligned_in_smp;

struct fq_flow_head
{
  struct fq_flow *first;
  struct fq_flow *last;
};

struct fq_sched_data
{
  struct fq_flow_head new_flows;

  struct fq_flow_head old_flows;

  struct fq_flow_head co_flows;

  struct rb_root delayed; /* for rate limited flows */
  u64 time_next_delayed_flow;
  u64 ktime_cache; /* copy of last ktime_get_ns() */
  unsigned long unthrottle_latency_ns;

  struct fq_flow internal; /* for non classified or high prio packets */
  u32 quantum;
  u32 initial_quantum;
  u32 flow_refill_delay;
  u32 flow_plimit;             /* max packets per flow */
  unsigned long flow_max_rate; /* optional max rate per flow */
  u64 ce_threshold;
  u64 horizon;     /* horizon in ns */
  u32 orphan_mask; /* mask for orphaned skb */
  u32 low_rate_threshold;
  struct rb_root *fq_root;
  u8 rate_enable;
  u8 fq_trees_log;
  u8 horizon_drop;
  u32 flows;
  u32 inactive_flows;
  u32 throttled_flows;

  u64 stat_gc_flows;
  u64 stat_internal_packets;
  u64 stat_throttled;
  u64 stat_ce_mark;
  u64 stat_horizon_drops;
  u64 stat_horizon_caps;
  u64 stat_flows_plimit;
  u64 stat_pkts_too_long;
  u64 stat_allocation_errors;

  u32 timer_slack; /* hrtimer slack in ns */
  struct qdisc_watchdog watchdog;
};

int getRbTreeHeight(struct rb_node *node)
{
  // printf("* FUNC: getRbTreeHeight\n");
  if (node == NULL)
    return 0;
  else
  {
    /* compute the height of each subtree */
    int lheight = getRbTreeHeight(node->rb_left);
    int rheight = getRbTreeHeight(node->rb_right);
    /* use the larger one */
    if (lheight > rheight)
    {
      return (lheight + 1);
    }
    else
    {
      return (rheight + 1);
    }
  }
}

void printGivenLevel(struct rb_node *root, int level)
{
  if (root == NULL)
    return;
  if (level == 1)
    printf("D ");
  else if (level > 1)
  {
    printGivenLevel(root->rb_left, level - 1);
    printGivenLevel(root->rb_right, level - 1);
  }
}

void printLevelOrder(struct rb_node *root_node)
{
  int h = getRbTreeHeight(root_node);
  int i;
  for (i = 1; i <= h; i++)
  {
    printGivenLevel(root_node, i);
    printf("\n");
  }
}

int getRbTreeSize(struct rb_node *node)
{
  if (node == NULL)
    return 0;
  else
    return (getRbTreeSize(node->rb_left) + 1 + getRbTreeSize(node->rb_right));
}

int valuePresentInArray(unsigned val, unsigned arr[], int lengthOfarray)
{
  printf("value to be searched is %u\n", val);
  printf("* FUNC: valuePresentInArray\n");

  int i;

  for (i = 0; i < lengthOfarray; i++)
  {
    printf("In Array Present function Array Value is  %u\n", arr[i]);
    if (arr[i] == val)
    {
      printf("In Array Present function match occured value present index value of %d\n", i);
      return i;
    }
  }
  printf("value NOT present\n");
  return -1;
}

static void resetFlowid(unsigned arr[], int lengthOfarray)
{
  int i;

  for (i = 0; i < lengthOfarray; i++)
  {
    arr[i] = -1;
  }
}

static void fq_flow_add_tail(struct fq_flow_head *head, struct fq_flow *flow)
{
  if (head->first)
    head->last->next = flow;
  else
    head->first = flow;
  head->last = flow;
  flow->next = NULL;
}

// ! coflow & flow is not needed
int Promotecoflows(struct fq_flow_head *oldFlow, struct fq_flow_head *newFlow,
                   struct fq_flow_head *coflowhead, struct fq_flow *flow,
                   struct fq_flow *coflow, unsigned arr[], int lengthOfarray)
{
  printf("In Promotecoflows \n");

  printf("size of array %d\n", lengthOfarray);

  struct fq_flow_head *head;

  struct fq_flow *prev, *newflowhead, *oldflowhead;

  int flag = 0;

  head = newFlow;

  flow = head->first;

  int oldnew = 0;

loop:
  if (!flow)
  {
  loop2:
    head = oldFlow;
    flow = head->first;
    flag = 0;
    oldnew = 1;
    if (!flow)
    {

    loop3:
       printf("promotion completed \n");
      return 1;
    }
  }

  /*if(newFlow->first->socket_hash)
  printk("value of newflows head is %u\n", newFlow->first->socket_hash);
  if(oldFlow->first->socket_hash)
  printk("value of oldflows head is  %u\n", oldFlow->first->socket_hash);
  printk("value of flag is  %d\n", flag);*/

  if (!flag)
  {
    flow = head->first;
    flag = 1;
  }

  int arraylength = lengthOfarray;

  int rValue = valuePresentInArray(flow->socket_hash, arr, arraylength);

  if (rValue != -1)
  {

    if (head->first == flow)
    {

      prev = flow->next;
      head->first = flow->next;
      fq_flow_add_tail(coflowhead, flow);
      flow = prev;

      if (!flow)
      {

        if (!oldnew)
          goto loop2;

        else
          goto loop3;
      }

      goto loop;
    }

    else
    {
      prev = head->first;

      while (!(prev->next == flow))
      {

        prev = prev->next;
      }

      prev->next = flow->next;

      fq_flow_add_tail(coflowhead, flow);

      flow = prev->next;

      if (!flow)
      {

        if (!oldnew)
          goto loop2;

        else
          goto loop3;
      }
      goto loop;
    }
  }

  else
  {
    flow = flow->next;

    if (!flow)
    {

      if (!oldnew)
        goto loop2;

      else
        goto loop3;
    }

    goto loop;
  }

  return 1;
}

static void fq_flow_set_detached(struct fq_flow *f) { f->age = 100 | 1UL; }

static bool fq_flow_is_detached(const struct fq_flow *f)
{
  return !!(f->age & 1UL);
}

static struct fq_flow *fq_classify(struct sk_buff *skb, struct fq_sched_data *q)
{
  printf("\n* FUNC: fq_classify\n");

  // struct rb_node **p, *parent;
  // struct sock *sk = skb->sk;
  // struct rb_root *root;
  // struct fq_flow *f;

  // * HEON:NOTE avoiding garbage values
  struct rb_node **p = NULL;
  struct rb_node *parent = NULL;
  struct sock *sk = skb->sk;
  struct rb_root *root = NULL;
  struct fq_flow *f = NULL;

  printf("sk->hash IN:fq_classify: %u\n", sk->sk_hash);
  printf("skb->tstmp: %d\n", skb->tstamp);

  // printk("In add values address pair is  : %lld \n ", sk->sk_portpair);
  // printk("In add values destination port is  : %lld \n ", sk->sk_dport);
  // printk("In add values hash pair is  : %d \n ", skb_get_hash(skb));

  // /* warning: no starvation prevention... */
  // if (unlikely((skb->priority & TC_PRIO_MAX) == TC_PRIO_CONTROL))
  //   return &q->internal;

  /* SYNACK messages are attached to a TCP_NEW_SYN_RECV request socket
   * or a listener (SYNCOOKIE mode)
   * 1) request sockets are not full blown,
   *    they do not contain sk_pacing_rate
   * 2) They are not part of a 'flow' yet
   * 3) We do not want to rate limit them (eg SYNFLOOD attack),
   *    especially if the listener set SO_MAX_PACING_RATE
   * 4) We pretend they are orphaned
   */
  // if (!sk || sk_listener(sk))
  // {
  //   unsigned long hash = skb_get_hash(skb) & q->orphan_mask;

  //   /* By forcing low order bit to 1, we make sure to not
  //    * collide with a local flow (socket pointers are word aligned)
  //    */
  //   sk = (struct sock *)((hash << 1) | 1UL);

  //   // printk("hash value in fq classify  after sk creation : %lu \n ", hash );

  //   skb_orphan(skb);
  // }
  // else if (sk->sk_state == TCP_CLOSE)
  // {
  //   unsigned long hash = skb_get_hash(skb) & q->orphan_mask;

  //   // printk("hash  value in fq classify in else if  of each flow is  : %lu \n
  //   // ",hash );
  //   /*
  //    * Sockets in TCP_CLOSE are non connected.
  //    * Typical use case is UDP sockets, they can send packets
  //    * with sendto() to many different destinations.
  //    * We probably could use a generic bit advertising
  //    * non connected sockets, instead of sk_state == TCP_CLOSE,
  //    * if we care enough.
  //    */
  //   sk = (struct sock *)((hash << 1) | 1UL);
  // }
  // * HEON:NOTE: the commented code above has to be checked later

  // * HEON:NOTE: two different flows cannot have the same sk value.
  // * HEON:NOTE: a hash table is being used, which bases its key on the sk value for unique identification - the value of the hash keys are not individual flows, but an rb_tree of flows
  printf("* BEFORE: getting root\n");

  // root = &q->fq_root[hash_ptr(sk, q->fq_trees_log)];
  printf("sk->sk_hash: %u\n", sk->sk_hash);
  printf("q->fq_trees_log: %u\n", q->fq_trees_log);
  printf("hash operation result: %u\n", sk->sk_hash % q->fq_trees_log);
  root = &q->fq_root[sk->sk_hash % q->fq_trees_log];

  if (root == NULL)
  {
    printf("ERROR: root is NOT initialized\n");
  }

  // * HEON:NOTE: checking for maximum number of allowed flows -> not needed
  // if (q->flows >= (2U << q->fq_trees_log) && q->inactive_flows > q->flows / 2)
  //   fq_gc(q, root, sk);

  p = &root->rb_node;
  printf("p != NULL: %d\n", p != NULL);

  // ! *p is failing
  // printf("*p != NULL: %d\n", *p != NULL);
  // printf("rb color: %u\n", (*p)->__rb_parent_color);

  parent = NULL;

  // * HEON:ADD - traverse rb_tree
  // printf("\n*** RBTREE CUSTOM TRAVERSAL ***\n");
  // printf("root != NULL: 1\n");
  // struct rb_node *ptr = (struct rb_node *)root->rb_node;
  // f = rb_entry(parent, struct fq_flow, fq_node);
  // printf("f != NULL: %d\n", f != NULL);
  // printf("tree-trav: f->socket_hash: %u\n", f->socket_hash);
  // printf("*** END OF TRAV ***\n\n");

  printf("* BEFORE: loop\n");
  while (*p)
  {
    printf("** LOOP - while (*p)\n");
    parent = *p;

    // get the flow
    f = rb_entry(parent, struct fq_flow, fq_node);
    printf("** LOOP - f->sk->sk_hash: %u\n", f->sk->sk_hash);

    // Here, f->sk is the socket of the retrieved flow (which is initially the flow of the root rb_node in the tree), and sk is skb->sk, meaning it's the socket of the packet
    // So basically this binary search operation is to check whether the packet being processed is from an existing (OLD, existing in the tree) flow or not. If it's a match, that means the packet belongs to a flow that already exists in the tree, hence an old flow.
    if (f->sk == sk)
    {
      printf("*** MATCH!\n");
      /* socket might have been reallocated, so check
       * if its sk_hash is the same.
       * It not, we need to refill credit with
       * initial quantum
       */

      // ! HEON:TODO - uncommented this whole block below regarding pflowid
      int lengthOfarray = 0;

      int i;

      for (i = 0; i < (sizeof(pFlowid) / sizeof(pFlowid[0])); i++)
      {
        lengthOfarray++;
      }

      printf("**1 skb->sk == sk => %d\n", skb->sk == sk);
      printf("**1 f->socket_hash != sk->sk_hash => %d\n", f->socket_hash != sk->sk_hash);
      printf("**1 f->socket_hash => %u\n", f->socket_hash);
      printf("**1 sk->sk_hash => %u\n", sk->sk_hash);
      printf("**1 unlikely => %d", unlikely(skb->sk == sk && f->socket_hash != sk->sk_hash));

      // ? HEON: what is this condition?
      if (unlikely(skb->sk == sk && f->socket_hash != sk->sk_hash))
      {
        f->credit = q->initial_quantum;
        f->socket_hash = sk->sk_hash;
        // printk("flow hash in rb tree value of each flow is  : %u \n
        // ",f->socket_hash );
        if ((pFlowid[0] == -1) && (pFlowid[1] == -1))
        {
          pFlowid[0] = f->socket_hash;

          printf("flow pflowid 1 hash in rb tree value of each flow : %u\n ", pFlowid[1]);

          if (pFlowid[0] == 0)
          {
            resetFlowid(pFlowid, lengthOfarray);
          }
        }

        if ((pFlowid[0] != -1) && (pFlowid[1] == -1))
        {
          int lVal =
              valuePresentInArray(f->socket_hash, pFlowid, lengthOfarray);

          if (pFlowid[0] != f->socket_hash)
            pFlowid[1] = f->socket_hash;

          printf("flow pflowid 1 hash in rb tree value of each flow : %u\n ", pFlowid[1]);

          if ((pFlowid[1] == 0))
          {
            resetFlowid(pFlowid, lengthOfarray);
          }
        }

        // if (q->rate_enable)
        //   smp_store_release(&sk->sk_pacing_status, SK_PACING_FQ);
        // if (fq_flow_is_throttled(f))
        //   fq_flow_unset_throttled(q, f);
        // f->time_next_packet = 0ULL;
      }

      return f;
    }
    if (f->sk > sk)
    {
      printf("*** NO MATCH: going RIGHT!\n");
      p = &parent->rb_right;
    }
    else
    {
      printf("*** NO MATCH: going LEFT!\n");
      p = &parent->rb_left;
    }
  }

  // *** NEW FLOW ***

  printf("* NEW FLOW\n");
  f = malloc(sizeof(struct fq_flow));
  // f = kmem_cache_zalloc(fq_flow_cachep, GFP_ATOMIC | __GFP_NOWARN);
  // if (unlikely(!f))
  // {
  //   q->stat_allocation_errors++;
  //   return &q->internal;
  // }
  /* f->t_root is already zeroed after kmem_cache_zalloc() */

  fq_flow_set_detached(f);
  f->sk = sk;
  if (skb->sk == sk)
  {
    f->socket_hash = sk->sk_hash;
    // if (q->rate_enable)
    // smp_store_release(&sk->sk_pacing_status, SK_PACING_FQ);
  }
  f->credit = q->initial_quantum;

  printf("f->sk->sk_hash: %u\n", f->sk->sk_hash);
  printf("f->credit: %d\n", f->credit);

  rb_link_node(&f->fq_node, parent, p);
  rb_insert_color(&f->fq_node, root);

  q->flows++;
  q->inactive_flows++;
  printf("flow hash in after classification  : %u \n ", f->socket_hash);

  printf("* END-FUNC: fq_classify\n");
  return f;
}

static void flow_queue_add(struct fq_flow *flow, struct sk_buff *skb)
{
  printf("\n* FUNC: flow_queue_add\n");
  struct rb_node **p, *parent;
  struct sk_buff *head, *aux;

  head = flow->head;
  if (!head ||
      fq_skb_cb(skb)->time_to_send >= fq_skb_cb(flow->tail)->time_to_send)
  {
    printf("!head OR fq_skb_cb(skb)->time_to_send >= fq_skb_cb(flow->tail)->time_to_send\n");

    if (!head)
    {
      printf("!head == TRUE\n");
      flow->head = skb;
      printf("flow->head->sk->sk_hash: %u\n", flow->head->sk->sk_hash);
    }
    else
    {
      printf("!head == FALSE\n");
      flow->tail->next = skb;
    }
    flow->tail = skb;
    skb->next = NULL;

    printf("* END-FUNC: flow_queue_add\n");
    return;
  }

  p = &flow->t_root.rb_node;
  parent = NULL;

  printf("* BEFORE: while (*p)\n");
  while (*p)
  {
    parent = *p;
    aux = rb_to_skb(parent);
    if (fq_skb_cb(skb)->time_to_send >= fq_skb_cb(aux)->time_to_send)
    {
      printf("** RIGHT\n");
      p = &parent->rb_right;
    }
    else
    {
      printf("** LEFT\n");
      p = &parent->rb_left;
    }
  }
  // printk("In add values skb after classification to check in add is  : %d \n
  // ", skb_get_hash(skb));
  printf("* BEFORE: rb_link_node & rb_insert_color\n");

  rb_link_node(&skb->rbnode, parent, p);
  rb_insert_color(&skb->rbnode, &flow->t_root);

  printf("* END-FUNC: flow_queue_add\n");
}

static struct fq_sched_data *fq_init()
{
  struct fq_sched_data *q = malloc(sizeof(struct fq_sched_data));

  q->flow_plimit = 100;
  // HEON: 9000 as mtu
  q->quantum = 2 * 9000;
  q->initial_quantum = 10 * 9000;
  // getconf CLK_TCK -> 100
  q->flow_refill_delay = 100 / 100 * 4;
  // q->quantum = 2 * psched_mtu(qdisc_dev(sch));
  // q->initial_quantum = 10 * psched_mtu(qdisc_dev(sch));
  // q->flow_refill_delay = msecs_to_jiffies(40);
  q->flow_max_rate = ~0UL;
  q->time_next_delayed_flow = ~0ULL;
  q->rate_enable = 1;

  q->new_flows.first = NULL;
  q->old_flows.first = NULL;
  q->co_flows.first = NULL;

  q->delayed = RB_ROOT;

  // ? HEON:Q: is this malloc correct? fq_root seems to be used as an array for multiple rb trees (just like a string containing characters). Should we initialize using the array notation instead of malloc?
  // ? https://stackoverflow.com/questions/10468128/how-do-you-make-an-array-of-structs-in-c
  q->fq_root = malloc(sizeof(struct rb_root) * 10);
  // q->fq_root[10];

  // * HEON:NOTE: fq_trees_logs == the number of buckets in the hash, basically the number of rbtrees
  q->fq_trees_log = log2(1024);
  q->orphan_mask = 1024 - 1;
  q->low_rate_threshold = 550000 / 8;

  return q;
}

// TODO: not tested yet
// * goes to the rb_tree the given flow is in, and returns the first packet of the leftmost flow in that rb_tree (which has the earliest time to send) (remember that rb_tree's key for sorting is the time to send)
static struct sk_buff *fq_peek(struct fq_flow *flow)
{
  struct sk_buff *skb = skb_rb_first(&flow->t_root);
  struct sk_buff *head = flow->head;

  if (!skb)
    return head;

  if (!head)
    return skb;

  if (fq_skb_cb(skb)->time_to_send < fq_skb_cb(head)->time_to_send)
    return skb;
  return head;
}

static inline unsigned int qdisc_pkt_len(const struct sk_buff *skb)
{
  return qdisc_skb_cb(skb)->pkt_len;
}

// TODO:
static void fq_erase_head(struct Qdisc *sch, struct fq_flow *flow, struct sk_buff *skb)
{
  if (skb == flow->head)
  {
    flow->head = skb->next;
  }
  else
  {
    rb_erase(&skb->rbnode, &flow->t_root);
    skb->dev = qdisc_dev(sch);
  }
}

// TODO:
static inline void skb_mark_not_on_list(struct sk_buff *skb)
{
  skb->next = NULL;
}

// TODO:
/* Remove one skb from flow queue.
 * This skb must be the return value of prior fq_peek().
 */
static void fq_dequeue_skb(struct Qdisc *sch, struct fq_flow *flow, struct sk_buff *skb)
{
  fq_erase_head(sch, flow, skb);
  skb_mark_not_on_list(skb);
  flow->qlen--;
  // qdisc_qstats_backlog_dec(sch, skb);
  sch->q.qlen--;
}

// TODO: not tested yet
// * HEON:ADD - param: struct fq_sched_data *q
static struct sk_buff *fq_dequeue(struct Qdisc *sch, struct fq_sched_data *q)
{
  printf("\n*FUNC: fq_dequeue\n");

  // struct fq_sched_data *q = qdisc_priv(sch);
  // * Dummy fq_sched_data passed

  struct fq_flow_head *head;
  struct sk_buff *skb;
  struct fq_flow *f, *coflow;
  unsigned long rate;
  int dcounter = 0;
  int coflowcounter = 0;
  u32 plen;
  u64 now = 100; // * manual value of 100
  int lengthOfarray = 0;
  int i;
  int prevarray[2];
  int flag[2] = {0, 0};

  for (i = 0; i < (sizeof(pFlowid) / sizeof(pFlowid[0])); i++)
  {
    lengthOfarray++;
  }

  for (i = 0; i < lengthOfarray; i++)
  {
    prevarray[i] = -1;
  }

  if (!sch->q.qlen)
  {
    printf("sch->q.qlen == 0");
    return NULL;
  }

  // * priority packets, not for now
  // skb = fq_peek(&q->internal);
  // if (unlikely(skb))
  // {
  //   fq_dequeue_skb(sch, &q->internal, skb);
  //   goto out;
  // }

  // * edge case where packets are rate limited
  // q->ktime_cache = now = ktime_get_ns();
  // fq_check_throttled(q, now);

  /*dequeuing using barrier process*/

  printf("BEFORE: begin\n");

begin:
  printf("CFL reached\n");
  head = &q->co_flows;
  if (!head->first)
  {
    printf("NFL reached\n");
    head = &q->new_flows;
    if (!head->first)
    {
      printf("OFL reached\n");
      head = &q->old_flows;
      if (!head->first)
      {
        printf("DEADEND reached\n");
        // * for saving resources - basically reducing number of check operations by putting to sleep
        // if (q->time_next_delayed_flow != ~0ULL)
        //   qdisc_watchdog_schedule_range_ns(
        //       &q->watchdog, q->time_next_delayed_flow, q->timer_slack);
        return NULL;
      }
    }
  }

  f = head->first;

  // * lengthOfarray will be 2 (set by using pFlowid) -> meaning first two flows will be considered as part of the coflow task
  int rValue = valuePresentInArray(f->socket_hash, pFlowid, lengthOfarray);

  if (rValue != -1)
  {
    printf("CF: VALUE FOUND\n");
    if (flag[rValue] == 0)
    {
      coflowcounter++;
      flag[rValue] = 1;
    }
  }

  // printk("rValue is   : %d \n ", rValue);

  // printk("barrier value  : %d \n ", barrier[dcounter]);

  // Breach and membership of the flow is checked once it is satisfied all the
  // flows are added to co-flow set at once
  if ((rValue != -1) && (barrier[dcounter] == 3))
  {
    printf("*** Coflow BREACH! -> promoting coflows\n");
    barrier[dcounter] = 0;
    //head->first = f->next;
    // printk("adding all co-flows together \n");
    Promotecoflows(&q->old_flows, &q->new_flows, &q->co_flows, f, coflow,
                   pFlowid, lengthOfarray);
        printf("promoting co-flows goto begin\n");           
        goto begin;            
  }

  // * prepare for next set of flows/packets
  if (!barrier[dcounter] && (coflowcounter == lengthOfarray))
  {
    dcounter++;
    coflowcounter = 0;
  }

  /*demotion is defualt and we need not use any specific function because of how
   * the flows are added to old flows if 	cedit is not enough to send the
   * packets*/

  // TODO: fq_enqueue adds credit -> check later if it does, or this will result in unwanted behavior
  // * put the flow at the back of the old flows list
  // * HEON: ig this means one flow can repeatedly reach this line -> one flow having sufficient credit to transmit many times -> so every time it transmits sth, it is checked at this line whether it has enough credit -> if not, then it has exhausted all of its credits, ready to be moved to the back of the line
  printf("f->credit == %d\n", f->credit);
  if (f->credit <= 0)
  {
    printf("f->credit <= 0: TRUE\n");
    f->credit += q->quantum;
    printf("Give CREDIT to flow: f->credit: %d\n", f->credit);
    head->first = f->next;
    // * HEON: this is where we mainly expect it to move the new flows to the old flows list
    printf("MOVING: NEW flow to OLD flow && iterating to NEXT flow due to NO CREDIT\n\n");
    fq_flow_add_tail(&q->old_flows, f);
    goto begin;
  }

  // * gets leftmost (earliest time to send) sk_buff
  // * the head sbk/pkt of the flow is the key for organizing flows in the rb_tree
  // * we get the leftmost flow's head packet to run a max() to get the higher time to send
  printf("BEFORE - fq_peek(f)\n");
  skb = fq_peek(f);
  printf("skb != NULL: %d\n", skb != NULL);
  // printf("sbk->sk->sk_hash: %u\n", skb->sk->sk_hash);
  if (skb)
  {
    printf("if (skb) TRUE branch\n");
    // * gets the farthest time to send -> if it is larger than current time (now), don't process packet
    u64 time_next_packet =
        max_t(u64, fq_skb_cb(skb)->time_to_send, f->time_next_packet);

    // * time_next_packet is equal to tstamp of skb user inputted
    printf("time_next_packet: %u\n", time_next_packet);
    printf("now: %u\n", now);

    if (now < time_next_packet)
    {
      printf("now < time_next_packet == TRUE\n");
      head->first = f->next;
      f->time_next_packet = time_next_packet;
      // fq_flow_set_throttled(q, f);
      goto begin;
    }

    // * explicit congestion notification
    // prefetch(&skb->end);
    // if ((s64)(now - time_next_packet - q->ce_threshold) > 0)
    // {
    //   INET_ECN_set_ce(skb);
    //   q->stat_ce_mark++;
    // }
    fq_dequeue_skb(sch, f, skb);
  }
  else
  {
    printf("if (skb) FALSE branch\n");
    // * if there's no pkts to send for current flow, proceed to next flow
    head->first = f->next;
    /* force a pass through old_flows to prevent starvation */
    if ((head == &q->new_flows) && q->old_flows.first)
    {
      printf("MOVING: NEW flow to OLD flow && going back to BEGIN due to NO PACKETS\n\n");
      fq_flow_add_tail(&q->old_flows, f);
    }
    else
    {
      fq_flow_set_detached(f);
      q->inactive_flows++;
    }
    goto begin;
  }

  // plen = qdisc_pkt_len(skb);
  plen = skb->plen;
  printf("sbk->tstamp: %u\n", skb->tstamp);
  printf("plen: %u\n", plen);
  f->credit -= plen;
  printf("AFTER plen subtraction: f->credit == %d\n", f->credit);

  // * pacing
  // if (!q->rate_enable)
  //   goto out;

  // * flow_max_rate is initialized as ~0UL
  rate = q->flow_max_rate;

  /* If EDT time was provided for this skb, we need to
   * update f->time_next_packet only if this qdisc enforces
   * a flow max rate.
   */
  if (!skb->tstamp)
  {
    if (skb->sk)
      rate = min(skb->sk->sk_pacing_rate, rate);

    if (rate <= q->low_rate_threshold)
    {
      f->credit = 0;
    }
    else
    {
      plen = max(plen, q->quantum);
      if (f->credit > 0)
        goto out;
    }
  }
  if (rate != ~0UL)
  {
    u64 len = (u64)plen * NSEC_PER_SEC;

    if (likely(rate))
      len = div64_ul(len, rate);
    /* Since socket rate can change later,
     * clamp the delay to 1 second.
     * Really, providers of too big packets should be fixed !
     */
    if (unlikely(len > NSEC_PER_SEC))
    {
      len = NSEC_PER_SEC;
      // * HEON:ADDED - init if not initialized
      if (q->stat_pkts_too_long == NULL)
      {
        q->stat_pkts_too_long = 0;
      }
      q->stat_pkts_too_long++;
    }
    /* Account for schedule/timers drifts.
     * f->time_next_packet was set when prior packet was sent,
     * and current time (@now) can be too late by tens of us.
     */
    if (f->time_next_packet)
      len -= min(len / 2, now - f->time_next_packet);
    f->time_next_packet = now + len;
  }
out:
  // qdisc_bstats_update(sch, skb);
  printf("OUT\n");
  return skb;
}

// * need a dummy skb & sch
static int fq_enqueue(struct fq_sched_data *q, struct sk_buff *skb, struct Qdisc *sch, struct sk_buff **to_free)
{
  printf("*FUNC: fq_enqueue\n");

  printf("skb hash: %u\n", skb->sk->sk_hash);

  // struct fq_sched_data *q = qdisc_priv(sch);
  // * DUMMY fq_sched_data
  struct fq_flow *f = NULL;

  // if (unlikely(sch->q.qlen >= sch->limit))
  //   return qdisc_drop(skb, sch, to_free);

  printf("*BEFORE: if (!skb->tstamp)\n");
  if (!skb->tstamp)
  {
    printf("!skb->tstamp -> TRUE \n");
    // * replaced ktime func with time()
    fq_skb_cb(skb)->time_to_send = q->ktime_cache = (u64)time(NULL);

    printf("skb->tstamp: %ld\n", skb->tstamp);
  }
  else
  {
    printf("!skb->tstamp -> FALSE\n");
    /* Check if packet timestamp is too far in the future.
     * Try first if our cached value, to avoid ktime_get_ns()
     * cost in most cases.
     */
    // if (fq_packet_beyond_horizon(skb, q))
    // {
    //   /* Refresh our cache and check another time */
    //   q->ktime_cache = ktime_get_ns();
    //   if (fq_packet_beyond_horizon(skb, q))
    //   {
    //     if (q->horizon_drop)
    //     {
    //       q->stat_horizon_drops++;
    //       return qdisc_drop(skb, sch, to_free);
    //     }
    //     q->stat_horizon_caps++;
    //     skb->tstamp = q->ktime_cache + q->horizon;
    //   }
    // }

    fq_skb_cb(skb)->time_to_send = skb->tstamp;
  }

  printf("* BEFORE: fq_classify\n");
  f = fq_classify(skb, q);
  printf("\n");

  printf("f->sk->sk_hash: %u\n", f->sk->sk_hash);
  printf("f->socket_hash: %u\n", f->socket_hash);
  // if (unlikely(f->qlen >= q->flow_plimit && f != &q->internal))
  // {
  //   q->stat_flows_plimit++;
  //   return qdisc_drop(skb, sch, to_free);
  // }

  f->qlen++;
  // qdisc_qstats_backlog_inc(sch, skb);
  if (fq_flow_is_detached(f))
  {
    fq_flow_add_tail(&q->new_flows, f);
    
       int lengthOfarray = 0;

       int i;

      for (i = 0; i < (sizeof(pFlowid) / sizeof(pFlowid[0])); i++)
      {
        lengthOfarray++;
      }   
      
             if ((pFlowid[0] == -1) && (pFlowid[1] == -1))
        {
          pFlowid[0] = f->socket_hash;
          printf(
              "flow pflowid 0 hash in rb tree value of each flow is  : %u \n ",
              pFlowid[0]);

        }

        if ((pFlowid[0] != -1) && (pFlowid[1] == -1))
        {

          if (pFlowid[0] != f->socket_hash)
            pFlowid[1] = f->socket_hash;

          printf(
              "flow pflowid 1 hash in rb tree value of each flow is  : %u \n ",
              pFlowid[1]);
        }

    if (time_after(100, f->age + q->flow_refill_delay))
      f->credit = max_t(u32, f->credit, q->quantum);
    q->inactive_flows--;
  }

  /* Note: this overwrites f->age */

  // printk("hash of flow value : %u \n ", (f->socket_hash & q->orphan_mask) );

  /*printk("skb get hash value  : %u \n ", skb_get_hash(skb));
     unsigned long pHash = skb_get_hash(skb) & q->orphan_mask;
     printk("pHash value  : %lu \n ", pHash); */

  // * fq_classify identifies the flow itself
  // * flow_queue_add identifies which flow the packet belongs to
  flow_queue_add(f, skb);

  printf("\n* BEFORE: coflow logic\n");

  printf("q->new_flows.first->credit: %d\n", q->new_flows.first->credit);
  printf("q->new_flows.first->age: %u\n", q->new_flows.first->age);
  printf("q->new_flows.first->socket_hash: %u\n", q->new_flows.first->socket_hash);

  /*
     setting the barrier bits
     there are 100000 barriers for the co-flow
     individual bits are set by two variabes
     time to send being modified
   */

  int lengthOfarray = 0;

  int i;

  for (i = 0; i < (sizeof(pFlowid) / sizeof(pFlowid[0])); i++)
  {
    lengthOfarray++;
  }
  int pValue = valuePresentInArray(f->socket_hash, pFlowid, lengthOfarray);

  if (pValue != -1)
  {
    barrier[barriercounter_flow[pValue]] =
        barrier[barriercounter_flow[pValue]] | 1 << pValue;

     fq_skb_cb(skb)->time_to_send = skb->tstamp;

    barriercounter_flow[pValue]++;
  }

  // if (unlikely(f == &q->internal))
  // {
  //   q->stat_internal_packets++;
  // }
  sch->q.qlen++;

  return 1;
}
