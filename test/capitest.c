#include "trodesnetwork_c.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stddef.h>
int main(){
    AbstractModuleClient_t *client = amc_new("C_client", "127.0.0.1", 49152);
    amc_initialize(client);

    const char *ntrodes[] = {"1", "2", "5", "6", "7", "8", "10"};
    int16_t data[7];
    LFPConsumer_t *datastream = amc_subscribeLFPData(client, 10, ntrodes, 7);
    lfp_initialize(datastream);

    int i = 0; 
    while(i < 15000){
        int n = lfp_available(datastream, 100);
        for(int j = 0; j < n; ++j){
            uint32_t t = lfp_getData(datastream, data);
            i++;
            printf("%u\t%hd\t%hd\t%hd\t%hd\t%hd\t%hd\t%hd\n", 
            t,data[0],data[1],data[2],data[3],data[4],data[5],data[6]);

        }
    }

    amc_closeConnections(client);
    amc_destroy(client);
    return 0;
}