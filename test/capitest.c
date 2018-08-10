#include "trodesnetwork_c.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stddef.h>

void quit(void* args){
    printf("**quitfn\n");
    *(int*)args = 1;
}

int main(){
    int q = 0;
    AbstractModuleClient_t *client = amc_new("C_client", "127.0.0.1", 49152);
    amc_initialize(client);
    amc_registerRecvQuitFn(client, &quit, &q);

    const char *ntrodes[] = {"1", "2", "5", "6", "7", "8", "10"};
    int16_t data[7];
    LFPConsumer_t *datastream = amc_subscribeLFPData(client, 100, ntrodes, 7);
    lfp_initialize(datastream);

    int i = 0; 
    while(!q){
        int n = lfp_available(datastream, 100);
        for(int j = 0; j < n; ++j){
            uint32_t t = lfp_getData(datastream, data);
            printf("%u\t%lu\t%hd\t%hd\t%hd\t%hd\t%hd\t%hd\t%hd\n", 
            t, system_time()-lfp_lastSysTimestamp(datastream), 
            data[0],data[1],data[2],data[3],data[4],data[5],data[6]);
            i++;
        }
    }

    amc_closeConnections(client);
    amc_destroy(client);
    return 0;
}