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

void testpromotecoflows(struct Qdisc *sch, struct fq_sched_data *q)
{

  struct fq_flow f;
  struct fq_flow g;
  struct fq_flow h;
  struct fq_flow i;
  struct fq_flow j;
  struct fq_flow coflow;

  f.sk = 1;
  g.sk = 2;
  h.sk = 3;
  i.sk = 4;
  j.sk = 5;

  f.socket_hash = f.sk + 10;

  g.socket_hash = g.sk + 10;

  i.socket_hash = i.sk + 10;

  h.socket_hash = h.sk + 10;

  j.socket_hash = j.sk + 10;

  fq_flow_add_tail(&q->new_flows, &f);

  fq_flow_add_tail(&q->new_flows, &g);

  fq_flow_add_tail(&q->new_flows, &h);

  // fq_flow_add_tail(&q->old_flows, &j);

  unsigned arr[] = {11, 12};

  Promotecoflows(&q->old_flows, &q->new_flows, &q->co_flows, &f, &coflow, arr);
}

void testfq(struct Qdisc *sch, struct fq_sched_data *q)

{

  int vArraytest = arrayTestFunction();
  if (vArraytest)
    printk("Array test  Passed");
  else
    printk("Array test Failed");

  int vResettest = resetTestFunction();

  if (vResettest)
    printk("Reset Array test  Passed");
  else
    printk("Reset Arraytest Failed");

  testpromotecoflows(sch, q);
}