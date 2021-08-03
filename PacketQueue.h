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
    void Enqueue_request(InjectReqMsg, int);
    void Enqueue_response(InjectResMsg, int);
    std::list<InjectReqMsg> DeQueue_request(int);
    std::list<InjectResMsg> DeQueue_response(int);
    void cleanUp_request_queue(int);
    void cleanUp_response_queue(int);
    int request_queue_size(){
        return req_queue.size();
    }
    int response_queue_size(){
        return resp_queue.size();
    }
    void Debug_print_request_queue_index(){
        std::map<int, std::list<InjectReqMsg> >::iterator it;
        for(it = req_queue.begin(); it != req_queue.end(); ++it){
            std::cout << it->first << "\t";
        }
        std::cout << "\n";
    }


private:
    std::map<int, std::list<InjectReqMsg> > req_queue;
    std::map<int, std::list<InjectResMsg> > resp_queue;
};


#endif
