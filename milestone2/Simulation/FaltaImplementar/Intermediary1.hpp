#pragma once
#include <string>
#include <vector>
#include "Connection.hpp"
#include "../RouteTable.hpp"
#include "../Logger.hpp"
#include "../Control.hpp"

class Intermediary1 {
public:
    // Una Connection hacia el cliente, otra hacia el servidor
    Intermediary1(Connection clientConn, Connection serverConn);

    void run(Control& interControl, Control& serverControl);

    // Para red de intermediarios: agrega un canal peer
    void addPeer(Connection peerConn);

private:
    Connection clientConn;
    Connection serverConn;
    std::vector<Connection> peers;    // canales con otros intermediarios
    RouteTable routeTable;

    void waitOnServer(Control& serverControl);
    void signalToClient(Control& interControl);
    void initRouteTable();

    void handleListFigures(const Packet& req);
    void handleGetFigure(const Packet& req);

    Packet askServer(PacketType type, std::string figureName,int segment = 1, int origId = 0);

    std::string multicastToIntermediaries(const std::string& figureName,int segment);
};