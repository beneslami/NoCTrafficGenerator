//
// Created by Ben on 6/30/21.
//
#include <map>
#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "globals.h"
#include "ReadFile.h"
#include "TrafficGenerator.h"

//Set this to 0 to debug without connecting to booksim
#define CONNECT 1

static unsigned long long cycle;
std::map<int, InjectReqMsg> inTransitPackets;
std::map<int, struct TrafficGenerator::transaction_t> inTransitTransactions;
int messageId = 0;
SocketStream m_channel;
PacketQueue packet_queue;

void connect() {
#if CONNECT
    // connect to network simulator
	m_channel.connect(NS_HOST, NS_PORT);

	// send request to initialize
	InitializeReqMsg req;
	InitializeResMsg res;
    std::cout << req.type <<": Sending Initiating packet to Booksim\n";
	m_channel << req >> res;
	if(res.size){
	    std::cout << res.type <<": initiating Booksim and Traffic Generator done successfully\n";
	}
    std::cout << "==================Initialization Completed==================\n";
#endif
}

void exit() {
#if CONNECT
    // Notify network we are quitting
	QuitReqMsg req;
	QuitResMsg res;
	m_channel << req >> res;
#endif
}

void TrafficGenerator::initiateMessage(int source, int destination, int packetSize, int time, int address) {
    InjectReqMsg packet;
    packet.source = source;
    packet.dest = destination;
    packet.cl = 0;
    packet.network = 0;
    packet.packetSize = packetSize;
    packet.msgType = 0;
    packet.address = address;
    packet_queue.Enqueue(packet, time);
}

void TrafficGenerator::Inject() {
    std::list<InjectReqMsg> packets = packet_queue.DeQueue(cycle);
    std::list<InjectReqMsg>::iterator it;
    for(it = packets.begin(); it != packets.end(); ++it) {
        sendPacket(*it);
    }
    packet_queue.cleanUp(cycle);
}

void TrafficGenerator::sendPacket(InjectReqMsg &req) {
    req.id = messageId;
    if((int) req.address == -1) {
        req.address = messageId;
        inTransitTransactions[req.address].source = req.source;
        inTransitTransactions[req.address].dest = req.dest;
        inTransitTransactions[req.address].acks_received = 0;
        inTransitTransactions[req.address].data_received = false; //TODO: check to see the type of packet
    }
    messageId++;
    inTransitPackets[req.id] = req;
#if CONNECT
    InjectResMsg res;
    m_channel << req >> res;
    if(res.size) {
        std::cout << "1- src: " << req.source << "\tdst: " << req.dest << "\tid: " << req.id << "\ttype: " << req.type
                  << "\tsize: " << req.size << "\tcycle: " << cycle << "\tsuccessfully injected to BookSim"
                  << std::endl;
    }
#endif
}

void react(EjectResMsg ePacket) {
    std::map<int, InjectReqMsg>::iterator it = inTransitPackets.find(ePacket.id);
    if(it == inTransitPackets.end()) {
        std::cerr << "Error: couldn't find in transit packet " << ePacket.id << std::endl;
        exit(-1);
    }

    InjectReqMsg request = it->second;
    std::cout << "2- src: " << request.source << "\tdst: " << request.dest << "\tid: " << request.id << "\ttype: " << request.type
              << "\tsize: " << request.size << "\tcycle: " << cycle << "\trequest received." << std::endl;
    InjectReqMsg response;
    inTransitPackets.erase(it);

    std::map<int, TrafficGenerator::transaction_t>::iterator trans = inTransitTransactions.find(request.address);

    switch(request.msgType) {
        case READ_REQUEST:
            std::cout << "Read Request arrived\n";
            break;
        case READ_REPLY:
            std::cout << "Read Reply arrived\n";
            break;
        case WRITE_REQUEST:
            std::cout << "Write Request arrived\n";
            break;
        case WRITE_REPLY:
            std::cout << "Write Reply arrived\n";
            break;
        default:
            std::cout << "unKnown packet arrived\n";
            break;
    }

    if(trans->second.Completed()) {
        inTransitTransactions.erase(trans);
    }
}

void TrafficGenerator::Eject() {
#if CONNECT
    EjectReqMsg req; //The request to the network
    EjectResMsg res; //The response from the network
    bool hasRequests = true; //Whether there are more requests from the network

    //Loop through all the network's messages
    while(hasRequests) {
        m_channel << req >> res;
        if(res.id >= 0) {
            //Add responses to list
            if(res.id > -1) {
                react(res);
            }
        }
        else{
            std::cout << res.type << " :No packet from BookSim\n";
        }
        //Check if there are more messages from the network
        hasRequests = res.remainingRequests;
    }
#endif
}

void TrafficGenerator::Run() {
    std::vector<int>thresholds;
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
        //Connect to network simulator
        connect();
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
            while(byteInject == 0){
                byteInject = byte.Generate();
            }
            int threshold = inter_arrival.at(byteInject).Generate();
            thresholds.push_back(threshold);
            int i = 0;
            while(i < threshold){ //todo: still problematic
                for(int j = 0; j < numCores; j++) {
                    int source = src.Generate();
                    int destination = dst.Generate();
                    while (source == destination) {
                        destination = dst.Generate();
                    }
                    initiateMessage(source, destination, byteInject, cycle + i + 1, 11111);
                }
                i++;
            }
            Inject();
            Eject();
            cycle += threshold;
#if CONNECT
            StepReqMsg req;
		    StepResMsg res;
		    m_channel << req >> res;
#endif
        }
    }
    exit();
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