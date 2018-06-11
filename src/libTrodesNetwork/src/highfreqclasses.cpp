#include "libTrodesNetwork/highfreqclasses.h"
#include <czmq.h>

#define waittimeout     "$WT"
#define regdt           "$REG"
#define regdone         "regdone"
#define deregdt         "$DEREG"
#define deregdone       "deregdone"
//************************************************************
//High Frequency Pub
//************************************************************
HighFreqPub::HighFreqPub() : hfPub(NULL) {

}

HighFreqPub::~HighFreqPub() {
    if (hfPub != NULL) {
        zsock_destroy(&hfPub);
        hfPub = NULL;
    }
}

int HighFreqPub::initialize(std::string addr, int port) {
    if(addr == ""){
        addr = "tcp://127.0.0.1";
        port = 49153;
    }
    sockEndpoint = addr;
    std::string front = addr.append(":");
    std::string newAddress = front;
    newAddress += "*[" + std::to_string(port) + "-]"; // zsock_bind() takes in a port value of *[p-], which means bind to first available port starting at "p"
    hfPub = zsock_new(ZMQ_PUB);
    int aPort = zsock_bind(hfPub, "%s", newAddress.c_str()); //%s just reducing warnings, one at a time
    if(aPort == -1){
        return -1;
    }
    sockEndpoint = front.append(std::to_string(aPort));
    return 0;
}

void HighFreqPub::publishData(void *data, size_t dataSize, int64_t timestamp){
    zframe_t *dFrame = zframe_new(data, dataSize+sizeof(int64_t)); //data is copied into a new zframe + space for int64_t timestamp
    *(int64_t*)(zframe_data(dFrame)+dataSize) = timestamp; // Set last 64bits of data to be the timestamp
    zframe_send(&dFrame, hfPub, 0);
}

void HighFreqPub::publishData(void *data, size_t dataSize) {
    publishData(data, dataSize, zclock_time());
}

std::string HighFreqPub::getAddress() const{
    return sockEndpoint;
}
//************************************************************
//High Frequency Sub
//************************************************************

HighFreqSub::HighFreqSub(HighFreqDataType hf, SubType type)
    : actor(NULL),
      sub(NULL),
//      registered(false),
      sockType(type)
{
    registerDataType(hf);
}

HighFreqSub::~HighFreqSub(){
    if(actor)
        zactor_destroy(&actor);
    zsock_destroy(&sub);
}

HighFreqDataType HighFreqSub::getType(){
    return dt;
}

int HighFreqSub::registerDataType(HighFreqDataType hf){
    dt = hf;
    sub = zsock_new_sub(dt.getSockAddr().c_str(), "");
    if(!sub){
        return -1;
    }
//    registered = true;
    zsock_set_rcvhwm(sub, 100);
    if(actor){
        zstr_send(actor, regdt);
        char *recv = zstr_recv(actor);
        if(streq(recv, regdone)){
            freen(recv);
            return 0;
        }else{
            freen(recv);
            return -1;
        }
    }
    return 0;
}

int HighFreqSub::deregister(){
    dt = HighFreqDataType();
    zstr_send(actor, deregdt);
    char * recv = zstr_recv(actor);
    if(streq(recv, deregdone)){
        freen(recv);
        return 0;
    }else{
        freen(recv);
        return -1;
    }
}

int HighFreqSub::pipeController(zloop_t *loop, zsock_t *reader, void *arg){
    zframe_t *f = zframe_recv(reader);
    HighFreqSub *self = (HighFreqSub*)arg;
    if(zframe_streq(f, "$TERM")){
        return -1;
    }
    else if(zframe_streq(f, deregdt)){
        zloop_reader_end(loop, self->sub);
        zsock_destroy(&self->sub);
        zstr_send(reader, deregdone);
    }
    else if(zframe_streq(f, regdt)){
        if(!self->sub)
            self->sub = zsock_new_sub(self->getType().getSockAddr().c_str(), "");
        self->sub_reader(loop);
        zstr_send(reader, regdone);
    }
    else{
        self->customCommands(loop, reader, f);
    }
    zframe_destroy(&f);
    return 0;
}

bool default_consumer_queue_filter(void* data, size_t bytes, void *args){
    return true;
}

HFSubConsumer::HFSubConsumer(HighFreqDataType hf, size_t bfsize, consumer_queue_filter filter)
    : HighFreqSub(hf, ST_CONSUMER),
      buffersize(bfsize+1),
      msgsize(hf.getByteSize()),
      queuefilter(filter),
      framequeue(buffersize, 1, 0),
      ptoken(framequeue),
      replacetoken(framequeue),
      consumetoken(framequeue),
      req(NULL),
      rep(NULL),
      waiting(false),
      lasttimestamp(0)
{

}

HFSubConsumer::~HFSubConsumer(){
    if(req)
        zsock_destroy(&req);
    for(size_t i = 0; i < framequeue.size_approx(); i++){
        zframe_t* frame;
        if(framequeue.try_dequeue(consumetoken,frame))
            zframe_destroy(&frame);
    }
}

void HFSubConsumer::initialize(){
    if(!actor)
        actor = zactor_new(ringbufController, this);
    //TODO: verify actor successfully initialized
    req = zsock_new_req(rependpoint.c_str());
    pollitem = (zmq_pollitem_t*)malloc(sizeof(zmq_pollitem_t));//{zsock_resolve(req), 0, ZMQ_POLLIN, 0};
    pollitem->socket = zsock_resolve(req);
    pollitem->fd = 0;
    pollitem->events = ZMQ_POLLIN;
    pollitem->revents = 0;
}

void HFSubConsumer::sub_reader(zloop_t *loop){
    zframe_t *f;
    while(framequeue.try_dequeue(replacetoken, f)){
        zframe_destroy(&f);
        f = NULL;
    }
    zloop_reader(loop, sub, bufferwriter, this);
    zloop_reader(loop, rep, requesthandler, this);
}

void HFSubConsumer::ringbufController(zsock_t *pipe, void *args){
    HFSubConsumer *self = (HFSubConsumer*)args;
    self->usrpipe = pipe;

    //Bind rep to custom endpoint
    self->rep = zsock_new(ZMQ_REP);
    zsock_bind(self->rep, "inproc://rep_%p", args);
    self->rependpoint = zsock_endpoint(self->rep);

    //Initialize loop
    zsock_signal(pipe, 0);
    zloop_t *loop = zloop_new();
    zloop_reader(loop, pipe, pipeController, self);
    self->sub_reader(loop);
    zloop_start(loop);
    zloop_destroy(&loop);
    zsock_destroy(&self->rep);
}

void HFSubConsumer::customCommands(zloop_t *loop, zsock_t *reader, zframe_t *f){

}

int HFSubConsumer::waitTimer(zloop_t *loop, int timer_id, void *arg){
    //If timeout happens before message comes in
    HFSubConsumer *self = (HFSubConsumer*)arg;
    if(self->waiting){
        self->waiting = false;
        zstr_send(self->usrpipe, "X");
    }
    return 0;
}
int HFSubConsumer::bufferwriter(zloop_t *loop, zsock_t *reader, void *arg){
    HFSubConsumer *self = (HFSubConsumer*)arg;
    zframe_t *frame = zframe_recv(reader);
    self->handle_enqueue(frame);
    return 0;
}

int HFSubConsumer::handle_enqueue(zframe_t *frame){
    //Only add to queue if it passes queuefilter
    if(!queuefilter(zframe_data(frame), zframe_size(frame), this)){
        zframe_destroy(&frame);
        return 0;
    }
    //Pseudo ring buffer, pops oldest so that ConcurrentQueue doesn't allocate new block and acts like concurrent ringbuffer
    if(framequeue.size_approx() == buffersize-1){
        //Do this first before replacing with new frame
        zframe_t* tmp;
        if(framequeue.try_dequeue(replacetoken, tmp)){
            zframe_destroy(&tmp);
        }
    }
    framequeue.enqueue(ptoken, frame);

    if(waiting){
        zsock_bsend(rep, "8", static_cast<uint64_t>(framequeue.size_approx()), NULL);
        waiting = false;
    }
    return 0;
}
int HFSubConsumer::requesthandler(zloop_t *loop, zsock_t *reader, void *arg){
    HFSubConsumer *self = (HFSubConsumer*)arg;
    zframe_t *frame = zframe_recv(reader);
    self->handle_request(frame);
    zframe_destroy(&frame);
    return 0;
}

int HFSubConsumer::handle_request(zframe_t *frame){
    if(!frame){
        return 0;
    }
    if(zframe_streq(frame, waittimeout)){
        //User is waiting for new data to come in. if something came in since then, reply back immediately.
        if(framequeue.size_approx()){
            zsock_bsend(rep, "8", static_cast<uint64_t>(framequeue.size_approx()), NULL);
            waiting = false;
        }
        //Else, set bool to handle later
        else{
            waiting = true;
        }
    }
    else{
        //Got some unknown request? reply anyways to avoid blocking
        zsock_bsend(rep, "8", 0);
    }
    return 0;
}

size_t HFSubConsumer::readData(void *dest, size_t size){
    zframe_t* frame = NULL;
    if(framequeue.try_dequeue(consumetoken, frame)){
        //Queue is not empty, parse it, copy data, get latency, destroy frame
        memcpy(dest, zframe_data(frame), size);
        lasttimestamp = *(int64_t*)(zframe_data(frame)+zframe_size(frame)-sizeof(int64_t));
//        lastlatency = zclock_usecs() - lasttimestamp;
        zframe_destroy(&frame);
    }
    else{
        //Queue is empty, return 0. User should wait for data via available()
        return 0;
    }


    return size;
}

size_t HFSubConsumer::available(long timeout){
    size_t num = framequeue.size_approx();
    if(num){
        return num;
    }
//    else if(timeout <= 1){ //TODO: Maybe optimize with this?
//    }
    else{
        uint64_t avail;
        //Polling could have timed out last time. First check if there is a message waiting to be grabbed.
        //If there is, it must be an old value since the framequeue size we got was 0. Toss it and try again.
        zmq_poll(pollitem, 1, 0);
        if(pollitem->revents & ZMQ_POLLIN){
            zsock_brecv(req, "8", &avail);
        }

        //Request again after clearing any messages.
        zstr_send(req, waittimeout);
        zmq_poll(pollitem, 1, timeout);
        if(pollitem->revents & ZMQ_POLLIN){
            zsock_brecv(req, "8", &avail);
            return avail;
        }
    }
    return num;
}

//int64_t HFSubConsumer::lastLatency() const{
//    return lastlatency;
//}

int64_t HFSubConsumer::lastSysTimestamp() const{
    return lasttimestamp;
}


void default_callback_fn(void *data, size_t bytes, void *args){
}

HFSubWorker::HFSubWorker(HighFreqDataType hf, int numthreads, hfs_data_callback_fn userfn, void *args)
    :HighFreqSub(hf, ST_WORKER),
      nthreads(numthreads),
      userfunction(userfn),
      userargs(args)
{
}

HFSubWorker::~HFSubWorker(){

}

void HFSubWorker::initialize(){
    if(!actor)
        actor = zactor_new(distributionController, this);
}

void HFSubWorker::sub_reader(zloop_t *loop){
    zloop_reader(loop, sub, subtopush, this);
}

void HFSubWorker::customCommands(zloop_t *loop, zsock_t *reader, zframe_t *f){

}

void HFSubWorker::distributionController(zsock_t *pipe, void *args){
    zsock_signal(pipe, 0);
    HFSubWorker *self = (HFSubWorker*)args;

    //Create push socket

    self->push = zsock_new(ZMQ_PUSH);
    zsock_bind(self->push, "inproc://push_%p", args);
    self->pullendpoint = zsock_endpoint(self->push);

    //Start loop controlling actor and sub sockets
    zloop_t *loop = zloop_new();
    zloop_reader(loop, pipe, pipeController, self);
    self->sub_reader(loop);

    //Start worker threads
    std::vector<zactor_t*> workers;
    for(int i = 0; i < self->nthreads; i++){
        zactor_t *worker = zactor_new(workerthreadController, self);
        workers.push_back(worker);
    }

    zloop_start(loop);
    zsock_destroy(&self->push);
    zloop_destroy(&loop);
    for(auto worker : workers){
        zactor_destroy(&worker);
    }
}

int HFSubWorker::subtopush(zloop_t *loop, zsock_t *reader, void *arg){
    HFSubWorker *self = (HFSubWorker*)arg;
    zframe_t *frame = zframe_recv(reader);
    zframe_send(&frame, self->push, 0);
    return 0;
}

void HFSubWorker::workerthreadController(zsock_t *pipe, void *args){
    zsock_signal(pipe, 0);
    HFSubWorker *self = (HFSubWorker*)args;
    zsock_t *pull = zsock_new(ZMQ_PULL);
    zsock_connect(pull, self->pullendpoint.c_str(), NULL);
    zloop_t *loop = zloop_new();
    zloop_reader(loop, pull, pulltocallback, self);
    zloop_reader(loop, pipe, pipeController, self);
    zloop_start(loop);
    zsock_destroy(&pull);
    zloop_destroy(&loop);
}

int HFSubWorker::pulltocallback(zloop_t *loop, zsock_t *reader, void *arg){
    HFSubWorker *self = (HFSubWorker*)arg;
    zframe_t *frame = zframe_recv(reader);
    self->userfunction(zframe_data(frame), zframe_size(frame), self->userargs);
    zframe_destroy(&frame);
    return 0;
}

LFPConsumer::LFPConsumer(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs)
    : HFSubConsumer(hf, ringbufsize),
      args(parseargs)
{
    sockType = ST_FILTERED_CONSUMER;
}

LFPConsumer::~LFPConsumer(){

}

void LFPConsumer::initialize(){
    //indices are list of indices for each ntrode requested
    //Here, convert into {startbyte, len} so that nearby indices are collapsed
    std::vector<int> newindices;
    int prev = -2;
    for(auto const i : args.indices){
        if(i-prev == 1){//If i is right after the previous index
            newindices.back()++;
        }
        else{//else its a new separate grouping
            newindices.push_back(i);
            newindices.push_back(1);
        }
        prev = i;
    }
    for(auto const i : newindices) std::cout << "-i" << i;
    std::cout << std::endl;
    args.indices = newindices;
    std::cout << std::endl;
    temp.resize(dt.getByteSize()/sizeof(int16_t));
    HFSubConsumer::initialize();
}

timestamp_t LFPConsumer::getData(int16_t *data){
//    lfpPacket packet;
    readData(temp.data(), temp.size()*sizeof(int16_t));
    size_t pos = 0;
    for(size_t i = 0; i < args.indices.size(); i+=2){
        // memcpy(data+pos, temp.data()+args.indices[i], args.indices[i+1]*sizeof(int16_t));
        std::copy(temp.data()+args.indices[i], 
                  temp.data()+args.indices[i]+args.indices[i+1], 
                  data+pos);
        pos += args.indices[i+1];
    }
    return {*(uint32_t*)temp.data(), lastSysTimestamp()};
}

std::vector<std::string> LFPConsumer::getNTrodesRequested() const{
    return args.dataRequested;
}


SpikesConsumer::SpikesConsumer(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs, int np)
    : HFSubConsumer(hf, ringbufsize),
      args(parseargs),
      maxpoints(np)
{
    sockType = ST_FILTERED_CONSUMER;
    //buffer to hold max possible size of spikes packet
    //ntrodeid + cluster + timestamp + waveformlength (in int2d's) + waveform memory
    tempbufsize = spikePacket::headersize() + sizeof(int) + sizeof(int2d)*maxpoints;
    tempbuffer = new byte[tempbufsize];
}

SpikesConsumer::~SpikesConsumer(){
    delete tempbuffer;
}

void SpikesConsumer::initialize(){
    zsock_set_unsubscribe(sub, "");
    for(size_t i = 0; i < args.indices.size(); i+=2){
        zmq_setsockopt(zsock_resolve(sub), ZMQ_SUBSCRIBE, &args.indices[i], 2*sizeof(int));
    }
    HFSubConsumer::initialize();
}

timestamp_t SpikesConsumer::getData(spikePacket* packet){
    //Can't just memcpy buffer into the packet, have to do it in chunks b/c of std::vector waveform
    //Get "header" info (ntrode, cluster, timestamp)
    readData(tempbuffer, tempbufsize);
    memcpy(packet, tempbuffer, spikePacket::headersize());
    //get waveform points
    int npoints = *(int*)(tempbuffer + spikePacket::headersize());
    int2d* ptr = (int2d*)(tempbuffer + spikePacket::headersize() + sizeof(int));
    packet->points.reserve(npoints);
    packet->points.clear();
    packet->points.insert(packet->points.end(), ptr, ptr+npoints);
    return {packet->timestamp, lastSysTimestamp()};
}

std::vector<std::string> SpikesConsumer::getNTrodesRequested() const{
    return args.dataRequested;
}



AnalogConsumer::AnalogConsumer(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs)
    : HFSubConsumer(hf, ringbufsize),
      temp_buffer(hf.getByteSize()),
      args(parseargs)
{
    sockType = ST_FILTERED_CONSUMER;
}

AnalogConsumer::~AnalogConsumer(){

}

void AnalogConsumer::initialize(){
    //Get interleaved byte location
    il_byte_loc = args.indices.front();
    args.indices.erase(args.indices.begin());

    //Create masks for recognizing which interleaved data user wants
    for(size_t i = 0; i < args.indices.size(); i+=2){
//        int byte = args.indices[i];
        int bit = args.indices[i+1];
        if(bit == -1) continue; //Ignore everything if not interleaved
        if(il_byte_loc < 0){
            //if il byte location is not set, but user wants il data
            std::cerr << "AnalogConsumer error: User requested interleaved data but id byte location not set\n";
            il_byte_loc = 0; // TODO: let user handle error
        }
        uint8_t m = 0;
        m |= 1 << bit; //set the id bit
        il_id_masks.push_back(m);
        prev_interleaved.push_back(0); //set prev interleaved value
    }
    HFSubConsumer::initialize();
}

timestamp_t AnalogConsumer::getData(int16_t *dest){
    if(!readData(temp_buffer.data(), temp_buffer.size())){
        return {0,0};
    }
    byte *src = (byte*)temp_buffer.data() + sizeof(uint32_t);
    int offset = 0;
    int il_i = 0;
    for(size_t i = 0; i < args.indices.size(); i += 2){
        int byt = args.indices[i];
        int bit = args.indices[i+1];
        if(bit == -1){
            //Non interleaved
            dest[offset] = *(int16_t*)(src+byt);
        }
        else{
            //Interleaved
            //Compare mask[il_i] and packet's il id byte
            if(il_id_masks[il_i] & src[il_byte_loc]){
                //If packet contains this type of data
                dest[offset] = *(int16_t*)(src+byt);
                prev_interleaved[il_i] = dest[offset];
            }
            else{
                //Else, use prev il data
                dest[offset] = prev_interleaved[il_i];
            }
            il_i++;
        }
        offset++;
    }
    return {*(uint32_t*)(temp_buffer.data()), lastSysTimestamp()}; //Timestamp at beginning of packet
}

std::vector<std::string> AnalogConsumer::getChannelsRequested() const{
    return args.dataRequested;
}

//data = zframe_data
//bytes = zframe_size
//args = this (hfsubconsumer*)
bool DigitalConsumer::digital_queue_filter(void* data, size_t bytes, void* args){
    DigitalConsumer* self = (DigitalConsumer*)args;
    byte* bdata = (byte*)data+sizeof(uint32_t);// ignore the timestamp at the front
    //The rest of the data should be [0x55, digital bits ... ]
    bool diff = false;
    for(size_t i = 0; i < self->digital_mask.size(); i++){
        if((self->prev_digital[i] ^ bdata[i]) & (self->digital_mask[i])){
            //If masked data (only bits we care about) is different from the previous
            diff = true;
        }
        self->prev_digital[i] = bdata[i];
    }
    return diff;
}

DigitalConsumer::DigitalConsumer(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs)
    : HFSubConsumer(hf, ringbufsize, digital_queue_filter),
      temp_buffer(hf.getByteSize()),
      args(parseargs)
{
    sockType = ST_FILTERED_CONSUMER;
}

DigitalConsumer::~DigitalConsumer(){

}

void DigitalConsumer::initialize(){
    //Create mask for the digital lines we want to look at
    digital_mask.assign(args.indices.front(), 0);
    args.indices.erase(args.indices.begin());
    for(size_t i = 0; i < args.indices.size(); i+=2){
        int byte = args.indices[i];
        int bit = args.indices[i+1];
        digital_mask[byte] |= 1 << bit; //Set the bit
    }

    //Create prevDigital
    prev_digital.assign(digital_mask.size(), 0);

    HFSubConsumer::initialize();
}

//
timestamp_t DigitalConsumer::getData(int16_t *dest){
    if(!readData(temp_buffer.data(), temp_buffer.size())){
        return {0,0};
    }
    byte *src = temp_buffer.data() + sizeof(uint32_t); // start of digital data is after timestamp
    int offset = 0;
    for(size_t i = 0; i < args.indices.size(); i += 2){
        const int byt = args.indices[i];
        const int bit = args.indices[i+1];
        //Unlike the rest, digital data is one bit at a time. May be a waste, but we are converting
        //the 0/1's to int16_t numbers for convenience
        //1<<bit creates mask for bit position, & operation to isolate that bit,
        //shift the bit to the first position to get 0 or 1, store it at the second byte of a int16_t num
        dest[offset] = (int16_t)((src[byt] & (1 << bit)) >> bit); //Convert to 2byte int
        ++offset;
    }
    return {*(uint32_t*)(temp_buffer.data()), lastSysTimestamp()}; //Timestamp at beginning of packet
}

std::vector<std::string> DigitalConsumer::getChannelsRequested() const{
    return args.dataRequested;
}

NeuralConsumer::NeuralConsumer(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs)
    : HFSubConsumer(hf, ringbufsize),
      tempinput(hf.getByteSize()/sizeof(int16_t), 0),
      args(parseargs)
{
    sockType = ST_FILTERED_CONSUMER;
}

NeuralConsumer::~NeuralConsumer(){

}

void NeuralConsumer::initialize(){
    std::vector<int> new_indices;
    int prev = -2;
    for(auto const &i : args.indices){
        if(i-prev == 1){//If i is right after previous index
            new_indices.back()++;
        }
        else{//else its a new separate grouping
            new_indices.push_back(i);
            new_indices.push_back(1);
        }
        prev = i;
    }
    for(auto const &i:new_indices) std::cout << "-" << i;
    std::cout << std::endl;
    args.indices = new_indices;
    HFSubConsumer::initialize();
}

timestamp_t NeuralConsumer::getData(int16_t *dest){
    readData(tempinput.data(), tempinput.size()*sizeof(int16_t));
    int16_t *src = tempinput.data() + sizeof(uint32_t)/sizeof(int16_t); //timestamp is first
    int offset = 0;
//     for(const auto &i : args.indices){
//         //i is the ith channel. so i*sizeof(int16_t) is the offset after the first one.
// //        memcpy(dst+offset, src + i*sizeof(int16_t), sizeof(int16_t));
//         dest[offset] = *(int16_t*)(src+i*sizeof(int16_t));
//         offset++;
//     }
    for(unsigned int i = 0; i < args.indices.size(); i+=2){
        std::copy(src+args.indices[i], src+args.indices[i]+args.indices[i+1], dest+offset);
        offset += args.indices[i+1];
    }
    return {*(uint32_t*)tempinput.data(), lastSysTimestamp()}; //Timestamp at beginning of packet
}

std::vector<std::string> NeuralConsumer::getChannelsRequested() const{
    return args.dataRequested;
}

HFFilteredConsumer::HFFilteredConsumer(HighFreqDataType hf, int ringbufsize, hfs_data_filter_fn *fn, HFParsingInfo parserargs)
    : HFSubConsumer(hf, ringbufsize),
      filter(fn),
      args(parserargs),
      inputsize(dt.getByteSize()),
      input(new byte[inputsize])
{
    sockType = ST_FILTERED_CONSUMER;
}

HFFilteredConsumer::~HFFilteredConsumer(){
    delete [] input;
}

bool HFFilteredConsumer::isValid(){
    return args.indices.size()!=0 && dt.isValid();
}
uint32_t HFFilteredConsumer::getData(int16_t *dest){
    readData(input, inputsize);
    return filter(dest, input, &args, inputsize);
}
