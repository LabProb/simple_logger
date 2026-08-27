// main.cpp
// Test program for the logging system (log.h / log.cpp).
// Builds on the API used in the task: getLogger(), getLogger(prefix),
// Logger(prefix), logger(DEBUG) << ..., logger << ... (default INFO).
//
// Compile with: g++ -std=c++11 main.cpp log.cpp -pthread -o test_logger

#include "simple_log.h"

#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

// A worker that logs several messages with different levels and sleeps randomly
void worker(const std::string &prefix, int id, int messages) {
  auto logger = getLogger(prefix);
  std::mt19937_64 rng(static_cast<unsigned long>(std::hash<std::thread::id>{}(
                          std::this_thread::get_id())) ^
                      id);
  std::uniform_int_distribution<int> sleep_ms(1, 50);
  std::uniform_int_distribution<int> level_pick(0, 3);

  for (int i = 0; i < messages; ++i) {
    int lvl = level_pick(rng);
    switch (lvl) {
    case 0:
      logger(INFO) << "worker#" << id << " info message " << i;
      break;
    case 1:
      logger(DEBUG) << "worker#" << id << " debug message " << i;
      break;
    case 2:
      logger(WARNING) << "worker#" << id << " warning message " << i;
      break;
    case 3:
      logger(ERROR) << "worker#" << id << " error message " << i;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms(rng)));
  }

  // Also demonstrate default INFO via operator<< without explicit level
  logger << "worker#" << id << " finished";
}

int main() {
  // Basic single-line log
  auto rootLogger = getLogger();
  rootLogger << "Starting test of logger";

  // Spawn several threads to stress concurrent logging
  const int threadCount = 6;
  const int messagesPerThread = 40;
  std::vector<std::thread> threads;
  threads.reserve(threadCount);

  // Launch threads with different prefixes
  for (int i = 0; i < threadCount; ++i) {
    std::string prefix = (i % 2 == 0) ? ("T" + std::to_string(i))
                                      : std::string(); // some with empty prefix
    threads.emplace_back(worker, prefix, i + 1, messagesPerThread);
  }

  // Also demonstrate creating a local Logger and using it in the main thread
  {
    Logger local{"main_local"};
    local(DEBUG) << "Local logger debug message";
    local(WARNING) << "Local logger warning message";
  }

  // Wait for threads to finish
  for (auto &t : threads) {
    if (t.joinable())
      t.join();
  }

  rootLogger << "All threads joined. Test finished.";

  // Quick smoke tests for API variants used in the task
  {
    // default logger, default INFO
    auto lg = getLogger();
    lg << "Smoke: default INFO message";

    // explicit prefix via factory
    auto lg2 = getLogger("smoke");
    lg2(DEBUG) << "Smoke: debug with prefix";

    // direct construction
    Logger lg3{"constructed"};
    lg3(ERROR) << "Smoke: error from constructed logger";
  }

  return 0;
}
