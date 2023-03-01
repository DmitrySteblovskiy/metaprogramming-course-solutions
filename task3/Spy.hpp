#pragma once
#include <concepts>

template <class T> class Spy {
private:
  T value_;
  void *logger_ = nullptr;

  class LogInvoke {
  private:
    Spy<T> *spy_ptr;
    size_t i = 0;

  public:
    LogInvoke(Spy<T> *ptr) : spy_ptr(ptr) {}

    ~LogInvoke() {}

    T *operator->() { return &spy_ptr->value_; }
  };

public:
  explicit Spy(T);

  T &operator*() { return value_; }

  const T &operator*() const { return value_; }

  Spy()
    requires std::default_initializable<T>
      : value_(T{}) {}

  ~Spy() {
    if (logger_ != nullptr)
      delete_logger(logger_);
  }

  LogInvoke operator->() {
    ++i;
    return LogInvoke(this);
  }

  void setLogger();

  template <std::invocable<unsigned int> Logger>
    requires void
  setLogger(Logger &&logger);
};