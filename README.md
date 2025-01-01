# eventloop

> A header only asyncronous runtime for C

Define asyncronous computation in tasks and enque them on the runtime to executed. The runtime spawns multiple threads to work off the queue. Each task can enque new tasks and depend their execution on them.

# Usage

Copy the [eventloop.h file](/eventloop.h) in any include directory of your project. Use the header for the declarations. One compilation unit of your project must be used to compile the project. Simply include the header with the `EL_EVENTLOOP_IMPL` macro defined to use the current compilation unit for compiling the library like:

```C
#define EL_EVENTLOOP_IMPL
#include "eventloop.h"
```

This runtime can be used to enable asyncronous computation in any part of your application but is usually set up in the main function. Where an asyncronous main function is enqued and the runtime is started. This asyncronous main function enques new tasks when desired.

The library has a few macros to reduce the boilerplate code when creating asyncronous tasks instead of regular functions.

Check out the [examples](/examples/) to see how the library can be used.

# Test

To run the tests compile the `test.c` file and run it.

```sh
cc test.c && ./a.out
```
