//
// Created by Ben on 7/19/21.
//

#ifndef TRAFFICGEN_H
#define TRAFFICGEN_H

#include <list>
#include <cmath>
#include <limits>
#include <vector>
#include <sstream>
#include "flit.hpp"
#include "stats.hpp"
#include "module.hpp"
#include "interface.h"
#include "globals.hpp"
#include "traffic.hpp"
#include "routefunc.hpp"
#include "outputset.hpp"
#include "injection.hpp"
#include "config_utils.hpp"
#include "buffer_state.hpp"
#include "trafficmanager.hpp"
#include "booksim_config.hpp"
#include "networks/network.hpp"
#include "packet_reply_info.hpp"

class TrafficGen : public TrafficManager{
protected:
    virtual void _Step();
    virtual void _RetireFlit( Flit *, int);
    void _GeneratePacket(int, int, int, int, int, int, void* const, int);
    vector<vector<vector<list<Flit *> > > > _input_queue;
public:
    static TrafficGen* result;
    TrafficGen(const Configuration &, const vector<Network *> &);
    void Init();
    static TrafficGen* get_instance(const Configuration &, const vector<Network *> &);
};

#endif
