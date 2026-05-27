#ifndef CONTROL_H
#define CONTROL_H

#include <condition_variable>
#include <mutex>
#include <tuple>
/**
 * Agrupa elementos necesarios para sincronizar
 * 
*/

struct Control {
    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
};

#endif