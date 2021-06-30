//
// Created by Ben on 6/30/21.
//

#include "RandomGenerator.h"
#include <iostream>

int main(int argc, char **argv){
    if(argc != 4) {
        std::cerr << "Need 2 parameters: [MODEL FILE] [DURATION IN CYCLE]" << std::endl;
        return -1;
    }

    /* 1- open the model file and read argv[1]*/

    //unsigned int numCycles = (int) strtoul(argv[2], NULL, 0);

    /* 2- Run the generator with numCycles */

    return 0;
}
