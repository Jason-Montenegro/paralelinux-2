#include "Server1.hpp"

Server1::Server1(Connection conn1) : conn(conn1) {}

void Server1::run(Control& serverControl) {
    // señalizar que está listo
    {
        std::lock_guard<std::mutex> lock(serverControl.mtx);
        serverControl.ready = true;
        serverControl.cv.notify_one();
    }

    while (true) {
        Packet req = conn.inbound->wait_and_pop();

        if (req.type == PacketType::STOP) break;

        if (req.figureName == "ALL") {
            std::string catalog = repo.getCatalog();
            conn.outbound->push(
                Packet::response(req.id, PacketType::FIGURES_LIST, catalog));
        } else {
            if (repo.hasFigure(req.figureName)) {
                std::string data = repo.getFigure(req.figureName, req.segment);
                conn.outbound->push(
                    Packet::response(req.id, PacketType::FIGURE_FOUND, data));
            } else {
                conn.outbound->push(
                    Packet::response(req.id, PacketType::FIGURE_NOT_FOUND));
            }
        }
    }
}