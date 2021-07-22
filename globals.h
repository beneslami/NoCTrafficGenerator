//
// Created by Ben on 7/8/21.
//

#ifndef GLOBALS_H
#define GLOBALS_H

#include <map>
#include <iostream>

extern std::map<double, double>bytes;
extern std::map<int, std::map<double, double> > duration;
extern std::map<int, double>traffic;
extern std::string destin;
extern std::vector<float>byteValue;

extern std::map<int, std::map<float, float> >normal_stat;

extern unsigned long long numCycles;
extern int numCores;

#endif
