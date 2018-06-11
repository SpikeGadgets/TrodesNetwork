#ifndef HIGHFREQCLASSES_H
#define HIGHFREQCLASSES_H
//#include "networkincludes.h"
#include "networkDataTypes.h"
#include "trodesglobaltypes.h"
#include <concurrentqueue.h>
#include <string>
#include <atomic>
#include <numeric>

struct _zsock_t; typedef _zsock_t zsock_t;
struct _zactor_t; typedef _zactor_t zactor_t;
struct _zloop_t; typedef _zloop_t zloop_t;
struct _zframe_t; typedef _zframe_t zframe_t;
typedef struct zmq_pollitem_t zmq_pollitem_t;
typedef unsigned char byte;
class HighFreqPub {
public:
    HighFreqPub();
    ~HighFreqPub();

    int initialize(std::string addr = "tcp://127.0.0.1", int port = 49153);

    void publishData(void *data, size_t dataSize);

    void publishData(void *data, size_t dataSize, int64_t timestamp);

    std::string getAddress() const;

private:
    std::string sockEndpoint;
    zsock_t *hfPub; //! High Frequency publisher object
};

/*!
 * \brief The HighFreqSub base class
 * Usage: Base class for the two types of high frequency data subscribers
 * Base class contains the data type, actor, subscriber socket
 */
class HighFreqSub {
public:
    enum SubType {ST_NULL, ST_CONSUMER, ST_WORKER, ST_FILTERED_CONSUMER};

    HighFreqSub(HighFreqDataType hf, SubType type = ST_NULL);
    virtual ~HighFreqSub();

    virtual void initialize() = 0;

    //! Returns data type
    HighFreqDataType getType();

    //! Deregister this datatype and destroy sub socket
    int deregister();

    //! Register a new datatype and create sub socket for it
    int registerDataType(HighFreqDataType hf);

    inline SubType getSubSockType(void) {return(sockType);}

protected:
    HighFreqDataType dt;
    zactor_t *actor;
//    bool registered;
    zsock_t *usrpipe;
    zsock_t *sub;
    SubType sockType;
    static int pipeController(zloop_t *loop, zsock_t *reader, void *arg);

    virtual void customCommands(zloop_t *loop, zsock_t *reader, zframe_t *f) = 0;
    virtual void sub_reader(zloop_t *loop) = 0;


private:
};


struct MyTraits : public moodycamel::ConcurrentQueueDefaultTraits
{
    static const size_t BLOCK_SIZE = 256;		// Use bigger blocks
};

typedef bool (*consumer_queue_filter) (void *data, size_t bytes, void *args);
bool default_consumer_queue_filter(void* data, size_t bytes, void *args);
/*!
 * \brief The HFSubConsumer class
 * producer-consumer problem. Bufferwriter called when subscriber socket gets a message
 * readData is the consumer part, and consumes data.
 * Thread safe with atomic ints pointing to indices
 * Implemented with circular buffer. If writer is too fast, then oldest message will be tossed.
 * Incoming messages are always written, but if reader too slow, then not all will necessarily
 * be available to be read
 */
class HFSubConsumer : public HighFreqSub{
public:
    HFSubConsumer(HighFreqDataType hf, size_t bfsize, consumer_queue_filter filter = default_consumer_queue_filter);
    virtual ~HFSubConsumer();
    void initialize();

    //! Copies next data packet to provided dest buffer
    virtual size_t readData(void *dest, size_t size);

    //! returns number of unread data packets
    //! If timeout = 0, return immediately if no messages
    //! If timeout <0, blocking indefinitely
    //! Otherwise, timeout is in milliseconds
    size_t available(long timeout);

    //! Returns the latency of the last data popped by the user
    //! Latency here is measured from when the packet left the publisher to when the user popped the data
//    int64_t lastLatency() const;

    //! Returns the last sys timestamp the last read packet came with
    int64_t lastSysTimestamp() const;

private:
    size_t buffersize;
    size_t msgsize;
    consumer_queue_filter queuefilter;
    moodycamel::ConcurrentQueue<zframe_t*, MyTraits> framequeue;
    moodycamel::ProducerToken ptoken;
    moodycamel::ConsumerToken replacetoken;
    moodycamel::ConsumerToken consumetoken;
    zsock_t *req;
    zsock_t *rep;
    std::string rependpoint;
    bool waiting;
    zmq_pollitem_t *pollitem;
//    int64_t lastlatency;
    int64_t lasttimestamp;
//    unsigned long long totallatency;
//    unsigned long long totalpopped;

    void sub_reader(zloop_t *loop);
    void customCommands(zloop_t *loop, zsock_t *reader, zframe_t *f);
    static int waitTimer(zloop_t *loop, int timer_id, void *arg);
    static void ringbufController(zsock_t *pipe, void *args);
    static int bufferwriter(zloop_t *loop, zsock_t *reader, void *arg);
    static int requesthandler(zloop_t *loop, zsock_t *reader, void *arg);

    int handle_enqueue(zframe_t* frame);
    int handle_request(zframe_t* frame);
};

typedef void (*hfs_data_callback_fn) (void *data, size_t bytes, void *args);
void default_callback_fn(void *data, size_t bytes, void *args);
/*!
 * \brief The HFSubWorker class
 * task workers, distributed computing problem.
 * Creates n threads, and distributes incoming messages to be processed with callback function
 * to each thread.
 * Must be used in cases where order and time is less important.
 * Function passed in to be run must have signature (void *data, size_t bytes, void *args)
 *  - data is the pointer to the data provided, and bytes is num of bytes. (Must copy data if
 * meant to use it later, this data will be destroyed after the function is called)
 *  - args is a pointer to a var/struct/object for the user to use
 *
 * **NOTE: ORDER OF COMPLETION IS -NOT- GUARANTEED BETWEEN THE THREADS
 */
class HFSubWorker : public HighFreqSub {
public:
    HFSubWorker(HighFreqDataType hf, int numthreads, hfs_data_callback_fn userfn, void *args);
    ~HFSubWorker();
    void initialize();

private:
    int nthreads;
    zsock_t *push;
    std::string pullendpoint;
    hfs_data_callback_fn userfunction;
    void *userargs;

    void sub_reader(zloop_t *loop);

    void customCommands(zloop_t *loop, zsock_t *reader, zframe_t *f);
    static void distributionController(zsock_t *pipe, void *args);          //!< Creates push and loop for sub and push sockets
    static int subtopush(zloop_t *loop, zsock_t *reader, void *arg);        //!< Triggers when sub receives, sends through push

    static void workerthreadController(zsock_t *pipe, void *args);          //!< Creates pull and loop for pull socket
    static int pulltocallback(zloop_t *loop, zsock_t *reader, void *arg);   //!< Triggers when pull recieves, calls user callback
};



//! Filter function typedef
//! Takes in dest buffer, src buffer, num bytes, and void* to arguments, stored in some struct
//! Returns number of items copied
typedef uint32_t (hfs_data_filter_fn) (int16_t *dest, void *src, void *args, size_t packetsize);

struct HFParsingInfo
{
    // Indices of each item.
    // TODO: change this to a typedef for the indices. sizeof is not needed?
    std::vector<int> indices;   //!< All purpose array of indices
    size_t sizeOf;              //!< Size of all data in bytes
    std::vector<std::string> dataRequested;  //!< String of data requested by the user
    int dataLength;             //!< Number of data points(lfp and spikes are irregular so = 1)
};


class LFPConsumer : public HFSubConsumer{
public:
    LFPConsumer(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs);
    ~LFPConsumer();
    void initialize();
    timestamp_t getData(int16_t* data);
    std::vector<std::string> getNTrodesRequested() const;

protected:
    HFParsingInfo args;
    std::vector<int16_t> temp;

};


//Problem with spike data is that sometimes ntrodes have variable sizes
//Solution: Since spikes are relatively low frequency, having dynamic lengths of
//waveform points is ok. Underlying implementation may either be to reserve space
//for the worst case scenario, or have the ringbuffer be changed to use bytes (a cache of sorts)
//But the user will get a struct that contains a std::vector, which has reserved space to
//the max number of points possible in an ntrode, but may have less (capacity() vs size())
//When converting to python, a quick copy
class SpikesConsumer : public HFSubConsumer{
public:
    SpikesConsumer(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs, int np);
    ~SpikesConsumer();
    void initialize();
    timestamp_t getData(spikePacket *packet);
    std::vector<std::string> getNTrodesRequested() const;

protected:
    HFParsingInfo args;
    int maxpoints; //!< Maximum number of points in a waveform, where each point is an int2d

private:
    byte* tempbuffer;
    size_t tempbufsize;
};

class AnalogConsumer : public HFSubConsumer{
public:
    AnalogConsumer(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs);
    ~AnalogConsumer();
    void initialize();
    timestamp_t getData(int16_t *dest);
    std::vector<std::string> getChannelsRequested() const;

private:
    //Previous interleaved data. if message popped doesnt have user's desired interleaved
    //data, pass over previnterleaved values
    int il_byte_loc;
    std::vector<byte> il_id_masks; //masks for each il channel
    std::vector<int16_t> prev_interleaved;
    std::vector<byte> temp_buffer;
    HFParsingInfo args;
};

class DigitalConsumer : public HFSubConsumer{
public:
    DigitalConsumer(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs);
    ~DigitalConsumer();
    void initialize();
    timestamp_t getData(int16_t *dest);
    std::vector<std::string> getChannelsRequested() const;

private:
    static bool digital_queue_filter(void* data, size_t bytes, void* args);
    std::vector<byte> digital_mask; //vector of masks, where each mask is a vector of bytes
    std::vector<byte> prev_digital;
    std::vector<byte> temp_buffer;
//    std::vector<int16_t> prevDigital;
    HFParsingInfo args;
};

class NeuralConsumer : public HFSubConsumer{
public:
    NeuralConsumer(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs);
    ~NeuralConsumer();
    void initialize();
    timestamp_t getData(int16_t *dest);
    std::vector<std::string> getChannelsRequested() const;

private:
    std::vector<int16_t> tempinput;
    HFParsingInfo args;
};

class HFFilteredConsumer : public HFSubConsumer
{
public:
    HFFilteredConsumer(HighFreqDataType hf, int ringbufsize, hfs_data_filter_fn* fn, HFParsingInfo parserargs);
    ~HFFilteredConsumer();
    uint32_t getData(int16_t *dest);    //!< returns timestamp, copies data to dest
    bool isValid();
protected:
    hfs_data_filter_fn* filter;  //!< Function to retrieve
    HFParsingInfo args;
    size_t inputsize;
    byte *input;
};



#endif // HIGHFREQCLASSES_H
