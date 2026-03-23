#ifndef SIMPLE_LOG_H
#define SIMPLE_LOG_H

#include <sstream>
#include <string>

// Фіксовані рівні логування
enum LogLevel { INFO, DEBUG, WARNING, ERROR };

class Logger;

// Тимчасовий об'єкт, який збирає повідомлення і виводить його в деструкторі
class LogEntry {
public:
  LogEntry(const Logger &logger, LogLevel level);
  LogEntry(const LogEntry &) = delete;
  LogEntry &operator=(const LogEntry &) = delete;
  LogEntry(LogEntry &&) noexcept;
  ~LogEntry();

  // Додаємо будь-який тип, який підтримує operator<< у std::ostream
  template <typename T> LogEntry &operator<<(const T &value) {
    ss_ << value;
    return *this;
  }

  // Спеціалізація для маніпуляторів (std::endl тощо)
  using Manip = std::ostream &(*)(std::ostream &);
  LogEntry &operator<<(Manip manip);

private:
  const Logger &logger_;
  LogLevel level_;
  std::ostringstream ss_;
  bool moved_from_;
};

// Основний клас Logger
class Logger {
public:
  explicit Logger(const std::string &prefix = std::string());
  ~Logger() = default;

  // Повертає тимчасовий LogEntry з вказаним рівнем
  LogEntry operator()(LogLevel level) const;

  // За замовчуванням використовує INFO
  LogEntry operator<<(const std::string &msg) const;
  template <typename T> LogEntry operator<<(const T &value) const {
    LogEntry e(*this, INFO);
    e << value;
    return e;
  }

  const std::string &prefix() const noexcept { return prefix_; }

private:
  std::string prefix_;
};

// Функція-фабрика, як у main.cpp
Logger getLogger();
Logger getLogger(const std::string &prefix);

#endif // SIMPLE_LOG_H
