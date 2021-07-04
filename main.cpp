//
// Created by Ben on 6/30/21.
//

#include <iostream>
#include <string>
#include <chrono>
#include "TrafficGenerator.h"

unsigned long long numCycles;
int numCores;

int main(int argc, char **argv){
    if(argc < 3) {
        std::cerr << "Need 2 parameters: [MODEL FILE] [DURATION IN CYCLE]" << std::endl;
        return -1;
    }
    numCycles = std::stoull(argv[2]);
    numCores = std::stoi(argv[3]);
    TrafficGenerator TGen = TrafficGenerator();
    auto start = std::chrono::high_resolution_clock::now();
    TGen.Run();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
    std::cout << "duration: " << duration.count() <<"s\n";
    return 0;
}