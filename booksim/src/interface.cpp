//
// Created by Ben on 7/19/21.
//
#include "interface.h"
#include "IntersimConfig.h"
#include "routefunc.hpp"
#include "config_utils.hpp"
#include "TrafficGen.h"
#include "globals.hpp"
#include <stddef.h>

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
    _subnets = _icnt_config->GetInt("subnets");
    _vcs = config.GetInt("num_vcs");
    _host = _icnt_config->GetStr("_host");
    _port = _icnt_config->GetInt("_port");
    _flit_size = _icnt_config->GetInt("flit_width");
    if (_icnt_config->GetInt("input_buffer_size")) {
        _input_buffer_capacity = _icnt_config->GetInt("input_buffer_size");
    } else {
        _input_buffer_capacity = 9;
    }
    vector<int> mapping = _icnt_config->GetIntArray("mapping");
    for(int i = 0; i < mapping.size(); i++) {
        _node_map[i] = mapping[i];
    }
    _net = net;
    _n_shader = 4;
    _CreateBuffer();
    _CreateNodeMap(_n_shader, _traffic_manager->_nodes, _icnt_config->GetInt("use_map"));
    InitializeRoutingMap(*_icnt_config);
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
                *_channel << step;
                process_more = false;
                break;
            }
            case INJECT_REQ: {
                InjectReqMsg *req = (InjectReqMsg *) msg;
                push(req->source, req->dest, (void *) req, req->size, req->msgType);
                InjectResMsg res;
                res.type = ACKNOWLEDGE;
                *_channel << res;
                break;
            }
            case INJECT_RES: {
                InjectResMsg *resInject = (InjectResMsg *) msg;
                push(resInject->source, resInject->dest, (void *) resInject, resInject->size, resInject->msgType);
                InjectResMsg resAck;
                resAck.type = ACKNOWLEDGE;
                *_channel << resAck;
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
    unsigned int n_flits = (size / _flit_size) + ((size % _flit_size)? 1:0);
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
        std::cout << "I'm inside\n";
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
    /*
      READ_REQUEST  = 0,
      READ_REPLY    = 1,
      WRITE_REQUEST = 2,
      WRITE_REPLY   = 3,
      ANY_TYPE      = 4
      */
    bool has_buffer = false;
    unsigned int n_flits = (unsigned int)(size / _flit_size) + ((size % _flit_size)? 1:0);
    int icntID = _node_map.find(deviceID)->second;
    has_buffer = (_traffic_manager->_input_queue[0][icntID][0].size() + n_flits <= _input_buffer_capacity);
    if ((_subnets > 1))
        has_buffer = (_traffic_manager->_input_queue[1][icntID][0].size() +n_flits <= _input_buffer_capacity);
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

void Interface::_CreateNodeMap(unsigned n_shader, unsigned n_node, int use_map) {
    if (use_map) {
        map<unsigned, vector<unsigned> > preset_memory_map;
        {
            unsigned memory_node[] = {1, 3, 4, 6, 9, 11, 12, 14};
            preset_memory_map[16] = vector<unsigned>(memory_node, memory_node+8);
        }
        {
            unsigned memory_node[] = {3, 7, 10, 12, 23, 25, 28, 32};
            preset_memory_map[36] = vector<unsigned>(memory_node, memory_node+8);
        }
        {
            unsigned memory_node[] = {3, 15, 17, 29, 36, 47, 49, 61};
            preset_memory_map[64] = vector<unsigned>(memory_node, memory_node+sizeof(memory_node)/sizeof(unsigned));
        }
        {
            unsigned memory_node[] = {12, 20, 25, 28, 57, 60, 63, 92, 95,100,108};
            preset_memory_map[121] = vector<unsigned>(memory_node, memory_node+sizeof(memory_node)/sizeof(unsigned));
        }

        {
            unsigned memory_node[] = {0, 2, 4, 6, 8, 10, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 44, 46, 48, 50, 52, 54, 56, 58, 62, 64, 66, 68, 70, 72, 76, 78, 80, 82, 84, 86, 88, 90, 92, 96, 98, 100, 102, 104, 106, 110, 112, 114, 116, 118, 122, 124, 126, 128, 130, 132, 134 , 138, 140, 142};
            preset_memory_map[144] = vector<unsigned>(memory_node, memory_node+sizeof(memory_node)/sizeof(unsigned));
        }

        const vector<unsigned> &memory_node = preset_memory_map[n_node];
        if (memory_node.empty()) {
            cerr<<"ERROR!!! NO MAPPING IMPLEMENTED YET FOR THIS CONFIG"<<endl;
            assert(0);
        }

        // create node map
        unsigned next_node = 0;
        unsigned memory_node_index = 0;
        for (unsigned i = 0; i < n_shader; ++i) {
            while (next_node == memory_node[memory_node_index]) {
                next_node += 1;
                memory_node_index += 1;
            }
            _node_map[i] = next_node;
            next_node += 1;
        }
        for (unsigned i = n_shader; i < n_shader; ++i) {
            _node_map[i] = memory_node[i-n_shader];
        }
    } else { //not use preset map
        for (unsigned i=0;i<n_node;i++) {
            _node_map[i]=i;
        }
    }

    for (unsigned i = 0; i < n_node ; i++) {
        for (unsigned j = 0; j< n_node ; j++) {
            if ( _node_map[j] == i ) {
                _reverse_node_map[i]=j;
                break;
            }
        }
    }
}