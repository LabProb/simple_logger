#include "simple_log.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

// М'ютекс для атомарного виводу одного рядка в консоль
static std::mutex &get_cout_mutex() {
  static std::mutex m;
  return m;
}

// Перетворення LogLevel у рядок
static const char *level_to_string(LogLevel lvl) {
  switch (lvl) {
  case INFO:
    return "INFO";
  case DEBUG:
    return "DEBUG";
  case WARNING:
    return "WARNING";
  case ERROR:
    return "ERROR";
  default:
    return "UNKNOWN";
  }
}

static std::string current_datetime_string() {
  using namespace std::chrono;
  auto now = system_clock::now();

  // seconds part for std::put_time
  auto now_time_t = system_clock::to_time_t(now);
  std::tm tm;
#if defined(_MSC_VER)
  localtime_s(&tm, &now_time_t);
#else
  localtime_r(&now_time_t, &tm);
#endif

  // milliseconds part
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setw(3)
      << std::setfill('0') << ms.count();
  return oss.str();
}

// ---------------- Logger ----------------

Logger::Logger(const std::string &prefix) : prefix_(prefix) {}

LogEntry Logger::operator()(LogLevel level) const {
  return LogEntry(*this, level);
}

LogEntry Logger::operator<<(const std::string &msg) const {
  LogEntry e(*this, INFO);
  e << msg;
  return e;
}

Logger getLogger() { return Logger(); }

Logger getLogger(const std::string &prefix) { return Logger(prefix); }

// ---------------- LogEntry ----------------

LogEntry::LogEntry(const Logger &logger, LogLevel level)
    : logger_(logger), level_(level), ss_(), moved_from_(false) {}

LogEntry::LogEntry(LogEntry &&other) noexcept
    : logger_(other.logger_), level_(other.level_), ss_(), moved_from_(false) {
  ss_ << other.ss_.str();
  other.moved_from_ = true;
}

LogEntry::~LogEntry() {
  if (moved_from_)
    return; // якщо переміщено — нічого не робимо

  // Формуємо фінальний рядок
  std::ostringstream out;
  out << current_datetime_string() << "; " << level_to_string(level_) << "; ";

  // Префікс (якщо є) і id потоку
  if (!logger_.prefix().empty()) {
    out << logger_.prefix();
  }
  // thread id як числове представлення (через hash)
  std::hash<std::thread::id> hasher;
  auto tid_hash = hasher(std::this_thread::get_id());
  out << "(" << tid_hash << "): ";

  // Додаємо повідомлення
  out << ss_.str();

  // Гарантуємо, що весь рядок виводиться атомарно
  std::lock_guard<std::mutex> lock(get_cout_mutex());
  std::cout << out.str() << std::endl;
}

LogEntry &LogEntry::operator<<(Manip manip) {
  manip(ss_);
  return *this;
}
