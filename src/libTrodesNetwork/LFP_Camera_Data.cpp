#include "main.hpp"

#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char **argv)
{
#pragma GCC diagnostic pop
    spikePacket ssp;
    trodes_vid_packet svp;
    size_t vidsz;
    timestamp_t ts;

    MRMClient *client = new MRMClient();
    SpikesConsumer *spkcon;
    HFSubConsumer *vidcon;

    int res = client->initialize();
    if (res)
    {
        printf("Couldn't init client\n");
        delete client;
        return -1;
    }

    int num_tetrodes = 3;
    const int TRODES_SPIKE_BUF_SZ = 1024;

    std::vector<std::string> ntrodes;
    for (int t = 0; t < num_tetrodes; t++)
    {
        for (int i = 0; i <= 8; i++)
            ntrodes.push_back(std::to_string(t + 1) + "," + std::to_string(i));
    }
    spkcon = client->subscribeSpikeData(TRODES_SPIKE_BUF_SZ, ntrodes);
    spkcon->initialize();

    vidcon = client->subscribeHighFreqData("PositionData", "CameraModule");
    vidcon->initialize();

    vidsz = vidcon->getType().getByteSize();
    assert(vidsz == sizeof(trodes_vid_packet));

    //first clear out old data between prep function and starting
    while (vidcon->available(0))
        vidcon->readData(&svp, vidsz);

    while (spkcon->available(0))
        spkcon->getData(&ssp);

    uint32_t start_time = client->latestTrodesTimestamp();

    //main loop
    //bool wrote_excl_t_start = false, wrote_excl_t_end = true;
    uint32_t first_ts_spk, last_ts_spk;
    bool fs_spk = true;
    uint32_t first_ts_vid, last_ts_vid, latest_ts;
    bool fs_vid = true;

    int spk_count = 0;

    //!!!!! switch here, 500 for simulation, 10000 for recording
    int max_spk_count = 500;
    //int max_spk_count = 10000;

    while (spk_count < max_spk_count)
    {
        //receive data from trodes
        int n = spkcon->available(1);

        for (int i = 0; i < n; i++)
        {
            ts = spkcon->getData(&ssp);
            spk_count++;

            if (fs_spk)
            {
                //std::cout << "got first spike\n";
                fs_spk = false;
                first_ts_spk = ts.trodes_timestamp;
            }
            last_ts_spk = ts.trodes_timestamp;
            latest_ts = client->latestTrodesTimestamp();
            std::cout << "spike: " << last_ts_spk << '\t' << latest_ts << '\t' << latest_ts-last_ts_spk << '\n';

            //process spike
        }

        n = vidcon->available(1);

        for (int i = 0; i < n; i++)
        {
            size_t rdsz = vidcon->readData(&svp, vidsz);
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
            std::cout << "video: " << last_ts_vid << '\t' << latest_ts << '\t' << latest_ts-last_ts_vid << '\n';

            //process video
        }
    }

    uint32_t end_time = client->latestTrodesTimestamp();

    std::cout << "First:\n\t" << start_time << "\tlatestTrodesTimestamp\n\t" << first_ts_spk << "\tfirst spike\n\t" << first_ts_vid << "\tfirst video point\n";
    std::cout << "Last:\n\t" << end_time << "\tlatestTrodesTimestamp\n\t" << last_ts_spk << "\tlast spike\n\t" << last_ts_vid << "\tlast video point\n";

    delete client;
    return 0;
}
