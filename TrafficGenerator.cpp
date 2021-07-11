//
// Created by Ben on 6/30/21.
//

#include "TrafficGenerator.h"
#include "ReadFile.h"
#include "globals.h"
#include <list>
#include <map>
#include <iostream>
#include <fstream>

std::map<int, InjectReqMsg> inTransitPackets;
std::map<int, struct TrafficGenerator::transaction_t> inTransitTransactions;
PacketQueue packet_queue;
int messageId = 0;

void TrafficGenerator::sendPacket(InjectReqMsg &req) {
    req.id = messageId;
    if((int) req.address == -1) {
        req.address = messageId;
        inTransitTransactions[req.address].source = req.source;
        inTransitTransactions[req.address].dest = req.dest;
        inTransitTransactions[req.address].acks_received = 0;
    }
    messageId++;
    inTransitPackets[req.id] = req;
}

void TrafficGenerator::Inject() {
    std::list<InjectReqMsg> packets = packet_queue.DeQueue(numCycles);
    std::list<InjectReqMsg>::iterator it;

    for(it = packets.begin(); it != packets.end(); ++it) {
        sendPacket(*it);
    }
    packet_queue.cleanUp(numCycles);
}

void TrafficGenerator::Eject() {
    EjectReqMsg req; //The request to the network
    EjectResMsg res; //The response from the network
    bool hasRequests = true; //Whether there are more requests from the network

    //Loop through all the network's messages
    while(hasRequests) {
        //m_channel << req >> res; SocketStream m_channel;

        if(res.id >= 0) {
            //Add responses to list
            if(res.id > -1) {
                react(res);
            }
        }
        //Check if there are more messages from the network
        hasRequests = res.remainingRequests;
    }
}

void TrafficGenerator::react(EjectResMsg ePacket){
    std::map<int, InjectReqMsg>::iterator it = inTransitPackets.find(ePacket.id);
    if(it == inTransitPackets.end()) {
        std::cerr << "Error: couldn't find in-transit packet " << ePacket.id << std::endl;
        exit(-1);
    }
    InjectReqMsg request = it->second;
    InjectReqMsg response;
    inTransitPackets.erase(it);

    std::map<int, transaction_t>::iterator trans = inTransitTransactions.find(request.address);

    if(request.msgType == READ_REQUEST || request.msgType == WRITE_REQUEST) {

     }

    else if(request.msgType == READ_REPLY || request.msgType == WRITE_REPLY) {

    }

    if(trans->second.Completed()) {
        inTransitTransactions.erase(trans);
    }
}

void TrafficGenerator::Run() {
    std::ofstream myfile;
    myfile.open ("example.txt");
    int cycle = 1;
    RandomGenerator::UniformDistribution dst;
    if(!destin.compare("uniform")){
        dst = RandomGenerator::UniformDistribution(0, numCores-1);
    }
    else if(!destin.compare("normal")){
        dst = RandomGenerator::UniformDistribution(0, numCores-1);
    }

    RandomGenerator::CustomDistribution byte = RandomGenerator::CustomDistribution(bytes);

    std::map<int, RandomGenerator::CustomDistribution> inter_arrival;
    std::map<int, std::map<double, double> >::iterator it;
    for(it = duration.begin(); it != duration.end(); ++it){
        RandomGenerator::CustomDistribution temp = RandomGenerator::CustomDistribution(it->second);
        inter_arrival.insert(std::pair<int, RandomGenerator::CustomDistribution>(it->first, temp));
    }
    /*std::map<int, RandomGenerator::CustomDistribution>::iterator i;
    for(i = inter_arrival.begin(); i != inter_arrival.end(); ++i){
        std::cout << i->first << std::endl;
    }*/

    std::map<int, int>traffic;
    std::map<int, int>::iterator pointer;
    while(cycle < numCycles){
        int threshold;
        for (int src = 0; src < numCores; src++) {
            int source = src;
            int destination = dst.Generate();
            while (source == destination) {
                destination = dst.Generate();
            }
            if (destination != src) {
                int byteInject = byte.Generate();
                threshold = inter_arrival.at(byteInject).Generate();
                int i = 0;
                while(i < threshold){
                    if(traffic.find(cycle + i) != traffic.end()){
                        traffic[cycle + i] += byteInject;
                    }
                    else{
                        traffic[cycle +i ] = byteInject;
                    }
                    i++;
                }
            }
        }
        cycle += threshold;
    }

    for(pointer = traffic.begin(); pointer != traffic.end(); ++pointer){
       myfile << pointer->first << "," << pointer->second << std::endl;
   }

    myfile.close();
}

