#pragma once
#include "Packet.hpp"   
#include "../LegoRepository.hpp"
#include "../Logger.hpp"

class LegoManager1{
public:
    Packet handle(const Packet& request);
private:
    LegoRepository repo;
};