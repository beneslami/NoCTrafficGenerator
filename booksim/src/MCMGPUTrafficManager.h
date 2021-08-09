//
// Created by Ben on 8/8/21.
//

#ifndef MCMGPUTRAFFICMANAGER_H
#define MCMGPUTRAFFICMANAGER_H

#include <limits>
#include <vector>
#include <sstream>
#include "flit.hpp"
#include "stats.hpp"
#include "booksim.hpp"
#include "globals.hpp"
#include "interface.h"
#include "config_utils.hpp"
#include "booksim_config.hpp"
#include "trafficmanager.hpp"
#include "networks/network.hpp"
#include "packet_reply_info.hpp"

class MCMGPUTrafficManager : public TrafficManager{
protected:
    vector<vector<vector<list<Flit *> > > > _input_queue;
    virtual void _RetireFlit( Flit *f, int dest );
    virtual void _GeneratePacket(int source, int cl, int time, int subnet, int package_size, Flit::FlitType& packet_type, void* const data, int dest);
    virtual void _Step();
public:
    static MCMGPUTrafficManager *address;
    static MCMGPUTrafficManager* get_instance(const Configuration &config, const vector<Network *> & net);
    MCMGPUTrafficManager(const Configuration &config, const vector<Network *> & net);
    virtual ~MCMGPUTrafficManager();
};

#endif
