#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include "libTrodesNetwork/AbstractModuleClient.h"
#include "libTrodesNetwork/highfreqclasses.h"
#include "libTrodesNetwork/networkincludes.h"
using namespace boost::python;
namespace np = boost::python::numpy;

/*
This RAII structure ensures that threads created on the native C side
adhere to the laws of Python and ensure they grab the GIL lock when
calling into python

https://stackoverflow.com/questions/41346453/c-boost-python-class-overriding
https://docs.python.org/3/c-api/init.html#c.PyEval_InitThreads
*/
struct PyLockGIL
{

    PyLockGIL()
        : gstate(PyGILState_Ensure())
    {
    }

    ~PyLockGIL()
    {
        PyGILState_Release(gstate);
    }

    PyLockGIL(const PyLockGIL&) = delete;
    PyLockGIL& operator=(const PyLockGIL&) = delete;

    PyGILState_STATE gstate;
};


//https://stackoverflow.com/questions/5314319/how-to-export-stdvector
template<class T>
struct vecToList{
    static PyObject* convert(const std::vector<T>& vec){
        list* l = new list();
        for(size_t i = 0; i < vec.size(); i++){
            l->append(vec[i]);
        }
        return l->ptr();
    }
};

struct EventVecToDict{
    static PyObject* convert(const std::vector<EventDataType>& vec){
        list* l = new list();
        for(size_t i = 0; i < vec.size(); i++){
            dict d;
            d["event"] = vec[i].getName();
            d["origin"] = vec[i].getOrigin();
            l->append(d);
        }
        return l->ptr();
    }
};
template<class T>
std::vector<T> listToVec(list &l){
    std::vector<T> vec;
    int len = extract<int>(l.attr("__len__")());
    for(int i = 0; i < len; i++){
        vec.push_back(extract<T>(l[i]));
    }
    return vec;
}

struct TrodesMsgToList{
    static PyObject* convert(const TrodesMsg& m){
        TrodesMsg msg = m;
        list* l = new list();
        while(msg.numContents()){
            switch (msg.getformat()[0]) {
            case 'i':{
                int a;
                msg.popcontents("i",a);
                l->append(a);
                break;}
            case '1':{
                uint8_t a;
                msg.popcontents("1", a);
                l->append(a);
                break;}
            case '2':{
                uint16_t a;
                msg.popcontents("2", a);
                l->append(a);
                break;}
            case '4':{
                uint32_t a;
                msg.popcontents("4", a);
                l->append(a);
                break;}
            case '8':{
                uint64_t a;
                msg.popcontents("8", a);
                l->append(a);
                break;}
            case 's':{
                std::string a;
                msg.popcontents("s", a);
                l->append(a);
                break;}
            case 'b':{
                std::vector<byte> a;
                msg.popcontents("b", a);
                l->append(a);
                break;}
            case 'd':{
                double a;
                msg.popcontents("d", a);
                l->append(a);
                break;}
//            case 'n':{
//                NetworkDataType a;
//                msg.popcontents("n", a);
//                l->append(a);
//                break;}
            default:
                std::cout << "Invalid format type " << msg.getformat()[0] << "!\n";
                return l->ptr();
                break;
            }
        }
        return l->ptr();
    }
};

struct lfpPacketToList{
    static PyObject* convert(const lfpPacket& m){
        list* l = new list();
        l->append(m.ntrodeid);
        l->append(m.timestamp);
        l->append(m.value);
        return l->ptr();
    }
};
TrodesMsg new_TrodesMsg(str f, list& l){
    TrodesMsg msg;
    std::string format = extract<std::string>(f);
    int strl = format.length();
    for(int i = 0; i < strl; i++){
        switch (format[i]){
        case 'i':{
            int a = extract<int>(l[i]);
            msg.addcontents("i", a);
            break;}
        case '1':{
            uint8_t a = extract<uint8_t>(l[i]);
            msg.addcontents("1", a);
            break;}
        case '2':{
            uint16_t a = extract<uint16_t>(l[i]);
            msg.addcontents("2", a);
            break;}
        case '4':{
            uint32_t a = extract<uint32_t>(l[i]);
            msg.addcontents("4", a);
            break;}
        case '8':{
            uint64_t a = extract<uint64_t>(l[i]);
            msg.addcontents("8", a);
            break;}
        case 's':{
            std::string a = extract<std::string>(l[i]);
            msg.addcontents("s", a);
            break;}
        case 'b':{
            std::vector<byte> a = extract<std::vector<byte> >(l[i]);
            msg.addcontents("b", a);
            break;}
//            case 'n':{
//                NetworkDataType a;
//                msg.popcontents("n", a);
//                l.append(a);
//                break;}
        default:
            break;
        }
    }
    return msg;
}


class LFPConsumer_python : public LFPConsumer, public wrapper<LFPConsumer>{
public:
    LFPConsumer_python(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs)
        : LFPConsumer(hf, ringbufsize, parseargs),
          data(parseargs.indices.size())
    {
    }

    np::ndarray create_numpy_array(){
        list l;
//        l.append(boost::python::make_tuple("ntrodeid", "i4"));
//        l.append(boost::python::make_tuple("timestamp", "u4"));
//        l.append(boost::python::make_tuple("lfp", "i2"));
        return np::from_data(data.data(), np::dtype::get_builtin<int16_t>(),
                             boost::python::make_tuple(data.size()),
                             boost::python::make_tuple(sizeof(int16_t)),
                             owner);
    }
    timestamp_t getData_python(){
        return getData(data.data());
    }

private:
    std::vector<int16_t> data;
    boost::python::object owner;
};

class SpikesConsumer_python : public SpikesConsumer, public wrapper<SpikesConsumer>{
public:
    SpikesConsumer_python(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs, int np)
        : SpikesConsumer(hf, ringbufsize, parseargs, np),
          data(new spikePacket),
          spikearray(np::empty(boost::python::make_tuple(1), np::dtype::get_builtin<int16_t>()))
    {
        data->ntrodeid=0;
        data->cluster=0;
        data->timestamp=0;
        data->points.reserve(np);
    }

    np::ndarray create_numpy_array(){
        //x = np.zeros(1, dtype='int32, int32, uint32, npint16')
        list l;
        l.append(boost::python::make_tuple("ntrodeid", "i4"));
        l.append(boost::python::make_tuple("cluster", "i4"));
        l.append(boost::python::make_tuple("timestamp", "u4"));
        l.append(boost::python::make_tuple("waveform", "2i2",maxpoints));
        spikearray = np::zeros(boost::python::make_tuple(1), np::dtype(l));
        return spikearray;
    }

    timestamp_t getData_python(){
        timestamp_t t = getData(data);
        char* npd = spikearray.get_data();
        memcpy(npd, data, spikePacket::headersize());
        memset(npd+spikePacket::headersize(), 0, maxpoints*sizeof(int2d));
        memcpy(npd+spikePacket::headersize(), data->points.data(), (data->points.size())*sizeof(int2d));
        return t;
    }
private:
    spikePacket *data;
//    boost::python::object owner;
    np::ndarray spikearray;
};

class AnalogConsumer_python : public AnalogConsumer, public wrapper<AnalogConsumer>{
public:
    AnalogConsumer_python(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs)
        : AnalogConsumer(hf, ringbufsize, parseargs),
          data(parseargs.indices.size()/2, 0){}
    np::ndarray create_numpy_array(){
        return np::from_data(data.data(), np::dtype::get_builtin<int16_t>(),
                             boost::python::make_tuple(data.size()),
                             boost::python::make_tuple(sizeof(int16_t)),
                             owner);
    }
    timestamp_t getData_python(){
        return getData(data.data());
    }
private:
    std::vector<int16_t> data;
    boost::python::object owner;
};

class DigitalConsumer_python : public DigitalConsumer, public wrapper<DigitalConsumer>{
public:
    DigitalConsumer_python(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs)
        : DigitalConsumer(hf, ringbufsize, parseargs),
          data(parseargs.indices.size()/2, 0){}
    np::ndarray create_numpy_array(){
        return np::from_data(data.data(), np::dtype::get_builtin<int16_t>(),
                             boost::python::make_tuple(data.size()),
                             boost::python::make_tuple(sizeof(int16_t)),
                             owner);
    }
    timestamp_t getData_python(){
        return getData(data.data());
    }
private:
    std::vector<int16_t> data;
    boost::python::object owner;
};

class NeuralConsumer_python : public NeuralConsumer, public wrapper<NeuralConsumer>{
public:
    NeuralConsumer_python(HighFreqDataType hf, int ringbufsize, HFParsingInfo parseargs)
        : NeuralConsumer(hf, ringbufsize, parseargs),
          data(parseargs.indices.size(), 0){}
    np::ndarray create_numpy_array(){
        return np::from_data(data.data(), np::dtype::get_builtin<int16_t>(),
                             boost::python::make_tuple(data.size()),
                             boost::python::make_tuple(sizeof(int16_t)),
                             owner);
    }
    timestamp_t getData_python(){
        return getData(data.data());
    }

private:
    std::vector<int16_t> data;
    boost::python::object owner;
};

class HFSubConsumer_python : public HFSubConsumer, public wrapper<HFSubConsumer>{
public:
    HFSubConsumer_python(HighFreqDataType hfdt, int bufsize) : HFSubConsumer(hfdt, bufsize){}
    size_t readData_python(np::ndarray buffer){
//        return readData(PyMemoryView_GET_BUFFER(buffer)->buf, size);
        return readData(buffer.get_data(), buffer.get_dtype().get_itemsize());
    }
};

//http://www.boost.org/doc/libs/1_65_1/libs/python/doc/html/reference/high_level_components/boost_python_wrapper_hpp.html
class PythonModuleClient : public AbstractModuleClient, public wrapper<AbstractModuleClient>{
public:
    PythonModuleClient(std::string id, std::string addr, int port) : AbstractModuleClient(id, addr, port){
    }
    void recv_acquisition(std::string cmd, uint32_t time){
        PyLockGIL lock;
        if(override recv_acquisition = this->get_override("recv_acquisition")){
            try{
                recv_acquisition(cmd, time);
            }
            catch (const error_already_set&){
                std::cout << "Exception in recv_acquisition in application";
                PyErr_Print();
            }
            return;
        }
        AbstractModuleClient::recv_acquisition(cmd, time);
    }
    void default_recv_acquisition(std::string cmd, uint32_t time){
        this->AbstractModuleClient::recv_acquisition(cmd, time);
    }

    void recv_file_open(std::string filename){
        PyLockGIL lock;
        if(override recv_file_open = this->get_override("recv_file_open")){
            try{
                recv_file_open(filename);
            }
            catch (const error_already_set&){
                std::cout << "Exception in recv_file_open in application";
                PyErr_Print();
            }
            return;
        }
        AbstractModuleClient::recv_file_open(filename);
    }
    void default_recv_file_open(std::string filename){
        this->AbstractModuleClient::recv_file_open(filename);
    }

    void recv_file_close(){
        PyLockGIL lock;
        if(override recv_file_close = this->get_override("recv_file_close")){
            try{
                recv_file_close();
            }
            catch (const error_already_set&){
                std::cout << "Exception in recv_file_close in application";
                PyErr_Print();
            }
            return;
        }
        AbstractModuleClient::recv_file_close();
    }
    void default_recv_file_close(){
        this->AbstractModuleClient::recv_file_close();
    }

    void recv_source(std::string source){
        PyLockGIL lock;
        if(override recv_source = this->get_override("recv_source")){
            try{
                recv_source(source);
            }
            catch (const error_already_set&){
                std::cout << "Exception in recv_source in application";
                PyErr_Print();
            }
            return;
        }
        AbstractModuleClient::recv_source(source);
    }
    void default_recv_source(std::string source){
        this->AbstractModuleClient::recv_source(source);
    }

    void recv_quit(){
        PyLockGIL lock;
        if(override recv_quit = this->get_override("recv_quit")){
            try{
                recv_quit();
            }
            catch (const error_already_set&){
                std::cout << "Exception in recv_quit in application";
                PyErr_Print();
            }
            return;
        }
        AbstractModuleClient::recv_quit();
    }
    void default_recv_quit(){
        this->AbstractModuleClient::recv_quit();
    }
    void recv_time(uint32_t time){
        PyLockGIL lock;
        if(override recv_time = this->get_override("recv_time")){
            try{
                recv_time(time);
            }
            catch (const error_already_set&){
                std::cout << "Exception in recv_time in application";
                PyErr_Print();
            }
            return;
        }
        AbstractModuleClient::recv_time(time);
    }
    void default_recv_time(uint32_t time){
        this->AbstractModuleClient::recv_time(time);
    }

    void recv_timerate(int timerate){
        PyLockGIL lock;
        if(override recv_timerate = this->get_override("recv_timerate")){
            try{
                recv_timerate(timerate);
            }
            catch (const error_already_set&){
                std::cout << "Exception in recv_timerate in application";
                PyErr_Print();
            }
            return;
        }
        AbstractModuleClient::recv_timerate(timerate);
    }
    void default_recv_timerate(int timerate){
        this->AbstractModuleClient::recv_timerate(timerate);
    }

    void recv_event(std::string origin, std::string event, TrodesMsg &msg){
        PyLockGIL lock;
        if(override recv_event = this->get_override("recv_event")){
            try{
                recv_event(origin, event, msg);
            }
            catch (const error_already_set&){
                std::cout << "Exception in recv_event in application";
                PyErr_Print();
            }
            return;
        }
        AbstractModuleClient::recv_event(origin, event, msg);
    }
    void default_recv_event(std::string origin, std::string event, TrodesMsg &msg){
        this->AbstractModuleClient::recv_event(origin, event, msg);
    }
    void sendOutEvent_python(str event, str format, list& l){
        TrodesMsg m = new_TrodesMsg(format, l);
        sendOutEvent(extract<std::string>(event), m);
    }
    void sendMsgToModule_python(str module, str subject, str format, list& l){
        TrodesMsg m = new_TrodesMsg(format, l);
        sendMsgToModule(extract<std::string>(module), extract<std::string>(subject), m);
    }
    void sendMsgToTrodes_python(str subject, str format, list& l){
        TrodesMsg m = new_TrodesMsg(format, l);
        sendMsgToTrodes(extract<std::string>(subject), m);
    }
    HFSubConsumer* subhfd_python(std::string dataName, std::string originModule, size_t messageBufferLength){
        return (HFSubConsumer_python*)subscribeHighFreqData(dataName, originModule, messageBufferLength);
    }
    HFSubConsumer* subhfd_default_python(std::string dataName, std::string originModule){
        return (HFSubConsumer_python*)subscribeHighFreqData(dataName, originModule, 10);
    }
    LFPConsumer_python* sublfpdata_python(size_t buffersize, list& l){
        HighFreqDataType dt = state->find_hfdt(hfType_LFP, TRODES_NETWORK_ID);
        HFParsingInfo parseinfo = createLFPParsingInfo(listToVec<std::string>(l), dt.getDataFormat());
        LFPConsumer_python* newsub = new LFPConsumer_python(dt, buffersize, parseinfo);
        HFSubSockSettings sockinfo(dt, newsub->getSubSockType(), NULL, NULL);
        addHfTypeToSubbedList(sockinfo);
        return newsub;
    }
    SpikesConsumer_python* subspikesdata_python(size_t buffersize, list& l){
        HighFreqDataType dt = state->find_hfdt(hfType_SPIKE, TRODES_NETWORK_ID);
        HFParsingInfo parseinfo = createSpikesParsingInfo(listToVec<std::string>(l), dt.getDataFormat());
        size_t max_nchans = 0;
        for(auto const& nt : getTrodesConfig().getNTrodes()){
            if(nt.getHw_chans().size() > max_nchans){
                max_nchans = nt.getHw_chans().size();
            }
        }
        SpikesConsumer_python* newsub = new SpikesConsumer_python(dt, buffersize, parseinfo, max_nchans*POINTS_IN_WAVE);
        HFSubSockSettings sockinfo(dt, newsub->getSubSockType(), NULL, NULL);
        addSubToList(newsub);
        addHfTypeToSubbedList(sockinfo);
        return newsub;
    }
    AnalogConsumer_python* subanalogdata_python(size_t buffersize, list& l){
        HighFreqDataType dt = state->find_hfdt(hfType_ANALOG, TRODES_NETWORK_ID);
        HFParsingInfo parseinfo = createAnalogParsingInfo(listToVec<std::string>(l), dt.getDataFormat());
        AnalogConsumer_python* newsub = new AnalogConsumer_python(dt, buffersize, parseinfo);
        HFSubSockSettings sockinfo(dt, newsub->getSubSockType(), NULL, NULL);
        addSubToList(newsub);
        addHfTypeToSubbedList(sockinfo);
        return newsub;
    }
    DigitalConsumer_python* subdigitaldata_python(size_t buffersize, list& l){
        HighFreqDataType dt = state->find_hfdt(hfType_DIGITAL, TRODES_NETWORK_ID);
        HFParsingInfo parseinfo = createDigitalParsingInfo(listToVec<std::string>(l), dt.getDataFormat());
        DigitalConsumer_python* newsub = new DigitalConsumer_python(dt, buffersize, parseinfo);
        HFSubSockSettings sockinfo(dt, newsub->getSubSockType(), NULL, NULL);
        addSubToList(newsub);
        addHfTypeToSubbedList(sockinfo);
        return newsub;
    }
    NeuralConsumer_python* subneuraldata_python(size_t buffersize, list &l){
        if(isHfTypeCurrentlySubbed(hfType_NEURO, TRODES_NETWORK_ID)){
            return (NeuralConsumer_python*)getHfSubObject(hfType_NEURO, TRODES_NETWORK_ID);
        }

        HighFreqDataType dt = state->find_hfdt(hfType_NEURO, TRODES_NETWORK_ID);
        if(!dt.isValid()){
            std::cerr << error("Neural data could not be found on the network!");
            return NULL;
        }
        HFParsingInfo parseinfo = createNeuralParsingInfo(listToVec<std::string>(l), dt.getDataFormat());
        NeuralConsumer_python* newsub = new NeuralConsumer_python(dt, buffersize, parseinfo);
        HFSubSockSettings sockinfo(dt, newsub->getSubSockType(), NULL, NULL);
        addSubToList(newsub);
        addHfTypeToSubbedList(sockinfo);
        return newsub;
    }
};

BOOST_PYTHON_MODULE(trodesnetwork){
    PyEval_InitThreads();
    Py_Initialize();
    np::initialize();
    to_python_converter<std::vector<std::string>, vecToList<std::string> >();
    to_python_converter<std::vector<byte>, vecToList<byte> >();
    to_python_converter<std::vector<int>, vecToList<int> >();
    to_python_converter<std::vector<EventDataType>, EventVecToDict>();
    to_python_converter<std::vector<HighFreqDataType>, vecToList<HighFreqDataType> >();
    to_python_converter<TrodesMsg, TrodesMsgToList>();
    to_python_converter<lfpPacket, lfpPacketToList>();

    scope().attr("TRODES_NETWORK_ID") = TRODES_NETWORK_ID;
    scope().attr("DEFAULT_SERVER_ADDRESS") = DEFAULT_SERVER_ADDRESS;
    scope().attr("DEFAULT_SERVER_PORT") = DEFAULT_SERVER_PORT;
    scope().attr("NEURALSTREAM") = hfType_NEURO;
    scope().attr("ANALOGSTREAM") = hfType_ANALOG;
    scope().attr("DIGITALSTREAM") = hfType_DIGITAL;
    scope().attr("SPIKESTREAM") = hfType_SPIKE;
    scope().attr("LFPSTREAM") = hfType_LFP;
    scope().attr("acq_PLAY") = acq_PLAY;
    scope().attr("acq_STOP") = acq_STOP;
    scope().attr("acq_PAUSE") = acq_PAUSE;
    scope().attr("acq_SEEK") = acq_SEEK;

//    scope().attr("settle_CMD") = settle_CMD;

    class_<timestamp_t>("timestamp_t")
            .add_property("trodes_timestamp", &timestamp_t::trodes_timestamp)
            .add_property("system_timestamp", &timestamp_t::system_timestamp)
            ;
    def("systemTimeMSecs", &CZHelp::systemTimeMSecs);

    class_<HighFreqDataType>
            ("HighFreqDataType")
            .add_property("name", &HighFreqDataType::getName, &HighFreqDataType::setName)
            .add_property("origin", &HighFreqDataType::getOrigin, &HighFreqDataType::setOrigin)
            .add_property("dataFormat", &HighFreqDataType::getDataFormat, &HighFreqDataType::setDataFormat)
            .add_property("address", &HighFreqDataType::getSockAddr, &HighFreqDataType::setSockAddr)
            .add_property("byteSize", &HighFreqDataType::getByteSize, &HighFreqDataType::setByteSize)
            .def("isValid", &HighFreqDataType::isValid)
            .def("print", &HighFreqDataType::print)
            ;

    class_<StimulationCommand>
            ("StimulationCommand", init<>())
//            .def(init<int, int, int, uint16_t, uint8_t, uint16_t, uint8_t, uint16_t, uint16_t, uint16_t>())
            .def("setGroup", &StimulationCommand::setGroup)
            .def("setNoGroup", &StimulationCommand::setNoGroup)
            .def("setSlot", &StimulationCommand::setSlot)
            .def("setChannels",&StimulationCommand::setChannels)
            .def("setBiphasicPulseShape", &StimulationCommand::setBiphasicPulseShape)
            .def("setNumPulsesInTrain", &StimulationCommand::setNumPulsesInTrain)
            .def("isValid", &StimulationCommand::isValid)
            ;

    enum_<GlobalStimulationSettings::CurrentScaling>("CurrentScaling")
            .value("max10nA", GlobalStimulationSettings::max10nA)
            .value("max20nA", GlobalStimulationSettings::max20nA)
            .value("max50nA", GlobalStimulationSettings::max50nA)
            .value("max100nA", GlobalStimulationSettings::max100nA)
            .value("max200nA", GlobalStimulationSettings::max200nA)
            .value("max500nA", GlobalStimulationSettings::max500nA)
            .value("max1uA", GlobalStimulationSettings::max1uA)
            .value("max2uA", GlobalStimulationSettings::max2uA)
            .value("max5uA", GlobalStimulationSettings::max5uA)
            .value("max10uA", GlobalStimulationSettings::max10uA)
            ;

    class_<GlobalStimulationSettings>("GlobalStimulationSettings", init<>())
            .def("setVoltageScale", &GlobalStimulationSettings::setVoltageScale)
            ;

    class_<GlobalStimulationCommand>("GlobalStimulationCommand", init<>())
            .def("setStimEnabled", &GlobalStimulationCommand::setStimEnabled)
            .def("setResetSequencer", &GlobalStimulationCommand::setResetSequencer)
            .def("setAbortStimulation", &GlobalStimulationCommand::setAbortStimulation)
            .def("setClearDSPOffset", &GlobalStimulationCommand::setClearDSPOffset)
            ;
//    void (HighFreqPub::*pubdatax2)(void, size_t) = &HighFreqPub::publishData;
//    void (HighFreqPub::*pubdatax3)(void, size_t, int64_t) = &HighFreqPub::publishData;
//    class_<HighFreqPub, boost::noncopyable>
//            ("HighFreqPub")
//            .def("initialize", &HighFreqPub::initialize)
//            .def("publishData", pubdatax2)
//            .def("publishData", pubdatax3)
//            .def("getAddress", &HighFreqPub::getAddress)
//            ;

    class_<HFSubConsumer_python, boost::noncopyable>
            ("HFSubConsumer", init<HighFreqDataType, int>())
            .def("initialize", &HFSubConsumer_python::initialize)
            .def("readData", static_cast<size_t (HFSubConsumer::*)(np::ndarray)>(&HFSubConsumer_python::readData_python))
            .def("available", &HFSubConsumer_python::available)
            .def("getDataType", &HFSubConsumer_python::getType)
            .def("lastSysTimestamp", &HFSubConsumer::lastSysTimestamp)
            ;

    class_<HFParsingInfo>("HFParsingInfo")
            .def_readwrite("indices", &HFParsingInfo::indices)
            .def_readwrite("sizeOf", &HFParsingInfo::sizeOf)
            ;

    class_<LFPConsumer_python, bases<HFSubConsumer_python>, boost::noncopyable>
            ("LFPConsumer", init<HighFreqDataType, int, HFParsingInfo>())
            .def("initialize", &LFPConsumer_python::initialize)
            .def("getData", &LFPConsumer_python::getData_python)
            .def("available", &LFPConsumer_python::available)
            .def("create_numpy_array", &LFPConsumer_python::create_numpy_array)
            .def("lastSysTimestamp", &HFSubConsumer::lastSysTimestamp)
            .def("getNTrodesRequested", &LFPConsumer_python::getNTrodesRequested)
            ;

    class_<SpikesConsumer_python, bases<HFSubConsumer_python>, boost::noncopyable>
            ("SpikesConsumer", init<HighFreqDataType, int, HFParsingInfo, int>())
            .def("initialize", &SpikesConsumer_python::initialize)
            .def("getData",&SpikesConsumer_python::getData_python)
            .def("available", &SpikesConsumer_python::available)
            .def("create_numpy_array", &SpikesConsumer_python::create_numpy_array)
            .def("lastSysTimestamp", &HFSubConsumer::lastSysTimestamp)
            .def("getNTrodesRequested", &SpikesConsumer_python::getNTrodesRequested)
            ;

    class_<AnalogConsumer_python, bases<HFSubConsumer_python>, boost::noncopyable>
            ("AnalogConsumer", init<HighFreqDataType, int, HFParsingInfo>())
            .def("initialize", &AnalogConsumer_python::initialize)
            .def("getData",&AnalogConsumer_python::getData_python)
            .def("available", &AnalogConsumer_python::available)
            .def("create_numpy_array", &AnalogConsumer_python::create_numpy_array)
            .def("lastSysTimestamp", &HFSubConsumer::lastSysTimestamp)
            .def("getChannelsRequested", &AnalogConsumer_python::getChannelsRequested)
            ;

    class_<DigitalConsumer_python, bases<HFSubConsumer_python>, boost::noncopyable>
            ("AnalogConsumer", init<HighFreqDataType, int, HFParsingInfo>())
            .def("initialize", &DigitalConsumer_python::initialize)
            .def("getData",&DigitalConsumer_python::getData_python)
            .def("available", &DigitalConsumer_python::available)
            .def("create_numpy_array", &DigitalConsumer_python::create_numpy_array)
            .def("lastSysTimestamp", &HFSubConsumer::lastSysTimestamp)
            .def("getChannelsRequested", &DigitalConsumer_python::getChannelsRequested)
            ;

    class_<NeuralConsumer_python, bases<HFSubConsumer_python>, boost::noncopyable>
            ("NeuralConsumer", init<HighFreqDataType, int, HFParsingInfo>())
            .def("initialize", &NeuralConsumer_python::initialize)
            .def("getData",&NeuralConsumer_python::getData_python)
            .def("available", &NeuralConsumer_python::available)
            .def("create_numpy_array", &NeuralConsumer_python::create_numpy_array)
            .def("lastSysTimestamp", &HFSubConsumer::lastSysTimestamp)
            .def("getChannelsRequested", &NeuralConsumer_python::getChannelsRequested)
            ;

        class_<PythonModuleClient, boost::noncopyable>
            ("AbstractModuleClient", init<std::string, std::string, int>())
            .def("recv_acquisition", &AbstractModuleClient::recv_acquisition,&PythonModuleClient::default_recv_acquisition)
            .def("recv_file_open", &AbstractModuleClient::recv_file_open,&PythonModuleClient::default_recv_file_open)
            .def("recv_file_close", &AbstractModuleClient::recv_file_close,&PythonModuleClient::default_recv_file_close)
            .def("recv_source", &AbstractModuleClient::recv_source,&PythonModuleClient::default_recv_source)
            .def("recv_quit", &AbstractModuleClient::recv_quit,&PythonModuleClient::default_recv_quit)
            .def("recv_time", &AbstractModuleClient::recv_time,&PythonModuleClient::default_recv_time)
            .def("recv_timerate", &AbstractModuleClient::recv_timerate,&PythonModuleClient::default_recv_timerate)
            .def("recv_event", &AbstractModuleClient::recv_event,&PythonModuleClient::default_recv_event)
            .def("initialize", &PythonModuleClient::initialize)
            .def("subscribeToEvent", &PythonModuleClient::subscribeToEvent)
            .def("unsubscribeFromEvent", &PythonModuleClient::unsubscribeFromEvent)
            .def("sendMsgToTrodes", &PythonModuleClient::sendMsgToTrodes_python)
            .def("sendMsgToModule", &PythonModuleClient::sendMsgToModule_python)
            .def("sendOutEvent", &PythonModuleClient::sendOutEvent_python)
            .def("sendTimeRequest", &PythonModuleClient::sendTimeRequest)
            .def("sendTimeRateRequest", &PythonModuleClient::sendTimeRateRequest)
            .def("getID", &PythonModuleClient::getID)
            .def("getClients", &PythonModuleClient::getClients)
            .def("registerHighFreqData", &PythonModuleClient::registerHighFreqData)
            .def("deregisterHighFreqData", &PythonModuleClient::deregisterHighFreqData)
            .def("getHighFreqList", &PythonModuleClient::getHighFreqList)
            .def("subscribeHighFreqData", &PythonModuleClient::subhfd_python, return_value_policy<manage_new_object>())
            .def("subscribeHighFreqData", &PythonModuleClient::subhfd_default_python, return_value_policy<manage_new_object>())
            .def("unsubscribeHighFreqData", &PythonModuleClient::unsubscribeHighFreqData)
            .def("provideEvent", &PythonModuleClient::provideEvent)
            .def("unprovideEvent",&PythonModuleClient::unprovideEvent)
            .def("getEventList", &PythonModuleClient::getEventList)
            .def("subscribeLFPData", &PythonModuleClient::sublfpdata_python, return_value_policy<manage_new_object>())
            .def("subscribeSpikesData", &PythonModuleClient::subspikesdata_python, return_value_policy<manage_new_object>())
            .def("subscribeAnalogData", &PythonModuleClient::subanalogdata_python, return_value_policy<manage_new_object>())
            .def("subscribeDigitalData", &PythonModuleClient::subdigitaldata_python, return_value_policy<manage_new_object>())
            .def("subscribeNeuralData", &PythonModuleClient::subneuraldata_python, return_value_policy<manage_new_object>())
            .def("getAvailableTrodesData", &PythonModuleClient::getAvailableTrodesData)
            .def("initializeHardwareConnection", &PythonModuleClient::initializeHardwareConnection)
            .def("destroyHardwareConnection", &PythonModuleClient::destroyHardwareConnection)
            .def("sendSettleCommand", &PythonModuleClient::sendSettleCommand)
            .def("sendStimulationParams", &PythonModuleClient::sendStimulationParams)
            .def("sendClearStimulationParams", &PythonModuleClient::sendClearStimulationParams)
            .def("sendStimulationStartSlot", &PythonModuleClient::sendStimulationStartSlot)
            .def("sendStimulationStartGroup",&PythonModuleClient::sendStimulationStartGroup)
            .def("sendStimulationStopSlot", &PythonModuleClient::sendStimulationStopSlot)
            .def("sendStimulationStopGroup",&PythonModuleClient::sendStimulationStopGroup)
            .def("sendGlobalStimulationSettings", &PythonModuleClient::sendGlobalStimulationSettings)
            .def("sendGlobalStimulationCommand", &PythonModuleClient::sendGlobalStimulationCommand)
            .def("latestTrodesTimestamp", &PythonModuleClient::latestTrodesTimestamp)
            .def("sendAnnotationRequest", &PythonModuleClient::sendAnnotationRequest)
            ;
}
