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
#include <vector>
#include <algorithm>

//Set this to 0 to debug without connecting to booksim
#define CONNECT 0

std::map<int, InjectReqMsg> inTransitPackets;
std::map<int, struct TrafficGenerator::transaction_t> inTransitTransactions;
PacketQueue packet_queue;
int messageId = 0;

void TrafficGenerator::initiateMessage(int source, int destination, int packetSize, int time, int address) {
    InjectReqMsg packet;
    packet.source = source;
    packet.dest = destination;
    packet.cl = 0;
    packet.network = 0;
    packet.packetSize = packetSize;
    packet.msgType = 0; // TODO: change the code to distinguish packet type
    packet.address = address;
    //std::cout << "src: " << source << "\tdst: " << destination << "\tbyte: " << byteInject << "\tcycle: "<< cycle << std::endl;
    packet_queue.Enqueue(packet, time);
}

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
#if CONNECT
    InjectResMsg res;
	m_channel << req >> res;
#endif
}

void TrafficGenerator::Inject(int time) {
    std::list<InjectReqMsg> packets = packet_queue.DeQueue(time);
    std::list<InjectReqMsg>::iterator it;

    for(it = packets.begin(); it != packets.end(); ++it) {
        sendPacket(*it);
    }
    packet_queue.cleanUp(time);
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
    //std::ofstream myfile;
    std::vector<int>thresholds;
    //myfile.open ("example.txt");
    //std::map<int, double>::iterator pointer;
    int traffic_cycle = 0;
    int cycle = 1;
    if(numCores == 1){
        RandomGenerator::CustomDistribution byte = RandomGenerator::CustomDistribution(bytes);
        std::map<int, RandomGenerator::CustomDistribution> inter_arrival;
        std::map<int, std::map<double, double> >::iterator it;
        for(it = duration.begin(); it != duration.end(); ++it){
            RandomGenerator::CustomDistribution temp = RandomGenerator::CustomDistribution(it->second);
            inter_arrival.insert(std::pair<int, RandomGenerator::CustomDistribution>(it->first, temp));
        }
        while(cycle < numCycles){
            int threshold;
            double byteInject = byte.Generate();
            threshold = inter_arrival.at(byteInject).Generate();
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
            cycle += threshold;
        }
    }
    else{
        RandomGenerator::UniformDistribution dst;
        RandomGenerator::UniformDistribution src;
        if(!destin.compare("uniform")){
            dst = RandomGenerator::UniformDistribution(0, numCores-1);
            src = RandomGenerator::UniformDistribution(0, numCores-1);
        }
        else if(!destin.compare("normal")){
            dst = RandomGenerator::UniformDistribution(0, numCores-1);
            src = RandomGenerator::UniformDistribution(0, numCores-1);
        }

        RandomGenerator::CustomDistribution byte = RandomGenerator::CustomDistribution(bytes);

        std::map<int, RandomGenerator::CustomDistribution> inter_arrival;
        std::map<int, std::map<double, double> >::iterator it;
        for(it = duration.begin(); it != duration.end(); ++it){
            RandomGenerator::CustomDistribution temp = RandomGenerator::CustomDistribution(it->second);
            inter_arrival.insert(std::pair<int, RandomGenerator::CustomDistribution>(it->first, temp));
        }

        while(cycle < numCycles) {
            int byteInject = byte.Generate();
            int threshold = inter_arrival.at(byteInject).Generate();
            thresholds.push_back(threshold);
            int i = 0;
            while(i < threshold){
                int source = src.Generate();
                int destination = dst.Generate();
                while (source == destination) {
                    destination = dst.Generate();
                }
                if (traffic.find(cycle + i + 1) != traffic.end()) {
                    traffic[cycle + i + 1] += byteInject;
                } else {
                    traffic[cycle + i + 1] = byteInject;
                }
                initiateMessage(source, destination, byteInject, cycle + i + 1, 11111);
                i++;
            }
            Inject(cycle + i + 1);
            /*
             * eject
             */
            cycle += threshold;
        }
    }
    std::cout << packet_queue.Size() << std::endl;
    std::cout << inTransitPackets.size() << std::endl;
    /*for(pointer = traffic.begin(); pointer != traffic.end(); ++pointer){
       myfile << pointer->first << "," << pointer->second << std::endl;
    }
    myfile.close();*/
}

void TrafficGenerator::Run2() {
    std::ofstream myfile;
    myfile.open ("example.txt");
    int cycle = 1;
    int counter = 0;
    int rand;
    int traffic_cycle = 0;
    int source;
    int destination;
    std::map<int, std::map<float, float> >::iterator it;
    std::map<float, float>::iterator it2;
    if(numCores == 1) {
        for(it = normal_stat.begin(); it != normal_stat.end(); ++it) {
            for (it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                while (counter < 5) {
                    RandomGenerator::NormalDistribution normal = RandomGenerator::NormalDistribution(it2->first,
                                                                                                     it2->second);
                    rand = normal.generate();
                    while (rand < 0) {
                        rand = normal.generate();
                    }
                    traffic.insert(std::pair<int, int>(counter + cycle, rand));
                    counter += 1;
                }
            }
            cycle += counter;
            counter = 0;
        }
    }
    else {
        RandomGenerator::UniformDistribution dst;
        dst = RandomGenerator::UniformDistribution(0, numCores-1);
        /*if(!destin.compare("uniform")){
            dst = RandomGenerator::UniformDistribution(0, numCores-1);
        }
        else if(!destin.compare("normal")){
            dst = RandomGenerator::UniformDistribution(0, numCores-1);
        }*/
        for(it = normal_stat.begin(); it != normal_stat.end(); ++it) {
            for (it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                while (counter < 5) {
                    for(int src = 0; src < numCores; src++) {
                        RandomGenerator::NormalDistribution normal = RandomGenerator::NormalDistribution(it2->first,
                                                                                                         it2->second);
                        source = src;
                        destination = dst.Generate();
                        while (source == destination) {
                            destination = dst.Generate();
                        }
                        rand = normal.generate();
                        while (rand < 0) {
                            rand = normal.generate();
                        }
                        traffic_cycle += rand;
                        std::cout << "src: " << source << "\tdst: " << destination << "\tbyte: " << rand << "\tcycle: " << cycle + counter << std::endl;
                    }
                    traffic.insert(std::pair<int, int>(counter + cycle, traffic_cycle));
                    counter += 1;
                }
            }
            cycle += counter;
            counter = 0;
        }
    }
    std::map<int, double>::iterator it3;
    for(it3 = traffic.begin(); it3 != traffic.end(); ++it3){
        myfile << it3->first << ", " << it3->second << std::endl;
    }
}