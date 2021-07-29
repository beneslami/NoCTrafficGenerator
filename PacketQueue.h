//
// Created by Ben on 6/30/21.
//

#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H

#include "netstream/messages.h"
#include <list>
#include <map>

class PacketQueue {
public:
    PacketQueue();
    virtual ~PacketQueue();
    void Enqueue(InjectReqMsg, int);
    std::list<InjectReqMsg> DeQueue(int);
    void cleanUp(int time);
    int Size(){
        return queue.size();
    }
    int Size2(){
        for(int i = 0; i < queue.size(); i++){
            return queue[i].size();
        }
    }

private:
    std::map<int, std::list<InjectReqMsg> > queue;
};


#endif
