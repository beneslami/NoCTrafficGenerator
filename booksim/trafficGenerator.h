//
// Created by Ben on 7/19/21.
//

#ifndef TRAFFICGENERATOR_H
#define TRAFFICGENERATOR_H

#include "interface.h"
#include "src/flit.hpp"
#include "src/stats.hpp"
#include "src/module.hpp"
#include "src/network.hpp"
#include "src/traffic.hpp"
#include "src/routefunc.hpp"
#include "src/outputset.hpp"
#include "src/injection.hpp"
#include "src/config_utils.hpp"
#include "src/buffer_state.hpp"
#include "src/trafficmanager.hpp"
#include "src/networks/network.hpp"

class trafficGenerator : public TrafficManager{
private:
    struct PayLoad {
        int id;
        int subnetwork;
    };
    int  _flit_width;
    int _ideal_interconnect;
    TraceGenerator *_time_trace;

protected:
    Interface *_interface;

    virtual void _Step();
    virtual void _Inject();
    virtual bool _SingleSim();
    virtual void _RetireFlit( Flit *f, int dest );
    void _GeneratePacket(int source, int size, int cl, int time, int id, int subnetwork, int destination);

public:
    TrafficManager(const Configuration &config, const vector<BSNetwork *> & net);
    virtual ~TrafficManager();
};


#endif
