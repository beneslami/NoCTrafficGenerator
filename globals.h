//
// Created by Ben on 7/8/21.
//

#ifndef GLOBALS_H
#define GLOBALS_H

#include <map>
#include <iostream>

extern std::map<double, double>bytes;
extern std::map<int, std::map<double, double> > duration;
extern std::map<int, int>traffic;
extern std::string destin;
extern std::vector<int>byteValue;

extern std::vector<int>window_num;
extern std::map<double, double>normal_stat;

extern unsigned long long numCycles;
extern int numCores;

#endif
