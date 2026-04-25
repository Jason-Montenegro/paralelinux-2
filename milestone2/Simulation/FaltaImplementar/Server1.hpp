#ifndef SERVER1_H
#define SERVER1_H
#include "Connection.hpp"
#include "../Control.hpp"
#include "../LegoRepository.hpp"
class Server1 {
public:
    Server1(Connection conn);  // antes: ThreadQueue& i2S, ThreadQueue& s2I
    void run(Control& serverControl);
private:
    Connection conn;
    LegoRepository repo;
};
#endif