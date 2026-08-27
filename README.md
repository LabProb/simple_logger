# simple_logger

`simple_logger` is a small C++20 library for synchronous, line-oriented console logging.
It formats an entry when the temporary returned from the logging expression is destroyed.

```cpp
#include "simple_log.h"

auto logger = getLogger("service");
logger(INFO) << "started on port " << 8080;
logger.log(ERROR) << "request failed";
logger << "uses INFO by default";
```

Each line has this form:

```text
2026-08-27 12:34:56.123; INFO; service(123456): started on port 8080
```

## API and lifetime

`Logger` is a cheap value type containing immutable configuration: a prefix and an output
stream. `Logger` can be copied and used concurrently. A `LogEntry` snapshots the prefix when
it is created, so this is safe:

```cpp
std::optional<LogEntry> entry;
{
    Logger logger{"scope"};
    entry.emplace(logger(DEBUG));
}
*entry << "the logger object is already gone";
```

The default output is `std::cout`. For tests or an already-owned file stream, pass a stream:

```cpp
std::ostringstream capture;
Logger logger{"test", capture};
```

The caller owns that stream and must keep it alive until the `Logger` and every outstanding
`LogEntry` that uses it have been destroyed.

`LogEntry` is move-only. Its destructor is `noexcept`: failures while formatting or writing are
swallowed so that logging cannot terminate an application during stack unwinding. Insertion into
an entry can still throw at the insertion expression, as ordinary iostream use can.

## Thread safety

All writes are protected by one process-wide mutex. A complete formatted line is therefore not
interleaved with another logger line, including when multiple `Logger` objects target the same
stream. Formatting happens before the lock, keeping the critical section short. The logger is
synchronous; a slow output stream blocks only while its final line is written.

Logging from ordinary static destructors is best avoided. The mutex deliberately outlives static
destruction, but the lifetime of `std::cout` and user-provided streams cannot be guaranteed at
program shutdown.

## Build, test, and install

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
cmake --install build --prefix install
```

The exported CMake target is `simple_logger::simple_logger`; the installed example binary remains
`Logger` for compatibility.
