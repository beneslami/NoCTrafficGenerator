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
    while(cycle < numCycles){
        RandomGenerator::PoissonDistribution src_msgNum = RandomGenerator::PoissonDistribution(2.2);;
        RandomGenerator::UniformDistribution dst = RandomGenerator::UniformDistribution(0, 3);;
        RandomGenerator::BernoulliDistribution msgType = RandomGenerator::BernoulliDistribution(0.3);;

        for (int src = 0; src < numCores; src++){
            InjectReqMsg message;
            int source = src;
            int destination = dst.Generate();
            int numOfMsg = src_msgNum.Generate();
            if(numOfMsg != 0) {
                if (destination != src) {
                    RandomGenerator::CustomDistribution c;
                    message.source = source;
                    message.dest = destination;
                    message.packetSize = c.Generate();
                    message.network = 0;
                    message.id = messageId;
                    message.address = 0;
                    myfile << cycle << "," << message.packetSize << std::endl;
                }
            }
        }
        cycle += 1;


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
    myfile.close();
}

