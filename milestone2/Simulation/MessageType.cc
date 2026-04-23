#include "MessageType.hpp"


std::string requestTypeName(RequestType t) {
    switch (t) {
        case RequestType::LIST_FIGURES: return "LIST_FIGURES";
        case RequestType::GET_FIGURE:   return "GET_FIGURE";
        case RequestType::ASK_FIGURE:   return "ASK_FIGURE";
        case RequestType::STOP:         return "STOP";
    }
    return "UNKNOWN";
}

std::string responseTypeName(ResponseType t) {
    switch (t) {
        case ResponseType::FIGURE_FOUND:     return "FIGURE_FOUND";
        case ResponseType::FIGURE_NOT_FOUND: return "FIGURE_NOT_FOUND";
        case ResponseType::FIGURES_LIST:     return "FIGURES_LIST";
        case ResponseType::RETURN_FIGURE:    return "RETURN_FIGURE";
        case ResponseType::ERROR:            return "ERROR";
    }
    return "UNKNOWN";
}
