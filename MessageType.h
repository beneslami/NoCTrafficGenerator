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
        int size = sizeof(InjectReqMsg);
        int type = INJECT_REQ;
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
        int size = sizeof(InjectResMsg);
        int type = INJECT_RES;
    }
};

struct EjectReqMsg{
    EjectReqMsg(){
        int size = sizeof(EjectReqMsg);
        int type = EJECT_REQ;
    }
};

struct EjectResMsg{
    EjectResMsg(){
        int size = sizeof(EjectResMsg);
        int type = EJECT_RES;
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
