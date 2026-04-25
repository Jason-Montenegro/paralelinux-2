#pragma once
#include "Connection.hpp"
#include "../Logger.hpp"
#include "../Control.hpp"

class ClientConsoleHandler; // forward declaration — implementación propia

class Client1 {
public:
    explicit Client1(Connection conn);
    void run(Control& interControl, std::mutex& coutMtx);

private:
    Connection conn;
    void waitOnIntermediary(Control& interControl) const;
};