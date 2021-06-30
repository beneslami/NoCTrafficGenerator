//
// Created by Ben on 6/30/21.
//

#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H

#include "MessageType.h"
#include <list>
#include <map>

class PacketQueue {
public:
    PacketQueue();
    virtual ~PacketQueue();
    void Enqueue(InjectReqMsg packet, int time);
    std::list<InjectReqMsg> DeQueue(int time);
    void cleanUp(int time);
    int Size(){
        return queue.size();
    }

private:
    std::map<int, std::list<InjectReqMsg> > queue;
};


#endif
