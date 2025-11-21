# Test Suite for semaphore.hpp and threadmgr.hpp

This test program validates the multithreading management components in the CXXL library: `semaphore.hpp` and `threadmgr.hpp`.

## Test Coverage

### 1. Semaphore Tests

#### cxxlSemaphore
- **Basic Functionality Test** (`testSemaphoreBasic`)
  - Verifies that the semaphore correctly limits concurrent thread count
  - Test scenario: Set maximum 2 concurrent threads, submit 10 tasks
  - Validates that maximum concurrency does not exceed the limit

- **Block Functionality Test** (`testSemaphoreBlock`)
  - Verifies that the `block()` method prevents new threads from acquiring the semaphore
  - Test scenario: Set to block state while a thread holds the semaphore
  - Validates that `release()` can unblock the state

#### cxxlSemaphoreHelper
- **Automatic Management Test** (`testSemaphoreHelper`)
  - Verifies RAII-style automatic release mechanism
  - Tests that the helper object automatically releases the semaphore when leaving scope
  - Ensures no resource leaks occur

### 2. ThreadLimiter Tests

- **Basic Functionality Test** (`testThreadLimiterBasic`)
  - Verifies correct thread pool size limitation and task completion
  - Tests 10 tasks completing with a limit of 4 threads

- **Return Value Test** (`testThreadLimiterReturnValue`)
  - Verifies correct reception of task return values
  - Tests lambda functions with and without parameters
  - Retrieves results via `std::future`

- **Clear Task Test** (`testThreadLimiterClearTask`)
  - Verifies that `clearTask()` clears pending tasks
  - Confirms running tasks complete while pending tasks are cancelled

- **Stress Test** (`stressTestThreadLimiter`)
  - Submits 100 tasks to an 8-thread limiter
  - Validates stability and correctness under high load

### 3. ThreadPool Tests

- **Basic Functionality Test** (`testThreadPoolBasic`)
  - Verifies that the thread pool correctly executes all submitted tasks
  - Tests the `waitAllTask()` waiting mechanism

- **Return Value Test** (`testThreadPoolReturnValue`)
  - Verifies correct handling of different return types (int, string)
  - Tests parameter passing mechanism

- **Clear Task Test** (`testThreadPoolClearTask`)
  - Verifies that `clearTask()` clears the pending task queue

- **Wait and Clear Test** (`testThreadPoolWaitAllTaskAndClear`)
  - Verifies that `waitAllTaskAndClear()` waits for all tasks to complete and cleans up resources
  - Confirms that new tasks cannot be submitted after cleanup

- **Stress Test** (`stressTestThreadPool`)
  - Submits 100 tasks to an 8-thread pool
  - Validates stability under high load

## Build and Execution

### Prerequisites
- C++20 or higher
- CXXL library (including `semaphore.hpp` and `threadmgr.hpp`)
- Compiler with multithreading support

### Compilation
```bash
g++ -std=C++20 main.cpp -o test -lpthread -I<CXXL_INCLUDE_PATH>
```

Or using CMake:
```bash
mkdir build && cd build
cmake ..
make
```

### Running Tests
```bash
./test
```

## Interpreting Test Results

The test program outputs results for each test case:
- `[PASS]` - Test passed
- `[FAIL]` - Test failed (with reason for failure)

A summary is displayed at the end:
```
========================================
測試結果: X passed, Y failed
總計: Z tests
========================================
```

Return values:
- `0` - All tests passed
- `1` - Some tests failed

## Feature Coverage

| Class | Test Item | Feature Description |
|-------|-----------|---------------------|
| **cxxlSemaphore** | Concurrency Limiting | Limits the number of threads accessing resources simultaneously |
| | Block/Release | Pause and resume semaphore usage |
| **cxxlSemaphoreHelper** | RAII Management | Automated wait/release management |
| **ThreadLimiter** | Thread Count Limiting | Dynamically creates threads but limits maximum count |
| | Task Queue | Manages pending task queue |
| | Return Value Handling | Retrieves task results via futures |
| **ThreadPool** | Thread Pool | Reuses a fixed number of threads |
| | Task Scheduling | Assigns tasks to idle threads |
| | Lifecycle Management | Controls thread pool creation and destruction |

## Test Statistics

Total of 13 test cases:
- Semaphore tests: 3
- ThreadLimiter tests: 4
- ThreadPool tests: 4
- Stress tests: 2

## Notes

- Tests use `std::this_thread::sleep_for` to simulate time-consuming operations; actual execution time may vary by system
- Some tests depend on timing and may occasionally fail on heavily loaded systems
- Stress tests create many threads and tasks; recommend running in environments with sufficient resources
- Tests use atomic operations (`std::atomic`) to ensure correctness in multithreaded environments

## Related Files

- [main.cpp](main.cpp) - Main test program
- `semaphore.hpp` - CXXL Semaphore implementation
- `threadmgr.hpp` - CXXL ThreadLimiter and ThreadPool implementation

## License

This test program is part of the CXXL project.
