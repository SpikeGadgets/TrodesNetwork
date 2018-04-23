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

    binarydata encode() const;
    void decode(const binarydata &data);


    enum PulseShapeErrorCodes {
        periodTooShort = 0x01,
        notChargeBalanced = 0x02
    };


    bool isValid();

    //User control commands used to create the stimulation pattern
    bool setGroup(int groupNum);
    void setNoGroup();
    bool setSlot(int slot);
    bool setChannels(int cathodeNTrode, int cathodeChan, int anodeNTrode, int anodeChan);
    uint8_t setBiphasicPulseShape(uint16_t leadingPulseWidth_Samples, uint8_t leadingPulseAmplitude, uint16_t secondPulseWidth_Samples, uint8_t secondPulseAmplitude, uint16_t interPhaseDwell_Samples, uint16_t pulsePeriod_Samples, uint16_t startDelay_Samples);
    void setNumPulsesInTrain(uint16_t pulses);

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
    uint16_t getCathodeNTrodeID() const;
    uint16_t getAnodeNTrodeID() const;
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
    uint16_t cathodeNtrodeID;
    uint16_t anodeNtrodeID;
    uint8_t slot;

    //Validation flags
    bool channelIsSet;
    bool slotIsSet;
    bool sequencerIsSet;

};


class GlobalStimulationSettings : public NetworkDataType{
public:
    GlobalStimulationSettings();

    enum CurrentScaling {
        max10nA = 0x0,
        max20nA = 0x1,
        max50nA = 0x2,
        max100nA = 0x3,
        max200nA = 0x4,
        max500nA = 0x5,
        max1uA = 0x6,
        max2uA = 0x7,
        max5uA = 0x8,
        max10uA = 0x9
    };

    void setVoltageScale(CurrentScaling s);
    //void setSettleAll(bool s);
    //void setSettleChip(bool s);
    //void setUseFastSettle(bool s);
    //void setChargeRecoverySwitch(bool s);
    //void setUseDCAmp(bool s);

    binarydata encode() const;
    void decode(const binarydata &data);
    inline CurrentScaling currentScaling() {return scaleValue;}
    inline bool settleAll() {return settle_All_Channels_If_One_Channel_Is_Settling;}
    inline bool settleChip() {return settle_Same_Chip_If_One_Channel_Is_Settling;}
    inline bool useFastSettle() {return use_Fast_Settle_For_Amp_Settle_Mode;}
    inline bool chargeRecoverySwitch() {return use_Switch_For_Charge_Recovery;}
    inline bool useDCAmp() {return use_DC_amp;}

private:
    //Global mode settings
    CurrentScaling scaleValue;
    bool settle_All_Channels_If_One_Channel_Is_Settling;
    bool settle_Same_Chip_If_One_Channel_Is_Settling;
    bool use_Fast_Settle_For_Amp_Settle_Mode; //false makes it use amplifier low frequency cutoff instead
    bool use_Switch_For_Charge_Recovery; //false makes it use current-limited drivers instead
    bool use_DC_amp;  //for 10 bit DC amplifiers. False makes it use 16 bit AC amplifiers



};

class GlobalStimulationCommand : public NetworkDataType {
public:
    GlobalStimulationCommand();
    void setStimEnabled(bool s);
    void setResetSequencer();
    void setAbortStimulation();
    void setClearDSPOffset();

    binarydata encode() const;
    void decode(const binarydata &data);

    inline bool stimEnabled() {return enableStimulation;}
    inline bool sequencerReset() {return resetSequencerCmd;}
    inline bool stimulationAborted() {return killStimulationCmd;}
    inline bool dspOffsetCleared() {return clearDSPOffsetRemovalCmd;}

private:

    //Commands executed once
    bool resetSequencerCmd;
    bool killStimulationCmd;
    bool clearDSPOffsetRemovalCmd;
    bool enableStimulation;  //must be set to false to program new stim commands, and true to trigger them

};


#endif // TRODESGLOBALTYPES_H
