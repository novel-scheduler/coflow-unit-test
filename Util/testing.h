#include <assert.h>
#include "../Test/TestUtilities.h"

// Used for enabling logs
#define DEBUG 1
char log[100] = {'\0'};

typedef struct dummy
{
  int num;
} dummy;

typedef enum
{
  LinkedList, // generic linked list, not really needed
  Array,      // generic array, not really needed
  FlowList,   // same as LL, but more specific
} DATATYPE;

typedef struct expectObj
{
  void *dataObj;
  DATATYPE dataObjType;
  void (*printDataTypeString)();
  void (*toContain)(int num);
  void (*toBeOfDateType)(DATATYPE type);
  void (*toHaveLengthOf)(int num);
} expectObj;

// initialize global expect object
expectObj *eo;

/**
 * Prints the function name if DEBUG is true (1).
 */
void logFunctionCall(char *funcName)
{
  if (DEBUG)
  {
    printf("<FUNC: %s> \n", (char *)funcName);
  }
}

void logMessage()
{
  if (DEBUG)
  {
    printf("%s", log);
  }
}

/**
 * Expect function that works like Jest.
 */
expectObj *expect(void *obj)
{
  eo->dataObj = obj;
  return eo;
}

/**
 * Gets the string value of the given type.
 */
char *getDataTypeString(DATATYPE type)
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

/**
 * Prints the name of the expect object's data object type.
 */
void printDataTypeString()
{
  logFunctionCall("printDataTypeString");
  printf("%s\n", (char *)getDataTypeString(eo->dataObjType));
}

void toBeOfDateType(DATATYPE type)
{
  logFunctionCall("toBeOfDateType");
  assert(eo->dataObjType == type);
}

// TODO:
void toHaveAtIndex(expectObj *eo)
{
  logFunctionCall("toHaveAtIndex");
}

// TODO:
void toContain(int num)
{
  logFunctionCall("toContain");
}

/**
 * Tests whether the data type is a list, and then tests
 * whether the list has the same length as the given number.
 */
void toHaveLengthOf(int num)
{
  logFunctionCall("toHaveLengthOf");

  // Assert that the type is FlowList (LL) // TODO:
  assert(eo->dataObjType == FlowList);

  // Get length of FlowList
  struct fq_flow *ptr = (struct fq_flow *)eo->dataObj;
  int listLength = 0;
  while (ptr != NULL)
  {
    listLength++;
    sprintf(log, "socket_hash: %d\n", ptr->socket_hash);
    logMessage();
    ptr = ptr->next;
  }
  sprintf(log, "LENGTH: %d", listLength);
  logMessage();

  // Assert that the FlowList is of the given length
  assert(listLength == num);
}

/**
 * Frees a LL by iterating through it.
 */
void freeFlowList(struct fq_flow *listHead)
{
  struct fq_flow *ptr = listHead;
  struct fq_flow *tmp = NULL;
  while (ptr != NULL)
  {
    tmp = ptr;
    ptr = ptr->next;
    free(tmp);
  }
}