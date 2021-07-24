//
// Created by Ben on 7/19/21.
//

#include "TrafficGen.h"

TrafficGen::TrafficGen(const Configuration &config, const vector<Network *> &net) : TrafficManager(config, net) {

}

TrafficGen::~TrafficGen(){

}

void TrafficGen::_RetireFlit(Flit *f, int dest){

}

void TrafficGen::_GeneratePacket(int source, int size, int cl, int time, int id, int subnetwork, int destination) {

}

void TrafficGen::_Inject() {

}

void TrafficGen::_Step() {

}

bool TrafficGen::_SingleSim() {

}