#ifndef SIMULATION_H
#define SIMULATION_H

#include <string>

class Simulation{
private:
    std::string switchIp;
    std::string clientIp;
    std::string intermediaryIp;
public:
    Simulation(std::string switchIp, std::string clientIp,std::string intermediaryIp);
    void run();
    ~Simulation() = default;
};



#endif