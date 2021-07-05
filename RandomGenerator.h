//
// Created by Ben on 6/30/21.
//

#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#include <iostream>
#include <random>
#include <map>

namespace RandomGenerator {
    static std::random_device rd;
    static std::mt19937 mt_rng(rd());

    class BernoulliDistribution{
    public:
        BernoulliDistribution(){

        }

        BernoulliDistribution(double p) {
            dist = new std::bernoulli_distribution(p);
        }

        ~BernoulliDistribution() {
        }

        void SetProbability(double p) {
            dist = new std::bernoulli_distribution(p);
        }

        bool Generate() {
            return (*dist)(mt_rng);
        }

    private:
        std::bernoulli_distribution* dist;
    };

    class UniformDistribution{
    public:
        UniformDistribution(){

        }

        UniformDistribution(int min, int max) {
            dist = new std::uniform_int_distribution<int>(min, max);
        }

        ~UniformDistribution() {
        }

        void SetMinMax(int min, int max) {
            dist = new std::uniform_int_distribution<int>(min, max);
        }

        int Generate() {
            return (*dist)(mt_rng);
        }
    private:
        std::uniform_int_distribution<int>* dist;
    };

    class NormalDistribution{
    public:
        NormalDistribution(float mean, float std){
            dist = new std::normal_distribution<float>(mean, std);
        }

        ~NormalDistribution(){
        }

        float generate(){
            return (*dist)(mt_rng);
        }

    private:
        std::normal_distribution<float>* dist;

    };

    class ExponentialDistribution{
    public:
        ExponentialDistribution(){

        }

        ExponentialDistribution(double lambda){
            _lambda = lambda;
        }

        double Generate(){
            std::exponential_distribution<double> dist(_lambda);
            return dist(mt_rng);
        }

    private:
        double _lambda;
        std::exponential_distribution<double>* dist;
    };

    class PoissonDistribution{
    public:
        PoissonDistribution(){

        }

        PoissonDistribution(double lambda){
            _lambda = lambda;
        }
        int Generate(){
            std::poisson_distribution<int> dist(_lambda);
            return dist(mt_rng);
        }
    private:
        int _lambda;
        std::poisson_distribution<int>* dist;
    };

    class CustomDistribution{
    public:
        CustomDistribution(std::map<double, double> bytes){
            std::map<double, double>::iterator it;
            for(it = bytes.begin(); it != bytes.end(); ++it){
                distribution[it->first] = it->second;
            }
            int range = 0;
            for(it = distribution.begin(); it != distribution.end(); ++it){
                range += it->second;
            }

            for(it = distribution.begin(); it != distribution.end(); ++it){
                pdf[it->first] = it->second/range;
            }
            double prev = 0;
            for(it = pdf.begin(); it != pdf.end(); ++it){
                cdf[it->first] = it->second + prev;
                prev = cdf[it->first];
            }
        }

        ~CustomDistribution(){

        }

        double Generate(){
            std::uniform_real_distribution<double> dist = std::uniform_real_distribution<double>(0, 1);
            double prob = dist(mt_rng);
            for(auto it = cdf.begin(); it != cdf.end(); ++it){
                if(prob <= it->second){
                    return it->first;
                }
            }
        }

    private:
        std::map<double, double> distribution;
        std::discrete_distribution<double>* dist;
        std::map<double, double> cdf;
        std::map<double, double> pdf;
    };
};


#endif
