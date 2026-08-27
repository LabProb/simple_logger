#include "simple_log.h"

#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {

#define CHECK(condition)                                                        \
  do {                                                                          \
    if (!(condition)) {                                                        \
      return 1;                                                                 \
    }                                                                           \
  } while (false)

std::size_t line_count(const std::string &text) {
  std::size_t count = 0;
  for (const char character : text) {
    count += character == '\n';
  }
  return count;
}

} // namespace

int main() {
  std::ostringstream output;
  Logger logger{"unit", output};

  logger(INFO) << "first";
  logger.log(ERROR) << "second";
  logger << "third";
  logger(INFO) << "fourth" << std::endl;

  const auto initial = output.str();
  CHECK(initial.find("; INFO; unit(") != std::string::npos);
  CHECK(initial.find(": first\n") != std::string::npos);
  CHECK(initial.find("; ERROR; unit(") != std::string::npos);
  CHECK(initial.find(": second\n") != std::string::npos);
  CHECK(initial.find(": third\n") != std::string::npos);
  CHECK(initial.find(": fourth\n\n") != std::string::npos);

  std::optional<LogEntry> entry;
  {
    Logger temporary{"snapshot", output};
    entry.emplace(temporary(DEBUG));
  }
  *entry << "logger may be destroyed before its entry";
  entry.reset();
  CHECK(output.str().find("; DEBUG; snapshot(") != std::string::npos);

  std::ostringstream concurrent_output;
  Logger concurrent{"parallel", concurrent_output};
  constexpr int thread_count = 8;
  constexpr int messages_per_thread = 50;
  std::vector<std::thread> threads;
  threads.reserve(thread_count);
  for (int thread = 0; thread < thread_count; ++thread) {
    threads.emplace_back([&concurrent, thread] {
      for (int message = 0; message < messages_per_thread; ++message) {
        concurrent(WARNING) << thread << ':' << message;
      }
    });
  }
  for (auto &thread : threads) {
    thread.join();
  }
  CHECK(line_count(concurrent_output.str()) == thread_count * messages_per_thread);
}
