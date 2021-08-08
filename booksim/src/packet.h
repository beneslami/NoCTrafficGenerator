//
// Created by Ben on 8/1/21.
//

#ifndef PACKET_H
#define PACKET_H


class Packet {
private:
    int _source;
    int _destination;
    int _size;
    int _messageType;
    int _network;
    int _cl;
public:
    Packet();
    ~Packet();
    void set_source(int source){
        _source = source;
    }
    void set_destination(int destination){
        _destination = destination;
    }
    void set_size(int size){
        _size = size;
    }
    void set_messageType(int type){
        _messageType = type;
    }
    void set_network(int net){
        _network = net;
    }
    void set_class(int cl){
        _cl = cl;
    }
    int get_source(){
        return _source;
    }
    int get_destination(){
        return _destination;
    }
    int get_size(){
        return _size;
    }
    int get_messageType(){
        return _messageType;
    }
    int get_network(){
        return _network;
    }
    int get_class(){
        return _cl;
    }
};


#endif