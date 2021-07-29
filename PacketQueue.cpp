//
// Created by Ben on 6/30/21.
//

#include "PacketQueue.h"
#include <list>

PacketQueue::PacketQueue() {
}

PacketQueue::~PacketQueue() {
}

void PacketQueue::Enqueue(InjectReqMsg packet, int time) {
    queue[time].push_back(packet);
}

std::list<InjectReqMsg> PacketQueue::DeQueue(int time) {
    return queue[time];
}

void PacketQueue::cleanUp(int time) {
    queue[time].clear();
    queue.erase(queue.find(time));
}