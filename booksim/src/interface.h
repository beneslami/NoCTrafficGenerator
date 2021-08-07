//
// Created by Ben on 7/19/21.
//

#ifndef INTERFACE_H
#define INTERFACE_H

#include <map>
#include <queue>
#include "module.hpp"
#include "config_utils.hpp"
#include "netstream/messages.h"
#include "networks/network.hpp"
#include "netstream/socketstream.h"

class TrafficGen;
class IntersimConfig;

struct RequestPacket {
    int source;
    int dest;
    int id;
    int size;
    int type;
    int network;
    int cl;
};

struct ReplyPacket {
    int source;
    int dest;
    int id;
    int size;
    int type;
    int network;
    int cl;
};

class Interface {
public:
    Interface(const Configuration &, const vector<Network *> & net);
    static Interface* get_instance(Configuration const & , vector<Network *> const & );
    ~Interface();
    void push(unsigned input_deviceID, unsigned output_deviceID, void *data, unsigned int size, int type);
    void *pop(unsigned);
    int Init();
    int Step();
    void WriteOutBuffer(int, int, Flit*);
    void Transfer2BoundaryBuffer(int, int);
    Flit *GetEjectedFlit(int, int);
    bool Busy()const;
    bool HasBuffer(unsigned, unsigned int) const;
    void _CreateBuffer();
protected:
    static Interface* interface_result;
    class _BoundaryBufferItem {
    public:
        _BoundaryBufferItem():_packet_n(0) {}
        inline unsigned Size(void) const { return _buffer.size(); }
        inline bool HasPacket() const { return _packet_n; }
        void* PopPacket();
        void* TopPacket() const;
        void PushFlitData(void* data,bool is_tail);
        bool empty(){ return _packet_n == 0 ? true : false; }

    private:
        queue<void *> _buffer;
        queue<bool> _tail_flag;
        int _packet_n;
    };
    typedef queue<Flit*> _EjectionBufferItem;

private:
    SocketStream _listenSocket;
    SocketStream *_channel;
    std::string _host;
    int _port;

    unsigned _n_shader;
    vector<Network *> _net;
    int _vcs;
    int _subnets;
    unsigned _flit_size;
    IntersimConfig* _icnt_config;
    TrafficGen *_traffic_manager;
    map<unsigned, unsigned> _node_map;
    map<unsigned, unsigned> _reverse_node_map;
    vector<vector<int> > _round_robin_turn;

    vector<vector<vector<_BoundaryBufferItem> > > _boundary_buffer;
    unsigned int _boundary_buffer_capacity;
    vector<vector<vector<_BoundaryBufferItem> > > _ejection_buffer;
    unsigned int _ejection_buffer_capacity;
    vector<vector<queue<Flit* > > > _ejected_flit_queue;
    unsigned int _input_buffer_capacity;
};

#endif
