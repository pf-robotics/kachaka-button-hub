#pragma once

#include <Arduino.h>

namespace kb {

// Usage:
//
//  kb::Mutex g_mutex;
//  ...
//  if (kb::LockGuard lock(g_mutex); lock) {
//    // lock is acquired
//    ...
//  }

class Mutex {
 public:
  explicit Mutex() : mutex_(xSemaphoreCreateMutex()) {
    if (mutex_ == nullptr) {
      Serial.println("Failed to create mutex");
      abort();
    }
  }
  ~Mutex() { vSemaphoreDelete(mutex_); }

  SemaphoreHandle_t& get() { return mutex_; }

 private:
  Mutex(const Mutex&) = delete;
  Mutex& operator=(const Mutex&) = delete;
  Mutex(Mutex&&) = delete;
  Mutex& operator=(Mutex&&) = delete;

  SemaphoreHandle_t mutex_;
};

class LockGuard {
 public:
  explicit LockGuard(Mutex& mutex)
      : mutex_(mutex),
        locked_(xSemaphoreTake(mutex_.get(), portMAX_DELAY) == pdTRUE) {}
  explicit LockGuard(Mutex& mutex, const int timeout_msec)
      : mutex_(mutex),
        locked_(xSemaphoreTake(mutex_.get(),
                               timeout_msec / portTICK_PERIOD_MS) == pdTRUE) {}
  ~LockGuard() {
    if (locked_) {
      xSemaphoreGive(mutex_.get());
    }
  }

  operator bool() const { return locked_; }

 private:
  LockGuard(const LockGuard&) = delete;
  LockGuard& operator=(const LockGuard&) = delete;
  LockGuard(LockGuard&&) = delete;
  LockGuard& operator=(LockGuard&&) = delete;

  Mutex& mutex_;
  bool locked_;
};

}  // namespace kb
