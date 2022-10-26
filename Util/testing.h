/**
 * WIP: This is the main file that I am working on
 */

#include <json-c/json.h>
#include <json-c/json_util.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
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

void printFlowsList(char *flowsListType, struct fq_flow *flowsListHead)
{
  struct fq_flow *ptr = flowsListHead;
  int index = 1;

  char *indentationWithVerticalLine = " |    ";
  char *plainIndentation = "      ";

  char *emptyLinewithVerticalLine = " |\n";
  char *plainEmptyLine = "\n";

  printf("\n*** Printing %s Flows List ***\n", flowsListType);
  while (ptr != NULL)
  {
    char *indentation;
    char *emptyLine;

    if (ptr->next != NULL)
    {
      indentation = indentationWithVerticalLine;
      emptyLine = emptyLinewithVerticalLine;
    }
    else
    {
      indentation = plainIndentation;
      emptyLine = plainEmptyLine;
    }

    printf("[%d] Flow %s\n", index++, ptr->flowName);
    printf("%s- socket_hash: %u\n", indentation, ptr->socket_hash);
    printf("%s", emptyLine);
    ptr = ptr->next;
  }
  printf("*******************************\n\n");
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

// ***** ENQUEUE & DEQUEUE testing function *****

int validate_enqueue_dequeue_count()
{
}

/**
 * Reads from a json file and executes enqeuue & dequeue.
 */
void enqueue_dequeue(char *fileName, struct Qdisc *sch, struct fq_sched_data *q, struct dequeued_pkt_info **dq_LL_head, struct dequeued_pkt_info **dq_LL_tail)
{
  // Set up flow map
  // struct json_object *flow_map = json_object_new_object();

  // struct json_object *flow_F_socket_hash = json_object_new_int64(1);
  // json_object_object_add(flow_map, "F", flow_F_socket_hash);

  // struct json_object *flow_G_socket_hash = json_object_new_int64(2);
  // json_object_object_add(flow_map, "G", flow_G_socket_hash);

  // struct json_object *flow_H_socket_hash = json_object_new_int64(3);
  // json_object_object_add(flow_map, "H", flow_H_socket_hash);

  // struct json_object *flow_K_socket_hash = json_object_new_int64(4);
  // json_object_object_add(flow_map, "K", flow_K_socket_hash);

  // Create sockets for each flow
  // (remember: sk_hash is used for rb_tree hashing, and sock itself is used for unique identification of flows)
  struct sock *f_sock = (struct sock *)malloc(sizeof(struct sock));
  f_sock->sk_hash = 111;
  struct sock *g_sock = (struct sock *)malloc(sizeof(struct sock));
  g_sock->sk_hash = 222;
  struct sock *h_sock = (struct sock *)malloc(sizeof(struct sock));
  h_sock->sk_hash = 333;
  struct sock *k_sock = (struct sock *)malloc(sizeof(struct sock));
  k_sock->sk_hash = 444;

  // json_object_to_file("test-creation.json", flow_map);

  // Read the enqueue/dequeue operations from JSON file
  FILE *fp;
  char buffer[4096 * 100];

  fp = fopen(fileName, "r");
  fread(buffer, 4096 * 100, 1, fp);
  fclose(fp);

  struct json_object *parsed_json;

  struct json_object *operations;

  struct json_object *operation;
  struct json_object *operation_type;
  struct json_object *operation_flow_id;
  struct json_object *operation_pkt_id;
  struct json_object *operation_pkt_len;
  struct json_object *operation_pkt_tstamp;

  char *operation_type_str;
  char *operation_flow_id_str;
  char *operation_pkt_id_str;
  int operation_pkt_len_int;
  int64_t operation_pkt_tstamp_int;

  parsed_json = json_tokener_parse(buffer);

  // This could fail if buffer is not large enough (failed on 1024)
  json_object_object_get_ex(parsed_json, "operations", &operations);

  // printf("BEFORE: num operation log\n");
  // char *type = json_type_to_name(json_object_get_type(operations));
  // printf("type => %s\n", type);

  size_t num_operations = json_object_array_length(operations);
  printf("*** Number of operations: %lu ***\n\n", num_operations);

  // Validate the number of enqueues/dequeues
  // ex) if the order is EE DDD E, it is INVALID, because dequeue happens one more time than the number of enqueues at a certain point
  int balance_counter = 0;
  for (size_t i = 0; i < num_operations; i++)
  {
    operation = json_object_array_get_idx(operations, i);
    json_object_object_get_ex(operation, "operation", &operation_type);
    operation_type_str = json_object_get_string(operation_type);

    if (strcmp(operation_type_str, "ENQUEUE") == 0)
    {
      balance_counter++;
    }
    else if (strcmp(operation_type_str, "DEQUEUE") == 0)
    {
      balance_counter--;
      if (balance_counter < 0)
      {
        printf("ERROR: Invalid Order Of Operations\n");
        return;
      }
    }
  }

  // printf("AFTER: num operation log\n");
  // printf("\n");
  for (size_t i = 0; i < num_operations; i++)
  {
    operation = json_object_array_get_idx(operations, i);

    // Get values from each packet obj
    json_object_object_get_ex(operation, "operation", &operation_type);
    json_object_object_get_ex(operation, "flow_id", &operation_flow_id);
    json_object_object_get_ex(operation, "pkt_id", &operation_pkt_id);
    json_object_object_get_ex(operation, "plen", &operation_pkt_len);
    json_object_object_get_ex(operation, "tstamp", &operation_pkt_tstamp);

    operation_type_str = json_object_get_string(operation_type);
    printf("operation_type => %s\n", operation_type_str);

    struct sk_buff *to_free = (struct sk_buff *)malloc(sizeof(struct sk_buff));

    if (strcmp(operation_type_str, "ENQUEUE") == 0)
    {
      operation_flow_id_str = json_object_get_string(operation_flow_id);
      operation_pkt_id_str = json_object_get_string(operation_pkt_id);
      operation_pkt_len_int = json_object_get_int(operation_pkt_len);
      operation_pkt_tstamp_int = json_object_get_int(operation_pkt_tstamp);

      // printf("operation_flow_id => %s\n", operation_flow_id_str);
      // printf("operation_pkt_id => %s\n", operation_pkt_id_str);
      // printf("operation_pkt_len => %d\n", operation_pkt_len_int);
      // printf("operation_pkt_tstamp => %d\n", operation_pkt_tstamp_int);

      // construct packet
      struct sk_buff *packet = (struct sk_buff *)malloc(sizeof(struct sk_buff));
      packet->flow_id = operation_flow_id_str;
      packet->pkt_id = operation_pkt_id_str;
      packet->plen = operation_pkt_len_int;
      packet->tstamp = operation_pkt_tstamp_int;

      if (strcmp(packet->flow_id, "F") == 0)
      {
        packet->sk = f_sock;
      }
      else if (strcmp(packet->flow_id, "G") == 0)
      {
        packet->sk = g_sock;
      }
      else if (strcmp(packet->flow_id, "H") == 0)
      {
        packet->sk = h_sock;
      }
      else if (strcmp(packet->flow_id, "K") == 0)
      {
        packet->sk = k_sock;
      }

      // printf("packet sk_hash => %u\n", packet->sk->sk_hash);
      // printf("\n");

      // * ENQUEUE packet
      fq_enqueue(q, packet, sch, &to_free);
    } // END IF OPERATION == "ENQUEUE"
    else
    {
      // * DEQUEUE packet
      fq_dequeue(sch, q, dq_LL_head, dq_LL_tail);
    } // END OF OPERATION == "DEQUEUE"
  }
}

// Test creating a new json obj and writing to file
void test_json_creation()
{
  // struct json_object *obj = json_object_new_object();
  // struct json_object *val = json_object_new_string("NEW STRING");
  // json_object_object_add(obj, "key1", val);

  struct json_object *flow_map = json_object_new_object();
  struct json_object *flow_F_socket_hash = json_object_new_string("1");
  json_object_object_add(flow_map, "F", flow_F_socket_hash);

  // from json_util.h
  json_object_to_file("test-creation.json", flow_map);
}

int getDequeuedInfoListLength(struct dequeued_pkt_info *listHead)
{
  struct dequeued_pkt_info *ptr = listHead;

  int listLength = 0;
  while (ptr != NULL)
  {
    listLength++;
    ptr = ptr->next;
  }

  return listLength;
}

void printDequeuedInfoList(struct dequeued_pkt_info *listHead)
{
  struct dequeued_pkt_info *ptr = listHead;
  int index = 1;

  char *indentationWithVerticalLine = " |    ";
  char *plainIndentation = "      ";

  char *emptyLinewithVerticalLine = " |\n";
  char *plainEmptyLine = "\n";

  printf("\n***** Printing Dequeued Packets Info List *****\n\n");
  while (ptr != NULL)
  {
    char *indentation;
    char *emptyLine;

    if (ptr->next != NULL)
    {
      indentation = indentationWithVerticalLine;
      emptyLine = emptyLinewithVerticalLine;
    }
    else
    {
      indentation = plainIndentation;
      emptyLine = plainEmptyLine;
    }

    printf("[%d] Flow %s\n", index++, ptr->flow_id);
    printf("%s- socket_hash: %u\n", indentation, ptr->socket_hash);
    printf("%s- packet id: %s\n", indentation, ptr->pkt_id);
    printf("%s- packet tstamp: %d\n", indentation, ptr->tstamp);
    printf("%s- packet len: %d\n", indentation, ptr->plen);
    printf("%s", emptyLine);
    ptr = ptr->next;
  }

  printf("*******************************\n\n");
}