#include "Simulation.hpp"

int main() {
    Simulation app(
        "10.0.0.0",   // IP del switch
        "10.0.0.1",   // IP del cliente
        "10.0.0.2"    // IP del intermediario
    );
    app.run();
    return 0;
}