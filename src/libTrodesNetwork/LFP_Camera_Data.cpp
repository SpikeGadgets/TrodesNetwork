/*
    Example code 
        
    1. Run Trodes w/ data streaming, open cameraModule, and turn on tracking. 

    2. Build and run this file:
        g++ -std=c++14 -I /home/kevin/trodesnetworkinstall/include/ -L /home/kevin/trodesnetworkinstall/lib/ LFP_Camera_Data.cpp -o LFP_Camera_Data -lTrodesNetwork -lpthread
        LD_LIBRARY_PATH=/home/kevin/trodesnetworkinstall/lib/ ./LFP_Camera_Data
*/


#include "TrodesNetwork/AbstractModuleClient.h"

typedef struct __attribute__((__packed__)) {
	uint32_t ts;
	int32_t line_segment;
	double pos_on_segment;
	int16_t x,y;
} trodes_vid_packet;

int main(int argc, char **argv)
{
    AbstractModuleClient *client = new AbstractModuleClient("LFP_Camera", "tcp://127.0.0.1", 49152);
    SpikesConsumer *spkconn;
    HFSubConsumer *vidconn;

    int res = client->initialize();
    if (res){
        printf("Couldn't init client\n");
        delete client;
        return -1;
    }
    
    std::cout << "Initialized client\n";

    spikePacket ssp;
    trodes_vid_packet svp;
    size_t vidsz;
    timestamp_t ts;

    int num_tetrodes = 3;
    const int TRODES_SPIKE_BUF_SZ = 1024;

    std::vector<std::string> ntrodes;
    for (int t = 0; t < num_tetrodes; t++)
    {
        for (int i = 0; i <= 8; i++)
            ntrodes.push_back(std::to_string(t + 1) + "," + std::to_string(i));
    }

    //Create and initialize spike connection and video position connection
    spkconn = client->subscribeSpikeData(TRODES_SPIKE_BUF_SZ, ntrodes);
    spkconn->initialize();

    vidconn = client->subscribeHighFreqData("PositionData", "CameraModule");
    vidconn->initialize();

    vidsz = vidconn->getType().getByteSize();
    assert(vidsz == sizeof(trodes_vid_packet));


    //first clear out old data between prep function and starting
    while (vidconn->available(0))
        vidconn->readData(&svp, vidsz);

    while (spkconn->available(0))
        spkconn->getData(&ssp);

    uint32_t start_time = client->latestTrodesTimestamp();

    //main loop
    uint32_t first_ts_spk, last_ts_spk;
    bool first_spk = true;
    uint32_t first_ts_vid, last_ts_vid, latest_ts;
    bool fs_vid = true;

    int spk_count = 0;

    //!!!!! switch here, 500 for simulation, 10000 for recording
    int max_spk_count = 500;

    while (spk_count < max_spk_count)
    {
        //receive data from trodes
        int n = spkconn->available(1);

        for (int i = 0; i < n; i++)
        {
            ts = spkconn->getData(&ssp);
            spk_count++;

            if (first_spk)
            {
                first_spk = false;
                first_ts_spk = ts.trodes_timestamp;
            }
            last_ts_spk = ts.trodes_timestamp;
            latest_ts = client->latestTrodesTimestamp();

            //process spike here
            std::cout << "spike: " << last_ts_spk << '\t' << latest_ts << '\t' << latest_ts-last_ts_spk << '\n';
        }

        n = vidconn->available(1);

        for (int i = 0; i < n; i++)
        {
            size_t rdsz = vidconn->readData(&svp, vidsz);
            if (rdsz != vidsz)
            {
                printf("Getting weird results from video data subscription!");
                return -1;
            }

            if (fs_vid)
            {
                std::cout << "got first video point\n";
                fs_vid = false;
                first_ts_vid = svp.ts;
            }
            last_ts_vid = svp.ts;
            latest_ts = client->latestTrodesTimestamp();

            //process video here
            std::cout << "video: " << last_ts_vid << '\t' << latest_ts << '\t' << latest_ts-last_ts_vid << '\n';
        }
    }

    uint32_t end_time = client->latestTrodesTimestamp();

    std::cout << "First:\n\t" << start_time << "\tlatestTrodesTimestamp\n\t" << first_ts_spk << "\tfirst spike\n\t" << first_ts_vid << "\tfirst video point\n";
    std::cout << "Last:\n\t" << end_time << "\tlatestTrodesTimestamp\n\t" << last_ts_spk << "\tlast spike\n\t" << last_ts_vid << "\tlast video point\n";

    delete client;
    return 0;
}
