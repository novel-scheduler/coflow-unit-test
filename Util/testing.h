#include <assert.h>

typedef struct dummy
{
  int num;
} dummy;

typedef enum
{
  LinkedList,
  Array,
  FlowList,
} DATATYPE;

typedef struct expectObj
{
  void *dataObj;
  DATATYPE dataObjType;
  void (*toContain)(struct expectObj *eo, int num);
  void (*printDataTypeName)(struct expectObj *eo);
} expectObj;

expectObj *expect(expectObj *eo, void *obj)
{
  eo->dataObj = obj;
  return eo;
}

char *getTypeName(DATATYPE type)
{
  switch (type)
  {
  case LinkedList:
  {
    return "LinkedList";
  }
  case Array:
  {
    return "Array";
  }
  case FlowList:
  {
    return "FlowList";
  }
  }
  return "F";
}

void printDataTypeName(expectObj *eo)
{
  printf("<FUNC: printDataTypeName> \n");
  printf("%s\n", (char *)getTypeName(eo->dataObjType));
}

void toHaveAtIndex(expectObj *eo)
{
}

void toContain(expectObj *eo, int num)
{
  printf("<FUNC: toContain> \n");
  assert(eo->dataObjType == num);
}

void toHaveLengthOf(expectObj *eo, int num)
{
}