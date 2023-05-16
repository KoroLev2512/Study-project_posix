#include <iostream>
#include <cstdlib>
#include "producer_consumer.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Check arguments" << std::endl;
        return -1;
    }
    int threadsCount = atoi(argv[1]);
    int sleepTime = atoi(argv[2]);
    bool debug = false;
    if (argc == 4 && std::string(argv[3]) == "-debug") {
        debug = true;
    }
    std::cout << run_threads(threadsCount, sleepTime, debug) << std::endl;
    return 0;
}


