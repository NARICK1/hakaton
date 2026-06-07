#include <iostream>
#include <thread>
#include <chrono>
int main() {
    std::cout << "test" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::cout << "done" << std::endl;
    return 0;
}
