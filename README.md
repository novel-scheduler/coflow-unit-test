# Coflow Unit Test


### IMPORTANT FILES
- /Test/TestUtilities.h -> main scheduler implementation of kernel logic in user space
- /main.c -> main testing area 


```
make clean
make main
./main (runs executable)
```

- The above commands will run the test inside /main.c

Inside main() of main.c, you can comment/uncomment or add test suites to try out different tests.

### Testing Library
/Util/testing.h is a testing library whose functionalities are used in /main.c. Most of the core testing functionalities come from this file.

### JSON Input
In /Util/enq_dq_operations.json, you can specify a series of enqueue and dequeue operations in a JSON format. 
(The JSON functions are from [json-c](https://github.com/json-c/json-c))

### Coflow IDs
In /Util/coflow_ids.json, you can specify the co flow ids. These ids will be used to determine whether a flow is considered a co flow. (This is only for the user space)

### Pure
The /pure directory contains kernel_space.c, which is the scheduler code that is meant to be run on the keernel. After making new changes to /Test/TestUtilities.h, you can run a diffing operation on the current and old version of /Test/TestUtilities.h to locate the changes, and apply them to /pure/kernel_space.c. Once the changes are applied, /pure/kernel_space.c is to be compiled into a .ko file that is ready to be loaded to the kernel. 

