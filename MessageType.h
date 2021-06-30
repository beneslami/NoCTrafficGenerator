//
// Created by Ben on 6/30/21.
//

#ifndef MESSAGETYPE_H
#define MESSAGETYPE_H

#include <stdint.h>

#define INJECT_REQ  	0
#define INJECT_RES  	1
#define EJECT_REQ  		2
#define EJECT_RES  		3

struct InjectReqMsg{
    InjectReqMsg(){
        size = sizeof(InjectReqMsg);
        type = INJECT_REQ;
    }
    int source;
    int dest;
    int id;
    int packetSize;
    int network;
    int cl;
    int miss_pred;
    int msgType;
    int coType;
    unsigned long long address;
};

struct InjectResMsg{
    InjectResMsg(){
        size = sizeof(InjectResMsg);
        type = INJECT_RES;
    }
};

struct EjectReqMsg{
    EjectReqMsg(){
        size = sizeof(EjectReqMsg);
        type = EJECT_REQ;
    }
};

struct EjectResMsg{
    EjectResMsg(){
        size = sizeof(EjectResMsg);
        type = EJECT_RES;
    }
    int id;
    int remainingRequests;
    int source;
    int dest;
    int packetSize;
    int network;
    int cl;
    int miss_pred;
};

#endif
