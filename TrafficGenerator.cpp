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
    b.insert(std::pair<double, double>(8.0, 19798.0));
    b.insert(std::pair<double, double>(136.0, 95318.0));
    RandomGenerator::CustomDistribution byte = RandomGenerator::CustomDistribution(b);
    RandomGenerator::UniformDistribution dst = RandomGenerator::UniformDistribution(0, 3);

    b.clear();
    b.insert(std::pair<double, double>(1, 1192));
    b.insert(std::pair<double, double>(2, 692));
    b.insert(std::pair<double, double>(3, 433));
    b.insert(std::pair<double, double>(4, 280));
    b.insert(std::pair<double, double>(5, 186));
    b.insert(std::pair<double, double>(6, 143));
    b.insert(std::pair<double, double>(7, 106));
    b.insert(std::pair<double, double>(8, 74));
    b.insert(std::pair<double, double>(9, 55));
    b.insert(std::pair<double, double>(10, 49));
    b.insert(std::pair<double, double>(11, 25));
    b.insert(std::pair<double, double>(12, 35));
    b.insert(std::pair<double, double>(13, 21));
    b.insert(std::pair<double, double>(14, 14));
    b.insert(std::pair<double, double>(15, 14));
    b.insert(std::pair<double, double>(16, 3));
    b.insert(std::pair<double, double>(17, 5));
    b.insert(std::pair<double, double>(19, 2));
    b.insert(std::pair<double, double>(20, 4));
    b.insert(std::pair<double, double>(21, 4));
    b.insert(std::pair<double, double>(22, 1));
    b.insert(std::pair<double, double>(23, 1));
    b.insert(std::pair<double, double>(24, 2));
    b.insert(std::pair<double, double>(25, 6));
    b.insert(std::pair<double, double>(26, 2));
    b.insert(std::pair<double, double>(27, 2));
    b.insert(std::pair<double, double>(28, 8));
    b.insert(std::pair<double, double>(29, 1));
    b.insert(std::pair<double, double>(30, 1));
    b.insert(std::pair<double, double>(31, 2));
    b.insert(std::pair<double, double>(32, 1));
    b.insert(std::pair<double, double>(33, 1));
    b.insert(std::pair<double, double>(35, 1));
    b.insert(std::pair<double, double>(41, 1));
    b.insert(std::pair<double, double>(143, 1));
    b.insert(std::pair<double, double>(332, 1));
    RandomGenerator::CustomDistribution byte_8 = RandomGenerator::CustomDistribution(b);

    b.clear();
    b.insert(std::pair<double, double>(1, 1090));
    b.insert(std::pair<double, double>(2, 647));
    b.insert(std::pair<double, double>(3, 418));
    b.insert(std::pair<double, double>(4, 279));
    b.insert(std::pair<double, double>(5, 164));
    b.insert(std::pair<double, double>(6, 165));
    b.insert(std::pair<double, double>(7, 131));
    b.insert(std::pair<double, double>(8, 103));
    b.insert(std::pair<double, double>(9, 89));
    b.insert(std::pair<double, double>(10, 78));
    b.insert(std::pair<double, double>(11, 73));
    b.insert(std::pair<double, double>(12, 56));
    b.insert(std::pair<double, double>(13, 62));
    b.insert(std::pair<double, double>(14, 59));
    b.insert(std::pair<double, double>(15, 69));
    b.insert(std::pair<double, double>(16, 47));
    b.insert(std::pair<double, double>(17, 46));
    b.insert(std::pair<double, double>(18, 47));
    b.insert(std::pair<double, double>(19, 37));
    b.insert(std::pair<double, double>(20, 42));
    b.insert(std::pair<double, double>(21, 37));
    b.insert(std::pair<double, double>(22, 42));
    b.insert(std::pair<double, double>(23, 41));
    b.insert(std::pair<double, double>(24, 33));
    b.insert(std::pair<double, double>(25, 34));
    b.insert(std::pair<double, double>(26, 36));
    b.insert(std::pair<double, double>(27, 31));
    b.insert(std::pair<double, double>(28, 32));
    b.insert(std::pair<double, double>(29, 32));
    b.insert(std::pair<double, double>(30, 37));
    b.insert(std::pair<double, double>(31, 35));
    b.insert(std::pair<double, double>(32, 23));
    b.insert(std::pair<double, double>(33, 24));
    b.insert(std::pair<double, double>(34, 27));
    b.insert(std::pair<double, double>(35, 28));
    b.insert(std::pair<double, double>(36, 28));
    b.insert(std::pair<double, double>(37, 26));
    b.insert(std::pair<double, double>(38, 18));
    b.insert(std::pair<double, double>(39, 24));
    b.insert(std::pair<double, double>(40, 21));
    b.insert(std::pair<double, double>(41, 22));
    b.insert(std::pair<double, double>(42, 17));
    b.insert(std::pair<double, double>(43, 20));
    b.insert(std::pair<double, double>(44, 24));
    b.insert(std::pair<double, double>(45, 22));
    b.insert(std::pair<double, double>(46, 7));
    b.insert(std::pair<double, double>(47, 11));
    b.insert(std::pair<double, double>(48, 12));
    b.insert(std::pair<double, double>(49, 18));
    b.insert(std::pair<double, double>(50, 8));
    b.insert(std::pair<double, double>(51, 12));
    b.insert(std::pair<double, double>(52, 18));
    b.insert(std::pair<double, double>(53, 17));
    b.insert(std::pair<double, double>(54, 11));
    b.insert(std::pair<double, double>(55, 16));
    b.insert(std::pair<double, double>(56, 10));
    b.insert(std::pair<double, double>(57, 8));
    b.insert(std::pair<double, double>(58, 6));
    b.insert(std::pair<double, double>(59, 17));
    b.insert(std::pair<double, double>(60, 4));
    b.insert(std::pair<double, double>(61, 14));
    b.insert(std::pair<double, double>(62, 9));
    b.insert(std::pair<double, double>(63, 9));
    b.insert(std::pair<double, double>(64, 8));
    b.insert(std::pair<double, double>(65, 10));
    b.insert(std::pair<double, double>(66, 9));
    b.insert(std::pair<double, double>(67, 8));
    b.insert(std::pair<double, double>(68, 7));
    b.insert(std::pair<double, double>(69, 7));
    b.insert(std::pair<double, double>(70, 6));
    b.insert(std::pair<double, double>(71, 7));
    b.insert(std::pair<double, double>(72, 2));
    b.insert(std::pair<double, double>(73, 7));
    b.insert(std::pair<double, double>(74, 12));
    b.insert(std::pair<double, double>(75, 4));
    b.insert(std::pair<double, double>(76, 4));
    b.insert(std::pair<double, double>(77, 8));
    b.insert(std::pair<double, double>(78, 3));
    b.insert(std::pair<double, double>(79, 11));
    b.insert(std::pair<double, double>(80, 5));
    b.insert(std::pair<double, double>(81, 5));
    b.insert(std::pair<double, double>(82, 3));
    b.insert(std::pair<double, double>(83, 4));
    b.insert(std::pair<double, double>(84, 7));
    b.insert(std::pair<double, double>(85, 3));
    b.insert(std::pair<double, double>(86, 5));
    b.insert(std::pair<double, double>(87, 3));
    b.insert(std::pair<double, double>(88, 4));
    b.insert(std::pair<double, double>(89, 3));
    b.insert(std::pair<double, double>(90, 1));
    b.insert(std::pair<double, double>(91, 3));
    b.insert(std::pair<double, double>(92, 3));
    b.insert(std::pair<double, double>(93, 3));
    b.insert(std::pair<double, double>(94, 2));
    b.insert(std::pair<double, double>(96, 2));
    b.insert(std::pair<double, double>(97, 4));
    b.insert(std::pair<double, double>(98, 5));
    b.insert(std::pair<double, double>(99, 2));
    b.insert(std::pair<double, double>(100, 4));
    b.insert(std::pair<double, double>(101, 3));
    b.insert(std::pair<double, double>(102, 1));
    b.insert(std::pair<double, double>(103, 1));
    b.insert(std::pair<double, double>(104, 1));
    b.insert(std::pair<double, double>(105, 2));
    b.insert(std::pair<double, double>(106, 1));
    b.insert(std::pair<double, double>(107, 1));
    b.insert(std::pair<double, double>(108, 1));
    b.insert(std::pair<double, double>(110, 2));
    b.insert(std::pair<double, double>(113, 1));
    b.insert(std::pair<double, double>(115, 4));
    b.insert(std::pair<double, double>(117, 1));
    b.insert(std::pair<double, double>(119, 3));
    b.insert(std::pair<double, double>(120, 3));
    b.insert(std::pair<double, double>(121, 1));
    b.insert(std::pair<double, double>(122, 2));
    b.insert(std::pair<double, double>(123, 2));
    b.insert(std::pair<double, double>(125, 2));
    b.insert(std::pair<double, double>(127, 1));
    b.insert(std::pair<double, double>(129, 1));
    b.insert(std::pair<double, double>(131, 5));
    b.insert(std::pair<double, double>(132, 1));
    b.insert(std::pair<double, double>(133, 2));
    b.insert(std::pair<double, double>(134, 1));
    b.insert(std::pair<double, double>(135, 2));
    b.insert(std::pair<double, double>(136, 3));
    b.insert(std::pair<double, double>(137, 2));
    b.insert(std::pair<double, double>(138, 2));
    b.insert(std::pair<double, double>(139, 2));
    b.insert(std::pair<double, double>(141, 4));
    b.insert(std::pair<double, double>(145, 1));
    b.insert(std::pair<double, double>(147, 1));
    b.insert(std::pair<double, double>(148, 2));
    b.insert(std::pair<double, double>(152, 2));
    b.insert(std::pair<double, double>(153, 2));
    b.insert(std::pair<double, double>(155, 2));
    b.insert(std::pair<double, double>(158, 1));
    b.insert(std::pair<double, double>(160, 2));
    b.insert(std::pair<double, double>(162, 1));
    b.insert(std::pair<double, double>(166, 2));
    b.insert(std::pair<double, double>(167, 1));
    b.insert(std::pair<double, double>(168, 1));
    b.insert(std::pair<double, double>(169, 2));
    b.insert(std::pair<double, double>(170, 1));
    b.insert(std::pair<double, double>(171, 1));
    b.insert(std::pair<double, double>(173, 2));
    b.insert(std::pair<double, double>(174, 2));
    b.insert(std::pair<double, double>(175, 1));
    b.insert(std::pair<double, double>(180, 2));
    b.insert(std::pair<double, double>(182, 1));
    b.insert(std::pair<double, double>(183, 1));
    b.insert(std::pair<double, double>(186, 1));
    b.insert(std::pair<double, double>(190, 1));
    b.insert(std::pair<double, double>(192, 3));
    b.insert(std::pair<double, double>(194, 1));
    b.insert(std::pair<double, double>(196, 1));
    b.insert(std::pair<double, double>(203, 1));
    b.insert(std::pair<double, double>(204, 1));
    b.insert(std::pair<double, double>(205, 1));
    b.insert(std::pair<double, double>(209, 1));
    b.insert(std::pair<double, double>(210, 1));
    b.insert(std::pair<double, double>(224, 1));
    b.insert(std::pair<double, double>(234, 1));
    b.insert(std::pair<double, double>(246, 1));
    b.insert(std::pair<double, double>(247, 1));
    b.insert(std::pair<double, double>(253, 2));
    b.insert(std::pair<double, double>(257, 1));
    b.insert(std::pair<double, double>(273, 1));
    b.insert(std::pair<double, double>(280, 1));
    b.insert(std::pair<double, double>(285, 1));
    b.insert(std::pair<double, double>(315, 1));
    b.insert(std::pair<double, double>(372, 1));
    b.insert(std::pair<double, double>(384, 1));
    b.insert(std::pair<double, double>(389, 1));
    b.insert(std::pair<double, double>(401, 1));
    b.insert(std::pair<double, double>(488, 1));
    b.insert(std::pair<double, double>(497, 1));
    b.insert(std::pair<double, double>(1239, 1));
    RandomGenerator::CustomDistribution byte_136 = RandomGenerator::CustomDistribution(b);


    while(cycle < numCycles){
        for (int src = 0; src < numCores; src++){
            int source = src;
            int destination = dst.Generate();
            if (destination != src) {
                int byteInject = byte.Generate();
                if(byteInject == 8){
                    int i = 0;
                    int threshold = byte_8.Generate();
                    while(i < threshold){
                        myfile << cycle + i << "," << byteInject << std::endl;
                        i += 1;
                    }
                    cycle += threshold;
                }
                else if(byteInject == 136){
                    int i = 0;
                    int threshold = byte_136.Generate();
                    while(i < threshold){
                        myfile << cycle + i << "," << byteInject << std::endl;
                        i += 1;
                    }
                    cycle += threshold;
                }
            }
        }

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

