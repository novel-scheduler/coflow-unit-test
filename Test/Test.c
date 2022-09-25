#include "./TestUtilities.h"

int arrayTestFunction(void)
{

  int retVal;

  u32 arr[] = {1, 2, 3, 4, 5, 7, 8, 9, 12, 13, 15, 16};

  int lengthOfarray = 0;

  int i;

  for (i = 0; i < (sizeof(arr) / sizeof(arr[0])); i++)
  {
    lengthOfarray++;
  }

  u32 sampleValright;

  get_random_bytes(&sampleValright, sizeof(sampleValright));

  sampleValright %= 5;

  sampleValright++;

  u32 sampleVal;

  get_random_bytes(&sampleVal, sizeof(sampleVal));

  sampleVal %= 16;

  u32 sampleValwrng;

  get_random_bytes(&sampleValwrng, sizeof(sampleValwrng));

  sampleValwrng %= 10;

  sampleValwrng = sampleValwrng + plimit;

  retVal = valuePresentInArray(sampleValright, arr, lengthOfarray);
  if (retVal == -1)
    return 0;

  // printk("return Value if given right value of %d is %d\n", sampleValright,retVal);

  // retVal = valuePresentInArray(sampleVal, arr, lengthOfarray);

  // printk("return Value if given random value  of %d is %d\n", sampleVal, retVal);

  retVal = valuePresentInArray(sampleValwrng, arr, lengthOfarray);
  if (retVal != -1)
    return 0;

  return 1;

  // printk("return Value if given wrong value of %d  is %d\n", sampleValwrng,retVal);
}

int resetTestFunction(void)
{
  int retVal;

  u32 arr[] = {1, 2, 3, 4, 5, 7, 8, 9, 12, 13, 15, 16};

  int lengthOfarray = 0;

  int i;

  for (i = 0; i < (sizeof(arr) / sizeof(arr[0])); i++)
  {
    lengthOfarray++;
  }

  resetFlowid(arr, lengthOfarray);

  for (i = 0; i < lengthOfarray; i++)
  {

    if (arr[i] != -1)
      return 0;
  }
  return 1;
}

int testpromotecoflows(struct Qdisc *sch, struct fq_sched_data *q, unsigned arr[], int lengthOfarray)
{

  struct fq_flow f;
  struct fq_flow g;
  struct fq_flow h;
  struct fq_flow i;
  struct fq_flow j;
  struct fq_flow coflow;
  struct fq_flow_head oldFlowshead;
  struct fq_flow_head newFlowshead;
  struct fq_flow_head coFlowshead;

  f.sk = 1;
  g.sk = 2;
  h.sk = 3;
  i.sk = 4;
  j.sk = 5;
  coflow.sk = 6;
  coFlowshead = q->co_flows;
  oldFlowshead = q->old_flows;
  newFlowshead = q->new_flows;

  struct fq_flow_head *coFlowsheadptr = &(q->co_flows);

  struct fq_flow_head *newFlowsheadptr = &(q->new_flows);

  struct fq_flow_head *oldFlowsheadptr = &(q->co_flows);

  f.socket_hash = f.sk + 10;

  // printk("value of f %u\n", f.socket_hash);

  g.socket_hash = g.sk + 10;

  // printk("value of g %u\n", g.socket_hash);

  i.socket_hash = i.sk + 10;

  // printk("value of i %u\n", i.socket_hash);

  h.socket_hash = h.sk + 10;

  // printk("value of h %u\n", h.socket_hash);

  j.socket_hash = j.sk + 10;

  // printk("value of j %u\n", j.socket_hash);

  coflow.socket_hash = coflow.sk + 10;

  fq_flow_add_tail(&q->new_flows, &f);

  fq_flow_add_tail(&q->new_flows, &g);

  fq_flow_add_tail(&q->new_flows, &j);

  // fq_flow_add_tail(&q->co_flows, &coflow);

  fq_flow_add_tail(&q->old_flows, &h);

  fq_flow_add_tail(&q->old_flows, &i);

  // unsigned arr[] = {7921,7922,7923};

  // unsigned arr[] = {7925};

  // unsigned arr[] = {7921};

  // unsigned arr[] = {7922};

  // unsigned arr[] = {7923};

  // unsigned arr[] = {7924};

  // unsigned arr[] = {7926};

  // unsigned superarr[8][5] = {{7923,7921,7922,7924,7925}, {7921,7922,7923}, {7921}, {7922}, {7925}, {7923}, {7924},{7926}}  ;

  // 7921

  // 7921->7922->7925->NULL
  // 7922->7925->NULL, 7921->coflow

  // 7922
  // 7921->7922->7925->NULL
  // 7921->7925->NULL, 7922->coflow

  // 7923->NULL
  // 7926->FULL
  // 7921,7922,7923
  // 7925->NULL

  /*int lengthOfarray = 0;
  int a;
  for (a = 0; a < (sizeof(arr) / sizeof(arr[0])); a++) {
    lengthOfarray++;
  }*/

  // printk("size of array %d\n", lengthOfarray);

  int varx = Promotecoflows(&q->old_flows, &q->new_flows, &q->co_flows, &f, &coflow, arr, lengthOfarray);
  // singleton case as unit test

  // printk("promote co flows done\n");

  struct fq_flow *coflowptr = coFlowsheadptr->first;

  // printk("value of coflows %u\n", coflowptr->socket_hash);

  // coFlowsheadptr->first = coflowptr->next;

  // coflowptr = coflowptr->next;

  if (coflowptr)
    // printk("value of coflows next %u\n", coflowptr->socket_hash);

  if (!coflowptr)
  {

    // printk("there are no co flows to promote \n");
    return 1;
  }

  while (coflowptr)
  {

    // printk("value of coflows %u\n", coflowptr->socket_hash);

    int retVal = valuePresentInArray(coflowptr->socket_hash, arr, lengthOfarray);

    if (retVal == -1)
    {
      // printk("test failed \n");
      return 0;
    }

    coflowptr = coflowptr->next;
  }

  // printk ("promote co flows test success");
  return 1;
}