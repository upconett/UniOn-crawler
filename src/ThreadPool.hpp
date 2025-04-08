#include <iostream>
#include <thread>
#include <functional>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <future>


class ThreadPool {

    static const std::size_t max_queue_size = 100;

private:
    const short size;
    std::queue<std::function<void()>> queue;
    std::thread* threads;

    std::condition_variable cv;
    std::condition_variable queue_not_full;
    std::mutex mutex;

    bool thread_pool_works;

protected:
    void manager_func(short thread_id) {
        std::unique_lock<std::mutex> lock(mutex);
        while (thread_pool_works) {
            cv.wait(lock, [this]() 
                { return !queue.empty() || !thread_pool_works; }
            );
            if (!thread_pool_works) { break; }
            if (!queue.empty()) {
                std::function<void()> task = queue.front();
                queue.pop();
                queue_not_full.notify_one();

                lock.unlock() ;
                task();
                lock.lock();
            }
        }
    }

public:
    ThreadPool(short size)
    : size(size),
      thread_pool_works(true),
      threads(new std::thread[size])
    {
        for (int id = 0; id < size; id++) {
            threads[id] = std::thread(
                [this, id](){ manager_func(id); }
            );
        }
    }

    ~ThreadPool() {
        thread_pool_works = false;
        cv.notify_all();
        for (int id = 0; id < size; id++) {
            threads[id].join();
        }
    }

    template <typename Func, typename... Args>
    void add_task(Func&& f, Args&&... args) {
        using ReturnType = std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>;

        auto bounded_args = std::make_tuple(std::forward<Args>(args)...);
        auto task_ptr = std::make_shared<std::packaged_task<ReturnType()>>(
            [f = std::forward<Func>(f), args = std::move(bounded_args)]() mutable {
                return std::apply(std::move(f), std::move(args));
            }
        );

        auto wrapper_func = [task_ptr](){ (*task_ptr)(); };

        {
            // std::lock_guard<std::mutex> lock(mutex);
            std::unique_lock lock(mutex);
            queue_not_full.wait(lock, [this](){ return queue.size() < max_queue_size; });

            queue.push(wrapper_func);
            cv.notify_one();
        }
    }

};
