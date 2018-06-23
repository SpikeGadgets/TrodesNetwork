#ifndef NETWORKINCLUDES_H
#define NETWORKINCLUDES_H
// #define __USE_MINGW_ANSI_STDIO 0
// #include <malamute.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <mutex>

#include "CZHelp.h"
#include "networkDataTypes.h"
#include "trodesmsg.h"
#include "highfreqclasses.h"
#include "trodesglobaltypes.h"

struct _mlm_client_t; typedef _mlm_client_t mlm_client_t;
//*********************************************************
//API LEVEL DEFINES
//*********************************************************

//request/reply message types
#define     INFO_TIME            "infoTime"
#define     INFO_TIMERATE        "infoTR"
#define     INFO_CONFIG          "infoConf"  //request/replies related to configuration information
#define     ANNOTATION_REQ       "annotate"

//notification types
#define     NOTE_CONFIG_CHANGED  "config_changed"
#define     hardware_update      "HUPD"




/*! \struct HFSubSockSettings
 * Struct that contains all settings related to HighFreqSub sockets.
 * Specifically, these objects are used by the module's MlmWrapper to retain all
 * information about currently active subscriptions so the these subscriptions
 * can be dynamically recreated without user input.
 */
struct HFSubSockSettings {
    HFSubSockSettings(HighFreqDataType hf, HighFreqSub::SubType type, hfs_data_callback_fn foo, void *arr)
        : dataType(hf), subSockType(type), userFoo(foo), args(arr){}
    HFSubSockSettings(){}
    HighFreqDataType dataType;
    HighFreqSub::SubType subSockType;

    //worker specific stuff.
    hfs_data_callback_fn userFoo;
    void *args;
};




class network_pimpl : public NetworkDataType
{
public:
    network_pimpl();
    ~network_pimpl();
    std::string getEndpoint() const;
    void setEndpoint(const std::string &value);

    std::string getAddress() const;
    void setAddress(const std::string &value);

    int getPort() const;
    void setPort(int value);

    void insert_client(const std::string &value);
    void remove_client(const std::string &value);
    bool client_exists(const std::string &value) const;
    std::set<std::string> getClients() const;

    void insert_event(const EventDataType &value);
    void remove_event(const EventDataType &value);
    std::vector<EventDataType> getEvents() const;

    void insert_hfdt(const HighFreqDataType &value);
    void remove_hfdt(const HighFreqDataType &value);
    HighFreqDataType find_hfdt(const std::string &name, const std::string &origin);
    bool hfdt_exists(const HighFreqDataType &value);
    bool hfdt_exists(const std::string &name, const std::string &origin);
    std::vector<HighFreqDataType> getHfdts() const;

    TrodesSource getTrodes_source() const;
    void setTrodes_source(const TrodesSource &value);

    TrodesAcquisition getTrodes_acquisition() const;
    void setTrodes_acquisition(const TrodesAcquisition &value);

    std::string getTrodes_filename() const;
    void setTrodes_filename(const std::string &value);

    std::string getHardware_endpoint() const;
    void setHardware_endpoint(const std::string &value);

    std::string getHardware_address() const;
    void setHardware_address(const std::string &value);

    int getHardware_port() const;
    void setHardware_port(int value);

    binarydata encode() const;
    void decode(const binarydata &data);

    std::string getTimestamp_endpoint() const;
    void setTimestamp_endpoint(const std::string &value);

private:
    std::string                     endpoint;
    std::string                     address;
    int                             port;
    std::set<std::string>           clients;

    std::vector<EventDataType>      events;
    std::vector<HighFreqDataType>   hfdts;

    TrodesSource                    trodes_source;
    TrodesAcquisition               trodes_acquisition;
    std::string                     trodes_filename;

    std::string                     hardware_endpoint;
    std::string                     hardware_address;
    int                             hardware_port;

    std::string                     timestamp_endpoint;
//    std::vector<HFSubSockSettings>  subbed_hfdts;
//    std::vector<HighFreqDataType>   provided_hfdts;
//    std::vector<HighFreqSub*>       hf_subscribers;
};
/*! \class MlmWrap
 * Malamute wrapper to be used in Trodes server as well as module clients
 * This serves as the message handler, as well as the interface connecting
 * ZeroMQ and Qt, or any other framework. The public functions should never contain anything related
 * to zeromq.
 */
class MlmWrap {
public:
    //Public interface

    //! Constructor
    //! Has default values for address, port, and iothreads
    //! Will also take care of the following zsys settings: SIGTERM handler, HWM's, logging
    MlmWrap(const char* _id, const char *addr, int ePort);
    MlmWrap(const char* _id, const char *e = DEFAULT_SERVER_ENDPOINT);

    //! Destructor
    //! calls destroy function for mlm_client.
    virtual ~MlmWrap();

    //! Connects to broker, starts zactor and zloop
    int initialize();

    //! Public accessor to check whether or not the object has been successfully initialized and connected to the network
    bool isInitialized() const;

    bool isConnectedToBroker() const;

    //! Get ID of client
    std::string getID() const;

    //! Access the Trodes Configuration information
    TrodesConfig getTrodesConfig();

    //! Ask if the Trodes Configuration is valid
    bool isTrodesConfigValid() const;

    //! Get network server endpoint
    std::string getEndpoint() const;

    //! Get network server address
    std::string getAddress() const;

    //! Get network server port
    int getPort() const;

    //! Get all clients on network
    std::vector<std::string> getClients();

    //! Get the configuration information
    void getConfigInfo(void);

    //! \brief Register published data with a module's network client object
    //! This function registers a dedicated data publisher with a module's client, allowing that
    //! information to be sent to other modules and connected too.  This function also sends a
    //! message to all the other modules on the network that the data type is available.
    int registerHighFreqData(HighFreqDataType dataType);

    //! \brief Deregister published data with a module's network client object
    //! This funtion deregisters a dedicated data publisher with a module's client and sends a
    //! message to all other modules on the network that the data type is no longer available
    int deregisterHighFreqData(HighFreqDataType dataType);

    //! Provide an event that is broadcast on this client's stream and modules can subscribe to
    int provideEvent(const char *event);

    //! "Unprovide" an event broadcast on this clients stream
    int unprovideEvent(const char *event);

    //! Request a client for their events list
    std::vector<EventDataType> getEventList();

    //! Get events list
    std::vector<HighFreqDataType> getHighFreqList();

protected:

    //! Sends message to a stream (a client can only send to one stream)
    int sendStream(const char *subject, TrodesMsg &tmsg);

    //! Sends message to mailbox of another client
    int sendMessage(const char *address, const char *subject, TrodesMsg &tmsg, const char *tracker = "", uint32_t timeout=0);

    //! Sends message to the mailboxes of all other clients
    int sendMessageToAll( const char *subject, TrodesMsg &tmsg, const char *tracker = "", uint32_t timeout=0);

    //! Sends service request to another client
    int sendRequest(const char *address, const char *subject, TrodesMsg &tmsg, const char *tracker = "", uint32_t timeout=0);

    //! Send event out to stream
    int sendEvent(const std::string &event, TrodesMsg &tmsg);
    //! Virtual functions
    /*! These virtual functions must be defined. They are called when receiving messages
     * User must decode the content themselves. In addition, member variables used should be kept to
     * a minimum. These virtual functions will be called from the actor thread, not the main thread that
     * sending messages come from.
     */
    virtual int processCommandMsg(std::string cmdType, TrodesMsg &msg) {return(0);}
    virtual int processEventMsg(const char *sender, const char *event, TrodesMsg &msg) {return(0);}
    virtual int processRequestMsg(const char *sender, std::string reqType, TrodesMsg &msg) {return(0);}
    virtual int processReplyMsg(std::string repType, TrodesMsg &msg) {return(0);}
    virtual int processNotification(const char *sender, std::string noteType, TrodesMsg &msg) {return(0);}

    virtual int processOtherMsg(const char *sender, const char *subject, TrodesMsg &msg) {std::cout << "Other msg type [" << subject << "] not recognized.\n"; return(0);}
    virtual int processTimer(int timer_id){return 0;}
    //!Subscribes to a stream
    int subscribeStream(const char *stream, const char *pattern);

    //! Unsubscribes from all patterns of a stream
    int removeSubscriptions(const char *stream);

    //!Subscribes to an event
    int subscribeEvent(const std::string &origin, const std::string &event);

    //! \brief Assigns a stream for itself to write to.
    //! \details All clients should have their own stream to write to, for their events
    int setProducer(const char *stream);

    //! \brief Provides a service for others to use.
    //! Can include requests for certain data, requests to perform an action, etc
    //! Can be called any number of times
    int provideService(const char *regExpr);

    int sendStream(const char *subject, const char *picture, ...);
    int sendStream(const char *subject, const char *picture, va_list argptr);
    int sendMessage(const char *address, const char *subject, const char *tracker, uint32_t timeout, const char* picture, ...);
    int sendMessage(const char *address, const char *subject, const char *tracker, uint32_t timeout, const char* picture, va_list argptr);
    int sendRequest(const char *address, const char *subject, const char *tracker, uint32_t timeout, const char* picture, ...);
    int sendRequest(const char *address, const char *subject, const char *tracker, uint32_t timeout, const char* picture, va_list argptr);

    //Convenience functions for all things high freq
    bool isHfTypeCurrentlySubbed(std::string dataName, std::string moduleOrigin);
    int addHfTypeToSubbedList(HFSubSockSettings socketInfo);
    int removeHfTypeFromSubbedList(HighFreqDataType rData);
    HFSubSockSettings getSubbedHFSockSettings(std::string dataName, std::string moduleOrigin);
    HighFreqDataType getSubbedHfType(std::string dataName, std::string moduleOrigin);
    int addSubToList(HighFreqSub *newSub);
    int removeSubFromList(HighFreqDataType subType); //this will also delete the subscriber
    HighFreqSub* getHfSubObject(std::string dataName, std::string originModule);
    HFSubConsumer *createConsumerSub(HighFreqDataType dt, size_t messageBufferLength);
    HFSubWorker *createWorkerSub(HighFreqDataType dt, int numThreads, hfs_data_callback_fn userFoo, void *args);

    //Create Trodes data parsing info
    //! \brief Create Neural data parsing info given list of ntrode,nthchannel
    //! ex: < "2, 3", "4, 1", "31, 1" > (ntrode 2 3rd channel, ntrode4 1st channel, ntrode31 1ist channel
    HFParsingInfo createNeuralParsingInfo(std::vector<std::string> channels, std::string dataformat);

    //! \brief Create LFP data parsing info given list of ntrode id's
    //! ex: < 2, 3, 4, 5 > (ntrodes 2,3, 4, 5)
    HFParsingInfo createLFPParsingInfo(std::vector<std::string> ntrodes, std::string dataformat);

    //! \brief Create analog parsing info given a list of device,channel names
    HFParsingInfo createAnalogParsingInfo(std::vector<std::string> channels, std::string dataformat);

    //! \brief Create digital parsing info given a list of device,channel names
    HFParsingInfo createDigitalParsingInfo(std::vector<std::string> channels, std::string dataformat);

    //! \brief Create Spikes parsing info given a list of ntrode,cluster
    //! ex: <"2,0", "4,2"> ntrode 2, unclustered, and ntrode 4 cluster 2
    HFParsingInfo createSpikesParsingInfo(std::vector<std::string> clusters, std::string dataformat);

    //! Block until a tracker string returns (not mlm implemented, self implemented)
    //! Returns rest of msg if successful
    //! Returns null if timeout or unsuccessful
    bool blockforreply(std::string tracker, uint32_t timeoutms);

    //! set Trodes Config
    void setTrodesConfig(TrodesConfig _config);

    //! ID of client in the network
    std::string            id;
    //! Internal state of network object
    network_pimpl   *state;

    //! \brief Trodes will create a router socket here and modules will create a req socket
    bool init_hardware_connection();
    void destroy_hardware_connection();

    //! \brief send message to trodes hardware socket
    //! send stim params, clear params, start slot, start group, stop slot, stop group
    //! \return verification from trodes if successfully received
    bool sendHardwareMessage(TrodesMsg &msg);

    std::string error(const std::string s);

    mlm_client_t    *client;        //!< Malamute client
private:
    //Private member variables
    zloop_t         *reactor;       //!< ZLoop that handles incoming and outgoing messages
    zactor_t        *actor;         //!< ZActor thread that holds the zloop
    zsock_t         *usrpipe;       //!< ZSock that gets called when user sends out msg
    zsock_t         *mlmpipe;       //!< ZSock that gets called when mlm has a message for the client
    zsock_t         *hardwaresock;  //!< Direct socket connection between trodes and modules

    //TODO: move trackers to state
    std::map<std::string, int> trackers; //!< List of trackers for blocking until a reply arrives
    bool            initialized;   //!< Track whether or not the client's actor has been initialized
    bool            connected;     //!< Track whether or not the client was able  to connect to the broker

    //TODO: move config to state
    TrodesConfig    config;        //!< Trodes Configuration information tracked by all modules

    //TODO: move to abstractmoduleclient, access using process* functions?
    std::vector<HFSubSockSettings> subbedHfDataTypes; //!< List of all high frequency data types currently subscribed to by this module
    std::vector<HighFreqSub*> hfSubs;           //!< List of all hf subscriptions

    //TODO: is this necessary?
    std::mutex actormutex;

    //Helper functions
    //TODO: organize this mess
    void create(const char* _id, const char *e);
    bool checkBrokerConnected(const char *endpoint, int tries, mlm_client_t *temp, int timeout = 1000);
    void verifyID(mlm_client_t *temp);
    std::vector<std::string> getCurConnectedClients(mlm_client_t* temp);
    HFFilteredConsumer* createFilteredConsumer(HighFreqDataType dt, size_t buffersize, hfs_data_filter_fn* filter, HFParsingInfo parser);
    bool verifyTrodesDataFormat(std::vector<int> &startlengths, std::string dataformat);
    NDevice findTrodesDevice(std::string dv);
    NDeviceChannel findTrodesChannel(const NDevice &device, std::string ch);
    int calculateDeviceChannelInfoIndices(std::vector<int> &indices, std::vector<int> &startlengths, int startbyte, int bit);

    //Private member functions
    int receiveStream(const char *sender, const char *subject, const char *format, TrodesMsg &msg);
    int receiveMailbox(const char *sender, const char *subject, const char *format, TrodesMsg &msg);
    int receiveService(const char *sender, const char *subject, const char *format, TrodesMsg &msg);

    int processMlmCommandMsg(TrodesMsg &msg); //currently only routes commands from TRODES, chane the receiveStream implementation if you want to send cmds from other modules
    int processMlmEventMsg(const char *sender, const char *event, TrodesMsg &msg);
    int processMlmNetworkNotification(const char *sender, TrodesMsg &msg);
    int processMlmRequestMsg(const char *sender, TrodesMsg &msg);
    int processMlmReplyMsg(TrodesMsg &msg);


    //! Calls appropriate virtual functions for stream messages
    //! Return 0. only return any other number if critical error.
    int route_stream_msgs(const char *address, const char *sender, const char *subject, zmsg_t *msg);

    //! Calls appropriate functions for mailbox messages
    //! Return 0. only return any other number if critical error.
    int route_mailbox_msgs(const char *address,
                            const char *sender,  const char *subject, const char *tracker, zmsg_t *msg);

    //! Calls appropriate virtual functions for service messages
    //! Return 0. only return any other number if critical error.
    int route_service_msgs(const char *address,
                           const char *sender, const char *subject, const char *tracker, zmsg_t *msg);


    //Static function handles
    //!Zloop task
    /*! Zloop task function running on zactor that will create zloop, connect handlers,
     * and keep running as long as client is alive.
     */
    static void message_reactor_task(zsock_t *pipe, void *args);


    //! Incoming message handler
    /*! Function will get called when message appears on socket, call mlm_client_recv,
     *  parse message type, and call appropriate virtual functions
     */
    static int handle_mlm_msgpipe(zloop_t *loop, zsock_t *reader, void *arg);


    //! Outgoing message handler
    /*! Function will get called when user calls the sendMessage function, which sends message
     * to inproc pipe, which triggers this function. It will then package everything to send
     * using either send, sendto, or sendfor. Must go through this to avoid race conditions and using
     * the mlm sockets at the same time for incoming and outgoing
     */
    static int handle_usr_msgpipe(zloop_t *loop, zsock_t *reader, void *arg);

    static int handle_abs_timer(zloop_t *loop, int timer_id, void *arg);

    static int block_reply_timeout(zloop_t *loop, int timer_id, void *arg);
    static int recv_hardware_request(zloop_t *loop, zsock_t *reader, void *arg);
};

class BrokerClient;

/*! \class CentralBroker
 *  The CentralBroker class used only in Trodes to launch and contain a malamute broker.  This class
 *  also contains an attached client that is used by other clients to communicate directly to the
 *  broker.
 */
class CentralBroker {
public:
    CentralBroker(const char *endP);
    CentralBroker(const char *address, int port);
    void create(const char *endP);
    ~CentralBroker(void);


    std::string getEndpoint() const;
    std::string getAddress() const;
    int getPort() const;

private:
    zactor_t *broker;
    zactor_t *logger;
    BrokerClient *brokerHandle; //the brokerHandle handles all direct information queries to/from the central broker

    std::string endpoint;
    std::string address;
    int port;
    static void logging_reactor_task(zsock_t *pipe, void *args);
    static int logging_shutdown_pipe(zloop_t *loop, zsock_t *reader, void *arg);
    static int logging_forward_log(zloop_t *loop, zsock_t *reader, void *arg);
};

/*! \class BrokerClient
 *  The BrokerClient class used by the CentralBroker class as a communication access point.
 *  Other clients attached to the CentralBroker use the BrokerClient to communicate directly
 *  directly with the CentralBroker.
 *  This is a separate client so that in the future, multiple instances of trodes can be used.
 */
class BrokerClient : public MlmWrap {
public:
    BrokerClient(const char *id, const char *addr, int port, zactor_t *broker = NULL);
    void start();
    void sendNotification(TrodesMsg& msg);
    ~BrokerClient(void);

protected:
    int processOtherMsg(const char *sender, const char *subject, TrodesMsg &msg);

private:
    zactor_t *brokerObj;
};



#endif // NETWORKINCLUDES_H
