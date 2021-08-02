//
// Created by Ben on 6/30/21.
//

#include "PacketQueue.h"
#include <list>

PacketQueue::PacketQueue() {
}

PacketQueue::~PacketQueue() {
}

void PacketQueue::Enqueue_request(InjectReqMsg packet, int time){
    req_queue[time].push_back(packet);
}

void PacketQueue::Enqueue_response(InjectResMsg packet, int time){
    resp_queue[time].push_back(packet);
}

std::list<InjectReqMsg> PacketQueue::DeQueue_request(int time) {
    return req_queue[time];
}

std::list<InjectResMsg> PacketQueue::DeQueue_response(int time) {
    return resp_queue[time];
}

void PacketQueue::cleanUp_request_queue(int time) {
    req_queue[time].clear();
    req_queue.erase(req_queue.find(time));
}

void PacketQueue::cleanUp_response_queue(int time) {
    resp_queue[time].clear();
    resp_queue.erase(resp_queue.find(time));
}