#include "Client1.hpp"
#include "ClientConsoleHandler.hpp" 

Client1::Client1(Connection conn1) : conn(conn1) {}

void Client1::run(Control& interControl, std::mutex& coutMtx) {
    this->waitOnIntermediary(interControl);

    ClientConsoleHandler console(coutMtx);
    logger.log(Entidad::CLIENTE, TipoLog::SOLICITUD,
        "Cliente en linea, iniciando sesion interactiva.");

    while (true) {
        // getUserRequest devuelve Packet en vez de Message
        Packet request = console.getUserRequest();

        const bool esStop = (request.type == PacketType::STOP);

        logger.log(Entidad::CLIENTE, TipoLog::SOLICITUD,
            "Cliente envia solicitud tipo=" + packetTypeName(request.type)
            + " figura=\"" + request.figureName + "\"");

        // enviar al intermediario por outbound
        this->conn.outbound->push(request);

        if (esStop) {
            logger.log(Entidad::CLIENTE, TipoLog::SOLICITUD,
                "Cliente envio STOP, cerrando hilo.");
            break;
        }

        // esperar respuesta del intermediario por inbound
        Packet response = this->conn.inbound->wait_and_pop();

        logger.log(Entidad::CLIENTE, TipoLog::RESPUESTA,
            "Cliente recibio respuesta tipo=" + packetTypeName(response.type)
            + " id=" + std::to_string(response.id));

        console.showResponse(response);
    }
}

void Client1::waitOnIntermediary(Control& interControl) const {
    std::unique_lock<std::mutex> lock(interControl.mtx);
    interControl.cv.wait(lock, [&interControl] {
        return interControl.ready;
    });
}