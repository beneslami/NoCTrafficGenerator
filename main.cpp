//
// Created by Ben on 6/30/21.
//

#include <iostream>
#include <string>
#include "TrafficGenerator.h"

unsigned long long numCycles;

int main(int argc, char **argv){
    if(argc < 2) {
        std::cerr << "Need 2 parameters: [MODEL FILE] [DURATION IN CYCLE]" << std::endl;
        return -1;
    }
    numCycles = std::stoull(argv[2]);
    TrafficGenerator TGen = TrafficGenerator();
    TGen.Run();

    return 0;
}