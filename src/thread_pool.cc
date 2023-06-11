#include "thread_pool.h"

#include <thread>

thread_pool::thread_pool(uint32_t num_threads) : running(false) {
  threads.resize(num_threads);
  for (uint32_t i = 0; i < num_threads; ++i) {
    threads[i] = std::thread(&thread_pool::worker, this, i);
  }
  running = true;
}

thread_pool::~thread_pool() {
  stop();
  for (auto &thread : threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

void thread_pool::enqueue(std::function<void()>&& task) {
  std::unique_lock lock(tasks_mutex);
  tasks.push(std::move(task));
  ++task_count;
  tasks_cond.notify_one();
}

void thread_pool::wait() {
  std::unique_lock lock(wait_mutex);
  wait_cond.wait(lock, [this] { return task_count == 0; });
}

void thread_pool::stop() {
  wait();
  running = false;
  tasks_cond.notify_all();
}

void thread_pool::worker(uint32_t thread_id) {
  while (running) {
    std::function<void()> task;
    {
      std::unique_lock lock(tasks_mutex);
      tasks_cond.wait(lock, [this] { return !tasks.empty() || !running; });
      if (!running) {
        return;
      }
      task = std::move(tasks.front());
      tasks.pop();
    }
    task();
    --task_count;
    if (task_count == 0) {
      wait_cond.notify_all();
    }
  }
}
