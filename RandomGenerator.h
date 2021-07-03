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
        ExponentialDistribution(double lambda, int intervals){
            _lambda = lambda;
            _intervals = intervals;
        }

        double Generate(){
            std::exponential_distribution<double> dist(_lambda);
            return dist(mt_rng);
        }

    private:
        double _lambda;
        int _intervals;
        std::exponential_distribution<double>* dist;
    };

    class PoissonDistribution{
    public:
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
};


#endif
