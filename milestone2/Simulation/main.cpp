#include <iostream>

#define ESC       "\x1B"
#define RESET     ESC "[0m"
#define BOLD      ESC "[1m"
#define RED       ESC "[38;5;196m"
#define DARK_RED  ESC "[38;5;160m"
#define YELLOW    ESC "[38;5;230m"
#define ORANGE    ESC "[38;5;208m"

#define CYAN      ESC "[38;5;51m"

#define BG_WHITE  ESC "[48;5;231m"
#define BG_DARK   ESC "[48;5;235m"

void printHeader() {
    std::string text = "Menu servidor de Piezas";

    int width = 80; // ancho consola
    int boxWidth = text.size() + 4; // espacio interno del cuadro
    int padding = (width - boxWidth) / 2;

    // línea superior
    for (int i = 0; i < padding; i++) std::cout << " ";
    std::cout << "┌";
    for (int i = 0; i < boxWidth - 2; i++) std::cout << "─";
    std::cout << "┐\n";

    // línea del texto
    for (int i = 0; i < padding; i++) std::cout << " ";
    std::cout << "│ " << text << " │\n";

    // línea inferior
    for (int i = 0; i < padding; i++) std::cout << " ";
    std::cout << "└";
    for (int i = 0; i < boxWidth - 2; i++) std::cout << "─";
    std::cout << "┘\n";
    std::cout << "\n";
    std::cout << "  " ORANGE "\t\t\"1\"" RESET  "  ->  " RESET
              << CYAN "LIST_FIGURES" RESET
              << "   Consultar figuras disponibles\n";
    std::cout << "  " ORANGE "\t\t\"2\"" RESET  "  ->  " RESET
              << CYAN "GET_FIGURE " RESET
              << "    Consultar piezas de una figura\n";
    std::cout << "  " ORANGE "\t\t\"0\"" RESET  "  ->  " RESET
              << CYAN "STOP       " RESET
              << "    Terminar la simulación\n";
    std::cout << "\n"  "  Cliente: " RESET;


}

int main() {std::cout << "\n";

    // si quieres color SOLO en el cuadro
    
    printHeader();


    return 0;
}