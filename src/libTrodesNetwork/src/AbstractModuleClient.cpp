#include "libTrodesNetwork/AbstractModuleClient.h"
#include <czmq.h>
AbstractModuleClient::AbstractModuleClient(const char* id, const char* addr, int port)
    : MlmWrap(id, addr, port),timestampsub(NULL) {
    if (isConnectedToBroker()) {
        initializeStreams();
    }
}

AbstractModuleClient::AbstractModuleClient(std::string id, std::string addr, int port)
    : MlmWrap(id.c_str(), addr.c_str(), port), timestampsub(NULL), timestampaddress("") {
    if (isConnectedToBroker()){
        initializeStreams();
    }
}


AbstractModuleClient::~AbstractModuleClient() {
    if(timestampsub)
        zsock_destroy(&timestampsub);
}

void AbstractModuleClient::initializeStreams(){
    //set the module as a subscriber to the general trodes commands stream
    try{
        this->subscribeStream(TRODES_NETWORK_ID, TRODES_CMD);
        this->subscribeStream(TRODES_NETWORK_ID, TRODES_NETWORK_NOTIFICATION);
    } catch(std::string &e){
        std::cout << e << std::endl;
    }

    //initialize module data stream
    try{
        this->setProducer(this->id.c_str());
    } catch(std::string &e){
        std::cout << e << std::endl;
    }
    timestampsub = NULL;
}

void AbstractModuleClient::subscribeToEvent(std::string origin, std::string event){
    subbedEvents.insert(event);
    subscribeEvent(origin, event);
}

void AbstractModuleClient::unsubscribeFromEvent(std::string origin, std::string event){
    //remove event
    subbedEvents.erase(event);

    //remove all from origin and resubscribe to subbed events
    removeSubscriptions(origin.c_str());
    for(auto &e : subbedEvents){
        subscribeEvent(origin, e);
    }
    if(streq(origin.c_str(), TRODES_NETWORK_ID)){
        this->subscribeStream(TRODES_NETWORK_ID, TRODES_CMD);
        this->subscribeStream(TRODES_NETWORK_ID, TRODES_NETWORK_NOTIFICATION);
    }
}

int AbstractModuleClient::sendMsgToTrodes(std::string subject, TrodesMsg &msg){
    int rc = sendMessage(TRODES_NETWORK_ID, subject.c_str(), msg);
    return rc;
}

int AbstractModuleClient::sendMsgToModule(std::string module, std::string subject, TrodesMsg &msg){
    int rc = sendMessage(module.c_str(), subject.c_str(), msg);
    return rc;
}

int AbstractModuleClient::sendOutEvent(const std::string &event, TrodesMsg &msg){
    int rc = sendEvent(event, msg);
    return rc;
}

void AbstractModuleClient::sendTimeRequest(){
//    TrodesMsg msg("s", INFO_TIME);
//    sendMsgToTrodes(TRODES_INFO_REQ, msg);
    recv_time(latestTrodesTimestamp());
}

void AbstractModuleClient::sendTimeRateRequest(){
    TrodesMsg msg("s", INFO_TIMERATE);
    sendMsgToTrodes(TRODES_INFO_REQ, msg);
}

void AbstractModuleClient::sendPlaybackCommand(std::string cmd, uint32_t time) {
    TrodesMsg msg("ss4", acquisition_CMD, cmd, time);
    sendMsgToTrodes(TRODES_CMD, msg); //tell trodes to relay the command
}


HFSubConsumer* AbstractModuleClient::subscribeHighFreqData(std::string dataName, std::string originModule, size_t messageBufferLength) {
//    std::cout << "mlmWRAP: Attempting to make a Consumer Sub to [" << dataName << "] from [" << originModule << "]\n";

    if (isHfTypeCurrentlySubbed(dataName, originModule)) { //if the module is already subbed, return the current sub object
        HighFreqSub* sub = getHfSubObject(dataName, originModule);
        if(sub->getSubSockType() == HighFreqSub::ST_CONSUMER)
            return (HFSubConsumer*)sub;
    }

    if (state->hfdt_exists(dataName, originModule)) { //check to make sure the requested type is provided by another module
        return( createConsumerSub( state->find_hfdt(dataName, originModule), messageBufferLength ) );
    }
    else {
        std::cerr << error("The specified type does not exist.\n");
        return(NULL);
    }
}

HFSubWorker* AbstractModuleClient::subscribeHighFreqData(std::string dataName, std::string originModule, hfs_data_callback_fn userFoo, void *args) {
    if (isHfTypeCurrentlySubbed(dataName, originModule)) { //if the module is already subbed, return the current sub object
        HighFreqSub* sub = getHfSubObject(dataName, originModule);
        if(sub->getSubSockType() == HighFreqSub::ST_WORKER)
            return (HFSubWorker*)sub;
    }

    if (state->hfdt_exists(dataName, originModule)) { //check to make sure the requested type is provided by another module
        return( createWorkerSub( state->find_hfdt(dataName, originModule), 1, userFoo, args ) );
    }
    else {
        std::cerr << error("The specified type does not exist. Could not create Subscriber\n");
        return(NULL);
    }
}



int AbstractModuleClient::unsubscribeHighFreqData(std::string dataName, std::string originModule) {
//    std::cout << "mlmWrap: Unsubscribing from [" << dataName << "-" << originModule <<"]\n";
    int retval = -1;
    if (isHfTypeCurrentlySubbed(dataName, originModule)) {
        removeSubFromList(getSubbedHfType(dataName, originModule));
        retval = removeHfTypeFromSubbedList(getSubbedHfType(dataName, originModule));
    }
    return(retval);
}


std::vector<std::string> AbstractModuleClient::getAvailableTrodesData(std::string data){
    std::vector<std::string> list;
    if(data == hfType_NEURO){
        list.push_back("Format: 'ntrode_id, nth channel'");
        for(auto &nt : getTrodesConfig().getNTrodes()){
            for(size_t ch = 0; ch < nt.getHw_chans().size(); ch++){
                list.push_back(nt.getId() + "," + std::to_string(ch+1));
            }
        }
    }
    else if(data == hfType_LFP){
        list.push_back("Format: 'ntrode_id'");
        for(auto &nt : getTrodesConfig().getNTrodes()){
            list.push_back(nt.getId());
        }
    }
    else if(data == hfType_ANALOG){
        list.push_back("Format: 'Device_name, channel_id'");
        for(auto &dv : getTrodesConfig().getDevices()){
            for(auto &ch : dv.getChannels()){
                if(ch.getType() == 1)
                    list.push_back(dv.getName() + "," + ch.getId());
            }
        }
    }
    else if(data == hfType_DIGITAL){
        list.push_back("Format: 'Device_name, channel_id'");
        for(auto &dv : getTrodesConfig().getDevices()){
            for(auto &ch : dv.getChannels()){
                if(ch.getType() == 0)
                    list.push_back(dv.getName() + "," + ch.getId());
            }
        }
    }
    else if(data == hfType_SPIKE){
        list.push_back("Format: 'ntrode_id, cluster'");
        for(auto &nt : getTrodesConfig().getNTrodes()){
            list.push_back(nt.getId() + ", [0-8]");
        }
    }
    return list;
}

NeuralConsumer* AbstractModuleClient::subscribeNeuralData(size_t buffersize, std::vector<std::string> channels){
    if(isHfTypeCurrentlySubbed(hfType_NEURO, TRODES_NETWORK_ID)){
        return (NeuralConsumer*)getHfSubObject(hfType_NEURO, TRODES_NETWORK_ID);
    }

    HighFreqDataType dt = state->find_hfdt(hfType_NEURO, TRODES_NETWORK_ID);
    if(!dt.isValid()){
        std::cerr << error("Neural data could not be found on the network!");
        return NULL;
    }
    HFParsingInfo parseinfo = createNeuralParsingInfo(channels, dt.getDataFormat());
    NeuralConsumer* newsub = new NeuralConsumer(dt, buffersize, parseinfo);
    HFSubSockSettings sockinfo(dt, newsub->getSubSockType(), NULL, NULL);
    addSubToList(newsub);
    addHfTypeToSubbedList(sockinfo);
    return newsub;
}

LFPConsumer* AbstractModuleClient::subscribeLFPData(size_t buffersize, std::vector<std::string> ntrodes){
    if(isHfTypeCurrentlySubbed(hfType_LFP, TRODES_NETWORK_ID)){
        return (LFPConsumer*)getHfSubObject(hfType_LFP, TRODES_NETWORK_ID);
    }

    HighFreqDataType dt = state->find_hfdt(hfType_LFP, TRODES_NETWORK_ID);
    if(!dt.isValid()){
        std::cerr << error("LFP data could not be found on the network!");
        return NULL;
    }
    HFParsingInfo parseinfo = createLFPParsingInfo(ntrodes, dt.getDataFormat());
    LFPConsumer* newsub = new LFPConsumer(dt, buffersize, parseinfo);
    HFSubSockSettings sockinfo(dt, newsub->getSubSockType(), NULL, NULL);
    addSubToList(newsub);
    addHfTypeToSubbedList(sockinfo);
    return newsub;
}

SpikesConsumer* AbstractModuleClient::subscribeSpikeData(size_t buffersize, std::vector<std::string> ntrodes){
    if(isHfTypeCurrentlySubbed(hfType_SPIKE, TRODES_NETWORK_ID)){
        return (SpikesConsumer*)getHfSubObject(hfType_SPIKE, TRODES_NETWORK_ID);
    }
    HighFreqDataType dt = state->find_hfdt(hfType_SPIKE, TRODES_NETWORK_ID);
    if(!dt.isValid()){
        std::cerr << error("Spikes data could not be found on the network!");
        return NULL;
    }
    HFParsingInfo parseinfo = createSpikesParsingInfo(ntrodes, dt.getDataFormat());
    size_t max_nchans = 0;
    for(auto const& nt : getTrodesConfig().getNTrodes()){
        if(nt.getHw_chans().size() > max_nchans){
            max_nchans = nt.getHw_chans().size();
        }
    }
    SpikesConsumer* newsub = new SpikesConsumer(dt, buffersize, parseinfo, max_nchans*POINTS_IN_WAVE);
    HFSubSockSettings sockinfo(dt, newsub->getSubSockType(), NULL, NULL);
    addSubToList(newsub);
    addHfTypeToSubbedList(sockinfo);
    return newsub;
}

DigitalConsumer* AbstractModuleClient::subscribeDigitalData(size_t buffersize, std::vector<std::string> channels){
    if(isHfTypeCurrentlySubbed(hfType_DIGITAL, TRODES_NETWORK_ID)){
        return (DigitalConsumer*)getHfSubObject(hfType_DIGITAL, TRODES_NETWORK_ID);
    }
    HighFreqDataType dt = state->find_hfdt(hfType_DIGITAL, TRODES_NETWORK_ID);
    if(!dt.isValid()){
        std::cerr << error("Digital data could not be found on the network!");
        return NULL;
    }
    HFParsingInfo parseinfo = createDigitalParsingInfo(channels, dt.getDataFormat());
    DigitalConsumer* newsub = new DigitalConsumer(dt, buffersize, parseinfo);
    HFSubSockSettings sockinfo(dt, newsub->getSubSockType(), NULL, NULL);
    addSubToList(newsub);
    addHfTypeToSubbedList(sockinfo);
    return newsub;
}

AnalogConsumer* AbstractModuleClient::subscribeAnalogData(size_t buffersize, std::vector<std::string> channels){
    if(isHfTypeCurrentlySubbed(hfType_ANALOG, TRODES_NETWORK_ID)){
        return (AnalogConsumer*)getHfSubObject(hfType_ANALOG, TRODES_NETWORK_ID);
    }
    HighFreqDataType dt = state->find_hfdt(hfType_ANALOG, TRODES_NETWORK_ID);
    if(!dt.isValid()){
        std::cerr << error("Analog data could not be found on th enetwork!");
        return NULL;
    }
    HFParsingInfo parseinfo = createAnalogParsingInfo(channels, dt.getDataFormat());
    AnalogConsumer* newsub = new AnalogConsumer(dt, buffersize, parseinfo);
    HFSubSockSettings sockinfo(dt, newsub->getSubSockType(), NULL, NULL);
    addSubToList(newsub);
    addHfTypeToSubbedList(sockinfo);
    return newsub;
}


void AbstractModuleClient::sendAnnotationRequest(std::string msg){
    TrodesMsg m("s4s", ANNOTATION_REQ, latestTrodesTimestamp(), msg);
    sendMsgToTrodes(TRODES_INFO_REQ, m);
}


bool AbstractModuleClient::initializeHardwareConnection(){
    subscribeStream(TRODES_NETWORK_ID, hardware_update);
    return init_hardware_connection();
}

void AbstractModuleClient::destroyHardwareConnection(){
    unsubscribeFromEvent(TRODES_NETWORK_ID, hardware_update);
    destroy_hardware_connection();
}

bool AbstractModuleClient::sendSettleCommand(){
    TrodesMsg m;
    m.addcontents("s", settle_CMD);
    return sendHardwareMessage(m);
}
bool AbstractModuleClient::sendStimulationParams(StimulationCommand command){
    TrodesMsg m;
    m.addcontents("sn", "SET", command.encode()); //TODO: add stimcommand obj
    return sendHardwareMessage(m);
}

bool AbstractModuleClient::sendClearStimulationParams(uint16_t slot){
    TrodesMsg m;
    m.addcontents("s2", "CLEAR", slot);
    return sendHardwareMessage(m);
}

bool AbstractModuleClient::sendStimulationStartSlot(uint16_t slot){
    TrodesMsg m;
    m.addcontents("ss2", "START", "SLOT", slot);
    return sendHardwareMessage(m);
}

bool AbstractModuleClient::sendStimulationStartGroup(uint16_t group){
    TrodesMsg m;
    m.addcontents("ss2", "START", "GROUP", group);
    return sendHardwareMessage(m);
}

bool AbstractModuleClient::sendStimulationStopSlot(uint16_t slot){
    TrodesMsg m;
    m.addcontents("ss2", "STOP", "SLOT", slot);
    return sendHardwareMessage(m);
}

bool AbstractModuleClient::sendStimulationStopGroup(uint16_t group){
    TrodesMsg m;
    m.addcontents("ss2", "STOP", "GROUP", group);
    return sendHardwareMessage(m);
}

bool AbstractModuleClient::sendGlobalStimulationSettings(GlobalStimulationSettings settings){
    TrodesMsg m;
    m.addcontents("sn", "SETGS", settings.encode());
    return sendHardwareMessage(m);
}

bool AbstractModuleClient::sendGlobalStimulationCommand(GlobalStimulationCommand command){
    TrodesMsg m;
    m.addcontents("sn", "SETGC", command.encode());
    return sendHardwareMessage(m);
}

int AbstractModuleClient::processCommandMsg(std::string cmdType, TrodesMsg &msg) {
    int rc = 0;
    if (cmdType == quit_CMD) {
        std::cout << "Client " << id << " got quit command.\n";
        recv_quit();
    }
    else if(cmdType == file_CMD){
        //Call user defined file commands
        std::string op = msg.popstr();
        if(streq(op.c_str(), file_OPEN)){
            std::string file = msg.popstr();
            state->setTrodes_filename(file);
            recv_file_open(file);
        }
        else if(streq(op.c_str(), file_CLOSE)){
            state->setTrodes_filename("");
            recv_file_close();
        }
    }
    else if(cmdType == source_CMD){
        int src;
        msg.popcontents("i", src);
        std::string src_string;
        switch (src) {
        case None:
            src_string = "None";
            break;
        case Fake:
            src_string = "Generator";
            break;
        case FakeSpikes:
            src_string = "Spikes Generator";
            break;
        case File:
            src_string = "File";
            break;
        case Ethernet:
            src_string = "Ethernet";
            break;
        case USBDAQ:
            src_string = "USB";
            break;
        case Rhythm:
            src_string = "Rhythm";
            break;
        default:
            break;
        }
        std::cerr << "got source " << src_string << "\n";
        recv_source(src_string);
    }
    else if(cmdType == acquisition_CMD){
        std::string type;
        uint32_t timestamp;
        msg.popcontents("s4",type,timestamp);
        recv_acquisition(type, timestamp);
    }
    else{
        std::cout << "[AMC]: Trodes Command [" << cmdType << "] not recognized.\n";
    }
    return rc;
}

int AbstractModuleClient::processEventMsg(const char *sender, const char *event, TrodesMsg &msg) {
    int rc = 0;
//    std::cout << "got event msg with " << msg.numContents() << "frames\n";
    recv_event(sender, event, msg);
    return rc;
}

int AbstractModuleClient::processRequestMsg(const char *, std::string reqType, TrodesMsg &) {
    int rc = 0;
    std::cout << "[AMC]: Request type [" << reqType << "] not recognized.\n";
    return rc;
}

int AbstractModuleClient::processReplyMsg(std::string repType, TrodesMsg &msg) {
    int rc = 0;
    if (repType == INFO_TIME) {
        uint32_t time;
        msg.popcontents("4", time);
        recv_time(time);
    }
    else if (repType == INFO_TIMERATE) {
        int timerate;
        msg.popcontents("i", timerate);
        recv_timerate(timerate);
    }
    else if (repType == INFO_CONFIG) {
        TrodesConfig newConfig;
        binarydata configdata;
        binarydata statedata;
        msg.popcontents("nn", configdata, statedata);
        newConfig.decode(configdata);
        if(newConfig.isValid())
            setTrodesConfig(newConfig);

        state->decode(statedata);
        subToTimestamps(state->getTimestamp_endpoint());
    }
    else {
        std::cout << "[AMC]: Reply type [" << repType << "] not recognized.\n";
    }
    return rc;
}

int AbstractModuleClient::processNotification(const char *, std::string noteType, TrodesMsg &msg) {
    int rc = 0;
//    std::cout << "AMC::processNotification: " << noteType << "\n";
    if (noteType == NOTE_CONFIG_CHANGED) {
        TrodesConfig newConfig;
        binarydata configdata;
        msg.popcontents("n", configdata);
        newConfig.decode(configdata);
        setTrodesConfig(newConfig);
    }
    else if(noteType == "TIMESTAMP_ADDRESS"){
        std::string addr = msg.popstr();
//        std::cout << "----address: |" << addr << "|\n";
        if(!subToTimestamps(addr)){
            std::cerr << error("Could not properly register timestamps socket!\n");
        }
    }
    else if(noteType == "TIMESTAMP_ADDRESS_DEREG"){
        //dereg timestamp
        if(!unsubToTimestamps()){
            std::cerr << error("Could not properly de-register timestamps socket!\n");
        }
    }
    else if(noteType == CLIENT_EXPIRED_MSG){
        auto const who = msg.popstr();
        state->remove_client(who);
    }
    else{
        std::cout << "[AMC]: Notification type [" << noteType << "] not recognized.\n";
    }
    return rc;
}

bool AbstractModuleClient::subToTimestamps(const std::string &address){
    if(address.empty()){
        return false;
    }
    HighFreqDataType hf(TIMESTAMPS_SOCK, TRODES_NETWORK_ID,"t", address, sizeof(lastTimestamp));
    if(timestampsub){
        if(zsock_is(timestampsub)){
            if(timestampaddress == address){
                return true;
            }
            else{
                zsock_destroy(&timestampsub);
                timestampaddress = "";
            }
        }
        else{
            timestampsub = NULL;
        }
    }
    timestampsub = zsock_new(ZMQ_SUB);
    zsock_set_conflate(timestampsub, 1);
    zsock_set_subscribe(timestampsub, "");
    if(zsock_connect(timestampsub, "%s", address.c_str())){
        zsock_destroy(&timestampsub);
        return false;
    }else{
        timestampaddress = address;
        state->setTimestamp_endpoint(address);
    }

    return true;
}

bool AbstractModuleClient::unsubToTimestamps(){
    state->setTimestamp_endpoint("");
    lastTimestamp = 0;
    if(timestampsub){
        zsock_destroy(&timestampsub);
        timestampaddress = "";
        return true;
    }
    return true;
}

uint32_t AbstractModuleClient::latestTrodesTimestamp(){
    if(!timestampsub){
        return 0;
    }
    //If zmq_recv succeeds, lastTimestamp is replaced. if not, the previous value is returned. win-win with no checks needed
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    if(zmq_msg_recv(&msg, zsock_resolve(timestampsub), ZMQ_DONTWAIT) == 12){
        lastTimestamp = *(uint32_t*)zmq_msg_data(&msg);
        lastsysTimestamp = *(int64_t*)((byte*)zmq_msg_data(&msg)+4);
    }
    zmq_msg_close(&msg);
    return lastTimestamp;
}

int AbstractModuleClient::processTimer(int timer_id){
    // zmq_msg_t msg;
    // zmq_msg_init_size(&msg, sizeof(int64_t)+sizeof(uint32_t));
    byte buf[12];
    int rc;
    for(int i = 0; i < 1000; ++i){
        // if(zmq_msg_recv(&msg, zsock_resolve(timestampsub), ZMQ_DONTWAIT) == 12){
        if((rc=zmq_recv(zsock_resolve(timestampsub), buf, 12, ZMQ_DONTWAIT))==12){
            // lastTimestamp = *(uint32_t*)zmq_msg_data(&msg);
            // lastsysTimestamp = *(int64_t*)((byte*)zmq_msg_data(&msg)+4);
            lastTimestamp = *(uint32_t*)buf;
            lastsysTimestamp = *(int64_t*)(buf+4);
            
        }
        else{
            // std::cerr << "zmq_msg_recv broke on iteration " << i << " with rc " << rc << std::endl;
            break;
        }
    }
    // zmq_msg_close(&msg);
    // std::cerr << "Purged all messages. Timestamp " << lastTimestamp << " " << lastsysTimestamp<< std::endl;
    return 0;
}