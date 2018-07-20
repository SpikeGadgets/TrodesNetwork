#ifndef TRODES_NETWORK_CAPI_H
#define TRODES_NETWORK_CAPI_H
#include <stdint.h>
//C_API
struct AbstractModuleClient_t;
typedef struct AbstractModuleClient_t AbstractModuleClient_t;
// typedef struct _TrodesMsg TrodesMsg_t;
// typedef struct _HFSubConsumer HFSubConsumer_t;
struct LFPConsumer_t;
typedef struct LFPConsumer_t LFPConsumer_t;


AbstractModuleClient_t* amc_new(const char* id, const char* addr, int port);
void                amc_destroy(AbstractModuleClient_t* client);
int                 amc_initialize(AbstractModuleClient_t* client);
void                amc_closeConnections(AbstractModuleClient_t* client);
void                amc_subscribeToEvent(AbstractModuleClient_t* client, const char* origin, const char* event);
void                amc_unsubscribeFromEvent(AbstractModuleClient_t* client, const char* origin, const char *event);
// int                 amc_sendMsgToTrodes(AbstractModuleClient_t* client, const char* subject, TrodesMsg_t *msg);
// int                 amc_sendMsgToModule(AbstractModuleClient_t* client, const char* module, const char* subject, TrodesMsg_t *msg);
// int                 amc_sendOutEvent(AbstractModuleClient_t* client, const char* event, TrodesMsg_t *msg);
uint32_t            amc_latestTrodesTimestamp(AbstractModuleClient_t* client);
// HFSubConsumer_t*    amc_subscribeHighFreqData(AbstractModuleClient_t* client, const char* dataName, const char* origin, size_t bufferlen);
const char*         amc_getID(AbstractModuleClient_t* client);
const char*         amc_getEndpoint(AbstractModuleClient_t* client);
const char*         amc_getAddress(AbstractModuleClient_t* client);
int                 amc_getPort(AbstractModuleClient_t* client);
LFPConsumer_t*      amc_subscribeLFPData(AbstractModuleClient_t* client, int buffersize, const char** ntrodes, int numntrodes);


void                lfp_initialize(LFPConsumer_t* lfp);
uint32_t            lfp_getData(LFPConsumer_t* lfp, int16_t* data);
int                 lfp_available(LFPConsumer_t* lfp, long timeout);
int64_t             lfp_lastSysTimestamp(LFPConsumer_t* lfp);


#endif