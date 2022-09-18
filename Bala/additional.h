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

int firstflag = 0;

int barrier[barrierNumber] = {0};

struct fq_flow *flowmapper[2];

char *bitmap[nFlows];

unsigned long pCoflow[nFlows];

unsigned long time_first, time_nw, time_elapsed;

struct fq_skb_cb {
  u64 time_to_send;
};

/*This function is used to check if a flow belongs to a co-flow set, all the
 * flows are identified by the f->socket_hash */

static inline struct fq_skb_cb *fq_skb_cb(struct sk_buff *skb) {
  qdisc_cb_private_validate(skb, sizeof(struct fq_skb_cb));
  return (struct fq_skb_cb *)qdisc_skb_cb(skb)->data;
}

/*
 * Per flow structure, dynamically allocated.
 * If packets have monotically increasing time_to_send, they are placed in O(1)
 * in linear list (head,tail), otherwise are placed in a rbtree (t_root).
 */
struct fq_flow {
  /* First cache line : used in fq_gc(), fq_enqueue(), fq_dequeue() */
  struct rb_root t_root;
  struct sk_buff *head; /* list of skbs for this flow : first skb */
  union {
    struct sk_buff *tail; /* last skb in the list */
    unsigned long age;    /* (jiffies | 1UL) when flow was emptied, for gc */
  };
  struct rb_node fq_node; /* anchor in fq_root[] trees */
  struct sock *sk;
  u32 socket_hash; /* sk_hash */
  int qlen;        /* number of packets in flow queue */

  /* Second cache line, used in fq_dequeue() */
  int credit;
  /* 32bit hole on 64bit arches */

  struct fq_flow *next; /* next pointer in RR lists */

  struct rb_node rate_node; /* anchor in q->delayed tree */
  u64 time_next_packet;
} ____cacheline_aligned_in_smp;

struct fq_flow_head {
  struct fq_flow *first;
  struct fq_flow *last;
};

struct fq_sched_data {
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



int valuePresentInArray(unsigned val, unsigned arr[], int lengthOfarray) {
  //printk("In valuepresent Array value of %u\n", val);

  int i;

  for (i = 0; i < lengthOfarray; i++) {
    //printk("In Array Present function Array Value is  %u\n", arr[i]);
    if (arr[i] == val) {
      //printk("In Array Present function match occured value present index value " "of %d\n", i);
      return i;
    }
  }
  return -1;
}

static void resetFlowid(unsigned arr[], int lengthOfarray) {
  int i;

  for (i = 0; i < lengthOfarray; i++) {
    arr[i] = -1;
  }
}

static void fq_flow_add_tail(struct fq_flow_head *head, struct fq_flow *flow) {
  if (head->first)
    head->last->next = flow;
  else
    head->first = flow;
  head->last = flow;
  flow->next = NULL;
}


static void Promotecoflows(struct fq_flow_head *old, struct fq_flow_head *new,
                           struct fq_flow_head *coflowhead,
                           struct fq_flow *flow, struct fq_flow *coflow,
                           unsigned arr[]) {
  printk("In Promotecoflows \n");

  // fq_flow_add_tail (coflowhead, flow);	//first flow identfied as part of
  // co-flow set will be added to co-flow set
  coflow = flow;

  struct fq_flow *temp = NULL;

  flow = new->first;
  int flag = 0;

loop:
  while (flow) {
    // check if f->next is NULL

    if (flow->next == NULL) break;

    int lengthOfarray = 0;

    int i;

    for (i = 0; i < (sizeof(arr) / sizeof(arr[0])); i++) {
      lengthOfarray++;
    }

    i = valuePresentInArray(flow->next->socket_hash, arr, lengthOfarray);
    if (i != -1) {
      // we detect the flow in the new flow set and then add to the co-flow set
      temp = flow->next;

      flow->next = flow->next->next;

      fq_flow_add_tail(coflowhead, temp);

      printk("In add co-flow hash of flow value : %u \n ",

             flow->socket_hash);
    }

    flow = flow->next;
  }

  temp = NULL;

  if (!flow && !flag) {
    flow = old->first;
    flag = 1;
    goto loop;
  }
  flow = coflow;
  printk("In add co-flow  hash of flow value : %u \n ", flow->socket_hash);
}
