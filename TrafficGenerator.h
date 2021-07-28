//
// Created by Ben on 6/30/21.
//

#ifndef TRAFFICGENERATOR_H
#define TRAFFICGENERATOR_H

#include "PacketQueue.h"
#include "MessageType.h"
#include "RandomGenerator.h"
#include "netstream/messages.h"
#include "netstream/socketstream.h"

class TrafficGenerator {
public:
    TrafficGenerator(){
    }
    void Run();
    void Run2();
    void Inject(int);
    void Eject();
    void sendPacket(InjectReqMsg&);
    struct transaction_t {
        int source;
        int dest;
        int acks_received;
        bool data_received;

        bool Completed() {
            return data_received;
        }
        transaction_t() : source(-1), dest(-1), acks_received(0), data_received(false){}
    };
    void initiateMessage(int, int, int, int, int);
};

#endif
