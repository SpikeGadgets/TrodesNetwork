#include <sstream>
#include <malamute.h>
#include "libTrodesNetwork/networkincludes.h"

//Internal #define's for use within this class:
#define TEMP_NETWORK_ID   "temp"
//User sends message
#define     streammsg       "STR"
#define     mailboxmsg      "MSG"
#define     requestmsg      "REQ"

//user requests
#define     getclients      "CLI"
#define     eventsrequest   "EVR"
#define     clientlist          "CLIENTLIST"
#define     streamlist          "STREAMLIST"
#define     eventslist          "EVENTSLIST"
#define     hfdtslist           "HFDTSLIST"

//Block until reply subject
#define     blockuntilrep       "BUR"
#define     blockrepsuccess     "BURs"
#define     blockrepfailure     "BURf"

//Network-wide notifications
#define     eventprovide    "EVP"
#define     eventunprovide  "EVU"
#define     hfdt_register   "hfdt_reg"      //High Frequency Data Type registered with module
#define     hfdt_deregister "hfdt_dereg"    //High Frequency Data Type deregistered with module

#define     gotrequest      "req"
#define     gotreply        "rep"

#define     validhfdt       "val"
#define     invalidhfdt     "!val"

//
#define     networkpimplid  "$npimp"
#define     addhsock        "addhsock"

//Events specific implementation
#define     eventprefix         "_evt."


//! Creates a zmsg using the picture provided (vargs version)
zmsg_t* czhelp_zmsg_createv(const char *picture, va_list argptr){
    zmsg_t *msg = zmsg_new ();
    while (*picture) {
//        std::cerr << picture << "\n";
        if (*picture == 'i')
            zmsg_addstrf (msg, "%d", va_arg (argptr, int));
        else if (*picture == '1')
            zmsg_addstrf (msg, "%" PRIu8, (uint8_t) va_arg (argptr, int));
        else if (*picture == '2')
            zmsg_addstrf (msg, "%" PRIu16, (uint16_t) va_arg (argptr, int));
        else if (*picture == '4')
            zmsg_addstrf (msg, "%" PRIu32, va_arg (argptr, uint32_t));
        else if (*picture == '8')
            zmsg_addstrf (msg, "%" PRIu64, va_arg (argptr, uint64_t));
        else if (*picture == 'u')    //  Deprecated, use 4 or 8 instead
            zmsg_addstrf (msg, "%ud", va_arg (argptr, uint));
        else if (*picture == 'd')
            zmsg_addstr (msg, std::to_string(va_arg (argptr, double)).c_str());
        else if (*picture == 's') {
            char *str = va_arg(argptr, char *);
            zmsg_addstr (msg, str);
        }
        else if (*picture == 'b') {
            //  Note function arguments may be expanded in reverse order,
            //  so we cannot use va_arg macro twice in a single call
            byte *data = va_arg (argptr, byte *);
            zmsg_addmem (msg, data, va_arg (argptr, int));
        }
        else if (*picture == 'c') {
            zchunk_t *chunk = va_arg (argptr, zchunk_t *);
            assert (zchunk_is (chunk));
            zmsg_addmem (msg, zchunk_data (chunk), zchunk_size (chunk));
        }
        else if (*picture == 'f') {
            zframe_t *frame = va_arg (argptr, zframe_t *);
            assert (zframe_is (frame));
            zmsg_addmem (msg, zframe_data (frame), zframe_size (frame));
        }
        else if (*picture == 'U') {
            zuuid_t *uuid = va_arg (argptr, zuuid_t *);
            zmsg_addmem (msg, zuuid_data (uuid), zuuid_size (uuid));
        }
        else if (*picture == 'p') {
            void *pointer = va_arg (argptr, void *);
            zmsg_addmem (msg, &pointer, sizeof (void *));
        }
        else if (*picture == 'h') {
            zhashx_t *hash = va_arg (argptr, zhashx_t *);
            zframe_t *frame = zhashx_pack (hash);
            zmsg_append (msg, &frame);
        }
        else if (*picture == 'm') {
            zframe_t *frame;
            zmsg_t *zmsg = va_arg (argptr, zmsg_t *);
            for (frame = zmsg_first (zmsg); frame;
                    frame = zmsg_next (zmsg) ) {
                zframe_t *frame_dup = zframe_dup (frame);
                zmsg_append (msg, &frame_dup);
            }
        }
        else if (*picture == 'n') {
            NetworkDataType *val = va_arg(argptr, NetworkDataType *);
            binarydata dat = val->encode();
            CZHelp::zmsg_addNDT(msg, dat);
        }
        else if (*picture == 'z')
            zmsg_addmem (msg, NULL, 0);
        else {
            zsys_error ("zsock: invalid picture element '%c'", *picture);
            assert (false);
        }
        picture++;
    }
    return msg;
}
//! Creates a zmsg using the picture provided
zmsg_t* czhelp_zmsg_create(const char *picture, ...){
    va_list argptr;
    va_start(argptr, picture);
    zmsg_t *msg = czhelp_zmsg_createv(picture, argptr);
    va_end(argptr);
    return msg;
}

//based heavily off of 'zsock_recv()' in zsock.c
int czhelp_zmsg_readv(zmsg_t* self, const char* picture, va_list argptr) {
    int retval = -1;
    if (self == NULL || picture == NULL)
        return(retval);
    retval = 0;
    while(*picture) {
        if (*picture == 'i') { //int
            char *str = zmsg_popstr(self);
            int *val = va_arg(argptr, int *); //retrieve int from the input list of argumetns
            if (val)
                *val = str? atoi (str): 0;
            freen(str);

        }
        else if (*picture == '1') { //uint8_t
            char *str = zmsg_popstr (self);
            uint8_t *val = va_arg (argptr, uint8_t *);
            if (val)
                *val = str? (uint8_t) atoi (str): 0;
            freen(str);
        }
        else if (*picture == '2') { //uint16_t
            char *str = zmsg_popstr (self);
            uint16_t *val = va_arg (argptr, uint16_t *);
            if (val)
                *val = str? (uint16_t) atol (str): 0;
            freen(str);
        }
        else if (*picture == '4') { //uint32_t
            char *str = zmsg_popstr (self);
            uint32_t *val = va_arg (argptr, uint32_t *);
            if (val)
                *val = str? (uint32_t) strtoul(str, NULL, 10): 0;
            freen(str);
        }
        else if (*picture == '8') { //uint64_t
            char *str = zmsg_popstr (self);
            uint64_t *val = va_arg (argptr, uint64_t *);
            if (val)
                *val = str? (uint64_t) atoi (str): 0;
            freen(str);
        }
        else if (*picture == 'd') {
            char *str = zmsg_popstr(self);
            double *val = va_arg(argptr, double *);
            if (val)
                *val = str ? (double) strtod(str, NULL) : 0;
            freen(str);
        }
        else if (*picture == 's') { //string
            char *str = zmsg_popstr(self);
            char **val = va_arg(argptr, char **);
            if (val) {
                *val = str;
            }
        }
        else if (*picture == 'b') { //byte *, size_t (2 args)
            zframe_t *curFrame = zmsg_pop (self);
            byte **data_p = va_arg (argptr, byte **);
            size_t *size = va_arg (argptr, size_t *);
            if (data_p) {
                if (curFrame) {
                    *size = zframe_size (curFrame);
                    *data_p = (byte *) malloc (*size);
                    memcpy (*data_p, zframe_data (curFrame), *size);
                }
                else {
                    *data_p = NULL;
                    *size = 0;
                }
            }
            zframe_destroy (&curFrame);
        }
        else if (*picture == 'c') { //zchunk_t *
            zframe_t *curFrame = zmsg_pop (self);
            zchunk_t **val = va_arg (argptr, zchunk_t **);
            if (val) {
                if (curFrame)
                    *val = zchunk_new (zframe_data (curFrame), zframe_size (curFrame));
                else
                    *val = NULL;
            }
            zframe_destroy (&curFrame);
        }
        else if (*picture == 'f') { //zframe_t *
            zframe_t *curFrame = zmsg_pop (self);
            zframe_t **val = va_arg (argptr, zframe_t **);
            if (val)
                *val = curFrame;
            else
                zframe_destroy (&curFrame);
        }
        else if (*picture == 'U') { //zuuid_t *
            zframe_t *curFrame = zmsg_pop (self);
            zuuid_t **val = va_arg (argptr, zuuid_t **);
            if (val) {
                if (curFrame) {
                    *val = zuuid_new ();
                    zuuid_set (*val, zframe_data (curFrame));
                }
                else
                    *val = NULL;
            }
            zframe_destroy (&curFrame);
        }
        else if (*picture == 'p') { //void *
            zframe_t *curFrame = zmsg_pop (self);
            void **val = va_arg (argptr, void **);
            if (val) {
                if (curFrame) {
                    if (zframe_size (curFrame) == sizeof (void *))
                        *val = *((void **) zframe_data (curFrame));
                    else
                        retval = -1;
                }
                else
                    *val = NULL;
            }
            zframe_destroy (&curFrame);
        }
        else if (*picture == 'h') { //zhashx_t *
            zframe_t *curFrame = zmsg_pop (self);
            zhashx_t **val = va_arg (argptr, zhashx_t **);
            if (val) {
                if (curFrame)
                    *val = zhashx_unpack (curFrame);
                else
                    *val = NULL;
            }
            zframe_destroy (&curFrame);
        }
        else if (*picture == 'n') { //NetworkDataType
            NetworkDataType *val = va_arg(argptr, NetworkDataType *);
            binarydata dat;
            CZHelp::zmsg_popNDT(self, dat);
            val->decode(dat);
        }
        else if (*picture == 'z') { //sends zero-frame (0 args)
            zframe_t *curFrame = zmsg_pop (self);
            if (curFrame && zframe_size (curFrame) != 0)
                retval = -1;
            zframe_destroy (&curFrame);
        }
        else {
            std::cout << "ERROR: formated data type not supported. (CZHelp::zmsg_read)\n";
        }
        picture++;
    }
    return(retval);
}
//reads a zmsg in the same manner as zsock_recv()
int czhelp_zmsg_read(zmsg_t* self, const char* picture, ...) {
    va_list argptr;
    va_start(argptr, picture);
    int retval = czhelp_zmsg_readv(self, picture, argptr);
    va_end(argptr);
    return(retval);
}

//************************************************************
//Malamute wrap
//************************************************************

MlmWrap::MlmWrap(const char* _id, const char *addr, int ePort)
    : state(new network_pimpl),
      client(NULL),
      reactor(NULL),
      actor(NULL),
      usrpipe(NULL),
      mlmpipe(NULL),
      hardwaresock(NULL)
{
    state->setAddress(addr);
    state->setPort(ePort);
    std::string ep;
    ep.append(addr);
    ep.append(":");
    ep.append(std::to_string(ePort));
    create(_id, ep.c_str());
}

MlmWrap::MlmWrap(const char *_id, const char *e)
{
    create(_id, e);
    std::string endpt = state->getEndpoint();
    state->setAddress(endpt.substr(endpt.find(':')));
    state->setPort(std::stoi(endpt.substr(endpt.find(':')+1)));
}

void MlmWrap::create(const char* _id, const char* e) {
    id = _id;
    client = mlm_client_new();
    actor = NULL;
    state->setEndpoint(e);
    initialized = false;
    connected = false;

    //Verifies broker exists, and finds a valid id to use while temp connected
    //Creates temp once, and passes it to checkBrokerConnected and verifyID, each needs to use temp
    //TODO: does broker and trodes need to go through this step?
    mlm_client_t *temp = mlm_client_new();
    if (!checkBrokerConnected(state->getEndpoint().c_str(), 3, temp)) {
        mlm_client_destroy(&client);
        client = NULL;
        return;
    }
    verifyID(temp);
    mlm_client_destroy(&temp);

    //Creates client with id
    int retval = mlm_client_connect(client, state->getEndpoint().c_str(), 1000, id.c_str());
    state->insert_client(id);
    if (retval != 0) { // if the client failed to be created
        mlm_client_destroy(&client);
        client = NULL;
        state->remove_client(id);
        std::cerr << error(std::string("Could not create client at the specified address [") + state->getEndpoint()) << std::endl;
        return;
    }
}

MlmWrap::~MlmWrap(){
    this->close();
    // this->removeAllSubs();
}

void MlmWrap::close(){
    if(actor){
        zactor_t *a = actor;
        zactor_destroy(&actor);
        zsock_wait(a);
    }
    else
        mlm_client_destroy(&client);
    //Destroying actor causes the rest of cleanup to happen in message_reactor_task
}

//Return 0 if successful, 1 if already initialized, and -1 if unsuccessful
int MlmWrap::initialize(){
    if (!connected){ //if the broker could not be connected to, try again once
        create(id.c_str(), state->getEndpoint().c_str());
        if(!connected){
            std::cerr << error("Client could not be initialized!") << std::endl;
            return -1;
        }
    }

    if(initialized){
        return 0;
    }
    actor = zactor_new(&MlmWrap::message_reactor_task, this);
    if(actor && client){ //both actor and client must be successfully created
        initialized = true;
        if (id != TRODES_NETWORK_ID && id != BROKER_NETWORK_ID) {
            //Request from Trodes the current state object (events, hfdts, and others)
            TrodesMsg configReq("s", INFO_CONFIG);
            sendMessage(TRODES_NETWORK_ID, TRODES_INFO_REQ, configReq);
            blockforreply(INFO_CONFIG, 500);
        }
        return 0;
    }
    else if (actor != NULL) {
        zactor_destroy(&actor);
        std::cerr << error("Client could not be initialized!") << std::endl;
        return -2;
    }
    else{
        std::cerr << error("Client could not be initialized!") << std::endl;
        return -1;
    }
}

void MlmWrap::getConfigInfo() {
    TrodesMsg req("s", INFO_CONFIG);
    sendMessage(TRODES_NETWORK_ID, TRODES_INFO_REQ, req); //ask Trodes for configuration information
}

bool MlmWrap::checkBrokerConnected(const char *endpoint, int tries, mlm_client_t* temp, int timeout) {
    bool retval = false;
    int rc = -1;

    if(mlm_client_connected(temp)) //Passed in temp is already connected and verified
        return true;
    while((rc = mlm_client_connect(temp, endpoint, 1000, TEMP_NETWORK_ID)) < 0 && --tries){
        zclock_sleep(100);
    }
    if (rc == 0) {
        retval = true;
    }
    else {
        retval = false;
    }
    connected = retval;
    return(retval);
}

void MlmWrap::verifyID(mlm_client_t* temp) {
    std::string _id = id;
    //broker and trodes should not be checking ...
    //TODO: maybe trodes should be checking to determine if it's the only trodes on the network...
    if (streq(_id.c_str(),BROKER_NETWORK_ID) || streq(_id.c_str(), TRODES_NETWORK_ID))
        return;

    auto cList = getCurConnectedClients(temp);
    for(const auto& i : cList){
        state->insert_client(i);
    }
    int counter = 1;
    //TODO: write in functionality to check for available names.  i.e. you can detect the number
    //of current active instances of a particular module, but what if Mod.1 and Mod.3 are active?
    //Then the fucntion will try to name the new module as Mod.3... fix it
    while(cList.size() > 0) {
        std::string curStr = cList.back();
        cList.pop_back(); //remove curString from the list
        if (curStr.find(_id) != std::string::npos)
            counter++;
    }
    if (counter > 1) {
//        id = _id;
        _id.append(".");
        _id.append(std::to_string(counter));
        id = _id;
        std::cout << "[MlmWrap] Name already taken. Renaming to " << id << "\n";
    }
}

std::vector<std::string> MlmWrap::getCurConnectedClients(mlm_client_t *temp) {

    std::vector<std::string> cList; //client list to return
    int rc = 0;

    //put this next part in while loop w/ sleep to constantly try to check until it does
    if(!mlm_client_connected(temp)){
        int tries = 5;
        while((rc = mlm_client_connect(temp, state->getEndpoint().c_str(), 1000, TEMP_NETWORK_ID)) < 0 && --tries){
            zclock_sleep(100);
        }
    }

    if (rc == 0) {
        zmsg_t* msg = zmsg_new();
        rc = mlm_client_sendtox(temp, BROKER_NETWORK_ID, clientlist, "s", NULL);
        zmsg_destroy(&msg);

        msg = mlm_client_recv(temp);
        char *listItem = zmsg_popstr(msg); //pull first string off the msg, it's 'm'
        freen(listItem);
        listItem = zmsg_popstr(msg); //pull second string off the msg, it's 'CLIENTLIST'
        freen(listItem);
        listItem = zmsg_popstr(msg);
        while (listItem != NULL) {
            std::string curItem = listItem;
            if(!(streq(listItem, BROKER_NETWORK_ID) || streq(listItem, TEMP_NETWORK_ID))){
                cList.push_back(curItem);
            }
            freen(listItem);
            listItem = zmsg_popstr(msg);
        }
        zmsg_destroy(&msg);
    }
    else{

    }
    return(cList);
}

bool MlmWrap::blockforreply(std::string tracker, uint32_t timeoutms){
    std::lock_guard<std::mutex> lock(actormutex);
    zsock_send(actor, "ss4", blockuntilrep, tracker.c_str(), timeoutms);
    zmsg_t *msg = zactor_recv(actor);
    char *status = zmsg_popstr(msg);
    bool success = false;
    if(streq(status, blockrepsuccess)){
        success = true;
    }
    else if(streq(status, blockrepfailure)){
        success = false;
    }
    else{
        std::cerr << error("Error with blockforreply() when waiting for ") << tracker << "\n";
    }
    return success;
}

bool MlmWrap::isInitialized() const{
    return initialized;
}

std::string MlmWrap::getID() const{
    return id;
}

TrodesConfig MlmWrap::getTrodesConfig(){
    return config;
}

bool MlmWrap::isTrodesConfigValid() const{
     return config.isValid();
}

void MlmWrap::setTrodesConfig(TrodesConfig _config){
    config.clear(); config = _config;
}

bool MlmWrap::isConnectedToBroker() const{
    return(connected);
}
std::vector<std::string> MlmWrap::getClients(){
    zmsg_t *msg = czhelp_zmsg_create("s", getclients, NULL);
    {
        std::lock_guard<std::mutex> lock(actormutex);
        zactor_send(actor, &msg);
        msg = zactor_recv(actor);
    }
    std::vector<std::string> clients;
    char *cl = zmsg_popstr(msg);
    while(cl){
        clients.push_back(cl);
        freen(cl);
        cl = zmsg_popstr(msg);
    }
    zmsg_destroy(&msg);
    return clients;
}

std::string MlmWrap::getEndpoint() const{
    return state->getEndpoint();
}

std::string MlmWrap::getAddress() const{
    return state->getAddress();
}

int MlmWrap::getPort() const{
    return state->getPort();
}

std::string MlmWrap::error(const std::string s){
    return std::string("[MlmWrap:") + id + "] " + s;
}

int MlmWrap::setProducer(const char *stream){
    int rc = mlm_client_set_producer(client, stream);
    if(rc){
        std::cerr << error(std::string("Could not set producer for ") + stream) << std::endl;
    }
    return rc;
}

int MlmWrap::provideService(const char *regExpr){
    int rc = mlm_client_set_worker(client, id.c_str(), regExpr);
    if(rc){
        std::cerr << error(std::string("Could not set worker for ") + regExpr) << std::endl;
    }
    return rc;
}

HFSubConsumer* MlmWrap::createConsumerSub(HighFreqDataType dt, size_t messageBufferLength){
//    std::cout << "creating subscriber \n";
    HFSubConsumer *newSub = new HFSubConsumer(dt, messageBufferLength);
    addSubToList(newSub);

    HFSubSockSettings sockInfo(dt, newSub->getSubSockType(), NULL, NULL);
    addHfTypeToSubbedList(sockInfo);
    //todo: add a check to see if newSub was created properly, if not, don't add the type to the subbedList

    return newSub;
}

HFSubWorker* MlmWrap::createWorkerSub(HighFreqDataType dt, int numThreads, hfs_data_callback_fn userFoo, void *args) {
    HFSubWorker *newSub = new HFSubWorker(dt, numThreads, userFoo, args);
    addSubToList(newSub);
    HFSubSockSettings sockInfo(dt, newSub->getSubSockType(), userFoo, args);
    addHfTypeToSubbedList(sockInfo);
    //todo: add a check to see if newSub was created properly, if not, don't add the type to the subbedList

    return(newSub);
}

HFFilteredConsumer* MlmWrap::createFilteredConsumer(HighFreqDataType dt, size_t buffersize, hfs_data_filter_fn* filter, HFParsingInfo parser){
    HFFilteredConsumer *newsub = new HFFilteredConsumer(dt, buffersize, filter, parser);
    addSubToList(newsub);
    HFSubSockSettings sockInfo(dt, newsub->getSubSockType(), NULL, NULL);
    addHfTypeToSubbedList(sockInfo);
    return newsub;
}


HFParsingInfo MlmWrap::createNeuralParsingInfo(std::vector<std::string> channels, std::string dataformat){
    HFParsingInfo info;
    for(auto &channel : channels){
        //Should be "ntrode id, nth channel"
        // size_t i;
        // int nt = -1, ch = -1;
        // nt = std::stoi(channel, &i);
        // ch = std::stoi(channel.substr(i+1));
        auto found = channel.find(",");
        if(found == std::string::npos){
            continue;
        }
        
        std::string ntstr, chstr;
        ntstr = CZHelp::trim_copy(channel.substr(0, channel.find(",")));
        chstr = CZHelp::trim_copy(channel.substr(channel.find(",")+1));

        NTrodeObj nt = config.getNTrodeByID(ntstr);
        if(!nt.isValid()){
            std::cerr << error("Could not find hw_chan at ntrode ") << ntstr << " " << chstr << "th channel.\n";
            return HFParsingInfo();
        }
        if(chstr == "*"){
            for(unsigned int i = 0; i < nt.getHw_chans().size(); ++i){
                info.indices.push_back(nt.getHWChan(i));
                info.dataRequested.push_back(ntstr+", "+std::to_string(i));
            }
        }
        else if(CZHelp::is_integer(chstr)){
            int ch = std::stoi(chstr);
            if(ch < 0 || ch >= (int)nt.getHw_chans().size()){
                std::cerr << error("Invalid hw chan ") << ch << "\n";
                return HFParsingInfo();
            }
            info.indices.push_back(nt.getHWChan(std::stoi(chstr)));
            info.dataRequested.push_back(ntstr + ", " + chstr);
        }
        else{
            std::cerr << error("Problem with hw channel string ") << chstr << "\n";
            return HFParsingInfo();
        }

        // try{ //Get hwchan number and add to list
        //     info.indices.push_back(config.getNTrode(nt-1).getHWChan(ch-1));
        // }
        // catch(const std::out_of_range& oor){
        //     std::cerr << error("Could not find hw_chan at ntrode ") << nt << " " << ch << "th channel.\n";
        //     return HFParsingInfo();
        // }
    }
    info.sizeOf = info.indices.size()*sizeof(int16_t);
    return info;
}

HFParsingInfo MlmWrap::createLFPParsingInfo(std::vector<std::string> ntrodes, std::string dataformat){
    HFParsingInfo info;
    for(auto &ntstr : ntrodes){
        int nid = -1;
        try{
            nid = std::stoi(ntstr);
        }catch(...){
            std::cerr << error("Could not parse ") << ntstr  << " into ntrode id"<< "\n";
            return HFParsingInfo();
        }
        //Ntrode id is larger than actual list of ntrodes
        if(nid < 1 || nid > (int)config.getNTrodes().size()){
            std::cerr << error("Could not find NTrode in config") << nid << "\n";
            return HFParsingInfo();
        }
        info.indices.push_back(nid-1 + 2); //User provides ntrode id, push back ntrode index (nid - 1) + 2(timestamp)
    }
    info.sizeOf = info.indices.size()*sizeof(int16_t);
    info.dataRequested = ntrodes;
    info.dataLength = ntrodes.size();
    return info;

}

//Channels = [device, channelid, ... ]
//dataformat = "t,start,length, start,length,..."
//return: indices = [il_id_byte_loc, byte, bit, ... ]
HFParsingInfo MlmWrap::createAnalogParsingInfo(std::vector<std::string> channels, std::string dataformat){
    std::vector<int> startlengths;
    if(!verifyTrodesDataFormat(startlengths, dataformat)){
        std::cerr << error("Bad data format from Trodes, aborting\n");
        return HFParsingInfo();
    }

    HFParsingInfo info;
    info.indices.push_back(-1);
    for(auto &channelstr : channels){
        std::string dv = CZHelp::trim_copy(channelstr.substr(0, channelstr.find(",")));
        std::string ch = CZHelp::trim_copy(channelstr.substr(channelstr.find(",")+1));

        //find channel
        NDevice device = findTrodesDevice(dv);
        NDeviceChannel channel = findTrodesChannel(device, ch);
        if(!channel.isValid()){
            std::cerr << error("Could not find ") << dv << ", " << ch << ", aborting\n";
            return HFParsingInfo();
        }

        //Checking for being analog
        if(channel.getType() != 1){
            std::cerr << error(ch) << " is not an analog channel. Stopping and returning empty parsing info\n";
            return HFParsingInfo();
        }

        //Calculate indices and append to info.indices
        int gap = calculateDeviceChannelInfoIndices(info.indices, startlengths, channel.getStartByte(),
                                                    (channel.getInterleavedDataByte()==-1) ? -1 : channel.getInterleavedDataBit());
        if(gap == -1){
            std::cerr << error("Could not calculate proper indices for ") << device.getName() << ", " << channel.getId() << "\n";
            return HFParsingInfo();
        }
        //If this is an interleaved channel
        if(channel.getInterleavedDataBit() != -1){
            info.indices[0] = device.getByteOffset()-gap;
        }
    }
    info.sizeOf = (info.indices.size()/2)*sizeof(int16_t);
    info.dataRequested = channels;
    info.dataLength = channels.size();
    return info;
}

//Channels = [device, channelid, ... ]
//dataformat = "t,start,length,start,length,..."
HFParsingInfo MlmWrap::createDigitalParsingInfo(std::vector<std::string> channels, std::string dataformat){
    std::vector<int> startlengths;
    if(!verifyTrodesDataFormat(startlengths, dataformat)){
        std::cerr << error("Bad data format from Trodes, aborting\n");
        return HFParsingInfo();
    }

    HFParsingInfo info;
    //Calculate size of digital packet, first item in info.indices
    info.indices.push_back(0);
    for(size_t i = 0; i < startlengths.size(); i+=2){
        info.indices.back() += startlengths[i+1];
    }

    for(auto &channelstr : channels){
        std::string dv = CZHelp::trim_copy(channelstr.substr(0, channelstr.find(",")));
        std::string ch = CZHelp::trim_copy(channelstr.substr(channelstr.find(",")+1));

        //find channel
        NDeviceChannel channel = findTrodesChannel(findTrodesDevice(dv), ch);
        if(!channel.isValid()){
            std::cerr << error("Could not find ") << dv << ", " << ch << ", aborting\n";
            return HFParsingInfo();
        }

        //Checking for being digital
        if(channel.getType() != 0){
            std::cerr << error(ch) << " is not an digital channel. Stopping and returning empty parsing info\n";
            return HFParsingInfo();
        }

        //Push back digital indices, (startbyte, startbit)
        calculateDeviceChannelInfoIndices(info.indices, startlengths, channel.getStartByte(), channel.getStartBit());
    }
    info.sizeOf = (info.indices.size()/2)*sizeof(int16_t);
    info.dataRequested = channels;
    info.dataLength = channels.size();
    return info;
}

NDevice MlmWrap::findTrodesDevice(std::string dv){
    //Finding device
    auto devicesvec = config.getDevices();
    auto deviceit = std::find_if(devicesvec.begin(), devicesvec.end(),
                               [dv](NDevice dev){return dev.getName() == dv;});
    if(deviceit == devicesvec.end()){
        return NDevice();
    }
    return *deviceit;
}

NDeviceChannel MlmWrap::findTrodesChannel(const NDevice& device, std::string ch){
    //Finding channel
//    NDevice* device = *deviceit;
    auto channelsvec = device.getChannels();
    auto channelit = std::find_if(channelsvec.begin(), channelsvec.end(),
                                  [ch](NDeviceChannel cha){return cha.getId() == ch;});
    if(channelit == channelsvec.end()){
        return NDeviceChannel();
    }
    return *channelit;
}
bool MlmWrap::verifyTrodesDataFormat(std::vector<int> &startlengths, std::string dataformat){
    //verify and parse data format
    if(dataformat[0] != 't' || dataformat[1] != ','){
        return false;
    }
    dataformat = dataformat.substr(2);
    std::stringstream ss( dataformat);
    while( ss.good() ){
        std::string substr;
        getline( ss, substr, ',' );
        startlengths.push_back( std::atoi(substr.c_str()) );
    }
    if(startlengths.size() % 2 != 0){
        return false;
    }
    return true;
}

int MlmWrap::calculateDeviceChannelInfoIndices(std::vector<int> &indices, std::vector<int>& startlengths, int startbyte, int bit){
    //push back index, adjusted for analog's dataformat
    //returns the gap, which when subtracted from startbyte (start byte in trodes packet), returns the startbyte of the actual analog packet
    int prev = 0;
    int gap = 0;
    for(size_t i = 0; i < startlengths.size(); i+=2){
        gap += startlengths[i] - prev;
        prev = startlengths[i] + startlengths[i+1];
        if(startbyte < startlengths[i] + startlengths[i+1]){
            //Channel in this block. subtract total gap
            indices.push_back(startbyte - gap);
            indices.push_back(bit);
            return gap;
        }
    }
    return -1;
}

HFParsingInfo MlmWrap::createSpikesParsingInfo(std::vector<std::string> clusters, std::string dataformat){
    HFParsingInfo info;
    for(auto const& cluster: clusters){
        //Should be "ntrode id, nth channel"
        size_t it;
        int nt = -1, cl = -1;
        //Get ntrode and cluster number
        try{
            nt = std::stoi(cluster, &it);
            cl = std::stoi(cluster.substr(it+1));
        }catch(...){
            std::cerr << error("Could not parse ") << cluster << "\n";
            return HFParsingInfo();
        }

        info.indices.push_back(nt);
        info.indices.push_back(cl);
    }
    info.sizeOf = info.indices.size()*sizeof(int16_t);
    info.dataRequested = clusters;
    info.dataLength = clusters.size();
    return info;
}
int MlmWrap::subscribeStream(const char *stream, const char *pattern){
    int rc = mlm_client_set_consumer(client, stream, pattern);
    if(rc){
        std::cerr << error(std::string("Could not subscribe to stream ") + stream + " pattern " + pattern) << std::endl;
    }
    return rc;
}
int MlmWrap::removeSubscriptions(const char *stream){
    int rc = mlm_client_remove_consumer(client, stream);
    if(rc){
        std::cerr << error(std::string("Could not remove subscriptions to ") + stream) << std::endl;
    }
    return rc;
}

int MlmWrap::subscribeEvent(const std::string &origin, const std::string &event){
    std::string evt = eventprefix + event;
    return subscribeStream(origin.c_str(), evt.c_str());
}

//IMPORTANT: Events stored do not have event prefix attached.
int MlmWrap::provideEvent(const char *event){
    int rc = -1;
    zmsg_t *msg = czhelp_zmsg_create("ss", eventprovide, event);
    std::lock_guard<std::mutex> lock(actormutex);
    rc = zactor_send(actor, &msg);
    if(rc){
        std::cerr << error("Could not send 'provide event' to actor") << std::endl;;
    }
    return rc;
}

int MlmWrap::unprovideEvent(const char *event){
    int rc = -1;
    zmsg_t *msg = czhelp_zmsg_create("ss", eventunprovide, event);
    std::lock_guard<std::mutex> lock(actormutex);
    rc = zactor_send(actor, &msg);
    if(rc){
        std::cerr << error("Could not send 'unprovide event' to actor") << std::endl;
    }
    return rc;
}

//IMPORTANT: Events listed do not have event prefix attached.
std::vector<EventDataType> MlmWrap::getEventList(){
    zmsg_t *msg = czhelp_zmsg_create("s", eventslist, NULL);
    {
        std::lock_guard<std::mutex> lock(actormutex);
        zactor_send(actor, &msg);
        msg = zactor_recv(actor);
    }
    char* first = zmsg_popstr(msg);
    if(!streq(first, eventslist)){
        std::cerr << error("Error retrieving event list\n");
    }
    std::vector<EventDataType> events;
    char *n = zmsg_popstr(msg);
    char *o = zmsg_popstr(msg);
    while(n && o){
        EventDataType ev(n, o);
        events.push_back(ev);
        freen(n); freen(o);
        n = zmsg_popstr(msg);
        o = zmsg_popstr(msg);
    }
    zmsg_destroy(&msg);
    return events;
}


int MlmWrap::registerHighFreqData(HighFreqDataType dataType) {
    dataType.setOrigin(id);
    if (!dataType.isValid()) {
        std::cerr << error("Data type is invalid, aborting.\n");
        return -1;
    }
    int rc = -1;
    {
        std::lock_guard<std::mutex> lock(actormutex);
        rc = zsock_send(actor, "sssssi", hfdt_register, dataType.getName().c_str(), dataType.getOrigin().c_str(),
                        dataType.getDataFormat().c_str(), dataType.getSockAddr().c_str(), dataType.getByteSize());
    }
    if(rc){
        std::cerr << error("Could not send 'register high frequency data type' to actor") << std::endl;
    }
    return rc;
}

int MlmWrap::deregisterHighFreqData(HighFreqDataType dataType) {
    dataType.setOrigin(id);
    if(!dataType.isValid()){
        std::cerr << error("Data type is invalid, aborting\n");
        return -1;
    }
    int rc = -1;
    {
        std::lock_guard<std::mutex> lock(actormutex);
        rc = zsock_send(actor, "sssssi", hfdt_deregister, dataType.getName().c_str(), dataType.getOrigin().c_str(),
                        dataType.getDataFormat().c_str(), dataType.getSockAddr().c_str(), dataType.getByteSize());
    }
    if(rc){
        std::cerr << error("Could not send 'deregister high frequency data type' to actor") << std::endl;
    }
    return rc;
}

std::vector<HighFreqDataType> MlmWrap::getHighFreqList(){
    zmsg_t *msg = czhelp_zmsg_create("s", hfdtslist, NULL);
    {
        std::lock_guard<std::mutex> lock(actormutex);
        zactor_send(actor, &msg);
        msg = zactor_recv(actor);
    }
    char* first = zmsg_popstr(msg);
    if(!streq(first, hfdtslist)){
        std::cerr << error("Error retrieving high freq list\n");
    }
    std::vector<HighFreqDataType> hfdts;
    zframe_t *f = zmsg_pop(msg);
    while(f){
        HighFreqDataType hf;
        hf.decode(binarydata((char*)zframe_data(f), zframe_size(f)));
        hfdts.push_back(hf);
        zframe_destroy(&f);
        f = zmsg_pop(msg);
    }
    zmsg_destroy(&msg);
    return hfdts;
}



int MlmWrap::sendStream(const char *subject, TrodesMsg &tmsg){
    int rc = -1;
    zmsg_t *zmsg = czhelp_zmsg_create("sssm", streammsg, subject, tmsg.getformat().c_str(), tmsg.getzmsg());
    std::lock_guard<std::mutex> lock(actormutex);
    rc = zactor_send(actor, &zmsg);
    return rc;
}

int MlmWrap::sendStream(const char* subject, const char* picture, ...) {
    va_list argptr;
    va_start(argptr, picture);
    int rc = sendStream(subject, picture, argptr);
    va_end(argptr);
    return rc;
}

int MlmWrap::sendStream(const char* subject, const char* picture, va_list argptr) {
    int rc = -1;
//    std::cerr << error("sending out stream ") << subject << "\n";
    zmsg_t *body = czhelp_zmsg_create(picture, argptr);
    zmsg_t *msg = czhelp_zmsg_create("sssm", streammsg, subject, picture, body);
    //actual message structure - sssm -- note that the first three strings constitute the header, and the msg the body
    std::lock_guard<std::mutex> lock(actormutex);
    rc = zactor_send(actor, &msg);
    return rc;
}

int MlmWrap::sendMessage(const char *address, const char *subject, TrodesMsg &tmsg, const char *tracker, uint32_t timeout){
    int rc = -1;
    zmsg_t *zmsg = czhelp_zmsg_create("ssss4sm", mailboxmsg, address, subject, tracker,
                                       timeout, tmsg.getformat().c_str(), tmsg.getzmsg());
    std::lock_guard<std::mutex> lock(actormutex);
    rc = zactor_send(actor, &zmsg);
    return rc;
}

int MlmWrap::sendMessageToAll(const char *subject, TrodesMsg &tmsg, const char *tracker, uint32_t timeout) {
//    std::set<std::string>::iterator iter = clients.begin();
    auto clients = state->getClients();
    for (auto iter = clients.begin(); iter != clients.end(); ++iter) {
        if (*iter == id)
            continue; //dont send a message to yourself

//        std::cerr << "Sending message to " << iter->c_str() << std::endl;
        TrodesMsg m = tmsg.copy();
        sendMessage(iter->c_str(), subject, m, tracker, timeout);
    }
    return 0;
}

int MlmWrap::sendMessage(const char *address, const char *subject,
        const char *tracker, uint32_t timeout, const char* picture, ...) {
    va_list argptr;
    va_start(argptr, picture);
    int rc = sendMessage(address, subject, tracker, timeout, picture, argptr);
    va_end(argptr);
    return rc;
}

int MlmWrap::sendMessage(const char *address, const char *subject,
        const char *tracker, uint32_t timeout, const char* picture, va_list argptr) {
    int rc = -1;
    zmsg_t *body = czhelp_zmsg_create(picture, argptr);
    std::cerr << "body created\n";
    zmsg_t *msg = czhelp_zmsg_create("ssss4sm", mailboxmsg, address, subject, tracker, timeout, picture, body);
    std::cerr << "msg created\n";
    //actual message structure - ssss4sm
    std::lock_guard<std::mutex> lock(actormutex);
    rc = zactor_send(actor, &msg);
    return rc;
}

int MlmWrap::sendRequest(const char *address, const char *subject, TrodesMsg &tmsg, const char *tracker, uint32_t timeout){
    int rc = -1;
    zmsg_t *zmsg = czhelp_zmsg_create("ssss4sm", requestmsg, address, subject, tracker,
                                       timeout, tmsg.getformat().c_str(), tmsg.getzmsg());
    std::lock_guard<std::mutex> lock(actormutex);
    rc = zactor_send(actor, &zmsg);
    return rc;
}

int MlmWrap::sendRequest(const char *address, const char *subject,
        const char *tracker, uint32_t timeout, const char* picture, ...) {
    va_list argptr;
    va_start(argptr, picture);
    int rc = sendRequest(address, subject, tracker, timeout, picture, argptr);
    va_end(argptr);
    return rc;
}

int MlmWrap::sendRequest(const char *address, const char *subject,
        const char *tracker, uint32_t timeout, const char* picture, va_list argptr) {
    int rc = -1;
    zmsg_t *body = czhelp_zmsg_create(picture, argptr);
    zmsg_t *msg = czhelp_zmsg_create("ssss4sm", requestmsg, address, subject, tracker, timeout, picture, body);
    //actual message structure - ssss4sm
    std::lock_guard<std::mutex> lock(actormutex);
    rc = zactor_send(actor, &msg);
    return rc;
}

int MlmWrap::sendEvent(const std::string &event, TrodesMsg &tmsg){
    std::string evt = std::string(eventprefix) + event;
    return sendStream(evt.c_str(), tmsg);
}

bool MlmWrap::isHfTypeCurrentlySubbed(std::string dataName, std::string moduleOrigin) {
    for (size_t i = 0; i < subbedHfDataTypes.size(); i++) {
        HighFreqDataType curType = subbedHfDataTypes.at(i).dataType;
        if (dataName == curType.getName() && moduleOrigin == curType.getOrigin()) {
            return(true); //if the type is in the availHfDataTypes list
        }
    }

    return(false);
}

//returns -1 if the type is already in the list, 0 if it was successfully added
int MlmWrap::addHfTypeToSubbedList(HFSubSockSettings socketInfo) {
    HighFreqDataType nData = socketInfo.dataType;
    if (isHfTypeCurrentlySubbed(nData.getName(), nData.getOrigin()))
        return(-1);
    subbedHfDataTypes.push_back(socketInfo);
    return(0);
}

//returns -1 if the type was not found in the list and 0 if it was successfully removed
int MlmWrap::removeHfTypeFromSubbedList(HighFreqDataType rData) {
    for (size_t i = 0; i < subbedHfDataTypes.size(); i++) {
        if (rData == subbedHfDataTypes.at(i).dataType) {
            subbedHfDataTypes.erase(subbedHfDataTypes.begin()+i);
            return(0);
        }
    }

    return(-1);
}

HFSubSockSettings MlmWrap::getSubbedHFSockSettings(std::string dataName, std::string moduleOrigin) {
    for (size_t i = 0; i < subbedHfDataTypes.size(); i++) {
        HighFreqDataType curType = subbedHfDataTypes.at(i).dataType;
        HFSubSockSettings curSettings = subbedHfDataTypes.at(i);
        if (dataName == curType.getName() && moduleOrigin == curType.getOrigin()) {
            return(curSettings);
        }
    }
    HFSubSockSettings emptySettings;
    emptySettings.dataType = HighFreqDataType();
    emptySettings.subSockType = HighFreqSub::ST_NULL;
    emptySettings.userFoo = NULL;
    emptySettings.args = NULL;
    return(emptySettings);
}

//returns the full HighFreqDataType object w/ the corresponding name and origin.  Returns an empty(invalid) object if it isn't available
HighFreqDataType MlmWrap::getSubbedHfType(std::string dataName, std::string moduleOrigin) {
    for (size_t i = 0; i < subbedHfDataTypes.size(); i++) {
        HighFreqDataType curType = subbedHfDataTypes.at(i).dataType;
        if (dataName == curType.getName() && moduleOrigin == curType.getOrigin()) {
            return(curType);
        }
    }

    return(HighFreqDataType());
}

int MlmWrap::addSubToList(HighFreqSub *newSub) {
    hfSubs.push_back(newSub);
    //todo: possibly check for duplicates?
    return(0);
}

int MlmWrap::removeSubFromList(HighFreqDataType subType) {
    for (size_t i = 0; i < hfSubs.size(); i++) {
        if (subType == hfSubs.at(i)->getType()) {
            HighFreqSub *sub = hfSubs.at(i);
            delete sub;
            sub = NULL;
            hfSubs.erase(hfSubs.begin()+i);
            return(0);
        }
    }
    return(-1);
}

void MlmWrap::removeAllSubs(){
    for (size_t i = 0; i < hfSubs.size(); i++){
        if(hfSubs.at(i)){
            HighFreqSub *sub = hfSubs.at(i);
            delete sub;
            hfSubs[i] = NULL;
        }
    }
}

//returns ptr to the HighFreqSub with the corresponding 'dataName' and 'originModule'
//returns NULL if the HighFreqSub does not exist
HighFreqSub* MlmWrap::getHfSubObject(std::string dataName, std::string originModule) {
    for (size_t i = 0; i < hfSubs.size(); i++) {
        if (hfSubs.at(i)->getType().getName() == dataName && hfSubs.at(i)->getType().getOrigin() == originModule) {
            return(hfSubs.at(i));
        }
    }
    return(NULL);
}


bool MlmWrap::init_hardware_connection(){
    if(hardwaresock != NULL && zsock_is(hardwaresock)){
        std::cerr << error("Hardware socket already initialized!\n");
        return false;
    }

    if(state->getHardware_endpoint().empty() && id != TRODES_NETWORK_ID){
        std::cerr << error("Hardware socket not initialized by Trodes yet\n");
        return false;
    }

    if(id == TEMP_NETWORK_ID || id == BROKER_NETWORK_ID){
        std::cerr << error("Error initializing hardware connection. ") << id << " cannot speak to hardware!\n";
        return false;
    }

    if(id == TRODES_NETWORK_ID){
    //1. create socket
        hardwaresock = zsock_new(ZMQ_ROUTER);
        if(hardwaresock == NULL){
            std::cerr << error("Error creating hardware router socket\n");
            return false;
        }
        std::string h_address = state->getAddress();
        int h_port = zsock_bind(hardwaresock, "%s", std::string(h_address + ":*[" +
                       std::to_string(state->getPort()+1) + "-]").c_str());
        if(h_port < 0){
            std::cerr << error("Error initializing Trodes hardware socket\n");
            return false;
        }

    //2. update state
        state->setHardware_address(h_address);
        state->setHardware_port(h_port);
        state->setHardware_endpoint(zsock_endpoint(hardwaresock));

    //3. add hardwaresock to zloop so that it can be handled automatically
        std::lock_guard<std::mutex> lock(actormutex);
        zstr_send(actor, addhsock);
    }
    else{
    //1. use the trodes hardware router socket address and connect (get from hardware state)
        hardwaresock = zsock_new(ZMQ_REQ);
        zsock_set_identity(hardwaresock, id.c_str());
        zsock_connect(hardwaresock, "%s", state->getHardware_endpoint().c_str());
        if(hardwaresock == NULL){
            std::cerr << error("Error creating hardware router socket at ") << state->getHardware_endpoint() << "\n";
            return false;
        }
    }
    return true;
}

void MlmWrap::destroy_hardware_connection(){
    zsock_destroy(&hardwaresock);
}

//req to router: no need for identity frames, automatically added when sending and taken off when receiving
//trodesmsg is constructed by the caller, defined on higher level (abstractmoduleclient and trodescentralserver)
bool MlmWrap::sendHardwareMessage(TrodesMsg &msg){
    if(!hardwaresock){
        std::cerr << error("Hardware connection not initialized yet!\n");
        return false;
    }
    zmsg_t *m = msg.getzmsg();
    int rc = zmsg_send(&m, hardwaresock);
    if(rc != 0){

    }
    m = zmsg_recv(hardwaresock);
    //--UNFINISHED HARDWARE
    //Check for success or fail
    std::cerr << "Hardware message received by Trodes (not hardware): " << zmsg_popstr(m) << std::endl;

    return true; //TODO: return success/failure
}

//Loop task that connects handlers
void MlmWrap::message_reactor_task(zsock_t *pipe, void *args){
    MlmWrap *self = (MlmWrap*)args;
    zsock_signal(pipe, 0);

    //Create loop
    zloop_t *loop = zloop_new();

    //Set reader for pipe for incoming messages
    zloop_reader(loop, mlm_client_msgpipe(self->client), MlmWrap::handle_mlm_msgpipe, self);
    //Set reader for pipe for outgoing messages
    zloop_reader(loop, pipe, MlmWrap::handle_usr_msgpipe, self);
    //Set timer that runs for Abstractmoduleclient to utilize
    zloop_timer(loop, 25, 0, MlmWrap::handle_abs_timer, self);
    //Set variables
    self->reactor = loop;
    self->usrpipe = pipe;
    self->mlmpipe = mlm_client_msgpipe(self->client);

    //Start loop
    zloop_start(loop);

    //End and deallocate
    zloop_reader_end(loop, self->usrpipe);
    zloop_reader_end(loop, self->mlmpipe);
    if(self->hardwaresock) zloop_reader_end(loop, self->hardwaresock);
    zloop_destroy(&loop);
    if(self->hardwaresock) zsock_destroy(&self->hardwaresock);
    mlm_client_destroy(&self->client);
    self->removeAllSubs();
    zsock_signal(pipe, 0);
}

int MlmWrap::receiveStream(const char *sender, const char *subject, const char *format, TrodesMsg &msg) {
    int rc = 0;
//    std::cerr << error("receive stream " ) << sender <<" " << subject << "\n";
    if (streq(subject, TRODES_CMD) && streq(sender, TRODES_NETWORK_ID)) { //trodes commands
        rc = processMlmCommandMsg(msg);
    }
    else if(streq(subject, TRODES_NETWORK_NOTIFICATION)){
        rc = processMlmNetworkNotification(sender, msg);
    }
    else if(strncmp(subject, eventprefix, strlen(eventprefix))==0){
        std::string event(subject);
        rc = processMlmEventMsg(sender, event.substr(strlen(eventprefix)).c_str(), msg);
    }
    else {
        rc = processOtherMsg(sender, subject, msg); //if the type is unknown, send it to the processOtherMsg function
    }
    return rc;
}

int MlmWrap::receiveMailbox(const char *sender, const char *subject, const char *format, TrodesMsg &msg) {
    int rc = 0;
    if (streq(subject, TRODES_CMD)) { //direct message commands
        rc = processMlmCommandMsg(msg);
    }
    else if (streq(subject, TRODES_INFO_REP)) { //mlm processes reply messages in a standardized way
        rc = processMlmReplyMsg(msg);
    }
    else if (streq(subject,TRODES_INFO_REQ)) { //mlm processes request messages in a standardized way
        rc = processMlmRequestMsg(sender, msg);
    }
    else if (streq(subject,TRODES_NETWORK_NOTIFICATION)) { //mlm processes internal network notifications
        rc = processMlmNetworkNotification(sender, msg);
    }
    else{
        rc = processOtherMsg(sender, subject, msg); //if the type is unknown, send it to the processOtherMsg function
    }
    return(rc);
}

int MlmWrap::receiveService(const char *sender, const char *subject, const char *format, TrodesMsg &msg) {
    //currently there are no service messages in use
    int rc = 0;
    rc = processOtherMsg(sender, subject, msg);
    return(rc);
}

int MlmWrap::processMlmCommandMsg(TrodesMsg &msg) {
    int rc = 0;
    std::string cmdType = msg.popstr();
    rc = processCommandMsg(cmdType, msg);
    return(rc);
}

int MlmWrap::processMlmEventMsg(const char *sender, const char *event, TrodesMsg &msg) {
    int rc = 0;
    rc = processEventMsg(sender, event, msg);
    return(rc);
}

int MlmWrap::processMlmNetworkNotification(const char *sender, TrodesMsg &msg) {
    int rc = 0;
    std::string noteType = msg.popstr();
    if (noteType == hfdt_register) { //High Frequency Data Type was registered by *sender
        HighFreqDataType registeredType;
        binarydata regdata;
        msg.popcontents("n", regdata);
        registeredType.decode(regdata);
        state->insert_hfdt(registeredType);

        if (isHfTypeCurrentlySubbed(registeredType.getName(), registeredType.getOrigin())) {
            //resub if the module had previously been subbed to this type and the user didn't unsub manually
            HFSubSockSettings sockInfo = getSubbedHFSockSettings(registeredType.getName(), registeredType.getOrigin());

            if (sockInfo.subSockType == HighFreqSub::ST_CONSUMER) {
                HFSubConsumer *sub = createConsumerSub(registeredType, 10);
                sub->initialize();
            }
            else if (sockInfo.subSockType == HighFreqSub::ST_WORKER) {
                HFSubWorker *sub = createWorkerSub(registeredType, 1, sockInfo.userFoo, sockInfo.args);
                sub->initialize();
            }
            //todo: add some way to save the buffer size information
        }
    }
    else if (noteType == hfdt_deregister) { //High Frequency Data Type was deregistered by *sender
        HighFreqDataType deregisteredType;
        binarydata regdata;
        msg.popcontents("n", regdata);
        deregisteredType.decode(regdata);
        state->remove_hfdt(deregisteredType);

        if (isHfTypeCurrentlySubbed(deregisteredType.getName(), deregisteredType.getOrigin())) {
            rc = removeSubFromList(deregisteredType);
        }
    }
    else if(noteType == eventprovide){
        std::string event = msg.popstr();
        std::string origin = msg.popstr();
        EventDataType ev(event, origin);
        state->insert_event(ev);
    }
    else if(noteType == eventunprovide){
        std::string name = msg.popstr();
        std::string origin = msg.popstr();
        EventDataType ev(name, origin);
        state->remove_event(ev);
    }
    else if(noteType == CLIENT_CONNECT_MSG){
        std::string who = msg.popstr();
        if(who != TEMP_NETWORK_ID && who != id && who != BROKER_NETWORK_ID){
            state->insert_client(who);
        }
        TrodesMsg m("s", who);
        processNotification(sender, noteType, m);
    }
    else if(noteType == CLIENT_DISCONNECT_MSG || noteType == CLIENT_EXPIRED_MSG){
        std::string who = msg.popstr();
        if(who != TEMP_NETWORK_ID && who != id && who != BROKER_NETWORK_ID && state->client_exists(who)){
            state->remove_client(who);
        }
       TrodesMsg m("s", who);
       processNotification(sender, noteType, m);
    }
    else {
        rc = processNotification(sender, noteType, msg);
    }
    return(rc);
}

int MlmWrap::processMlmRequestMsg(const char *sender, TrodesMsg &msg) {
    int rc = 0;
    std::string req = msg.popstr();
    rc = processRequestMsg(sender, req, msg);
    return(rc);
}

int MlmWrap::processMlmReplyMsg(TrodesMsg &msg) {
    int rc = 0;
    std::string type = msg.popstr();
    auto search = trackers.find(type);
    bool tracker = false;
    //There is a timer still out for this reply. Cancel it, remove it from map, reply success to unblock.
    if(search != trackers.end()){
        zloop_timer_end(reactor, search->second);
        trackers.erase(search);
        tracker = true;
    }

    rc = processReplyMsg(type, msg);
    if(tracker){
        zstr_send(usrpipe, blockrepsuccess);
    }
    return(rc);
}

//Routing functions
//*****
// streaming, mailboxes, and service requests.
int MlmWrap::route_stream_msgs(const char *address, const char *sender, const char *subject, zmsg_t *msg){
//    std::cout << "      [" << id << "] Routing stream msg " << subject << " from " << sender << std::endl;
    char *f = zmsg_popstr(msg);
    std::string format = f;
    freen(f);
    TrodesMsg m(format, msg);
    receiveStream(sender, subject, format.c_str(), m);
    return 0;
}

int MlmWrap::route_mailbox_msgs(const char *address, const char *sender, const char *subject, const char *tracker, zmsg_t *msg){
    char *f = zmsg_popstr(msg);
    std::string format = f;
    freen(f);

    TrodesMsg m(format, msg);
    receiveMailbox(sender, subject, format.c_str(), m);
    return 0;
}

int MlmWrap::route_service_msgs(const char *address, const char *sender, const char *subject, const char *tracker, zmsg_t *msg){
    char *f = zmsg_popstr(msg);
    std::string format = f;
    freen(f);
    TrodesMsg m(format, msg);
    receiveService(sender, subject, format.c_str(), m);
    return 0;
}


//Message coming in, call mlm_client_recv, call appropriate virtual functions
int MlmWrap::handle_mlm_msgpipe(zloop_t *loop, zsock_t *reader, void *arg){
    MlmWrap *self = (MlmWrap*)arg;
    zmsg_t *msg = mlm_client_recv(self->client);
    const char *cmd = mlm_client_command(self->client);
    if(streq(cmd, "STREAM DELIVER")){
        return self->route_stream_msgs(mlm_client_address(self->client),
                mlm_client_sender(self->client),
                mlm_client_subject(self->client),
                msg);
    }
    else if(streq(cmd, "MAILBOX DELIVER")){
        return self->route_mailbox_msgs(mlm_client_address(self->client),
                mlm_client_sender(self->client),
                mlm_client_subject(self->client),
                mlm_client_tracker(self->client),
                msg);
    }
    else if(streq(cmd, "SERVICE DELIVER")){
        return self->route_service_msgs(mlm_client_address(self->client),
                mlm_client_sender(self->client),
                mlm_client_subject(self->client),
                mlm_client_tracker(self->client),
                msg);
    }
    else{
        //Todo: Throw error? who would catch it, not zloop
        return 0;
    }
    return 0;
}

//Message going out from zactor pipe, call mlm_client_send*
int MlmWrap::handle_usr_msgpipe(zloop_t *loop, zsock_t *reader, void *arg){
    MlmWrap *self = (MlmWrap*)arg;
//    std::cerr << "handle_usr_msgpipe, calling zmsg_recv\n";
    zmsg_t *msg = zmsg_recv(reader);
//    std::cerr << "handle_usr_msgpipe, msg received\n";
    char *msgtype = zmsg_popstr(msg);
    int rc = 0;

    //****
    //Sending messages out
    if(streq(msgtype, streammsg)){
        char *subject = zmsg_popstr(msg);
        rc = mlm_client_send(self->client, subject, &msg ); //forward the message to the subs of the subject
        freen(subject);
    }
    else if(streq(msgtype, mailboxmsg)){
        char *addr, *subj, *track;
        uint32_t timeout;
        //Pop all except the user message (with its format header), leave that in message
        czhelp_zmsg_read(msg, "sss4", &addr, &subj, &track, &timeout);
        rc = mlm_client_sendto(self->client, addr, subj, track, timeout, &msg);
        freen(addr);
        freen(subj);
        freen(track);
    }
    else if(streq(msgtype, requestmsg)){
        char *addr, *subj, *track;
        uint32_t timeout;
        //Pop all except the user message (with its format header), leave that in message
        czhelp_zmsg_read(msg, "sss4", &addr, &subj, &track, &timeout);
        rc = mlm_client_sendfor(self->client, addr, subj, track, timeout, &msg);
        freen(addr);
        freen(subj);
        freen(track);
    }
    //****
    //Sending notifications out to other clients for new updates
    else if(streq(msgtype, eventprovide)){
        //User adding an event
        char *event = zmsg_popstr(msg);
        EventDataType ev(event, self->id);
        self->state->insert_event(ev);
        TrodesMsg tmsg("sss", eventprovide, event, self->id);
        self->sendMessageToAll(TRODES_NETWORK_NOTIFICATION, tmsg);
        freen(event);
    }
    else if(streq(msgtype, eventunprovide)){
        //User removing event
        char *event = zmsg_popstr(msg);
        EventDataType ev(event, self->id);
        self->state->remove_event(ev);
        TrodesMsg tmsg("sss", eventunprovide, event, self->id);
        self->sendMessageToAll(TRODES_NETWORK_NOTIFICATION, tmsg);
        freen(event);
    }
    else if(streq(msgtype, hfdt_register)){
        char *n, *o, *f, *s;
        int b;
        czhelp_zmsg_read(msg, "ssssi", &n,&o,&f,&s,&b);
        HighFreqDataType dataType(n,o,f,s,b);
        self->state->insert_hfdt(dataType);
        //send notification to all modules that a new data publisher is available
        TrodesMsg msg("sn", hfdt_register, dataType.encode());
        self->sendMessageToAll(TRODES_NETWORK_NOTIFICATION, msg);
        freen(n); freen(o); freen(f); freen(s);
    }
    else if(streq(msgtype, hfdt_deregister)){
        char *n, *o, *f, *s;
        int b;
        czhelp_zmsg_read(msg, "ssssi", &n,&o,&f,&s,&b);
        HighFreqDataType dataType(n,o,f,s,b);
        self->state->remove_hfdt(dataType);
        TrodesMsg msg("sn", hfdt_deregister, dataType.encode());
        self->sendMessageToAll(TRODES_NETWORK_NOTIFICATION, msg);
        freen(n); freen(o); freen(f); freen(s);
    }
    //****
    //Sending user requests back to user thread
    else if(streq(msgtype, getclients)){
        auto clients = self->state->getClients();
        for(const auto &i : clients){
            zmsg_addstr(msg, i.c_str());
        }
        zmsg_send(&msg, self->usrpipe);
    }
    else if(streq(msgtype, eventslist)){
        auto events = self->state->getEvents();
        zmsg_addstr(msg, eventslist);
        for(const auto &i : events){
            zmsg_addstr(msg, i.getName().c_str());
            zmsg_addstr(msg, i.getOrigin().c_str());
        }
        zmsg_send(&msg, self->usrpipe);
    }
    else if(streq(msgtype, hfdtslist)){
        auto hfdts = self->state->getHfdts();
        zmsg_addstr(msg, hfdtslist);
        for(auto &i : hfdts){
            binarydata bytes = i.encode();
            zmsg_addmem(msg, bytes.data(), bytes.size());
        }
        zmsg_send(&msg, self->usrpipe);
    }
    //****
    //Zloop related functions
    else if(streq(msgtype, blockuntilrep)){
        //Block until a reply occurs. 1. create timer in loop 2. Add tracker|timerid to map
        //If timeout occurs 1. Remove tracker|timerid from map. 2. cancel timer 3. reply failure
        //When reply arrives (and timer still active), 1. Remove timerid|tracker from map. 2. cancel timer 3. reply success to usrpipe
        //WHEN WAITING FOR REPLY, MUST SEND BACK BLOCKING REPLY BEFORE ACTUAL REPLY
        char *tracker = zmsg_popstr(msg);
        char *timestr= zmsg_popstr(msg);
        uint32_t timeout = strtoul(timestr, NULL, 10);
        int timerid = zloop_timer(loop, timeout, 1, MlmWrap::block_reply_timeout, self);
        self->trackers.insert({std::string(tracker), timerid});
        freen(tracker);
        freen(timestr);
    }
    else if(streq(msgtype, addhsock) && self->id == TRODES_NETWORK_ID){
        //Add hardware socket to zloop
        //ignore if not trodes
        zloop_reader(loop, self->hardwaresock, MlmWrap::recv_hardware_request, self);
    }
    else if(streq(msgtype, "$TERM")){
        rc = -1;
    }
    zmsg_destroy(&msg);
    freen(msgtype);
    return rc;
}

int MlmWrap::handle_abs_timer(zloop_t *loop, int timer_id, void *arg){
    MlmWrap *self = (MlmWrap*)arg;
    self->processTimer(timer_id);
    return 0;
}

int MlmWrap::block_reply_timeout(zloop_t *loop, int timer_id, void *arg){
    MlmWrap *self = (MlmWrap*)arg;
    for(auto iter = self->trackers.begin(); iter != self->trackers.end(); ++iter){
        if(iter->second == timer_id){
            self->trackers.erase(iter++);
            zloop_timer_end(loop, timer_id);
            zstr_send(self->usrpipe, blockrepfailure);
            return 0;
        }
    }
    return 0;
}

int MlmWrap::recv_hardware_request(zloop_t *loop, zsock_t *reader, void *arg){
    MlmWrap *self = (MlmWrap*)arg;
    //--UNFINISHED HARDWARE
    zmsg_t *msg = zmsg_recv(reader);
    char *sender = zmsg_popstr(msg);
    char *empty = zmsg_popstr(msg);
    if(sender && empty && strlen(empty)==0){
        free(empty);
    }
    else{
        std::cerr << self->error("Received malformed message from module\n");
        return 0;
    }
    TrodesMsg tmsg("s", msg);
    //call process message or something
    self->processRequestMsg(sender, "HARDWARE", tmsg);

    //processrequestmsg should empty out trodesmsg and fill it back with a reply
    zmsg_t *reply = zmsg_new();
    zmsg_addstr(reply, sender);
    zmsg_add(reply, zframe_new_empty());
    for(unsigned int i = 0; i < tmsg.numContents(); ++i)
        zmsg_addstr(reply, tmsg.popstr().c_str());
    zmsg_send(&reply, reader);
    return 0;
}
//************************************************************
//Central Broker Wrapper
//************************************************************

CentralBroker::CentralBroker(const char* endP) {
    create(endP);
}

CentralBroker::CentralBroker(const char* address, int port) {
    std::string ep;
    ep.append(address);
    ep.append(":");
    ep.append(std::to_string(port));
    create(ep.c_str());
}

void CentralBroker::create(const char *endP){
    initialized = false;
    endpoint.assign(endP);

    broker = zactor_new(mlm_server, NULL);

    //Bind broker to endpoint
    zsock_send(broker, "ss", "BIND", endpoint.c_str());
    //Set server timeouts to be 5000 ms
    zsock_send (broker, "sss", "SET", "server/timeout", "5000");

    //group 1 = anything before the colon, address
    //group 2 = any number of digits after colon, port
    zrex_t *addr_rex = zrex_new("(.*):(\\d*)");
    if(!zrex_matches(addr_rex, endP)){
        std::cout << "Error: Could not start broker handle. (CentralBroker::CentralBroker)\n";
    }
    address.assign(zrex_hit(addr_rex, 1));
    port = std::stoi(zrex_hit(addr_rex, 2));
    zrex_destroy(&addr_rex);

    //Start broker client
    brokerHandle = new BrokerClient(BROKER_NETWORK_ID, address.c_str(), port, broker);
    if(!brokerHandle->start()){
        return;
    }

    //Create thread for Broker Client that forwards logs to itself and parses it
    //TODO: just integrate this into the BrokerClient class
    logger = zactor_new(&CentralBroker::logging_reactor_task, this);
    initialized = true;
}
CentralBroker::~CentralBroker() {
    if(initialized){
        zactor_destroy(&logger);
    }
    delete brokerHandle;
    zactor_destroy(&broker);
    zsys_shutdown();
}

std::string CentralBroker::getEndpoint() const{
    return(endpoint.c_str());
}

std::string CentralBroker::getAddress() const
{
    return address;
}

int CentralBroker::getPort() const
{
    return port;
}

bool CentralBroker::isInitialized() const
{
    return initialized;
}

void CentralBroker::logging_reactor_task(zsock_t *pipe, void *args){
//    CentralBroker *self = (CentralBroker*)args;
    zsock_signal(pipe, 0);

    zsock_t *logging = zsock_new_sub("inproc://logging", "I:");
    zsys_set_logsender("inproc://logging");
    zsys_set_logstream(NULL);

    //Create loop
    zloop_t *loop = zloop_new();

    zloop_reader(loop, pipe, CentralBroker::logging_shutdown_pipe, args);
    zloop_reader(loop, logging, CentralBroker::logging_forward_log, args);

    //Start loop
    zloop_start(loop);

    //End and deallocate
    zloop_destroy(&loop);
    zsock_destroy(&logging);
    zsys_set_logsender(NULL);
}

int CentralBroker::logging_shutdown_pipe(zloop_t *loop, zsock_t *reader, void *arg){
//    CentralBroker *self = (CentralBroker*)arg;
    char *msg = zstr_recv(reader);
    int rc = 0;
    if(streq(msg, "$TERM")){
        rc = -1;
    }
    freen(msg);
    return rc;
}

int CentralBroker::logging_forward_log(zloop_t *loop, zsock_t *reader, void *arg){
    CentralBroker *self = (CentralBroker*)arg;
    char *log = zstr_recv(reader);
    int rc = 0;

    //group 1 = date, group 2 = time, group 3 = address, group 4 = action
    zrex_t *log_regex = zrex_new("I: (\\d\\d-\\d\\d-\\d\\d) (\\d\\d:\\d\\d:\\d\\d) .* address='(.*)' - (registering|closed|expired)");
    if(zrex_matches(log_regex, log)){
        std::string date, time, address, action;
        date.assign(zrex_hit(log_regex, 1));
        time.assign(zrex_hit(log_regex, 2));
        address.assign(zrex_hit(log_regex, 3));
        action.assign(zrex_hit(log_regex, 4));
        TrodesMsg msg;
        if(!streq(address.c_str(), TEMP_NETWORK_ID)){
            if(streq(action.c_str(), "closed")){
                msg.addstr(CLIENT_DISCONNECT_MSG);
                msg.addstr(address);
                self->brokerHandle->sendNotification(msg);
            }
            else if(streq(action.c_str(), "expired")){
                msg.addstr(CLIENT_EXPIRED_MSG);
                msg.addstr(address);
                self->brokerHandle->sendNotification(msg);
            }
            else if(streq(action.c_str(), "registering")){
                msg.addstr(CLIENT_CONNECT_MSG);
                msg.addstr(address);
                self->brokerHandle->sendNotification(msg);
            }
        }
    }
    zrex_destroy(&log_regex);
    freen(log);
    return rc;
}

//************************************************************
//Broker Client
//************************************************************

BrokerClient::BrokerClient(const char* id, const char* addr, int port, zactor_t *broker) : MlmWrap(id, addr, port) {
    brokerObj = broker;
}

BrokerClient::~BrokerClient() {

}

bool BrokerClient::start(){
    if(initialize()){
        //Could not initialize! 
        return false;
    }
    if(setProducer(TRODES_NETWORK_ID)){
        return false;
    }
    return true;
}

void BrokerClient::sendNotification(TrodesMsg &msg){
    TrodesMsg msg2 = msg.copy();
    sendStream(TRODES_NETWORK_NOTIFICATION, msg);
    sendMessage(TRODES_NETWORK_ID, TRODES_NETWORK_NOTIFICATION, msg2);
}

int BrokerClient::processOtherMsg(const char *sender, const char *subject, TrodesMsg &msg) {
    int rc = 0;
    if (streq(subject, clientlist)) {
        zstr_sendx (brokerObj, clientlist, NULL);
        zmsg_t* message = zmsg_recv (brokerObj);
        mlm_client_sendto(client, sender, clientlist, "", 0, &message);
    }
    else if (streq(subject, streamlist)) {
        zstr_sendx (brokerObj, streamlist, NULL);
        zmsg_t* message = zmsg_recv (brokerObj);

//        sendMessage(sender, streamlist, "", 0, "m", message, NULL);
        mlm_client_sendto(client, sender, streamlist, "", 0, &message);
    }
    else {
        std::cout << error("Subject ") << subject << " unknown to broker client.\n";
    }
    return(rc);
}


//***************************************************************
//**Internal network struct
//** -pimpl (pointer to impl(ementation)
//** -serves to make sure user and internals are never accessing the same data
//** -only the loop accesses the pimpl, not the user
//***************************************************************
network_pimpl::network_pimpl()
    : endpoint(""),
      address(""),
      port(-1),
      trodes_source(None),
      trodes_acquisition(Stop),
      trodes_filename(""),
      hardware_endpoint(""),
      hardware_address(""),
      hardware_port(-1),
      timestamp_endpoint("")
{

}

network_pimpl::~network_pimpl(){
    clients.clear();
    events.clear();
    hfdts.clear();
}

std::string network_pimpl::getEndpoint() const{
    return endpoint;
}

void network_pimpl::setEndpoint(const std::string &value){
    endpoint = value;
    if(endpoint.find("tcp://")==value.npos){
        endpoint.insert(0, "tcp://");
    }
}

std::string network_pimpl::getAddress() const{
    return address;
}

void network_pimpl::setAddress(const std::string &value){
    address = value;
}

int network_pimpl::getPort() const{
    return port;
}

void network_pimpl::setPort(int value){
    port = value;
}

void network_pimpl::insert_client(const std::string &value){
    clients.insert(value);
}

void network_pimpl::remove_client(const std::string &value){
    //First remove all events and hfdt's related to this client
    for(auto it = events.begin(); it != events.end(); ){
        if(it->getOrigin() == value)
            it = events.erase(it);
        else ++it;
    }
    //Remove hfdt's
    for(auto it = hfdts.begin(); it != hfdts.end(); ){
        if(it->getOrigin() == value)
            it = hfdts.erase(it);
        else ++it;
    }
    //Remove name from clients
    clients.erase(value);
}

bool network_pimpl::client_exists(const std::string &value) const{
    return clients.find(value) != clients.end();
}

std::set<std::string> network_pimpl::getClients() const{
    return clients;
}

void network_pimpl::insert_event(const EventDataType &value){
    for(auto const& ev : events){
        if(ev.getName() == value.getName() && ev.getOrigin() == value.getOrigin()){
            std::cerr << "---duplicate event " << ev.getName() << "\n";
            return;
        }
    }
    events.push_back(value);
}

void network_pimpl::remove_event(const EventDataType &value){
    for(auto it = events.begin(); it != events.end(); ++it){
        if(it->getName() == value.getName() && it->getOrigin() == value.getOrigin()){
            events.erase(it);
            return;
        }
    }
    std::cerr << "---could not find event to delete. name: " << value.getName() << " origin: " << value.getOrigin() << "\n";
}

std::vector<EventDataType> network_pimpl::getEvents() const{
    return events;
}

void network_pimpl::insert_hfdt(const HighFreqDataType &value){
    for(auto const& dt : hfdts){
        if(dt.getName()==value.getName() && dt.getOrigin()==value.getOrigin() && dt.getSockAddr()==value.getSockAddr()){
            std::cerr << "----duplicate hfdt " << value.getName() << "!\n";
            return;
        }
    }
    hfdts.push_back(value);
}
HighFreqDataType network_pimpl::find_hfdt(const std::string &name, const std::string &origin){
    for(auto const& dt : hfdts){
        if(dt.getName() == name && dt.getOrigin() == origin){
            return dt;
        }
    }
    return HighFreqDataType();
}
bool network_pimpl::hfdt_exists(const HighFreqDataType &value){
    for(auto const& dt : hfdts){
        if(dt.getName()==value.getName() && dt.getOrigin()==value.getOrigin()){
            return true;
        }
    }
    return false;
}

bool network_pimpl::hfdt_exists(const std::string &name, const std::string &origin){
    for(auto const& dt : hfdts){
        if(dt.getName()==name && dt.getOrigin()==origin){
            return true;
        }
    }
    return false;
}
void network_pimpl::remove_hfdt(const HighFreqDataType &value){
    for(auto it = hfdts.begin(); it != hfdts.end(); ++it){
        if(it->getName()==value.getName() && it->getOrigin()==value.getOrigin() && it->getSockAddr()==value.getSockAddr()){
            hfdts.erase(it);
            return;
        }
    }
    std::cerr << "---could not find hfdt " << value.getName() << " to delete\n";
}

std::vector<HighFreqDataType> network_pimpl::getHfdts() const{
    return hfdts;
}

TrodesSource network_pimpl::getTrodes_source() const
{
    return trodes_source;
}

void network_pimpl::setTrodes_source(const TrodesSource &value)
{
    trodes_source = value;
}

TrodesAcquisition network_pimpl::getTrodes_acquisition() const
{
    return trodes_acquisition;
}

void network_pimpl::setTrodes_acquisition(const TrodesAcquisition &value)
{
    trodes_acquisition = value;
}

std::string network_pimpl::getTrodes_filename() const
{
    return trodes_filename;
}

void network_pimpl::setTrodes_filename(const std::string &value)
{
    trodes_filename = value;
}

std::string network_pimpl::getHardware_endpoint() const
{
return hardware_endpoint;
}

void network_pimpl::setHardware_endpoint(const std::string &value)
{
hardware_endpoint = value;
}

std::string network_pimpl::getHardware_address() const
{
return hardware_address;
}

void network_pimpl::setHardware_address(const std::string &value)
{
hardware_address = value;
}

int network_pimpl::getHardware_port() const
{
return hardware_port;
}

void network_pimpl::setHardware_port(int value)
{
hardware_port = value;
}

binarydata network_pimpl::encode() const{
    int tsrc = (int)trodes_source;
    int tacq = (int)trodes_acquisition;
    return NetworkDataType::serializedata(endpoint, address, port, clients, events, hfdts,
                                          tsrc, tacq, trodes_filename,
                                          hardware_endpoint, hardware_address, hardware_port,
                                          timestamp_endpoint);
}

void network_pimpl::decode(const binarydata &data){
    int tsource, tacq;
    NetworkDataType::deserializedata(data, endpoint, address, port, clients, events, hfdts,
                                     tsource, tacq, trodes_filename,
                                     hardware_endpoint, hardware_address, hardware_port,
                                     timestamp_endpoint);
    trodes_source = (TrodesSource)tsource;
    trodes_acquisition = (TrodesAcquisition)tacq;
}

std::string network_pimpl::getTimestamp_endpoint() const
{
    return timestamp_endpoint;
}

void network_pimpl::setTimestamp_endpoint(const std::string &value)
{
    timestamp_endpoint = value;
}
