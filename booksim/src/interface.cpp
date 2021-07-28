//
// Created by Ben on 7/19/21.
//

#include "interface.h"

Interface::Interface(const Configuration &config, const vector<Network *> &net) {
    _channel = NULL;
    _sources = net[0]->NumNodes();
    _dests   = net[0]->NumNodes();

    _concentrate = true; //config.GetInt("fes2_concentrate") ? true : false;
    _duplicate_networks = config.GetInt("subnets");
    _host = "127.0.0.1"; //config.GetStr( "fes2_host");
    _port = 20211; //config.GetInt("fes2_port");
    vector<int> mapping; // config.GetIntArray("fes2_mapping");
    int a[8] = {0,0,1,1,2,2,3,3};
    for(int i = 0; i < 8 ; i++){
        mapping.push_back(a[i]);
    }
    for(int i = 0; i < mapping.size(); i++) {
        _node_map[i] = mapping[i];
    }

    _request_buffer = new queue<RequestPacket *> * [_duplicate_networks];
    for (int i=0; i < _duplicate_networks; i++ ) {
        _request_buffer[i] = new queue<RequestPacket *> [_sources];
    }
}

Interface::~Interface() {
    for ( int n = 0; n < _duplicate_networks; ++n ) {
        delete [] _request_buffer[n];
    }
    delete [] _request_buffer;
}

int Interface::Init() {
    if (_listenSocket.listen(NS_HOST, NS_PORT) < 0) {
        return -1;
    }

    // Wait for Traffic Generator to connect
    _channel = _listenSocket.accept();
#ifdef NS_DEBUG
    cout << "Traffic Generator instance connected" << endl;
#endif
    // Initialize client
    InitializeReqMsg req;
    InitializeResMsg res;
    *_channel >> req << res;

    return 0;
}

int Interface::Step() {
    bool process_more = true;
    StreamMessage *msg = NULL;

    while (process_more && _channel && _channel->isAlive())
    {
        // read message
        *_channel >> (StreamMessage*&) msg;
        std::cout << "message: " << msg->type << std::endl;
        switch(msg->type)
        {
            case STEP_REQ:
            {
                // acknowledge the receipt of step request
                // we're actually doing a little bit of work in parallel
                StepResMsg res;
                *_channel << res;

                // fall-through and perform one step of BookSim loop
                process_more = false;

                break;
            }
            case INJECT_REQ:
            {
                InjectReqMsg* req = (InjectReqMsg*) msg;

                // create packet to store in local queue
                RequestPacket* rp = new RequestPacket();
                rp->cl = req->cl;
                rp->dest = req->dest;
                rp->id = req->id;
                rp->network = req->network;
                rp->size = req->packetSize;
                rp->source = req->source;

                /*if (_trace_mode == 1) {
                    //_trace->writeTraceItem(GetSimTime(), rp->source, rp->dest,
                    //		rp->size, req->address, rp->network);
                    stringstream str;

                    str << rp->id << " " << GetSimTime() << " " << rp->source << " "
                        << rp->dest << " " << rp->size << " " << req->msgType
                        << " " << req->coType << " " << req->address;

                    _trace->writeTrace(str.str());
                }*/

                EnqueueRequestPacket(rp);

                // acknowledge receipt of packet to FeS2
                InjectResMsg res;
                *_channel << res;

                break;
            }
            case EJECT_REQ:
            {
                EjectReqMsg* req = (EjectReqMsg*) msg;
                EjectResMsg res;
                // create packet to store in local queue
                ReplyPacket* rp = DequeueReplyPacket();

                if (rp != NULL)
                {
                    res.source = rp->source;
                    res.dest = rp->dest;
                    res.network = rp->network;
                    res.id = rp->id;
                    res.cl = rp->cl;

                    free(rp);
                    rp = NULL;
                }
                else
                {
                    res.id = -1;
                }

                res.remainingRequests = getReplyQueueSize();

                *_channel << res;

                break;
            }
            case QUIT_REQ:
            {
                QuitResMsg res;
                *_channel << res;

                return 1; // signal that we're done

                break;
            }
            default:
            {
                cout << "<Interface::Step> Unknown message type: "
                     << msg->type << endl;
                break;
            }
        }

        // done processing message, destroy it
        StreamMessage::destroy(msg);
    }

    return 0;
}

int Interface::EnqueueRequestPacket(RequestPacket *packet) {
    _original_destinations[packet->id] = packet->dest;

    //_node_map is based off of the configuration file. See "fes2_mapping"
    packet->source 	= _node_map[packet->source];
    packet->dest 	= _node_map[packet->dest];

    // special case: single network
    if (_duplicate_networks == 1) {
        _request_buffer[0][packet->source].push(packet);
    } else {
        assert (packet->network < _duplicate_networks);
        _request_buffer[packet->network][packet->source].push(packet);
    }
    return 0;
}

RequestPacket *Interface::DequeueRequestPacket(int source, int network, int cl) {
    RequestPacket *packet = NULL;

    if (!_request_buffer[network][source].empty()) {
        packet = _request_buffer[network][source].front();
        if (packet->cl == cl) {
            _request_buffer[network][source].pop();
        } else {
            packet = 0;
        }
    }

    return packet;
}

int Interface::EnqueueReplyPacket(ReplyPacket *packet) {
    assert(_original_destinations.find(packet->id) != _original_destinations.end());

    if (_concentrate) {
        assert(_original_destinations.find(packet->id) != _original_destinations.end());
        assert(_original_destinations[packet->id]/2 == packet->dest);
        packet->source *= 2;
    }

    packet->dest = _original_destinations[packet->id];
    _original_destinations.erase(packet->id);

    _reply_buffer.push(packet);

    return 0;
}

ReplyPacket *Interface::DequeueReplyPacket() {
    ReplyPacket *packet = NULL;

    if (!_reply_buffer.empty()) {
        packet = _reply_buffer.front();
        _reply_buffer.pop();
    }

    return packet;
}