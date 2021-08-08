//
// Created by Ben on 8/8/21.
//

#include "MCMGPUTrafficManager.h"

MCMGPUTrafficManager::address = NULL;
MCMGPUTrafficManager * MCMGPUTrafficManager::get_instance(const Configuration &config, const vector<Network *> &net) {
    if(address == NULL){
        address = new MCMGPUTrafficManager(config, net);
    }
    else{
        return address;
    }
    return address;
}

MCMGPUTrafficManager::MCMGPUTrafficManager(const Configuration &config, const vector<Network *> &net) : TrafficManager(config, net){
    _input_queue.resize(_subnets);
    for ( int subnet = 0; subnet < _subnets; ++subnet) {
        _input_queue[subnet].resize(_nodes);
        for ( int node = 0; node < _nodes; ++node ) {
            _input_queue[subnet][node].resize(_classes);
        }
    }
}

MCMGPUTrafficManager::~MCMGPUTrafficManager(){

}