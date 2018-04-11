#include <iostream>
#include "libTrodesNetwork/AbstractModuleClient.h"
#include "libTrodesNetwork/highfreqclasses.h"
#include "libTrodesNetwork/CZHelp.h"

volatile bool quit = false;

class MyClient : public AbstractModuleClient{
public:
    MyClient(const char *i, const char *a, int p) : AbstractModuleClient(i,a,p){}
    void recv_quit(){
        std::cout << "got quit\n";
        quit = true;
    };
};

int main()
{
    std::cout << "Example executable" << std::endl;
    TrodesMsg msg;
    MyClient *client = new MyClient("id", "tcp://127.0.0.1", 49152);
    client->initialize();
    while(!quit){

    }
    std::cout << "Got quit" << std::endl;
    delete client;
    return EXIT_SUCCESS;
}
