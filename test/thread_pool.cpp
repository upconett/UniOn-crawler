#include <thread>
#include <functional>
#include <future>
#include <iostream>


class reusable_thread
{
    static short counter;
    const std::thread exact_;
public:
    const short id;

    template <typename T>
    std::future<T> assign_task(std::packaged_task<T()>&& task) {
        return task.get_future();
    }

    reusable_thread() 
    : id(counter) 
    {
        counter++; 
    }

    ~reusable_thread() 
    {
        counter--;
    }
};

short reusable_thread::counter = 0;


class thread_pool
{
    const reusable_thread* pool_;
public:
    const short size;

    template <typename T>
    std::future<T> run_task(std::packaged_task<T()> task) {
        return task.get_future();
    }

    thread_pool(short number_of_threads) : size(number_of_threads), pool_(new reusable_thread[size]) {};
};


int main() {
    thread_pool pool(2);
    std::future<int> f = pool.run_task(std::packaged_task([](){ return 2; }));
    return 0;
}