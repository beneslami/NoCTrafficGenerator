//
// Created by Ben on 7/8/21.
//
#include "globals.h"
#include <map>
#include <vector>

std::map<double, double>bytes;
std::map<int, std::map<double, double> > duration;
std::map<int, int>traffic;
std::string destin;
std::vector<int>byteValue;

std::vector<int>window_num;
std::map<double, double>normal_stat;

unsigned long long numCycles;
int numCores;