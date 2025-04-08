#include <iostream>
#include <chrono>
#include <future>
#include <thread>


void sleep(int seconds) { std::this_thread::sleep_for(std::chrono::seconds(seconds)); }

void first_test() {
    std::packaged_task<int(int)> increment_task([](int a){ sleep(2); return a+1; });
    std::future<int> f = increment_task.get_future();
    std::thread t(std::move(increment_task), 3);

    std::cout << "getting increment done...\n" << std::flush;
    f.wait();
    std::cout << "waited\n" << std::flush;
    std::cout << "result is: " << f.get() << std::endl;

    t.join();
}

void wait_for_multiple_futures() {
    std::packaged_task<int()> task4([](){ sleep(4); return 4; });
    std::packaged_task<int()> task2([](){ sleep(2); return 2; });

    std::future f4 = task4.get_future();
    std::future f2 = task2.get_future();
    
    std::thread t4(std::move(task4));
    std::thread t2(std::move(task2));

    std::cout << "created both tasks\n" << std::flush;

    f4.wait();
    std::cout << "waited for f" << f4.get() << '\n' << std::flush;
    f2.wait();
    std::cout << "waited for f" << f2.get() << '\n' << std::flush;

    t4.join(); t2.join();
}

void sequential_future() {
    std::packaged_task<int()> task([](){ return 777; });
    std::future f = task.get_future();

    // task();

    std::cout << "finished: " << f.get() << std::endl;
}

int main() {
    // wait_for_multiple_futures();
    sequential_future();
    return 0;
}