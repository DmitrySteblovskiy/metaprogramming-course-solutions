#pragma once

#include <concepts>
#include <functional>
#include <utility>

class PtrsEnum {
public:
  void (*logpt)(void *, size_t) = nullptr;
  void (*destrpt)(void *) = nullptr;
  void *(*coptr)(void *) = nullptr;
};

template <class T> class Spy {
private:
  T value_;
  void *logger_ = nullptr;
  PtrsEnum pointers_;
  size_t counter_ = 0;
  size_t sz_ = 0;
  using BigLogType = void (*)(void *, size_t);

public:
  explicit Spy(T &&value) : value_(std::forward<T>(value)) {}

  T &operator*() { return value_; }

  const T &operator*() const { return value_; }

  struct LogInvoke {
  private:
    Spy<T> *spy_ptr;

  public:
    LogInvoke(Spy<T> *ptr) : spy_ptr(ptr) {}

    ~LogInvoke() {
      if (spy_ptr->counter_ > 0) {
        void *curr = spy_ptr->logger_;
        if (curr != nullptr)
          spy_ptr->pointers_.logpt(curr, spy_ptr->counter_);
        spy_ptr->counter_ = 0;
      }
    }

    T *operator->() { return &spy_ptr->value_; }
  };

  LogInvoke operator->() {
    ++counter_;
    return LogInvoke(this);
  }

  Spy()
    requires std::default_initializable<T>
      : value_(T{}) {}

  ~Spy() {
    if (logger_ != nullptr)
      pointers_.destrpt(logger_);
  }

  void logExchange(const Spy &anoth) {
    if (logger_ != nullptr) {
      pointers_.destrpt(logger_);
      logger_ = nullptr;
    }
    if (anoth.logger_ != nullptr && anoth.pointers_.coptr != nullptr) {
      logger_ = anoth.pointers_.coptr(anoth.logger_);
    }
    pointers_ = anoth.pointers_;
  }

  Spy(const Spy &anoth)
    requires std::copyable<T>
      : value_(anoth.value_), counter_(0) {
    logExchange(anoth);
  }

  void logSwapper(Spy &&anoth) {
    std::swap(logger_, anoth.logger_);
    std::swap(pointers_, anoth.pointers_);
  }

  Spy(Spy &&anoth)
    requires std::movable<T>
      : value_(std::move(anoth.value_)) {
    logSwapper(std::forward<Spy>(anoth));
  }

  bool operator==(const Spy &anoth) const
    requires std::equality_comparable<T>
  {
    return value_ == anoth.value_;
  }

  Spy &operator=(const Spy &anoth)
    requires std::copyable<T>
  {
    if (this == &anoth)
      return *this;

    value_ = anoth.value_;
    logExchange(anoth);
    counter_ = 0;

    return *this;
  }

  Spy &operator=(Spy &&anoth)
    requires std::movable<T>
  {
    if (this == &anoth)
      return *this;

    value_ = std::move(anoth.value_);
    logSwapper(std::forward<Spy>(anoth));
    counter_ = 0;

    return *this;
  }

  template <std::invocable<size_t> Logger> void setHelperFunc(Logger &&logger) {
    // auto nextres = std::forward<Logger>(logger);
    if (logger_ != nullptr) {
      pointers_.destrpt(logger_);
    }
    logger_ = new std::remove_cvref_t<Logger>(std::forward<Logger>(logger));

    pointers_.logpt = +[](void *logger, size_t n) {
      std::invoke(*static_cast<std::remove_cvref_t<Logger> *>(logger), n);
    };

    pointers_.destrpt = +[](void *logger) {
      delete static_cast<std::remove_cvref_t<Logger> *>(logger);
    };
  }

  template <std::invocable<size_t> Logger>
    requires std::copyable<T> && std::copyable<std::remove_cvref_t<Logger>>
  void setLogger(Logger &&logger) {
    setHelperFunc(logger);

    pointers_.coptr = +[](void *logger) -> void * {
      return new std::remove_cvref_t<Logger>(
          *static_cast<std::remove_cvref_t<Logger> *>(logger));
    };
  }

  template <std::invocable<size_t> Logger>
    requires std::movable<T> && (!std::copyable<T>) &&
             std::move_constructible<std::remove_cvref_t<Logger>>
  void setLogger(Logger &&logger) {
    setHelperFunc(logger);
  }
};