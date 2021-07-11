//
// Created by Ben on 7/8/21.
//
#include "globals.h"
#include "TrafficGenerator.h"
#include "ReadFile.h"
#include <string>
#include <iostream>
#include <vector>


void destinationModel(std::ifstream& modelFile){
    std::string temp;
    modelFile >> temp;
    if(!temp.compare("DESTINATION_DISTRIBUTION-BEGIN")){
        modelFile >> temp;
        if(!temp.compare("uniform")){
            destin = "uniform";
        }
        else if (!temp.compare("normal")){
            destin = "normal";
        }
        while(!temp.compare("DESTINATION_DISTRIBUTION-END")){
            modelFile >> temp;
            /*  in case the destination distribution needs more data, add the processing code for destination
             * distribution here.*/
        }
    }

}

void byteModel(std::ifstream& modelFile){
    std::string temp;
    modelFile >> temp;
    while(temp.compare("BYTE_DISTRIBUTION-BEGIN") != 0){
        modelFile >> temp;
    }
    if(temp.compare("BYTE_DISTRIBUTION-BEGIN") == 0) {
        modelFile >> temp;
        while(temp.compare("BYTE_DISTRIBUTION-END") != 0){
            double a = std::stod(temp);
            byteValue.push_back(a);
            modelFile >> temp;
            double b = std::stod(temp);
            bytes.insert(std::pair<double, double>(a, b));
            modelFile >> temp;
        }
    }
}

void durationModel(std::ifstream& modelFile, std::string header, int bt){

    std::string head = header;
    head = head.append("BEGIN");
    std::string tail = header.append("END");
    std::map<double, double> temporaryList;
    std::string temp;
    modelFile >> temp;
    while(temp.compare(head) != 0){
        modelFile >> temp;
    }
    if(temp.compare(head) == 0) {
        modelFile >> temp;
        while(temp.compare(tail) != 0){
            double a = std::stod(temp);
            modelFile >> temp;
            double b = std::stod(temp);
            temporaryList.insert(std::pair<double, double>(a, b));
            modelFile >> temp;
        }
    }
    duration.insert(std::pair<int, std::map<double, double> >(bt, temporaryList));
    temporaryList.clear();
    head.clear();
    tail.clear();
}
void readModel(std::ifstream& modelFile){
    destinationModel(modelFile);
    byteModel(modelFile);
    for(int i=0; i< byteValue.size(); i++){
        std::string header;
        header.append("BYTE_");
        header.append(std::to_string(byteValue.at(i)));
        header.append("_INTERARRIVAL_DISTRIBUTION-");
        durationModel(modelFile, header, byteValue.at(i));
    }
}