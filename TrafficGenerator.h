//
// Created by Ben on 6/30/21.
//

#ifndef TRAFFICGENERATOR_H
#define TRAFFICGENERATOR_H

#include "PacketQueue.h"
#include "MessageType.h"
#include "RandomGenerator.h"

#define READ_REQUEST    1
#define WRITE_REQUEST   0
#define READ_REPLY      2
#define WRITE_REPLY     3

class TrafficGenerator {
public:
    TrafficGenerator(){
        std::cout << "TGen\n";
    }
    void Run();
    void Inject();
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
    void react(EjectResMsg);
};

#endif
