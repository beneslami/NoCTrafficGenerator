//
// Created by Ben on 7/19/21.
//

#ifndef INTERFACE_H
#define INTERFACE_H

#include <map>
#include <queue>
#include "module.hpp"
#include "config_utils.hpp"
#include "networks/network.hpp"
#include "netstream/messages.h"
#include "netstream/socketstream.h"

struct RequestPacket {
    int source;
    int dest;
    int id;
    int size;
    int network;
    int cl;
};

struct ReplyPacket {
    int source;
    int dest;
    int id;
    int network;
    int cl;
};

class Interface {
private:
    queue<RequestPacket *> **_request_buffer;
    queue<ReplyPacket *> _reply_buffer;
    map<int, int> _node_map;
    int _sources;
    int _dests;
    int _duplicate_networks;
    bool _concentrate;
    std::string _host;
    int _port;
    SocketStream *_channel;
    SocketStream _listenSocket;
    map<int, int> _original_destinations;
public:
    Interface( const Configuration &config, const vector<Network *> & net );
    ~Interface();

    int Init();
    int Step();

    int EnqueueRequestPacket(RequestPacket *packet);
    RequestPacket *DequeueRequestPacket(int source, int network, int cl);

    int getReplyQueueSize() { return _reply_buffer.size(); }
    int EnqueueReplyPacket(ReplyPacket *packet);
    ReplyPacket *DequeueReplyPacket();
};


#endif
