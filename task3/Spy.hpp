#pragma once

#include <concepts>
#include <functional>
#include <utility>

template <class T> class Spy {
private:
  T value_;
  void *logger_ = nullptr;
  unsigned int counter = 0;
  unsigned int sz = 0;                         //
  void (*logpt)(void *, unsigned int) = nullptr; //
  void (*destrpt)(void *) = nullptr;
  void *(*coptr)(void *) = nullptr;

public:
  using BigLogType = void (*)(void *, unsigned int);

  class LogInvoke {
  private:
    Spy<T> *spy_ptr;
    unsigned int i = 0;
    T *t;
    void *logger;
    BigLogType log;
    unsigned int *curr;

  public:
    // explicit Spy(T); // exp?
    explicit Spy(T &&t) : value_(std::forward<T>(t)) {}

    T &operator*() { return value_; }

    const T &operator*() const { return value_; }

    // T *operator->() { return &spy_ptr->value_; }
    LogInvoke(T *t, void *logger, BigLogType log, unsigned int *counter)
        : t(t), logger(logger), log(log), curr(counter) {}

    ~LogInvoke() {
      if (*curr > 0) {
        if (logger != nullptr) {
          log(logger, *curr);
        }
        *curr = 0;
      }
    }

    T *operator->() { return t; }
  };

  Spy()
    requires std::default_initializable<T>
      : value_(T{}) {}

  ~Spy() {
    if (destrpt != nullptr) {
      destrpt(logger_);
    }
  }

  LogInvoke operator->() {
    ++counter;
    return LogInvoke{&value_, logger_, logpt, &counter};
  }

  bool operator==(const Spy &anoth) const
    requires std::equality_comparable<T>
  {
    return value_ == anoth.value_;
  }

  //void setLogger();

  Spy(const Spy &anoth)
    requires std::copyable<T>
      : value_(anoth.value_), counter(0), logpt(anoth.logpt),
        destrpt(anoth.destrpt), coptr(anoth.coptr) {
    if (anoth.logger_ != nullptr) {
      logger_ = anoth.coptr(anoth.logger_);
    }
  }

  /*
    void SetSomePt(Spy &&anoth) {
      logpt = anoth.logpt;
      destrpt = anoth.destrpt;
      coptr = anoth.coptr;
    }*/

  Spy &operator=(const Spy &anoth)
    requires std::copyable<T>
  {
    if (this == &anoth) {
      return *this;
    }
    if (destrpt != nullptr) {
      destrpt(logger_);
    }

    value_ = anoth.value_;
    if (anoth.logger_ != nullptr) {
      logger_ = anoth.coptr(anoth.logger_);
    } else {
      logger_ = nullptr;
    }

    logpt = anoth.logpt;
    destrpt = anoth.destrpt;
    coptr = anoth.coptr;
    counter = 0;

    return *this;
  }

  /*
    Spy &SwapAllTheGuys(Spy &&anoth) {
      std::swap(logger_, anoth.logger_);
      std::swap(logpt, anoth.logpt);
      std::swap(destrpt, anoth.destrpt);
      std::swap(coptr, anoth.coptr);
      return anoth;
    }*/

  Spy(Spy &&anoth)
    requires std::movable<T>
      : value_(std::move(anoth.value_)), counter(0) {
    std::swap(logger_, anoth.logger_);
    std::swap(logpt, anoth.logpt);
    std::swap(destrpt, anoth.destrpt);
    std::swap(coptr, anoth.coptr);
    // anoth = SwapAllTheGuys(std::move(anoth)); // move? fwd?
  }

  Spy &operator=(Spy &&anoth)
    requires std::movable<T>
  {
    if (this == &anoth) {
      return *this;
    }
    value_ = std::move(anoth.value_);

    std::swap(logger_, anoth.logger_);
    std::swap(logpt, anoth.logpt);
    std::swap(destrpt, anoth.destrpt);
    std::swap(coptr, anoth.coptr);
    // anoth = SwapAllTheGuys(std::move(anoth));

    counter = 0;

    return *this;
  }

  template <std::invocable<unsigned int> Log_>
    requires(std::destructible<T>) && // unnecessary?
            (std::destructible<std::remove_cvref_t<Log_>>)
  void setHelperFunc(Log_ &&logger) {
    // auto nextres = std::forward<Log_>(logger);
    logger_ = new std::remove_cvref_t<Log_>(std::forward<Log_>(logger));

    logpt = +[](void *self, unsigned int qnt) {
      std::invoke(*static_cast<std::remove_cvref_t<Log_> *>(self), qnt);
    };

    destrpt = +[](void *self) {
      delete static_cast<std::remove_cvref_t<Log_> *>(self);
    };
  }

  template <std::invocable<unsigned int> Log_> // invoc
    requires(!std::copyable<T>) && (std::destructible<T>) &&
            (std::destructible<std::remove_cvref_t<Log_>>) &&
            (std::movable<T>) &&
            (std::move_constructible<std::remove_cvref_t<Log_>>)
  void setLogger(Log_ &&logger) {
    setHelperFunc(std::move(logger));
  }

  template <std::invocable<unsigned int> Log_>
    requires(std::copyable<T>) && (std::destructible<T>) &&
            (std::destructible<std::remove_cvref_t<Log_>>) &&
            (std::copyable<std::remove_cvref_t<Log_>>)
  void setLogger(Log_ &&logger) {
    setHelperFunc(std::move(logger));

    coptr = +[](void *self) -> void * {
      return new std::remove_cvref_t<Log_>(
          *static_cast<std::remove_cvref_t<Log_> *>(self));
    };
  }
};