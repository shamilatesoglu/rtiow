#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class thread_pool {
public:
  thread_pool(uint32_t num_threads);

  ~thread_pool();

  void enqueue(std::function<void()>&& task);

  void wait();

  void stop();

  size_t pool_size() const { return threads.size(); }

protected:
  void worker(uint32_t thread_id);

  std::atomic_bool running;
  std::vector<std::thread> threads;

  std::queue<std::function<void()>> tasks;
  std::mutex tasks_mutex;
  std::condition_variable tasks_cond;

  std::mutex wait_mutex;
  std::condition_variable wait_cond;
  std::atomic_uint64_t task_count = 0;
};