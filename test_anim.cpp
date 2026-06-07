#include <iostream>
#include <thread>
#include <chrono>
#include <string>
int main() {
    for (int i = 0; i < 10; i++) {
        std::cout << "test " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    std::cout << "done" << std::endl;
    return 0;
}
