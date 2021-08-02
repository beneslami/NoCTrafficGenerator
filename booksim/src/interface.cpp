//
// Created by Ben on 7/19/21.
//
#include "interface.h"
#include "IntersimConfig.h"
#include "routefunc.hpp"
#include "config_utils.hpp"
#include "TrafficGen.h"

Interface::Interface(const Configuration &config, const vector<Network *> &net) {
    _channel = NULL;
    _icnt_config = new IntersimConfig();
    _subnets = _icnt_config->GetInt("subnets");
    _host = _icnt_config->GetStr("_host");
    _port = _icnt_config->GetInt("_port");
    vector<int> mapping = _icnt_config->GetIntArray("mapping");
    for(int i = 0; i < mapping.size(); i++) {
        _node_map[i] = mapping[i];
    }
}

Interface::~Interface() {

}

int Interface::Init() {
    if (_listenSocket.listen(_host, _port) < 0) {
        return -1;
    }
    _channel = _listenSocket.accept();
#ifdef NS_DEBUG
    cout << "Traffic Generator instance connected" << endl;
#endif
    // Initialize client
    InitializeReqMsg req;
    InitializeResMsg res;
    *_channel >> req << res;
    std::cout << req.type <<  ":Receiving initiation packet from Traffic Generator\n";
    std::cout << res.type << ": Sending response to Traffic Generator\n";
    std::cout << "==================Initialization Completed==================\n";
    return 0;
}

int Interface::Step() {
    bool process_more = true;
    StreamMessage *msg = NULL;
    while (process_more && _channel && _channel->isAlive()){
        *_channel >> (StreamMessage*&) msg;
        switch(msg->type){
            case STEP_REQ: {
                StepResMsg step;
                *_channel << step;
                process_more = false;
                break;
            }
            case INJECT_REQ: {
                InjectReqMsg *req = (InjectReqMsg *) msg;
                push(req->source, req->dest, (void *) req, req->size, req->msgType);
                break;
            }
            case INJECT_RES: {
                InjectResMsg *resInject = (InjectResMsg *) msg;
                push(resInject->source, resInject->dest, (void *) resInject, resInject->size, resInject->msgType);
                break;
            }
            case EJECT_REQ: {
                EjectReqMsg *ejReq =(EjectReqMsg *) msg;
                Flit* f = (Flit*)(pop(ejReq->size));
                EjectResMsg _res;
                _res.source = f->src;
                _res.dest = f->dest;
                _res.msgType = f->type;
                _res.id = f->id;
                _res.network = f->subnetwork;
                _res.cl = f->cl;
                *_channel << _res;
                break;
            }
            case EJECT_RES: {
                break;
            }
            case QUIT_REQ: {
                QuitResMsg quit;
                *_channel << quit;
                return 1;
                break;
            }
            default: {
                cout << "<Interface::Step> Unknown message type: " << msg->type << endl;
                break;
            }
        }
        StreamMessage::destroy(msg);
    }
}

void Interface::push(unsigned input_deviceID, unsigned output_deviceID, void *data, unsigned int size, int packet_type) {
    assert(HasBuffer(input_deviceID, size));
    int output_icntID = _node_map[output_deviceID];
    int input_icntID = _node_map[input_deviceID];
    unsigned int n_flits = size / _flit_size + ((size % _flit_size)? 1:0);
    int subnet;
    if (_subnets == 1) {
        subnet = 0;
    }
    _traffic_manager->_GeneratePacket( input_icntID, n_flits, 0, _traffic_manager->_time, subnet, packet_type, data, output_icntID);
}

void* Interface::pop(unsigned deviceID) {
    int icntID = _node_map[deviceID];
    void* data = NULL;
    int subnet = 0;
    int turn = _round_robin_turn[subnet][icntID];
    for (int vc = 0; (vc < _vcs) && (data == NULL); vc++) {
        if (_boundary_buffer[subnet][icntID][turn].HasPacket()) {
            data = _boundary_buffer[subnet][icntID][turn].PopPacket();
        }
        turn++;
        if (turn == _vcs) turn = 0;
    }
    if (data) {
        _round_robin_turn[subnet][icntID] = turn;
    }
    return data;
}

void Interface::WriteOutBuffer(int subnet, int output_icntID, Flit *flit) {
    int vc = flit->vc;
    assert (_ejection_buffer[subnet][output_icntID][vc].Size() < _ejection_buffer_capacity);
    _ejection_buffer[subnet][output_icntID][vc].PushFlitData(flit, flit->tail);
}

void Interface::Transfer2BoundaryBuffer(int subnet, int output){
    Flit* flit;
    int vc;
    for (vc=0; vc<_vcs;vc++) {
        if ( !_ejection_buffer[subnet][output][vc].empty() && _boundary_buffer[subnet][output][vc].Size() < _boundary_buffer_capacity ) {
            flit = (Flit*)(_ejection_buffer[subnet][output][vc].TopPacket());
            assert(flit);
            _ejection_buffer[subnet][output][vc].PopPacket();
            _boundary_buffer[subnet][output][vc].PushFlitData( flit->data, flit->tail);
            _ejected_flit_queue[subnet][output].push(flit); //indicate this flit is already popped from ejection buffer and ready for credit return
            if ( flit->head ) {
                assert (flit->dest == output);
            }
        }
    }
}

Flit* Interface::GetEjectedFlit(int subnet, int node){
    Flit* flit = NULL;
    if (!_ejected_flit_queue[subnet][node].empty()) {
        flit = _ejected_flit_queue[subnet][node].front();
        _ejected_flit_queue[subnet][node].pop();
    }
    return flit;
}

bool Interface::Busy() const {
    bool busy = !_traffic_manager->_total_in_flight_flits[0].empty();
    if (!busy) {
        for (int s = 0; s < _subnets; ++s) {
            for (unsigned n = 0; n < _n_shader; ++n) {
                //FIXME: if this cannot make sure _partial_packets is empty
                assert(_traffic_manager->_input_queue[s][n][0].empty());
            }
        }
    } else {
        return true;
    }
    for (int s = 0; s < _subnets; ++s) {
        for (unsigned n=0; n < _n_shader; ++n) {
            for (int vc=0; vc<_vcs; ++vc) {
                if (_boundary_buffer[s][n][vc].HasPacket() ) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Interface::HasBuffer(unsigned deviceID, unsigned int size) const
{
    bool has_buffer = false;
    unsigned int n_flits = size / _flit_size + ((size % _flit_size)? 1:0);
    int icntID = _node_map.find(deviceID)->second;
    has_buffer = _traffic_manager->_input_queue[0][icntID][0].size() +n_flits <= _input_buffer_capacity;
    if ((_subnets > 1) && deviceID >= _n_shader && deviceID < _n_shader)
        has_buffer = _traffic_manager->_input_queue[1][icntID][0].size() +n_flits <= _input_buffer_capacity;
    return has_buffer;
}

void* Interface::_BoundaryBufferItem::PopPacket()
{
    assert (_packet_n);
    void * data = NULL;
    void * flit_data = _buffer.front();
    while (data == NULL) {
        assert(flit_data == _buffer.front()); //all flits must belong to the same packet
        if (_tail_flag.front()) {
            data = _buffer.front();
            _packet_n--;
        }
        _buffer.pop();
        _tail_flag.pop();
    }
    return data;
}

void* Interface::_BoundaryBufferItem::TopPacket() const
{
    assert (_packet_n);
    void* data = NULL;
    void* temp_d = _buffer.front();
    while (data==NULL) {
        if (_tail_flag.front()) {
            data = _buffer.front();
        }
        assert(temp_d == _buffer.front()); //all flits must belong to the same packet
    }
    return data;

}

void Interface::_BoundaryBufferItem::PushFlitData(void* data,bool is_tail)
{
    _buffer.push(data);
    _tail_flag.push(is_tail);
    if (is_tail) {
        _packet_n++;
    }
}