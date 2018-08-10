#include "libTrodesNetwork/AbstractModuleClient.h"
#include "libTrodesNetwork/trodesglobaltypes.h"
extern "C"{
    #include "trodesnetwork_c.h"
}

class CModuleClient : public AbstractModuleClient{
public:
    CModuleClient(const char* id, const char* addr, int port) 
        : AbstractModuleClient(id, addr, port), 
        quitfn(NULL), quitargs(NULL){

        }
    recv_quit_fn quitfn;
    void* quitargs;
    recv_file_open_fn openfn;
    void* openargs;
    recv_file_close_fn closefn;
    void* closeargs;
    recv_acquisition_fn acqfn;
    void* acqargs;
    recv_source_fn srcfn;
    void* srcargs;

protected:
    void recv_file_open(std::string f){
        if(openfn){
            openfn(openargs, f.c_str());
        }
    }
    void recv_file_close(){
        if(closefn){
            closefn(closeargs);
        }
    }
    void recv_acquisition(std::string acq, uint32_t t){
        if(acqfn){
            acqfn(acqargs, acq.c_str(), t);
        }
    }
    void recv_source(std::string src){
        if(srcfn){
            srcfn(srcargs, src.c_str());
        }
    }
    void recv_quit(){
        if(quitfn){
            quitfn(quitargs);
        }
    }
};

struct _abstractModuleClient_t{
    CModuleClient *client;
};

AbstractModuleClient_t* amc_new(const char* id, const char* addr, int port){
    // return reinterpret_cast<AbstractModuleClient_t*>(new AbstractModuleClient(id, addr, port));
    AbstractModuleClient_t *client = new AbstractModuleClient_t;
    client->client = new CModuleClient(id, addr, port);
    return client;
}

void amc_destroy(AbstractModuleClient_t* client){
    if(!client)
        return;
    // delete reinterpret_cast<AbstractModuleClient*>(client);
    delete client->client;
    delete client;
}

int amc_initialize(AbstractModuleClient_t* client){
    // return reinterpret_cast<AbstractModuleClient*>(client)->initialize();
    return client->client->initialize();
}
void amc_closeConnections(AbstractModuleClient_t* client){
    // reinterpret_cast<AbstractModuleClient*>(client)->closeConnections();
    client->client->closeConnections();
}

void amc_subscribeToEvent(AbstractModuleClient_t* client, const char* origin, const char* event){
    // reinterpret_cast<AbstractModuleClient*>(client)->subscribeToEvent(origin, event);
    client->client->subscribeToEvent(origin, event);
}

void amc_unsubscribeFromEvent(AbstractModuleClient_t* client, const char* origin, const char *event){
    // reinterpret_cast<AbstractModuleClient*>(client)->unsubscribeFromEvent(origin, event);
    client->client->unsubscribeFromEvent(origin, event);
}
void amc_registerRecvFileOpenFn(AbstractModuleClient_t* client, recv_file_open_fn fn, void *args){
    client->client->openfn = fn;
    client->client->openargs = args;
}
void amc_registerRecvFileCloseFn(AbstractModuleClient_t* client, recv_file_close_fn fn, void *args){
    client->client->closefn = fn;
    client->client->closeargs = args;
}
void amc_registerRecvAcquisitionFn(AbstractModuleClient_t* client, recv_acquisition_fn fn, void *args){
    client->client->acqfn = fn;
    client->client->acqargs = args;
}
void amc_registerRecvSourceFn(AbstractModuleClient_t* client, recv_source_fn fn, void *args){
    client->client->srcfn = fn;
    client->client->srcargs = args;
}
void amc_registerRecvQuitFn(AbstractModuleClient_t* client, recv_quit_fn fn, void* args){
    client->client->quitfn = fn;
    client->client->quitargs = args;
}

uint32_t amc_latestTrodesTimestamp(AbstractModuleClient_t* client){
    // reinterpret_cast<AbstractModuleClient*>(client)->latestTrodesTimestamp();
    return client->client->latestTrodesTimestamp();
}

const char* amc_getID(AbstractModuleClient_t* client){
    // return reinterpret_cast<AbstractModuleClient*>(client)->getID().c_str();
    return client->client->getID().c_str();
}

const char* amc_getEndpoint(AbstractModuleClient_t* client){
    // return reinterpret_cast<AbstractModuleClient*>(client)->getEndpoint().c_str();
    return client->client->getEndpoint().c_str();
}

const char* amc_getAddress(AbstractModuleClient_t* client){
    // return reinterpret_cast<AbstractModuleClient*>(client)->getAddress().c_str();
    return client->client->getAddress().c_str();
}

int amc_getPort(AbstractModuleClient_t* client){
    // return reinterpret_cast<AbstractModuleClient*>(client)->getPort();
    return client->client->getPort();
}

LFPConsumer_t* amc_subscribeLFPData(AbstractModuleClient_t* client, int buffersize, const char** ntrodes, int numntrodes){
    std::vector<std::string> nt;
    for(int i = 0; i < numntrodes; i++){
        nt.push_back(ntrodes[i]);
    }
    // return reinterpret_cast<LFPConsumer_t*>(reinterpret_cast<AbstractModuleClient*>(client)->subscribeLFPData(buffersize, nt));
    return reinterpret_cast<LFPConsumer_t*>(client->client->subscribeLFPData(buffersize, nt));
}




void lfp_initialize(LFPConsumer_t* lfp){
    reinterpret_cast<LFPConsumer*>(lfp)->initialize();
}

uint32_t lfp_getData(LFPConsumer_t* lfp, int16_t* data){
    return reinterpret_cast<LFPConsumer*>(lfp)->getData(data).trodes_timestamp;
}

int lfp_available(LFPConsumer_t* lfp, long timeout){
    return reinterpret_cast<LFPConsumer*>(lfp)->available(timeout);
}

int64_t lfp_lastSysTimestamp(LFPConsumer_t* lfp){
    return reinterpret_cast<LFPConsumer*>(lfp)->lastSysTimestamp();
}

void spk_initialize(SpikesConsumer_t* spk){
    reinterpret_cast<SpikesConsumer*>(spk)->initialize();
}
uint32_t spk_getData(SpikesConsumer_t* spk, spikePacket* packet){
    return reinterpret_cast<SpikesConsumer*>(spk)->getData(packet).trodes_timestamp;
}
int spk_available(SpikesConsumer_t* spk, long timeout){
    return reinterpret_cast<SpikesConsumer*>(spk)->available(timeout);
}
int64_t spk_lastSysTimestamp(SpikesConsumer_t* spk){
    return reinterpret_cast<SpikesConsumer*>(spk)->lastSysTimestamp();
}

int64_t system_time(){
    return CZHelp::systemTimeMSecs();
}