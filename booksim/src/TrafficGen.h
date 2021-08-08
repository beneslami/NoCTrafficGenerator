//
// Created by Ben on 7/19/21.
//

#ifndef TRAFFICGEN_H
#define TRAFFICGEN_H

#include <cmath>
#include <limits>
#include <vector>
#include <sstream>
#include "flit.hpp"
#include "stats.hpp"
#include "module.hpp"
#include "interface.h"
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
    Interface *_interface;
    virtual void _Step();
    virtual void _RetireFlit( Flit *, int);
    void _GeneratePacket(int, int, int, int, int, int, void* const, int);
    vector<vector<vector<list<Flit *> > > > _input_queue;
    int  _flit_width;
    int _ideal_interconnect;
public:
    TrafficGen(const Configuration &, const vector<Network *> &);
    virtual ~TrafficGen();
    void Init();
    int get_size(unsigned, unsigned, unsigned);
    bool is_empty(unsigned, unsigned, unsigned);
    friend class Interface;
};

#endif
