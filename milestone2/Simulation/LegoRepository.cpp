#include "LegoRepository.hpp"
#include <sstream>

LegoRepository::LegoRepository() {

    figures["Horse"] = {
        {
            {5, "brick 2x2 yellow"},
            {4, "brick 2x4 white"},
            {2, "brick 2x4 yellow"},
            {1, "brick 2x6 yellow"}
        },
        {
            {1, "brick 2x3 yellow "},
            {2, "brick 1x1 yellow eye"},
            {2, " brick 2x1 yellow"},
            {1, " brick 2x2 white"}
        }
    };

    figures["lion"] = {
        {
            {1, "brick 2x8 yellow"},
            {6, "brick 2x2 yellow"},
            {3, "brick 2x4 yellow"},
            {5, "brick 1x4 yellow"},
            {6, "brick 1x2 yellow"},
            {1, "brick 2x4 black"}
        },
        {
            {1, "plate 2x3 yellow"},
            {2, "brick 1x2 black"},
            {2, "brick 2x2 black"},
            {1, "brick 2x2 red"},
            {1, "brick 1x1 eyes dark grey"}
        }
    };

    figures["fish"] = {
        {
            {3, "brick 2x4 yellow"},
            {1, "brick 2x3 yellow"},
            {2, "brick 1x1 yellow eye"}
        },
        {
            {2, "brick 2x2 translucent blue"},
            {4, "brick 2x2 translucent red"}
        }
    };

    figures["giraffe"] = {
        {
            {4, "brick 2x2 black"},
            {4, "brick 2x2 yellow"},
            {6, "brick 2x1 yellow"},
            {6, "brick 2x1 black"},
            {1, "brick 2x6 yellow"}
        },
        {
            {2, "brick 2x4 yellow"},
            {1, "brick 2x3 yellow"},
            {1, "brick 1x4 yellow"},
            {1, "brick 2x3 red"},
            {2, "brick 1x1 eye yellow"}
        }
    };
}
std::string LegoRepository::getCatalog() {
       std::ostringstream oss;
    for (const auto& [name, parts] : figures) {
        oss << name << " (" << parts.size() << " segmentos)\n";
    }
    return oss.str();
}

std::string LegoRepository::getFigure(std::string name, size_t segment) {

    auto it = figures.find(name);
    if (it == figures.end()) {
        return "FIGURE_NOT_FOUND: figura desconocida \"" + name + "\"";
    }
    const Partes& parts = it->second;
 
    // segmento inválido -> usar segmento 1 por defecto
    if (segment == 0 || segment > parts.size()) {
        segment = 1;
    }
 
    const Segmento& seg = parts[segment - 1];
    std::ostringstream oss;
    oss << "Figura: " << name << " | Segmento: " << segment << "\n";
    for (size_t i = 0; i < seg.size(); ++i) {
        oss << "  " << (i + 1) << ". "
            << seg[i].first << "x  " << seg[i].second << "\n";
    }
    return oss.str();

}

bool LegoRepository::hasFigure(std::string name) {
    return figures.count(name) > 0;
}

std::unordered_set<std::string> LegoRepository::availableFigures() {
    std::unordered_set<std::string> names;
    for (const auto& [name, _] : figures) {
        names.insert(name);
    }
    return names;
}
