#ifndef THREADQUEUE_H
#define THREADQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <utility>

template <typename T>
class ThreadQueue{
private:
    std::queue<T> queue;
	std::condition_variable cv;
	std::mutex mtx;

public:
    void push(T item) {
		std::lock_guard<std::mutex> lock(this->mtx);    
		this->queue.push(std::forward<T>(item));
		this->cv.notify_one();
	}

	T wait_and_pop() {
        std::unique_lock<std::mutex> lock(this->mtx);  
        this->cv.wait(lock, [this] { return !this->queue.empty(); });
        T item = std::move(this->queue.front());
        this->queue.pop();
        return item;
    }

	bool empty() {
		std::lock_guard<std::mutex> lock(this->mtx);
		return this->queue.empty();
	}
};

#endif