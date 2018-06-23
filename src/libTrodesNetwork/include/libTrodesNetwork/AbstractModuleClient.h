#ifndef ABSTRACTMODULECLIENT_H
#define ABSTRACTMODULECLIENT_H
#include "networkincludes.h"
#include <set>

/*! \class AbstractModuleClient
 *  This base class is the layer that receives and stores all Trodes Network specific data and messages.
 *  This includes recording related commands, data types, etc.
 *
 *  Usage:
 *  Inherit from this class.
 *  Receiving messages:
 *      -Optionally reimplement any number of the recv_* functions, which are called when relevant messages come in
 *
 *  Sending messages:
 *      -Call the send* functions to send messages out across the network.
 *
 */
class AbstractModuleClient : public MlmWrap {
public:
    AbstractModuleClient(std::string id, std::string addr, int port);
    AbstractModuleClient(const char *id, const char* addr, int port);
    ~AbstractModuleClient(void);


    /*! \brief subscribeToEvent causes recv_event() to be called whenever a subscribed event is received
     * \param origin The module id that produces the event
     * \param event The name of the event */
    void subscribeToEvent(std::string origin, std::string event);

    /*! \brief unsubscribeFromEvent removes this module from "mailing list" of this event*/
    void unsubscribeFromEvent(std::string origin, std::string event);


    /*! \brief sendMsgToTrodes sends direct message to trodes, who only recognizes subject TRODES_CMD at the moment
     * \param subject At the moment should be TRODES_CMD
     * \return 0 if success, otherwise failed*/
    int sendMsgToTrodes(std::string subject, TrodesMsg &msg);


    /*! \brief sendMsgToModule sends direct message to another module*/
    int sendMsgToModule(std::string module, std::string subject, TrodesMsg &msg);


    /*! \brief sendOutEvent publishes an event out to whoever is listening*/
    int sendOutEvent(const std::string &event, TrodesMsg &msg);


    //! \brief Returns latest trodes timestamp
    uint32_t latestTrodesTimestamp();

    //! \brief sendTimeRequest asks Trodes for the current timestamp
    void sendTimeRequest();

    //! \brief sendTimeRateRequest asks trodes for the time rate
    void sendTimeRateRequest();

    void sendPlaybackCommand(std::string cmd, uint32_t time);

    //! \brief Subscribe to a high frequency data type of name 'dataName' from the module 'originModule'
    //! This function will check to see that both the requested type exists and is available to be subscribed
    //! to as well as whether or not a subscription to the requested type already exists.  If available and
    //! not curretnly subscribed to, the function will create a new HighFreqSub and return a pointer to it.
    //! If the subscriber already exists, the function will simply return a pointer to the HighFreqSub.  If
    //! for any reason the subscriber could not be created, the function will return a NULL ptr.
    HFSubConsumer *subscribeHighFreqData(std::string dataName, std::string originModule, size_t messageBufferLength = 10);
    HFSubWorker *subscribeHighFreqData(std::string dataName, std::string originModule, hfs_data_callback_fn userFoo, void *args = NULL);


    //! \brief Unsubscribe to a high frequency data type of name 'dataName' from the module 'originModule'
    //! Returns 0 if the type was successfully unsubscribed.  Returns -1 if the subscription did not exist or
    //! the unsubscribe action could not be completed.
    int unsubscribeHighFreqData(std::string dataName, std::string originModule);


    //! \brief Get all available data types
    //! In c++, use hfType_*
    std::vector<std::string> getAvailableTrodesData(std::string data);

    //! \brief subscribe to Neural data
    NeuralConsumer* subscribeNeuralData(size_t buffersize, std::vector<std::string> channels);

    //! \brief Subscribe to LFP Data
    LFPConsumer* subscribeLFPData(size_t buffersize, std::vector<std::string> ntrodes);

    //! \brief Subscribe to Spike data
    SpikesConsumer* subscribeSpikeData(size_t buffersize, std::vector<std::string> ntrodes);

    //! \brief Subscribe to digital data
    DigitalConsumer* subscribeDigitalData(size_t buffersize, std::vector<std::string> channels);

    //! \brief Subscribe to Analog data
    AnalogConsumer* subscribeAnalogData(size_t buffersize, std::vector<std::string> channels);


    //! \brief Sends an annotation request to trodes containing the message msg. Trodes timestamp is automatically
    //! included in the message as well as this module's name
    void sendAnnotationRequest(std::string msg);

    //! \brief initializeHardwareConnection instantiates req socket, and subscribes to hardware updates mlm stream
    bool initializeHardwareConnection();

    //! \brief deletes socket and unsubs from updates
    void destroyHardwareConnection();

    //! \brief send settle command to hardware
    bool sendSettleCommand();

    //! \brief sends the parameters via StimulationControl object to Trodes
    bool sendStimulationParams(StimulationCommand command);

    //! \brief sendClearStimulationParams
    bool sendClearStimulationParams(uint16_t slot);

    //! \brief send Stimulation Start Slot
    bool sendStimulationStartSlot(uint16_t slot);

    //! \brief send Stimulation Start Group
    bool sendStimulationStartGroup(uint16_t group);

    //! \brief send Stimulation Stop Slot
    bool sendStimulationStopSlot(uint16_t slot);

    //! \brief send Stimulation Stop Group
    bool sendStimulationStopGroup(uint16_t group);

    //! \brief send globalstimulationsettings
    bool sendGlobalStimulationSettings(GlobalStimulationSettings settings);

    //! \brief send globalstimulationcommand
    bool sendGlobalStimulationCommand(GlobalStimulationCommand command);

    //Variable names omitted to avoid a million compile warnings of parameter unused
    virtual void recv_file_open  (std::string){} //(filename)
    virtual void recv_file_close (){}
    virtual void recv_acquisition(std::string, uint32_t){} //(acquisition type, timestamp)
    virtual void recv_source     (std::string){} //(sourcename)
    virtual void recv_quit       (){}
    virtual void recv_time       (uint32_t t){} //(timestamp)
    virtual void recv_timerate   (int){} //(timerate)
    //! Event received. Reimplement to write in if/else statements for events
    virtual void recv_event      (std::string, std::string, TrodesMsg &){} //(origin, event, msg)
protected:

private:

    int processCommandMsg(std::string cmdType, TrodesMsg &msg);
    int processEventMsg(const char *sender, const char *event, TrodesMsg &msg);
    int processRequestMsg(const char *, std::string reqType, TrodesMsg &);
    int processReplyMsg(std::string repType, TrodesMsg &msg);
    int processNotification(const char *sender, std::string noteType, TrodesMsg &msg);
    int processTimer(int timer_id);
    void initializeStreams();

    std::set<std::string> subbedEvents;

    zsock_t* timestampsub;
    std::string timestampaddress;
    uint32_t lastTimestamp = 0;
    int64_t lastsysTimestamp = 0;
    bool subToTimestamps(const std::string &address);
    bool unsubToTimestamps();
};

#endif // ABSTRACTMODULECLIENT_H
