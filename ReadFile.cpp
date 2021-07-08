//
// Created by Ben on 7/8/21.
//
#include "globals.h"
#include "TrafficGenerator.h"
#include "ReadFile.h"
#include <string>
#include <iostream>
#include <vector>
std::vector<int>byteValue;


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
        std::cout << destin << std::endl;
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
    if(temp.compare("BYTE_DISTRIBUTION-BEGIN") == 0){
        //while(temp.compare("BYTE_DISTRIBUTION-END") != 0){
        std::string b, freq;
        modelFile >> b;
        std::cout << b << std::endl;
        modelFile >> freq;
        std::cout << freq << std::endl;


    }
}

void durationModel(std::ifstream& modelFile){

}
void readModel(std::ifstream& modelFile){
    std::cout << "Reading model\n";
    int pos = 0;
    destinationModel(modelFile);
    byteModel(modelFile);
    /*int i = byteValue.pop_back();
    while(i > -1){
        durationModel(modelFile);
        i = byteValue.pop_back();
    }*/
}