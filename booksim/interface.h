//
// Created by Ben on 7/19/21.
//

#ifndef INTERFACE_H
#define INTERFACE_H

#include <map>
#include <queue>
#include "src/module.hpp"
#include "src/config_utils.hpp"
#include "src/trace_generator.hpp"
#include "src/networks/network.hpp"

struct RequestPacket {
    int source;
    int dest;
    int id;
    int size;
    int network;
    int cl;
    int miss_pred;
};

struct ReplyPacket {
    int source;
    int dest;
    int id;
    int network;
    int cl;
    int miss_pred;
};

class Interface {
private:
    queue<RequestPacket *> **_request_buffer;
    queue<ReplyPacket *> _reply_buffer;
    map<int, int> _node_map;
    int _sources;
    int _dests;

public:
    Interface( const Configuration &config, const vector<BSNetwork *> & net );
    ~Interface();

    int Init();
    int Step();

    int EnqueueRequestPacket(RequestPacket *packet);
    RequestPacket *DequeueRequestPacket(int source, int network, int cl);

    int getReplyQueueSize() { return _reply_buffer.size(); }
    int EnqueueReplyPacket(FeS2ReplyPacket *packet);
    FeS2ReplyPacket *DequeueReplyPacket();

    int GenerateTestPackets();
};


#endif
