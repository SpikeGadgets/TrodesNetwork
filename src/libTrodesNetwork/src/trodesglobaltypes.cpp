#include "libTrodesNetwork/trodesglobaltypes.h"


size_t spikePacket::headersize(){
    return 2*sizeof(int)+sizeof(uint32_t);
}

size_t spikePacket::getMaxByteSize(int numchannels, int pointsperwaveform){
    return headersize() + sizeof(int) + numchannels*pointsperwaveform*sizeof(int2d);
}


StimulationCommand::StimulationCommand() {


    leadingPulseWidth_Samples = 0;
    leadingPulseAmplitude = 0;
    secondPulseWidth_Samples = 0;
    pulsePeriod_Samples = 0;
    numPulsesInTrain = 0;
    interPhaseDwell_Samples = 0;
    startDelay_Samples = 0; //Delay after go signal.  Useful for synchronizing multiple stim trains with relative delays
    anodePulseLeading = false;
    cathodeChannel = 0;
    anodeChannel = 0;
    _group = 0;
    _pulseMode = 0x01; //default is biphasic with dwell and cathode leading
    slot = 0;

    //Default settle and charge recovery behavior
    settleOn = true;
    preSettle_Samples = 10;
    postSettle_Samples = 10;
    chargeRecoveryDelay_Samples = 10;
    chargeRecoveryLength_Samples = 500;

    //Validation flags
    channelIsSet = false;
    slotIsSet = false;
    sequencerIsSet = false;


}

StimulationCommand::StimulationCommand(int slot, int cathodeChan, int anodeChan, uint16_t leadingPulseWidth_Samples, uint8_t leadingPulseAmplitude, uint16_t secondPulseWidth_Samples, uint8_t secondPulseAmplitude, uint16_t interPhaseDwell_Samples, uint16_t pulsePeriod_Samples, uint16_t startDelay_Samples)
    : StimulationCommand()
{
    setSlot(slot);
    setChannels(cathodeChan, anodeChan);
    setBiphasicPulseShape(leadingPulseWidth_Samples,leadingPulseAmplitude, secondPulseWidth_Samples, secondPulseAmplitude, interPhaseDwell_Samples, pulsePeriod_Samples, startDelay_Samples);
}

binarydata StimulationCommand::encode() const{
    return NetworkDataType::serializedata(_group, slot, cathodeChannel, anodeChannel,
                                          leadingPulseWidth_Samples, leadingPulseAmplitude,
                                          secondPulseWidth_Samples, secondPulseAmplitude,
                                          interPhaseDwell_Samples, pulsePeriod_Samples, startDelay_Samples);
}

void StimulationCommand::decode(const binarydata &data){
    NetworkDataType::deserializedata(data, _group, slot, cathodeChannel, anodeChannel,
                                     leadingPulseWidth_Samples, leadingPulseAmplitude,
                                     secondPulseWidth_Samples, secondPulseAmplitude,
                                     interPhaseDwell_Samples, pulsePeriod_Samples, startDelay_Samples);
    setSlot(slot);
    setChannels(cathodeChannel, anodeChannel);
    setBiphasicPulseShape(leadingPulseWidth_Samples, leadingPulseAmplitude,
                          secondPulseWidth_Samples, secondPulseAmplitude,
                          interPhaseDwell_Samples, pulsePeriod_Samples, startDelay_Samples);
}

bool StimulationCommand::isValid() {
    //Calculates whether or not the command is valid and returns the result
//    std::cerr << "-----valid: " << sequencerIsSet << slotIsSet << channelIsSet << "\n";
    return (sequencerIsSet && slotIsSet && channelIsSet);
}

bool StimulationCommand::setGroup(int groupNum) {
//    std::cerr << "set group " << groupNum << "\n";
    //Set the trigger group. Multiple slots can belong to the same trigger group, and can be triggered together.
    //bits 4-0 has the group number (up to 31), bit 5=0, bit 6=0, bit 7 has trigger enable)
    if (groupNum > 31) return false;
    if (groupNum < 0) return false;

    _group = 0x1F & groupNum; //set bits 0-4 to the group number
    _group |= 0x80; //set bit 7 to 1

    return true;

}

void StimulationCommand::setNoGroup() {
    //remove from trigger group
    _group = 0;
}

bool StimulationCommand::setSlot(int in_slot) {
//    std::cerr << "set slot " << in_slot << "\n";
    //Sets the trigger slot for the sequencer. The stim pattern can be triggered indiviually using this slot number.
    if ((in_slot > 255) || (in_slot < 0) ) {
        return false;
    }

    slot = (uint8_t)in_slot;
    slotIsSet = true;
    return true;
}

bool StimulationCommand::setChannels(int cathodeChan, int anodeChan) {
//    std::cerr << "set channels " << cathodeChan << " " << anodeChan << "\n";
    if ( (cathodeChan < 0) || (anodeChan < 0)) { //maybe also check that the channels are stim capable?
        return false;
    }

    cathodeChannel = cathodeChan;
    anodeChannel = anodeChan;
    channelIsSet = true;
    return true;
}

uint8_t StimulationCommand::setBiphasicPulseShape(uint16_t in_leadingPulseWidth_Samples,
                                               uint8_t in_leadingPulseAmplitude,
                                               uint16_t in_secondPulseWidth_Samples,
                                               uint8_t in_secondPulseAmplitude,
                                               uint16_t in_interPhaseDwell_Samples,
                                               uint16_t in_pulsePeriod_Samples,
                                               uint16_t in_startDelay_Samples) {

    //This function calculates the pulse train sequencer timeline

    //First we check if the parameters are safe and physically possible, and if not return the error code
    uint8_t returnCode = 0;
    if (in_pulsePeriod_Samples < (in_leadingPulseWidth_Samples+in_interPhaseDwell_Samples+in_secondPulseWidth_Samples)) {
        //Pulse period must be greater than the length of one pulse
        returnCode |= periodTooShort;
    }
    if (((uint32_t)in_leadingPulseAmplitude * in_leadingPulseWidth_Samples) != ((uint32_t)in_secondPulseAmplitude * in_secondPulseWidth_Samples)) {
        //The cathode and anode pulses must be charge balanced
        returnCode |= notChargeBalanced;
    }
    if (returnCode > 0) return returnCode;

    //Set internal variables
    //***********************
    leadingPulseWidth_Samples = in_leadingPulseWidth_Samples;
    leadingPulseAmplitude = in_leadingPulseAmplitude;
    secondPulseWidth_Samples = in_secondPulseWidth_Samples;
    secondPulseAmplitude = in_secondPulseAmplitude;
    interPhaseDwell_Samples = in_interPhaseDwell_Samples;
    pulsePeriod_Samples = in_pulsePeriod_Samples;
    startDelay_Samples = in_startDelay_Samples;
    //***********************


    //Calculate the exact timing of all marks in sequence
    _startPhaseOneMark = startDelay_Samples;
    _startPhaseTwoMark = _startPhaseOneMark+leadingPulseWidth_Samples;
    _startPhaseThreeMark = _startPhaseTwoMark+interPhaseDwell_Samples;
    _stimEndMark = _startPhaseThreeMark+secondPulseWidth_Samples;
    _stimRepeatMark = startDelay_Samples + pulsePeriod_Samples;
    _chargeRecoveryOnMark = _stimEndMark+chargeRecoveryDelay_Samples;
    _chargeRecoveryOffMark = _chargeRecoveryOnMark + chargeRecoveryLength_Samples;
    _endMark = _chargeRecoveryOffMark+1; //End sequence right after charge recovery is complete

    if (!settleOn) {
        //Settle is off and should never happen, so set to max value;
        _settleOnMark = 0xffff;
        _repeatedSettleOnMark = 0xffff;
    } else {
        //Settle is on

        //Start of first settle (before first pulse)
        if (_startPhaseOneMark >= preSettle_Samples) {
            _settleOnMark = _startPhaseOneMark - preSettle_Samples;
        } else {
            //If the delay to the first pulse is less than the pre settle period, start settle at trigger
            _settleOnMark = 0;
        }

        //End of all settles except the last one,
        //and beginning of all settles except the first one
        if ((_stimRepeatMark-preSettle_Samples-postSettle_Samples) < _stimEndMark) {
            //The pulse period is too short for a gap in settle between stim pulses,
            //so we just leave the settle on between pulses
            _repeatedSettleOffMark = 0xffff;
            _repeatedSettleOnMark = 0xffff;
        } else {
            _repeatedSettleOffMark = _stimEndMark + postSettle_Samples;
            _repeatedSettleOnMark = _stimRepeatMark-preSettle_Samples;
        }

        //End of the last settle after the last pulse
        if ( (_stimEndMark+postSettle_Samples) >= _endMark) {
            //If the post settle period is longer than the final refactory period,
            //end the final settle at the end of ref period instead
            _settleOffMark = _endMark;
        } else {
            _settleOffMark = _stimEndMark + postSettle_Samples;
        }

    }

    sequencerIsSet = true;
    return 0;
}

uint8_t StimulationCommand::getSlot() const
{
    return slot;
}

uint8_t StimulationCommand::get_group() const
{
    return _group;
}

uint16_t StimulationCommand::getNumPulsesInTrain() const
{
    return numPulsesInTrain;
}

uint8_t StimulationCommand::getPulseMode() const
{
    return _pulseMode;
}

uint16_t StimulationCommand::getSettleOnMark() const
{
    return _settleOnMark;
}

uint16_t StimulationCommand::getSettleOffMark() const
{
    return _settleOffMark;
}

uint16_t StimulationCommand::getStartPhaseOneMark() const
{
    return _startPhaseOneMark;
}

uint16_t StimulationCommand::getStartPhaseTwoMark() const
{
    return _startPhaseTwoMark;
}

uint16_t StimulationCommand::getStartPhaseThreeMark() const
{
    return _startPhaseThreeMark;
}

uint16_t StimulationCommand::getStimEndMark() const
{
    return _stimEndMark;
}

uint16_t StimulationCommand::getStimRepeatMark() const
{
    return _stimRepeatMark;
}

uint16_t StimulationCommand::getChargeRecoveryOnMark() const
{
    return _chargeRecoveryOnMark;
}

uint16_t StimulationCommand::getChargeRecoveryOffMark() const
{
    return _chargeRecoveryOffMark;
}

uint16_t StimulationCommand::getRepeatedSettleOnMark() const
{
    return _repeatedSettleOnMark;
}

uint16_t StimulationCommand::getRepeatedSettleOffMark() const
{
    return _repeatedSettleOffMark;
}

uint16_t StimulationCommand::getEndMark() const
{
    return _endMark;
}

uint16_t StimulationCommand::getCathodeChannel() const
{
    return cathodeChannel;
}

uint16_t StimulationCommand::getAnodeChannel() const
{
    return anodeChannel;
}

bool StimulationCommand::getAnodePulseLeading() const
{
    return anodePulseLeading;
}

uint8_t StimulationCommand::getSecondPulseAmplitude() const
{
    return secondPulseAmplitude;
}

uint8_t StimulationCommand::getLeadingPulseAmplitude() const
{
    return leadingPulseAmplitude;
}



GlobalStimulationSettings::GlobalStimulationSettings() {
    //Global mode settings
    settle_All_Channels_If_One_Channel_Is_Settling = false;
    settle_Same_Chip_If_One_Channel_Is_Settling = false;
    use_Fast_Settle_For_Amp_Settle_Mode = true; //false makes it use amplifier low frequency cutoff instead
    use_Switch_For_Charge_Recovery = true; //false makes it use current-limited drivers instead
    use_DC_amp = false;  //for 10 bit DC amplifiers. False makes it use 16 bit AC amplifiers
    scaleValue = max1uA;

}

void GlobalStimulationSettings::setVoltageScale(CurrentScaling s) {
    scaleValue = s;
}

binarydata GlobalStimulationSettings::encode() const{
    return NetworkDataType::serializedata(scaleValue);
}

void GlobalStimulationSettings::decode(const binarydata &data){
    CurrentScaling s;
    NetworkDataType::deserializedata(data, s);
    setVoltageScale(s);
}
//-----------------------------------------------------------------


GlobalStimulationCommand::GlobalStimulationCommand() {
    //Commands executed once
    resetSequencerCmd = false;
    killStimulationCmd = false;
    clearDSPOffsetRemovalCmd = false;
    enableStimulation = false;  //must be set to false to program new stim commands, and true to trigger them
}

void GlobalStimulationCommand::setStimEnabled(bool s) {
    enableStimulation = s;
}

void GlobalStimulationCommand::setAbortStimulation() {
    killStimulationCmd = true;
}

void GlobalStimulationCommand::setClearDSPOffset() {
    clearDSPOffsetRemovalCmd = true;
}

void GlobalStimulationCommand::setResetSequencer() {
    resetSequencerCmd = true;
}

binarydata GlobalStimulationCommand::encode() const{
    return NetworkDataType::serializedata(resetSequencerCmd, killStimulationCmd, clearDSPOffsetRemovalCmd, enableStimulation);
}

void GlobalStimulationCommand::decode(const binarydata &data){
    NetworkDataType::deserializedata(data, resetSequencerCmd, killStimulationCmd, clearDSPOffsetRemovalCmd, enableStimulation);
}
