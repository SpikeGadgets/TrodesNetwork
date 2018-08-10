#ifndef TRODES_NETWORK_CAPI_H
#define TRODES_NETWORK_CAPI_H
#include <stdint.h>
// #include "TrodesNetwork/trodesglobaltypes.h"
//C_API

typedef struct _abstractModuleClient_t AbstractModuleClient_t;
// typedef struct _TrodesMsg TrodesMsg_t;
// typedef struct _HFSubConsumer HFSubConsumer_t;
struct LFPConsumer_t;
typedef struct LFPConsumer_t LFPConsumer_t;
struct SpikesConsumer_t;
typedef struct SpikesConsumer_t SpikesConsumer_t;

/* AbstractModuleClient */
AbstractModuleClient_t* amc_new(const char* id, const char* addr, int port);
void                amc_destroy(AbstractModuleClient_t* client);
int                 amc_initialize(AbstractModuleClient_t* client);
void                amc_closeConnections(AbstractModuleClient_t* client);
void                amc_subscribeToEvent(AbstractModuleClient_t* client, const char* origin, const char* event);
void                amc_unsubscribeFromEvent(AbstractModuleClient_t* client, const char* origin, const char *event);
const char*         amc_getID(AbstractModuleClient_t* client);
const char*         amc_getEndpoint(AbstractModuleClient_t* client);
const char*         amc_getAddress(AbstractModuleClient_t* client);
int                 amc_getPort(AbstractModuleClient_t* client);

/* Sending out events and messages */

/* Streaming related functions */
uint32_t            amc_latestTrodesTimestamp(AbstractModuleClient_t* client);
LFPConsumer_t*      amc_subscribeLFPData(AbstractModuleClient_t* client, int buffersize, const char** ntrodes, int numntrodes);
SpikesConsumer_t*   amc_subscribeSpikesData(AbstractModuleClient_t* client, int buffersize, const char** ntrodes, int numntrodes);

/* Callback functions (replaces virtual fns) */
typedef void (*recv_file_open_fn) (void *args, const char* file);
typedef void (*recv_file_close_fn) (void *args);
typedef void (*recv_acquisition_fn) (void *args, const char* cmd, uint32_t timestamp);
typedef void (*recv_source_fn) (void *args, const char* source);
typedef void (*recv_quit_fn) (void *args);
void                amc_registerRecvFileOpenFn(AbstractModuleClient_t* client, recv_file_open_fn fn, void *args);
void                amc_registerRecvFileCloseFn(AbstractModuleClient_t* client, recv_file_close_fn fn, void *args);
void                amc_registerRecvAcquisitionFn(AbstractModuleClient_t* client, recv_acquisition_fn fn, void *args);
void                amc_registerRecvSourceFn(AbstractModuleClient_t* client, recv_source_fn fn, void *args);
void                amc_registerRecvQuitFn(AbstractModuleClient_t* client, recv_quit_fn fn, void *args);


/* LFP Stream*/
void                lfp_initialize(LFPConsumer_t* lfp);
uint32_t            lfp_getData(LFPConsumer_t* lfp, int16_t* data);
int                 lfp_available(LFPConsumer_t* lfp, long timeout);
int64_t             lfp_lastSysTimestamp(LFPConsumer_t* lfp);

/* Spikes Stream */
struct spikePacket;
typedef struct spikePacket spikePacket;
void                spk_initialize(SpikesConsumer_t* spk);
uint32_t            spk_getData(SpikesConsumer_t* spk, spikePacket* packet);
int                 spk_available(SpikesConsumer_t* spk, long timeout);
int64_t             spk_lastSysTimestamp(SpikesConsumer_t* spk);

/* Helper fns */
int64_t             system_time();
//C_API

#endif