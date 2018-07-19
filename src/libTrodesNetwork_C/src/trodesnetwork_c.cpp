#include "libTrodesNetwork/AbstractModuleClient.h"

extern "C"{
    #include "trodesnetwork_c.h"
}

AbstractModuleClient_t* amc_new(const char* id, const char* addr, int port){
    return reinterpret_cast<AbstractModuleClient_t*>(new AbstractModuleClient(id, addr, port));
}

void amc_destroy(AbstractModuleClient_t* client){
    if(!client)
        return;
    delete reinterpret_cast<AbstractModuleClient*>(client);
}

int amc_initialize(AbstractModuleClient_t* client){
    return reinterpret_cast<AbstractModuleClient*>(client)->initialize();
}
void amc_closeConnections(AbstractModuleClient_t* client){
    reinterpret_cast<AbstractModuleClient*>(client)->closeConnections();
}

void amc_subscribeToEvent(AbstractModuleClient_t* client, const char* origin, const char* event){
    reinterpret_cast<AbstractModuleClient*>(client)->subscribeToEvent(origin, event);
}

void amc_unsubscribeFromEvent(AbstractModuleClient_t* client, const char* origin, const char *event){
    reinterpret_cast<AbstractModuleClient*>(client)->unsubscribeFromEvent(origin, event);
}

uint32_t amc_latestTrodesTimestamp(AbstractModuleClient_t* client){
    reinterpret_cast<AbstractModuleClient*>(client)->latestTrodesTimestamp();
}

const char* amc_getID(AbstractModuleClient_t* client){
    return reinterpret_cast<AbstractModuleClient*>(client)->getID().c_str();
}

const char* amc_getEndpoint(AbstractModuleClient_t* client){
    return reinterpret_cast<AbstractModuleClient*>(client)->getEndpoint().c_str();
}

const char* amc_getAddress(AbstractModuleClient_t* client){
    return reinterpret_cast<AbstractModuleClient*>(client)->getAddress().c_str();
}

int amc_getPort(AbstractModuleClient_t* client){
    return reinterpret_cast<AbstractModuleClient*>(client)->getPort();
}

LFPConsumer_t* amc_subscribeLFPData(AbstractModuleClient_t* client, int buffersize, const char** ntrodes, int numntrodes){
    std::vector<std::string> nt;
    for(int i = 0; i < numntrodes; i++){
        nt.push_back(ntrodes[i]);
    }
    return reinterpret_cast<LFPConsumer_t*>(reinterpret_cast<AbstractModuleClient*>(client)->subscribeLFPData(buffersize, nt));
}




void lfp_initialize(LFPConsumer_t* lfp){
    reinterpret_cast<LFPConsumer*>(lfp)->initialize();
}

timestamp_t lfp_getData(LFPConsumer_t* lfp, int16_t* data){
    return reinterpret_cast<LFPConsumer*>(lfp)->getData(data);
}

size_t lfp_available(LFPConsumer_t* lfp, long timeout){
    return reinterpret_cast<LFPConsumer*>(lfp)->available(timeout);
}

int64_t lfp_lastSysTimestamp(LFPConsumer_t* lfp){
    return reinterpret_cast<LFPConsumer*>(lfp)->lastSysTimestamp();
}
