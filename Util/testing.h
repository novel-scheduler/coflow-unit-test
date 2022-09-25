/**
WIP: This is the main file that I am working on
*/

#include <assert.h>
#include <stdlib.h>
#include "../Test/TestUtilities.h"

// Used for enabling logs
#define DEBUG 0
char log[100] = {'\0'};

// Colors
#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

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
  void (*toBeOfDataType)(DATATYPE type);
  void (*toHaveLengthOf)(int num);
} expectObj;

// Global configurations
expectObj *eo; // initialize global expect object
int totalTestsNumber = 0;
int currentTestNumber = 0;

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

void red()
{
  printf("%s", KRED);
}

void green()
{
  printf("%s", KGRN);
}

void blue()
{
  printf("%s", KBLU);
}

void cyan()
{
  printf("%s", KCYN);
}

void magenta()
{
  printf("%s", KMAG);
}

void reset()
{
  printf("\033[0m");
}

void printTestingSessionStartText()
{
  magenta();
  printf("\n*------------- <Unit Testing Session> -------------*\n\n");
  reset();
}

void printTestingSessionEndText()
{
  magenta();
  printf("\n*------------- <END Of Session> -------------*\n\n");
  reset();
}

void printSingleTestStartText(char *testName, int curTestNum, int totTestsNum)
{
  blue();
  printf("<--- Test %d out of %d: [%s] --->\n", curTestNum, totTestsNum, testName);
  reset();
}

void printSingleTestPassText(int curTestNum)
{
  green();
  printf("<--- Passed Test %d --->\n\n", currentTestNumber);
  reset();
}

void printSingleTestFailText(int curTestNum)
{
  red();
  printf("<--- Failed Test %d --->\n\n", currentTestNumber);
  reset();
}

void checkForTestFail(int testHasFailed, int curTestNum)
{
  if (testHasFailed)
  {
    printSingleTestFailText(curTestNum);
    abort();
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

void toBeOfDataType(DATATYPE type)
{
  logFunctionCall("toBeOfDataType");

  currentTestNumber++;
  printSingleTestStartText("toBeOfDataType", currentTestNumber, totalTestsNumber);

  printf("eo->dataObjType: %d\n", eo->dataObjType);
  printf("Expected Type: %d\n", type);
  assert(eo->dataObjType == type);

  printSingleTestPassText(currentTestNumber);
}

// TODO:
void toHaveAtIndex(expectObj *eo)
{
  logFunctionCall("toHaveAtIndex");

  currentTestNumber++;
  printSingleTestStartText("toHaveAtIndex", currentTestNumber, totalTestsNumber);

  printSingleTestPassText(currentTestNumber);
}

// TODO:
void toContain(int num)
{
  logFunctionCall("toContain");

  currentTestNumber++;
  printSingleTestStartText("toContain", currentTestNumber, totalTestsNumber);

  printSingleTestPassText(currentTestNumber);
}

/**
 * Tests whether the data type is a list, and then tests
 * whether the list has the same length as the given number.
 */
void toHaveLengthOf(int num)
{
  logFunctionCall("toHaveLengthOf");
  int testFailed = 0;

  currentTestNumber++;
  printSingleTestStartText("toHaveLengthOf", currentTestNumber, totalTestsNumber);

  // Get length of FlowList
  struct fq_flow *ptr = (struct fq_flow *)eo->dataObj;
  int listLength = 0;
  while (ptr != NULL)
  {
    listLength++;
    ptr = ptr->next;
  }
  printf("List Length: %d\n", listLength);
  printf("Expected Length: %d\n", num);

  // Check that the FlowList is of the given length
  testFailed = listLength != num;
  checkForTestFail(testFailed, currentTestNumber);

  printSingleTestPassText(currentTestNumber);
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

/**
 * Iterates and prints entire linked list
 */
void printEntireLinkedList(struct fq_flow *listHead)
{
  struct fq_flow *ptr = listHead;
  int index = 0;
  while (ptr != NULL)
  {
    printf("Element [%d] of Linked List:\n", index++);
    printf("    socket_hash -> %u\n", (unsigned int)ptr->socket_hash); // TODO: test
    ptr = ptr->next;
  }
}