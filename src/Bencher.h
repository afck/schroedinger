#ifndef SCHROEDINGER_BENCHER_H
#define SCHROEDINGER_BENCHER_H

#include <iostream>
#include <map>
#include <stdio.h>
#include <string>
#include <time.h>

/// A simple stopwatch for benchmarking.
class Bencher {
public:
  explicit Bencher(bool active = true);
  void setActive(bool active);
  void bench(std::string s);
  void restart();
  void print();

private:
  bool active_;
  clock_t prevTicks_ = clock();
  std::map<std::string, int> count_;
  std::map<std::string, clock_t> ticks_;
};

/// Create a new Bencher. If active_ is false, the Bencher will ignore all calls
/// to the bench, restart and print methods.
Bencher::Bencher(bool active) : active_(active) {
  if (active_) {
    prevTicks_ = clock();
  }
}

/// Set the bencher to active_ or inactive_.
void Bencher::setActive(bool active) { active_ = active; }

/// Count the time since the previous restart or bench call, and adds it to
/// the samples belonging to the given category. Restart the stopwatch.
void Bencher::bench(std::string s) {
  if (active_) {
    if (count_.find(s) == count_.end()) {
      count_[s] = 0;
      ticks_[s] = 0;
    }
    count_[s]++;
    ticks_[s] += clock() - prevTicks_;
    restart();
  }
}

/// Restart the stopwatch.
inline void Bencher::restart() {
  if (active_) {
    prevTicks_ = clock();
  }
}

/// Print the average time of all samples taken, for each category.
void Bencher::print() {
  if (active_) {
    for (auto itr = count_.begin(); itr != count_.end(); itr++) {
      if (itr->second > 0) {
        std::string s = itr->first;
        std::cout << s << ": " << ticks_[s] / count_[s] << std::endl;
      }
    }
  }
}

#endif // SCHROEDINGER_BENCHER_H
