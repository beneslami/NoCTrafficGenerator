//
// Created by Ben on 7/8/21.
//

#ifndef GLOBALS_H
#define GLOBALS_H

#include <map>
#include <iostream>
#include "netstream/messages.h"

struct RequestPacket {
    int source;
    int dest;
    int id;
    int size;
    int type;
    int network;
    int cl;
};

struct ReplyPacket {
    int source;
    int dest;
    int id;
    int size;
    int type;
    int network;
    int cl;
};
const int READ_REQUEST = 0;
const int WRITE_REQUEST = 1;
const int READ_REPLY = 2;
const int WRITE_REPLY = 3;

extern std::map<double, double>bytes;
extern std::map<int, std::map<double, double> > duration;
extern std::map<int, double>traffic;
extern std::string destin;
extern std::vector<float>byteValue;
extern std::map<int, InjectReqMsg> inTransitPackets;
extern std::map<int, RequestPacket> inTransitTransactions;

extern std::map<int, std::map<float, float> >normal_stat;

extern unsigned long long numCycles;
extern int numCores;

#endif
