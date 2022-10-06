/**
 * WIP: This is the main file that I am working on
 */

#include <assert.h>
#include <stdlib.h>
#include "../Test/TestUtilities.h"

// Used for enabling logs
#define DEBUG 0
char debugLog[100] = {'\0'};

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
  void (*toBeDataTypeOf)(DATATYPE expectedType);
  void (*toHaveSocketHashAtIndexOf)(int expectedSocketHash, int expectedIndex);
  void (*toContainSocketHashOf)(int expectedSocketHash);
  void (*toHaveLengthOf)(int expectedLength);
  void (*toEqualFlowListOf)(struct fq_flow *expectedListHeadectedLength);
  void (*toEqualNumberValueOf)(int number);
} expectObj;

// Global configurations
expectObj *eo; // initialize global expect object
int totalTestsNumber = 0;
int currentTestNumber = 0;
int failedTestsCount = 0;

// *** LOG FUNCTIONS ***

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
    printf("%s", debugLog);
  }
}

// *** COLOR FUNCTIONS ***

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

// *** TEST PROGRESS PRINT FUNCTIONS ***

void printTestingSessionStartText()
{
  magenta();
  printf("\n*----------------------- <UNIT TESTING SESSION> -----------------------*\n\n");
  reset();
}

void printTestingSessionEndText()
{
  magenta();
  printf("\n*----------------------- <END OF SESSION> -----------------------*\n\n");
  reset();
}

void printTestingSuiteStartText(char *testSuiteName)
{
  magenta();
  printf("\n**---------- <Test Suite: %s> ----------**\n\n", testSuiteName);
  reset();
}

void printTestingSuiteEndText()
{
  magenta();
  printf("**------------------------------**\n\n\n");
  reset();
}

void printSingleTestStartText(char *testName, int curTestNum, int totTestsNum)
{
  blue();
  printf("<----- Test %d: [%s] ----->\n", curTestNum, testName);
  reset();
}

void printSingleTestPassText(int curTestNum)
{
  green();
  printf("<----- Passed Test %d ----->\n\n", currentTestNumber);
  reset();
}

void printSingleTestFailText(int curTestNum)
{
  red();
  printf("<----- Failed Test %d ----->\n\n", currentTestNumber);
  reset();
}

int checkForTestFail(int testHasFailed, int curTestNum)
{
  if (testHasFailed)
  {
    printSingleTestFailText(curTestNum);
    failedTestsCount++;
    return 1;
  }

  return 0;
}

void printTestingSessionResult()
{
  cyan();
  printf("\n***** Test Result: ");
  green();
  printf("%d passed", (totalTestsNumber - failedTestsCount));
  printf(", ");
  red();
  printf("%d failed ", failedTestsCount);
  cyan();
  printf("*****\n");
}

// *** TEST HELPER FUNCTIONS ***

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
  printf("%s\n", (char *)getDataTypeString(eo->dataObjType));
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

int getFlowListLength(struct fq_flow *listHead)
{
  struct fq_flow *ptr = listHead;

  int listLength = 0;
  while (ptr != NULL)
  {
    listLength++;
    ptr = ptr->next;
  }

  return listLength;
}

// *** EXPECT TEST FUNCTIONS ***

/**
 * Expect function. This is the building block for calling the toBe... functions.
 */
expectObj *expect(void *obj)
{
  totalTestsNumber++;

  eo->dataObj = obj;
  return eo;
}

void toBeDataTypeOf(DATATYPE expectedType)
{
  int testFailed = 0;

  currentTestNumber++;
  printSingleTestStartText("toBeDataTypeOf", currentTestNumber, totalTestsNumber);

  printf("eo->dataObjType: %d\n", eo->dataObjType);
  printf("Expected Type: %d\n", expectedType);
  testFailed = eo->dataObjType != expectedType;
  if (checkForTestFail(testFailed, currentTestNumber))
    return;

  printSingleTestPassText(currentTestNumber);
}

// TODO:
void toHaveSocketHashAtIndexOf(int expectedSocketHash, int expectedIndex)
{
  int testFailed = 0;

  currentTestNumber++;
  printSingleTestStartText("toHaveSocketHashAtIndexOf", currentTestNumber, totalTestsNumber);

  printSingleTestPassText(currentTestNumber);
}

// TODO:
void toContainSocketHashOf(int expectedSocketHash)
{
  int testFailed = 0;

  currentTestNumber++;
  printSingleTestStartText("toContainSocketHashOf", currentTestNumber, totalTestsNumber);

  printSingleTestPassText(currentTestNumber);
}

/**
 * Tests whether the list has the same length as the given number.
 */
void toHaveLengthOf(int expectedLength)
{
  int testFailed = 0;

  currentTestNumber++;
  printSingleTestStartText("toHaveLengthOf", currentTestNumber, totalTestsNumber);

  // Get length of FlowList
  struct fq_flow *ptr = (struct fq_flow *)eo->dataObj;
  int listLength = getFlowListLength(ptr);
  printf("List Length: %d\n", listLength);
  printf("Expected Length: %d\n", expectedLength);

  // Check that the FlowList is of the given length
  testFailed = listLength != expectedLength;
  if (checkForTestFail(testFailed, currentTestNumber))
    return;

  printSingleTestPassText(currentTestNumber);
}

/**
 * Socket hash is used to check the equality of two flow lists. If
 * socket hash is not unique, then we needed to use another variable
 * for unique identification.
 */
void toEqualFlowListOf(struct fq_flow *expectedListHead)
{
  int testFailed = 0;

  currentTestNumber++;
  printSingleTestStartText("toEqualFlowListOf", currentTestNumber, totalTestsNumber);

  // Get flow list from stored data object
  struct fq_flow *ptr1 = (struct fq_flow *)eo->dataObj;

  // get expected flow list
  struct fq_flow *ptr2 = expectedListHead;

  // Check for length equality
  testFailed = getFlowListLength(ptr1) != getFlowListLength(ptr2);
  if (checkForTestFail(testFailed, currentTestNumber))
    return;

  // Check for element equality
  while (ptr1 != NULL && ptr2 != NULL)
  {
    testFailed = ptr1->socket_hash != ptr2->socket_hash;
    if (checkForTestFail(testFailed, currentTestNumber))
      return;

    ptr1 = ptr1->next;
    ptr2 = ptr2->next;
  }

  printSingleTestPassText(currentTestNumber);
}

void toEqualNumberValueOf(int number)
{
  int testFailed = 0;

  currentTestNumber++;
  printSingleTestStartText("toEqualNumberValueOf", currentTestNumber, totalTestsNumber);

  printf("Data Object Number Value: %d\n", (int)eo->dataObj);
  printf("Expected Number Value: %d\n", number);

  testFailed = ((int)eo->dataObj) != number;
  if (checkForTestFail(testFailed, currentTestNumber))
    return;

  printSingleTestPassText(currentTestNumber);
}
