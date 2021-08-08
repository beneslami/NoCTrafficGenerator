//
// Created by Ben on 6/30/21.
//

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include "TrafficGenerator.h"
#include "ReadFile.h"
#include "globals.h"

int main(int argc, char **argv){
    if(argc < 3) {
        std::cerr << "Need 3 parameters: [MODEL FILE] [DURATION IN CYCLE] [NUMBER OF CORE]" << std::endl;
        return -1;
    }
    std::ifstream modelFile(argv[1]);
    if(!modelFile.good()) {
        std::cerr << "Could not open file " << argv[1] << std::endl;
        return -1;
    }
    numCycles = std::stoull(argv[2]);
    numCores = std::stoi(argv[3]);
    readModel(modelFile);
    TrafficGenerator TGen = TrafficGenerator(numCores);
    auto start = std::chrono::high_resolution_clock::now();
    TGen.Run();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
    std::cout << "duration: " << duration.count() <<"s\n";
    modelFile.close();
    return 0;
}