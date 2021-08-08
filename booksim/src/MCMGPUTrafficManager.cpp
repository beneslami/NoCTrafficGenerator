//
// Created by Ben on 8/8/21.
//

#include "MCMGPUTrafficManager.h"

MCMGPUTrafficManager::MCMGPUTrafficManager(const Configuration &config, const int &net) : TrafficManager(config, net){
    _input_queue.resize(_subnets);
    for ( int subnet = 0; subnet < _subnets; ++subnet) {
        _input_queue[subnet].resize(_nodes);
        for ( int node = 0; node < _nodes; ++node ) {
            _input_queue[subnet][node].resize(_classes);
        }
    }
    std::cout << "size: " << _input_queue.size() << std::endl;
}

MCMGPUTrafficManager::~MCMGPUTrafficManager(){

}