#ifndef TRODESGLOBALTYPES_H
#define TRODESGLOBALTYPES_H
#include <stdint.h>
#include "networkDataTypes.h"

static const char DEFAULT_SERVER_ENDPOINT[] = "tcp://127.0.0.1:49152";
static const char DEFAULT_SERVER_ADDRESS[] = "tcp://127.0.0.1";
static const int DEFAULT_SERVER_PORT = 49152;

static const char TRODES_NETWORK_ID[] = "Trodes";
static const char BROKER_NETWORK_ID[] = "Broker";

static const char CLIENT_CONNECT_MSG[] = "_connect";
static const char CLIENT_EXPIRED_MSG[] = "_expired";
static const char CLIENT_DISCONNECT_MSG[] = "_disconnect";

//Subjects

static const char TRODES_CMD[] = "cmd";
static const char TRODES_INFO_REQ[] = "info_req";
static const char TRODES_INFO_REP[] = "info_rep";
static const char TRODES_NETWORK_NOTIFICATION[] = "network_note";

static const int POINTS_IN_WAVE = 40;

static const char quit_CMD [] = "_Quit";
static const char file_CMD [] = "_File";
static const char file_OPEN [] = "_fopen";
static const char file_CLOSE [] = "_fclose";

static const char source_CMD [] = "_Source";
enum TrodesSource : int{
    None, Fake, FakeSpikes, File, Ethernet, USBDAQ, Rhythm
};

static const char acquisition_CMD [] = "_Acq";
enum TrodesAcquisition : int{
    Stop, Play, Pause, Seek
};

static const char acq_PLAY [] = "play";
static const char acq_STOP [] = "stop";
static const char acq_PAUSE [] = "pause";
static const char acq_SEEK [] = "seek";

static const char settle_CMD [] = "_settle";

static const char TIMESTAMPS_SOCK [] = "tr_timestamps";

struct timestamp_t{
    uint32_t trodes_timestamp;
    int64_t system_timestamp;
//    int64_t monotonic_timestamp;
};



//Predefined High Frequency Data Types
static const char hfType_NEURO[] = "neurostream";
static const char hfType_ANALOG[] = "analogstream";
static const char hfType_DIGITAL [] = "digitalstream";
static const char hfType_SPIKE[] = "spikestream";
static const char hfType_LFP[] = "lfpstream";



struct lfpPacket{
    int ntrodeid;
    uint32_t timestamp;
    int16_t value;
};

struct int2d{
    int16_t x, y;
};
struct spikePacket{
    int ntrodeid;
    int cluster;
    uint32_t timestamp;
    std::vector<int2d> points;
    static size_t headersize();
    static size_t getMaxByteSize(int numchannels, int pointsperwaveform);
};

class StimulationCommand : public NetworkDataType{
public:
    StimulationCommand();
    StimulationCommand(int slot, int cathodeChan, int anodeChan, uint16_t leadingPulseWidth_Samples, uint8_t leadingPulseAmplitude,
                       uint16_t secondPulseWidth_Samples, uint8_t secondPulseAmplitude, uint16_t interPhaseDwell_Samples,
                       uint16_t pulsePeriod_Samples, uint16_t startDelay_Samples);

    binarydata encode() const;
    void decode(const binarydata &data);

    enum GlobalStimCurrentScaling {
        max10nA = 10,
        max20nA = 20,
        max50nA = 50,
        max100nA = 100,
        max200nA = 200,
        max500nA = 500,
        max1uA = 1000,
        max2uA = 2000,
        max5uA = 5000,
        max10uA = 10000
    };

    enum PulseShapeErrorCodes {
        periodTooShort = 0x01,
        notChargeBalanced = 0x02
    };


    bool isValid();

    //User control commands used to create the stimulation pattern
    bool setGroup(int groupNum);
    void setNoGroup();
    bool setSlot(int slot);
    bool setChannels(int cathodeChan, int anodeChan);
    uint8_t setBiphasicPulseShape(uint16_t leadingPulseWidth_Samples, uint8_t leadingPulseAmplitude, uint16_t secondPulseWidth_Samples, uint8_t secondPulseAmplitude, uint16_t interPhaseDwell_Samples, uint16_t pulsePeriod_Samples, uint16_t startDelay_Samples);

//    uint8_t setBiphasicPulseShape_usec();
    uint8_t getSlot() const;
    uint8_t get_group() const;
    uint16_t getNumPulsesInTrain() const;
    uint8_t getPulseMode() const;
    uint16_t getSettleOnMark() const;
    uint16_t getSettleOffMark() const;
    uint16_t getStartPhaseOneMark() const;
    uint16_t getStartPhaseTwoMark() const;
    uint16_t getStartPhaseThreeMark() const;
    uint16_t getStimEndMark() const;
    uint16_t getStimRepeatMark() const;
    uint16_t getChargeRecoveryOnMark() const;
    uint16_t getChargeRecoveryOffMark() const;
    uint16_t getRepeatedSettleOnMark() const;
    uint16_t getRepeatedSettleOffMark() const;
    uint16_t getEndMark() const;
    uint16_t getCathodeChannel() const;
    uint16_t getAnodeChannel() const;
    bool getAnodePulseLeading() const;
    uint8_t getSecondPulseAmplitude() const;
    uint8_t getLeadingPulseAmplitude() const;

private:

    //Internal variables that are calculated and sent to hardware
    uint8_t _group;
    uint8_t _pulseMode;
    uint16_t _settleOnMark;
    uint16_t _settleOffMark;
    uint16_t _startPhaseOneMark;
    uint16_t _startPhaseTwoMark;
    uint16_t _startPhaseThreeMark;
    uint16_t _stimEndMark;
    uint16_t _stimRepeatMark;
    uint16_t _chargeRecoveryOnMark;
    uint16_t _chargeRecoveryOffMark;
    uint16_t _repeatedSettleOnMark;
    uint16_t _repeatedSettleOffMark;
    uint16_t _endMark;

    //Stim parameters
    uint16_t leadingPulseWidth_Samples;
    uint8_t leadingPulseAmplitude; //depends on glabal scaling value
    uint16_t secondPulseWidth_Samples;
    uint8_t secondPulseAmplitude; //depends on glabal scaling value
    uint16_t pulsePeriod_Samples;
    uint16_t numPulsesInTrain;
    uint16_t interPhaseDwell_Samples;
    uint16_t preSettle_Samples;
    uint16_t postSettle_Samples;
    uint16_t startDelay_Samples; //Delay after go signal.  Useful for synchronizing multiple stim trains with relative delays
    uint16_t chargeRecoveryDelay_Samples; //delay after last pulse before charge recovery mechanism turns on
    uint16_t chargeRecoveryLength_Samples; //length of charge recovery period
    bool  anodePulseLeading;
    bool  settleOn;
    uint16_t cathodeChannel;
    uint16_t anodeChannel;
    uint8_t slot;

    //Validation flags
    bool channelIsSet;
    bool slotIsSet;
    bool sequencerIsSet;

};

#endif // TRODESGLOBALTYPES_H
