//
// Created by Ben on 7/8/21.
//
#include "globals.h"
#include "TrafficGenerator.h"
#include "ReadFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>

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
            float a = std::stod(temp);
            byteValue.push_back(a);
            modelFile >> temp;
            double b = std::stod(temp);
            bytes.insert(std::pair<double, double>(a, b));
            modelFile >> temp;
        }
    }
}

void durationModel(std::ifstream& modelFile, std::string header, double bt){
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
    duration.insert(std::pair<double, std::map<double, double> >(bt, temporaryList));
    temporaryList.clear();
    head.clear();
    tail.clear();
}

void readModel(std::ifstream& modelFile){
    destinationModel(modelFile);
    byteModel(modelFile);
    std::vector<float>::iterator i;
    std::cout << std::fixed;
    for(i = byteValue.begin(); i != byteValue.end(); ++i){
        std::string header;
        header.append("BYTE_");
        char a[10];
        sprintf(a, "%.3f", *i);
        header.append(a);
        header.append("_INTERARRIVAL_DISTRIBUTION-");
        durationModel(modelFile, header, *i);
    }
}

void readModel2(std::ifstream& modelFile){
    std::vector<std::string> lines;
    std::string temp;
    double mean, std;
    int window_num;
    while(std::getline(modelFile, temp)){
        lines.push_back(temp);
    }
    std::vector<std::string>::iterator it;
    for(it = lines.begin(); it != lines.end(); ++it){
        std::stringstream str_stm;
        str_stm << *it;
        float temp_int;
        std::string temp_str;
        int counter  = 0;
        while(!str_stm.eof()){
            str_stm >> temp_str;
            if(std::stringstream(temp_str) >> temp_int){
                if(counter == 0){
                    window_num = temp_int;
                    counter += 1;
                }
                else if(counter == 1){
                    mean = temp_int;
                    counter += 1;
                }
                else if(counter == 2){
                    std = temp_int;
                    std::map<float, float>temp;
                    temp.insert(std::pair<float, float>(mean, std));
                    normal_stat.insert(std::pair<int, std::map<float, float> >(window_num, temp));
                    counter = 0;
                }
            }
            temp_str = "";
        }
    }
}