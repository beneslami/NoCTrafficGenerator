//
// Created by Ben on 7/19/21.
//
#include "interface.h"
#include "routefunc.hpp"
#include "config_utils.hpp"
#include "globals.hpp"
#include <stddef.h>
#include "MCMGPUTrafficManager.h"

int quit_flag = 0;
extern MCMGPUTrafficManager *_traffic_manager;
Interface* Interface::interface_result = NULL;
Interface* Interface::get_instance(Configuration const & config, vector<Network *> const & net) {
    if(interface_result == NULL){
        interface_result = new Interface(config, net);
        return interface_result;
    }
    return interface_result;
}

Interface::Interface(const Configuration &config, const vector<Network *> &net) {
    _channel = NULL;
    _icnt_config = new IntersimConfig();
    _host = _icnt_config->GetStr("_host");
    _port = _icnt_config->GetInt("_port");
    if (_icnt_config->GetInt("ejection_buffer_size")) {
        _ejection_buffer_capacity = _icnt_config->GetInt( "ejection_buffer_size" ) ;
    } else {
        _ejection_buffer_capacity = _icnt_config->GetInt( "vc_buf_size" );
    }

    _boundary_buffer_capacity = _icnt_config->GetInt( "boundary_buffer_size" ) ;
    assert(_boundary_buffer_capacity);
    if (_icnt_config->GetInt("input_buffer_size")) {
        _input_buffer_capacity = _icnt_config->GetInt("input_buffer_size");
    } else {
        _input_buffer_capacity = 9;
    }
    _subnets = _icnt_config->GetInt("subnets");
    _vcs = _icnt_config->GetInt("num_vcs");
    _n_shader = 4;  /* for now */
    _flit_size = _icnt_config->GetInt("flit_size");
    _CreateBuffer();
    Init();
}

Interface::~Interface() {

}

int Interface::Init() {
    if (_listenSocket.listen(_host, _port) < 0) {
        return -1;
    }
    std::cout << "listening on port " << _port << std::endl;
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

int Interface::GetIcntTime() const {
    return _traffic_manager->getTime();
}

Stats* Interface::GetIcntStats(const string &name) const {
    return _traffic_manager->getStats(name);
}

void Interface::_CreateBuffer() {
    unsigned nodes = _n_shader;
    _boundary_buffer.resize(_subnets);
    _ejection_buffer.resize(_subnets);
    _round_robin_turn.resize(_subnets);
    _ejected_flit_queue.resize(_subnets);

    for (int subnet = 0; subnet < _subnets; ++subnet) {
        _ejection_buffer[subnet].resize(nodes);
        _boundary_buffer[subnet].resize(nodes);
        _round_robin_turn[subnet].resize(nodes);
        _ejected_flit_queue[subnet].resize(nodes);

        for (unsigned node = 0; node < nodes; ++node){
            _ejection_buffer[subnet][node].resize(_vcs);
            _boundary_buffer[subnet][node].resize(_vcs);
        }
    }
}

int Interface::Step() {
    bool process_more = true;
    StreamMessage *msg = NULL;
    while (process_more && _channel && _channel->isAlive()){
        *_channel >> (StreamMessage*&) msg;
        switch(msg->type){
            case STEP_REQ: {
                StepResMsg step;
                if(_traffic_manager->get_cycleTime() == 20){
                    step.go_on = 0;
                    //quit_flag = 1;
                }
                else{
                    step.go_on = 1;
                    quit_flag = 0;
                }
                *_channel << step;
                process_more = false;
                break;
            }
            case INJECT_REQ: {
                InjectReqMsg *req = (InjectReqMsg *) msg;
                push(req->source, req->dest, (void *) req, req->size, req->msgType);
                std::cout << "REQ source: " << req->source << "\tdest: " << req->dest << "\tsize: " << req->size << "\tid: "<< req->id << " is received"<< std::endl;
                InjectResMsg res;
                res.type = ACKNOWLEDGE;
                *_channel << res;
                break;
            }
            case INJECT_RES: {
                InjectResMsg *resInject = (InjectResMsg *) msg;
                push(resInject->source, resInject->dest, (void *) resInject, resInject->size, resInject->msgType);
                std::cout << "RES source: " << resInject->source << "\tdest: " << resInject->dest << "\tsize: " << resInject->size << "\tid: "<< resInject->id << " is received"<< std::endl;
                InjectResMsg resAck;
                resAck.type = ACKNOWLEDGE;
                *_channel << resAck;
                break;
            }
            case EJECT_REQ: {
                EjectReqMsg *ejReq =(EjectReqMsg *) msg;
                Flit* f = (Flit*)(pop(ejReq->coreNum));
                EjectResMsg _res;
                if(f) {
                    _res.source = f->src;
                    _res.dest = f->dest;
                    _res.msgType = f->type;
                    _res.id = f->id;
                    _res.network = f->subnetwork;
                    _res.cl = f->cl;
                    _res.remainingRequests = 1;
                }
                else{
                    _res.source = -1;
                    _res.dest = -1;  /* source and destination -1 means no packet to eject */
                    _res.remainingRequests = 0; // needs to be changed
                }
                *_channel << _res;
                break;
            }
            case EJECT_RES: {
                break;
            }
            case QUIT_REQ: {
                QuitReqMsg *quitReq = (QuitReqMsg*)msg;
                QuitResMsg quit;
                quit.flag = 1;
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
    int output_icntID = output_deviceID;
    int input_icntID = input_deviceID;
    unsigned int n_flits = (size / _flit_size) + ((size % _flit_size)? 1:0);
    int subnet;
    if (_subnets == 1) {
        subnet = 0;
    }
    Flit::FlitType type;
    switch(packet_type){
        case 0:   type = Flit::READ_REQUEST;  break;
        case 1:   type = Flit::READ_REPLY;    break;
        case 2:   type = Flit::WRITE_REQUEST; break;
        case 3:   type = Flit::WRITE_REPLY;   break;
        default:  type = Flit::ANY_TYPE;      break;
    }
    _traffic_manager->_GeneratePacket( input_icntID, 0, _traffic_manager->_time, subnet, n_flits, type, data, output_icntID);
}

void* Interface::pop(unsigned deviceID) {
    int icntID = deviceID;
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
    for (vc=0; vc < _vcs; vc++) {
        if ( !_ejection_buffer[subnet][output][vc].empty() && _boundary_buffer[subnet][output][vc].Size() < _boundary_buffer_capacity ) {
            flit = (Flit*)(_ejection_buffer[subnet][output][vc].TopPacket());
            std::cout << "src: " <<  flit->src << "\tdst: " << flit->dest << "\tid: " << flit->id << std::endl;
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
    assert(_ejected_flit_queue[subnet][node].empty() != 0);
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
    unsigned int n_flits = (unsigned int)(size / _flit_size) + ((size % _flit_size)? 1:0);
    int icntID = deviceID;
    has_buffer = (_traffic_manager->_input_queue[0][icntID][0].size() + n_flits <= _input_buffer_capacity);
    if ((_subnets > 1))
        has_buffer = (_traffic_manager->_input_queue[1][icntID][0].size() + n_flits <= _input_buffer_capacity);
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

bool Interface::_BoundaryBufferItem::empty() {
    if(_packet_n == 0){
        return true;
    }
    else{
        return false;
    }
}