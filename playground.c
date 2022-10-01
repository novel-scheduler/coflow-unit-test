#include <stdio.h>
#include <stdlib.h>
// #include <time.h>
// #include "./Util/rbtree.h"
#include "./Lib/build_bug.h"

#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)

#define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))

// #define testSameType(ptr) __same_type(*(ptr), void)

#define container_of_old(ptr, type, member) ({				\
	void *__mptr = (void *)(ptr);					\
	static_assert(__same_type(*(ptr), ((type *)0)->member) ||	\
		      __same_type(*(ptr), void),			\
		      "pointer type mismatch in container_of()");	\
	((type *)(__mptr - offsetof(type, member))); })

// #define container_of_new(ptr, type, member) (type *)((char *)(ptr)-offsetof(type, member))

struct container
{
  int some_other_data;
  int this_data;
};

void getit(int *x)
{
  struct container *y = container_of_old(x, struct container, this_data);
  printf("%d\n", y->some_other_data); // Should print 5 which is the value of some_other_data in the struct below.
}

int main()
{

  // printf("Timestamp: %ld\n", (uint64_t)time(NULL));
  // printf("Timestamp: %d\n", (int)time(NULL));

  struct container *p;
  struct container *mycontainer;

  mycontainer = (struct container *)malloc(sizeof(*mycontainer));
  mycontainer->some_other_data = 5;
  mycontainer->this_data = 90;

  int *ptr = &mycontainer->this_data;

  printf("%d\n", *ptr);
  getit(ptr);
  free(mycontainer);

  return 0;
}