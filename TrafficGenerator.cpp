//
// Created by Ben on 6/30/21.
//

#include "TrafficGenerator.h"
#include <list>
#include <map>
#include <iostream>
#include <fstream>

extern unsigned long long numCycles;
extern int numCores;
int window = 5;
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
    std::map<double, double>b;
    b.insert(std::pair<double, double>(0.0, 84884.0));
    b.insert(std::pair<double, double>(8.0, 19798.0));
    b.insert(std::pair<double, double>(136.0, 95318.0));
    RandomGenerator::CustomDistribution byte = RandomGenerator::CustomDistribution(b);
    byte.show();
    RandomGenerator::UniformDistribution dst = RandomGenerator::UniformDistribution(0, 3);

    b.clear();
    b.insert(std::pair<double, double>(1, 76420));
    b.insert(std::pair<double, double>(2, 18823));
    b.insert(std::pair<double, double>(3, 8601));
    b.insert(std::pair<double, double>(4, 4796));
    b.insert(std::pair<double, double>(5, 2930));
    b.insert(std::pair<double, double>(6, 1626));
    b.insert(std::pair<double, double>(7, 903));
    b.insert(std::pair<double, double>(8, 484));
    b.insert(std::pair<double, double>(9, 227));
    b.insert(std::pair<double, double>(10, 152));
    b.insert(std::pair<double, double>(11, 69));
    b.insert(std::pair<double, double>(12, 32));
    b.insert(std::pair<double, double>(13, 16));
    b.insert(std::pair<double, double>(14, 13));
    b.insert(std::pair<double, double>(15, 8));
    b.insert(std::pair<double, double>(16, 6));
    b.insert(std::pair<double, double>(17, 1));
    b.insert(std::pair<double, double>(18, 1));
    b.insert(std::pair<double, double>(20, 1));
    b.insert(std::pair<double, double>(24, 1));
    b.insert(std::pair<double, double>(32, 1));
    b.insert(std::pair<double, double>(54, 1));
    b.insert(std::pair<double, double>(94, 1));
    b.insert(std::pair<double, double>(95, 1));
    b.insert(std::pair<double, double>(249, 1));
    b.insert(std::pair<double, double>(250, 1));
    b.insert(std::pair<double, double>(522, 1));
    RandomGenerator::CustomDistribution byte_0 = RandomGenerator::CustomDistribution(b);

    b.clear();
    b.insert(std::pair<double, double>(1, 5966));
    b.insert(std::pair<double, double>(2, 1332));
    b.insert(std::pair<double, double>(3, 746));
    b.insert(std::pair<double, double>(4, 434));
    b.insert(std::pair<double, double>(5, 262));
    b.insert(std::pair<double, double>(6, 158));
    b.insert(std::pair<double, double>(7, 126));
    b.insert(std::pair<double, double>(8, 73));
    b.insert(std::pair<double, double>(9, 55));
    b.insert(std::pair<double, double>(10, 40));
    b.insert(std::pair<double, double>(11, 37));
    b.insert(std::pair<double, double>(12, 19));
    b.insert(std::pair<double, double>(13, 26));
    b.insert(std::pair<double, double>(14, 14));
    b.insert(std::pair<double, double>(15, 11));
    b.insert(std::pair<double, double>(16, 10));
    b.insert(std::pair<double, double>(17, 4));
    b.insert(std::pair<double, double>(18, 5));
    b.insert(std::pair<double, double>(19, 6));
    b.insert(std::pair<double, double>(21, 4));
    b.insert(std::pair<double, double>(22, 2));
    b.insert(std::pair<double, double>(23, 2));
    b.insert(std::pair<double, double>(24, 2));
    b.insert(std::pair<double, double>(25, 2));
    b.insert(std::pair<double, double>(26, 4));
    b.insert(std::pair<double, double>(27, 2));
    b.insert(std::pair<double, double>(28, 1));
    b.insert(std::pair<double, double>(32, 1));
    b.insert(std::pair<double, double>(33, 1));
    b.insert(std::pair<double, double>(35, 1));
    b.insert(std::pair<double, double>(143, 1));
    b.insert(std::pair<double, double>(169, 1));
    b.insert(std::pair<double, double>(332, 1));
    RandomGenerator::CustomDistribution byte_8 = RandomGenerator::CustomDistribution(b);

    b.clear();
    b.insert(std::pair<double, double>(1, 18053));
    b.insert(std::pair<double, double>(2, 9499));
    b.insert(std::pair<double, double>(3, 5742));
    b.insert(std::pair<double, double>(4, 3398));
    b.insert(std::pair<double, double>(5, 1904));
    b.insert(std::pair<double, double>(6, 1116));
    b.insert(std::pair<double, double>(7, 602));
    b.insert(std::pair<double, double>(8, 328));
    b.insert(std::pair<double, double>(9, 210));
    b.insert(std::pair<double, double>(10, 81));
    b.insert(std::pair<double, double>(11, 57));
    b.insert(std::pair<double, double>(12, 31));
    b.insert(std::pair<double, double>(13, 14));
    b.insert(std::pair<double, double>(14, 3));
    b.insert(std::pair<double, double>(15, 3));
    b.insert(std::pair<double, double>(16, 2));
    b.insert(std::pair<double, double>(17, 1));
    b.insert(std::pair<double, double>(18, 2));
    b.insert(std::pair<double, double>(25, 1));
    b.insert(std::pair<double, double>(38, 1));
    b.insert(std::pair<double, double>(45, 1));
    b.insert(std::pair<double, double>(62, 1));
    b.insert(std::pair<double, double>(63, 9));
    b.insert(std::pair<double, double>(64, 8));
    b.insert(std::pair<double, double>(70, 1));
    b.insert(std::pair<double, double>(101, 1));
    RandomGenerator::CustomDistribution byte_136 = RandomGenerator::CustomDistribution(b);

    std::map<int, int>traffic;
    std::map<int, int>::iterator pointer;
    while(cycle < numCycles){
        int threshold;
        for (int src = 0; src < numCores; src++){
            int source = src;
            int destination = dst.Generate();
            while(source == destination){
                destination = dst.Generate();
            }
            if (destination != src) {
                int byteInject = byte.Generate();
                if(byteInject == 0){
                    threshold = byte_0.Generate();
                    int i = 0;
                    while(i < threshold){
                        if(traffic.find(cycle + i) != traffic.end()){
                            traffic[cycle+i] += byteInject;
                        }
                        else{
                            traffic[cycle+i] = byteInject;
                        }
                        i++;
                    }
                }
                else if(byteInject == 8){
                    threshold = byte_8.Generate();
                    int i = 0;
                    while(i < threshold){
                        if(traffic.find(cycle + i) != traffic.end()){
                            traffic[cycle+i] += byteInject;
                        }
                        else{
                            traffic[cycle+i] = byteInject;
                        }
                        i++;
                    }
                }
                else if(byteInject == 136){
                    threshold = byte_136.Generate();
                    int i = 0;
                    while(i < threshold){
                        if(traffic.find(cycle + i) != traffic.end()){
                            traffic[cycle + i] += byteInject;
                        }
                        else{
                            traffic[cycle + i] = byteInject;
                        }
                        i++;
                    }
                }
            }
        }
        cycle += threshold;
        /*
         * 3- queue the generated messages to corresponding queues
         * 4- call Enqueue
         * */

        /*Inject all of this cycles' messages into the network
         * 5- in Inject(), call DeQueue
         * 6- call send_packet() to fill inTransitTransactions
         * 7- queue cleanup
         * */
        //Inject();

        /*Eject from network
         * 8- loop through all network's messages
         * 9- in react() function, look inTransitPackets.find()
         * 10- based on the packet type, push the packet into the destination queue
         * */
        //Eject();

        /*
         * 11- step the network
         * */
    }

    for(pointer = traffic.begin(); pointer != traffic.end(); ++pointer){
        myfile << pointer->first << "," << pointer->second << std::endl;
    }
    myfile.close();
}

