//
// Created by Ben on 6/30/21.
//

#ifndef TRAFFICGENERATOR_H
#define TRAFFICGENERATOR_H

#include <vector>
#include "PacketQueue.h"
#include "MessageType.h"
#include "RandomGenerator.h"
#include "netstream/messages.h"
#include "netstream/socketstream.h"

class TrafficGenerator {
public:
    TrafficGenerator(int);
    ~TrafficGenerator();
    void Run();
    void Run2();
    void Inject();
    void Eject();
    void sendRequestPacket(InjectReqMsg&);
    void sendResponsePacket(InjectResMsg&);
    void initiateMessage(int, int, int, int, int, int);
private:
    int _numCore;
    std::vector<PacketQueue*> Core_queues;
};

#endif
