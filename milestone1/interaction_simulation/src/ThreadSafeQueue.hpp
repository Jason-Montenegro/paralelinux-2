// Copyright 2026 Paralelinux-2
#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class ThreadSafeQueue {
public:
	void push(T value) {
		std::lock_guard lock(this->mtx);
		this->queue.push(value);
		this->cv.notify_one();
	}

	T wait_and_pop() {
		std::unique_lock lock(this->mtx);
		this->cv.wait(lock, [&] { return !this->queue.empty(); });
		T value = std::move(this->queue.front());
		this->queue.pop();
		return value;
	}

	bool empty() {
		std::lock_guard lock(this->mtx);
		return this->queue.empty();
	}

private:
	std::queue<T> queue;
	std::condition_variable cv;
	std::mutex mtx;
};

#endif  // THREAD_SAFE_QUEUE_HPP
