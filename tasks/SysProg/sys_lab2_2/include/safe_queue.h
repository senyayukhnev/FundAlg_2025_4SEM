#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class SafeQueue {
private:
    std::queue<T> data;
    std::mutex mutex;
    std::condition_variable cv;

public:
    SafeQueue() = default;
    ~SafeQueue() = default;
    void push(const T &el);
    void push(T &&el);
    T pop();

    bool empty();
};

template <typename T>
bool SafeQueue<T>::empty() {
    std::lock_guard<std::mutex> lock(mutex);
    return data.empty();
}

template <typename T>
T SafeQueue<T>::pop() {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this] { return !data.empty(); });
    T tmp = data.front();
    data.pop();
    return tmp;
}

template <typename T>
void SafeQueue<T>::push(const T &el) {
    std::lock_guard<std::mutex> lock(mutex);
    data.push(el);
    cv.notify_one();
}

template <typename T>
void SafeQueue<T>::push(T &&el) {
    std::lock_guard<std::mutex> lock(mutex);
    data.push(el);
}