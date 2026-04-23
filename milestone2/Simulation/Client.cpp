#include "Client.hpp"
#include <optional>
 
Client::Client(ThreadQueue<Message>& clientToInter,ThreadQueue<Message>& interToClient)
    : clientToInter(clientToInter),interToClient(interToClient) {}

void Client::run(Control& interControl, std::mutex& coutMtx) {
    // esperar a que el interm esté listo para atender solicitudes
    this->waitOnIntermediary(interControl);
 
    ClientConsoleHandler console(coutMtx);
    logger.log(Entidad::CLIENTE, TipoLog::SOLICITUD,
        "Cliente en linea, iniciando sesion interactiva.");
 
    while (true) {
        // muestra menu y lee solicitud
        Message request = console.getUserRequest();
 
        const bool esStop = (request.reqType == RequestType::STOP);
 
        logger.log(Entidad::CLIENTE, TipoLog::SOLICITUD,
            "Cliente envía solicitud tipo=" + requestTypeName(request.reqType)
            + " body=\"" + request.body + "\"");
 
        // enviar la solicitud al interm
        this->clientToInter.push(request);
 
        // si es STOP no esperar respuesta, el sistema se cierra
        if (esStop) {
            logger.log(Entidad::CLIENTE, TipoLog::SOLICITUD,
                "Cliente envió STOP, cerrando hilo.");
            break;
        }
 
        //espera respuesta intermediario y la muestra
        Message response = this->interToClient.wait_and_pop();
 
        logger.log(Entidad::CLIENTE, TipoLog::RESPUESTA,
            "Cliente recibió respuesta tipo="
            + responseTypeName(response.respType)
            + " id=" + std::to_string(response.id));
 
        console.showResponse(response);
    }
}


void Client::waitOnIntermediary(Control& interControl) const {
    // bloquea hasta que Intermediary::signalToClient ponga ready=true
    std::unique_lock<std::mutex> lock(interControl.mtx);
    interControl.cv.wait(lock, [&interControl] {
        return interControl.ready;
    });
}
