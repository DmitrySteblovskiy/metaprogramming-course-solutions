#pragma once
#include <concepts>
#include <functional>
#include <utility>

template <class T> class Spy {
private:
  T value_;
  void *logger_ = nullptr;
  size_t counter = 0;
  size_t sz = 0;                                 //
  void (*logpt)(void *, unsigned int) = nullptr; //
  void (*destrpt)(void *) = nullptr;
  void *(*coptr)(void *) = nullptr;

  class LogInvoke {
  private:
    Spy<T> *spy_ptr;
    size_t i = 0;
    T *t;
    void *logger;
    void (*)(void *, unsigned int) log;
    size_t *curr;

  public:
    // T *operator->() { return &spy_ptr->value_; }
    LogInvoke(T *t, void *logger, void (*)(void *, unsigned int) log,
              size_t *counter)
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

public:
  explicit Spy(T); // exp?
  explicit Spy(T &&t) : value_(std::forward<T>(t)) {}

  T &operator*() { return value_; }

  const T &operator*() const { return value_; }

  Spy()
    requires std::default_initializable<T>
      : t_(T{}) {}

  ~Spy()
    requires std::destructible<T>
  {
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

  void setLogger();

  Spy(const Spy &anoth)
    requires std::copyable<T>
      : value_(anoth.value_), logpt(anoth.logpt), destrpt(anoth.destrpt),
        coptr(anoth.coptr), counter(0) {
    if (anoth.logger_ != nullptr) {
      logger_ = anoth.coptr(anoth.logger_);
    }
  }

  void SetSomePt(Spy &&anoth) {
    logpt = anoth.logpt;
    destrpt = anoth.destrpt;
    coptr = anoth.coptr;
  }

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

    SetSomePt(anoth);
    counter = 0;

    return *this;
  }

  Spy &&SwapAllTheGuys(Spy &&anoth) {
    std::swap(logger_, anoth.logger_);
    std::swap(logpt, anoth.logpt);
    std::swap(destrpt, anoth.destrpt);
    std::swap(coptr, anoth.coptr);
    return anoth;
  }

  Spy(Spy &&anoth)
    requires std::movable<T>
      : value_(std::move(anoth.value_)), counter(0) {
    anoth = SwapAllTheGuys(anoth);
  }

  Spy &operator=(Spy &&anoth)
    requires std::movable<T>
  {
    if (this == &anoth) {
      return *this;
    }
    value_ = std::move(anoth.value_);

    anoth = SwapAllTheGuys(anoth);

    counter = 0;

    return *this;
  }

  template <std::invocable<size_t> Log_>
    requires(std::destructible<T>) && // unnecessary?
            (std::destructible<std::remove_cvref_t<Log_>>)
  void setHelperFunc(Log_ &&logger) {
    auto nextres = std::forward<Log_>(logger);
    logger_ = new std::remove_cvref_t<Log_>(nextres);

    logpt = +[](void *self, size_t qnt) {
      std::invoke(*static_cast<std::remove_cvref_t<Log_> *>(self), qnt);
    };

    destrpt = +[](void *self) {
      delete static_cast<std::remove_cvref_t<Log_> *>(self);
    };
  }

  template <std::invocable<size_t> Log_> // invoc
    requires(!std::copyable<T>) && (std::destructible<T>) &&
            (std::destructible<std::remove_cvref_t<Log_>>) &&
            (std::movable<T>) &&
            (std::move_constructible<std::remove_cvref_t<Log_>>)
  void setLogger(Log_ &&logger) {
    setHelperFunc(logger);
  }

  template <std::invocable<size_t> Log_>
    requires(std::copyable<T>) && (std::destructible<T>) &&
            (std::destructible<std::remove_cvref_t<Log_>>) &&
            (std::copyable<std::remove_cvref_t<Log_>>)
  void setLogger(Log_ &&logger) {
    setHelperFunc(logger);

    coptr = +[](void *self) -> void * {
      return new std::remove_cvref_t<Log_>(
          *static_cast<std::remove_cvref_t<Log_> *>(self));
    };
  }
};