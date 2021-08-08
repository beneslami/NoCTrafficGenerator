//
// Created by Ben on 6/30/21.
//
#include <map>
#include <list>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "globals.h"
#include "ReadFile.h"
#include "TrafficGenerator.h"

//Set this to 0 to debug without connecting to booksim
#define CONNECT 1

static unsigned long long cycle = 1;
static int messageId;
SocketStream m_channel;

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

void TrafficGenerator::initiateMessage(int source, int destination, int packetSize, int time, int type, int id/*for generating response*/) {
    if(type == READ_REQUEST || type == WRITE_REQUEST){
        struct InjectReqMsg request;
        request.source = source;
        request.dest = destination;
        request.packetSize = packetSize;
        request.msgType = type;
        request.type = INJECT_REQ;
        request.id = messageId++;
        Core_queues[source]->Enqueue_request(request, time);
    }
    /*else if (type == READ_REPLY || type == WRITE_REPLY){
        struct InjectResMsg response;
        response.source = source;
        response.dest = destination;
        response.packetSize = packetSize;
        response.msgType = type;
        response.type = INJECT_RES;
        response.id = id;
        Core_queues[source]->Enqueue_response(response, time);
    }*/
}

void TrafficGenerator::Inject() {
    for(int i = 0; i < _numCore; i++){
        if(Core_queues[i]->request_queue_size() != 0) {
            std::list<InjectReqMsg> packets = Core_queues[i]->DeQueue_request(cycle+1);
            std::list<InjectReqMsg>::iterator it;
            for (it = packets.begin(); it != packets.end(); ++it) {
                std::cout << "src: " << it->source << "\tdst: " << it->dest << "\tsize: " << it->packetSize << "\ttype: " << it->msgType << "\t deQ request from Core " << i << " in cycle: " << cycle <<"\n";
                sendRequestPacket(*it);
            }
            Core_queues[i]->cleanUp_request_queue(cycle+1);
        }
        if(Core_queues[i]->response_queue_size() != 0) {
            std::list<InjectResMsg> packet = Core_queues[i]->DeQueue_response(cycle);
            std::list<InjectResMsg>::iterator it2;
            for (it2 = packet.begin(); it2 != packet.end(); ++it2) {
                it2->type = INJECT_RES;
                sendResponsePacket(*it2);
            }
            Core_queues[i]->cleanUp_response_queue(cycle);
        }
    }
}

void TrafficGenerator::sendRequestPacket(InjectReqMsg &request) {
    if((int) request.address == -1){
        request.address = messageId*2 + 1; /* just for fun */
        inTransitTransactions[request.address].source = request.source;
        inTransitTransactions[request.address].dest = request.dest;
        //inTransitTransactions[request.address].acks_received = 0;
    }
    inTransitPackets[request.id] = request;
#if CONNECT
    InjectResMsg res;
    m_channel << request >> res; /* send the request packet to BookSim*/
    if(res.type == ACKNOWLEDGE){
        std::cout << "Request packet " << request.id << " has been received by BookSim sucessfully\n";
    }
#endif
}

void TrafficGenerator::sendResponsePacket(InjectResMsg &response) {
#if CONNECT
    InjectResMsg res;
    m_channel << response >> res; /* send the response packet to BookSim*/
    if(res.type == ACKNOWLEDGE){
        std::cout << "Response packet " << response.id << " has been received by BookSim sucessfully\n";
    }
#endif
}

TrafficGenerator::TrafficGenerator(int numCore) {
    _numCore = numCore;
    for(int i = 0; i < _numCore; i++){
        PacketQueue *p = new PacketQueue();
        Core_queues.push_back(p);
    }
}

TrafficGenerator::~TrafficGenerator() {

}

void TrafficGenerator::Eject() {
#if CONNECT
    EjectReqMsg request;
    EjectResMsg response;
    bool hasRequests = true;
    while(hasRequests) {
        m_channel << request >> response;
        if (response.msgType == READ_REQUEST || response.msgType == WRITE_REQUEST) {
            struct InjectResMsg response_back;
            response_back.dest = response.source;
            response_back.source = response.dest;
            response_back.packetSize = response.packetSize * 2 /* just for now */;
            if (response.type == READ_REQUEST)
                response_back.msgType = READ_REPLY;
            else if (response.type == WRITE_REQUEST)
                response_back.msgType = WRITE_REPLY;
            response_back.type = INJECT_RES;
            response_back.id = response.id;
            Core_queues[response_back.source]->Enqueue_response(response_back, cycle);
        }
        else if (response.msgType == READ_REPLY || response.msgType == WRITE_REPLY) {
            std::map<int, InjectReqMsg>::iterator it = inTransitPackets.find(response.id);
            if (it == inTransitPackets.end()) {
                std::cerr << "Error: couldn't find in transit packet " << response.id << std::endl;
                exit(-1);
            }
            /* Need to check if the packet belongs to current node or not. if yes, then it's done, otherwise, it should re-inject to Booksim */
            std::cout << "src: " << response.source << "\tdst: " << response.dest << "\tsize: " << response.packetSize
                      << " is received in cycle: " << cycle << "\n";
        }
        hasRequests = response.remainingRequests;
    }
#endif
}

void TrafficGenerator::Run() {
    std::vector<int>thresholds;
    if(_numCore == 1){
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
            dst = RandomGenerator::UniformDistribution(0, _numCore-1);
            src = RandomGenerator::UniformDistribution(0, _numCore-1);
        }
        else if(!destin.compare("normal")){
            dst = RandomGenerator::UniformDistribution(0, _numCore-1);
            src = RandomGenerator::UniformDistribution(0, _numCore-1);
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
                for(int j = 0; j < _numCore; j++) {
                    int source = src.Generate();
                    int destination = dst.Generate();
                    while (source == destination) {
                        destination = dst.Generate();
                    }
                    initiateMessage(source, destination, byteInject, cycle + i + 1, byteInject%2/* Test */, 0);
                }
                i++;
            }
            std::cout << "cycle: " << cycle << std::endl;
            Inject();
            Eject();
            cycle += threshold;

#if CONNECT
            StepReqMsg req;
		    StepResMsg res;
		    m_channel << req >> res;
#endif
        }
        exit();
    }

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
    if(_numCore == 1) {
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
        dst = RandomGenerator::UniformDistribution(0, _numCore-1);
        /*if(!destin.compare("uniform")){
            dst = RandomGenerator::UniformDistribution(0, _numCore-1);
        }
        else if(!destin.compare("normal")){
            dst = RandomGenerator::UniformDistribution(0, _numCore-1);
        }*/
        for(it = normal_stat.begin(); it != normal_stat.end(); ++it) {
            for (it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
                while (counter < 5) {
                    for(int src = 0; src < _numCore; src++) {
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