/**
 * @file ThreadPool.h
 * @brief 简单高效的线程池实现
 * @author AI Assistant
 * @date 2025
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>

/**
 * @brief 线程池类
 *
 * 用于异步执行任务，避免UI线程阻塞
 */
class ThreadPool {
public:
    /**
     * @brief 构造函数
     * @param num_threads 线程数量（默认2）
     */
    explicit ThreadPool(size_t num_threads = 2) : stop_(false) {
        Start(num_threads);
    }

    /**
     * @brief 析构函数
     */
    ~ThreadPool() {
        Stop();
    }

    /**
     * @brief 启动线程池
     * @param num_threads 线程数量
     */
    void Start(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this] {
                            return stop_ || !tasks_.empty();
                        });

                        if (stop_ && tasks_.empty()) {
                            return;
                        }

                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }

                    task();
                }
            });
        }
    }

    /**
     * @brief 停止线程池
     */
    void Stop() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();

        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        workers_.clear();
    }

    /**
     * @brief 重启线程池（改变线程数量）
     * @param num_threads 新的线程数量
     */
    void Restart(size_t num_threads) {
        Stop();
        stop_ = false;
        tasks_ = std::queue<std::function<void()>>();
        Start(num_threads);
    }

    /**
     * @brief 提交任务到线程池
     * @param f 任务函数
     * @param args 参数
     * @return future对象
     */
    template<class F, class... Args>
    auto Enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            if (stop_) {
                throw std::runtime_error("Enqueue on stopped ThreadPool");
            }

            tasks_.emplace([task]() { (*task)(); });
        }
        condition_.notify_one();
        return res;
    }

    /**
     * @brief 获取当前线程数量
     */
    size_t GetThreadCount() const {
        return workers_.size();
    }

    /**
     * @brief 获取待处理任务数量
     */
    size_t GetTaskCount() const {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        return tasks_.size();
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
};

#endif // THREADPOOL_H
