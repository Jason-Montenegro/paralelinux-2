#include "Server.hpp"

Server::Server(ThreadQueue<Message>& interToServer,
               ThreadQueue<Message>& serverToInter)
    : interToServer(interToServer),
      serverToInter(serverToInter) {}
 
void Server::run(Control& serverControl) {
    // notifica que servidor puede recibir peticiones
    this->signalToIntermediary(serverControl);
    logger.log(Entidad::SERVIDOR, TipoLog::SOLICITUD,
        "Server: en linea, esperando peticiones del Intermediario");
 
    while (true) {
        Message request = this->interToServer.wait_and_pop();
 
        logger.log(Entidad::SERVIDOR, TipoLog::SOLICITUD,
            "Server: solicitud recibida #" + std::to_string(request.id)
            + " tipo=" + requestTypeName(request.reqType)
            + " body=\"" + request.body + "\"");
 
        if (request.reqType == RequestType::STOP) {
            logger.log(Entidad::SERVIDOR, TipoLog::SOLICITUD,
                "Server: STOP recibido, cerrando");
            break;
        }
 
        // El LegoManager aplica la lógica del protocolo sobre el repositorio
        Message response = this->manager.handle(request);
 
        logger.log(Entidad::SERVIDOR, TipoLog::RESPUESTA,
            "Server: respuesta #" + std::to_string(response.id)
            + " tipo=" + responseTypeName(response.respType));
 
        this->serverToInter.push(std::move(response));
    }
}
 
void Server::signalToIntermediary(Control& serverControl) const {
    // desbloquea al Intermediario que espera en waitOnServer
    std::lock_guard<std::mutex> lock(serverControl.mtx);
    serverControl.ready = true;
    serverControl.cv.notify_one();
}
