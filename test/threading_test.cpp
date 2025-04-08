#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>


void sleep(int seconds) {
    auto id = std::this_thread::get_id();
    std::cout << "[" << id << "] sleep for " << seconds << " seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    std::cout << "[" << id << "] woke up!" << std::endl;
}





int main() {
    std::thread task([](){ sleep(2); });
    sleep(4);
    task;
    return 0;
}
