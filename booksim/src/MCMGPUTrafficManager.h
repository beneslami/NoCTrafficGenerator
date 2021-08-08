//
// Created by Ben on 8/8/21.
//

#ifndef MCMGPUTRAFFICMANAGER_H
#define MCMGPUTRAFFICMANAGER_H

#include <vector>
#include "flit.hpp"
#include "stats.hpp"
#include "booksim.hpp"
#include "config_utils.hpp"
#include "booksim_config.hpp"
#include "trafficmanager.hpp"
#include "networks/network.hpp"

class MCMGPUTrafficManager : public TrafficManager{
protected:
    vector<vector<vector<list<Flit *> > > > _input_queue;
public:
    static MCMGPUTrafficManager *address;
    static MCMGPUTrafficManager* get_instance(const Configuration &config, const vector<Network *> & net);
    MCMGPUTrafficManager(const Configuration &config, const vector<Network *> & net);
    virtual ~MCMGPUTrafficManager();
};

#endif
