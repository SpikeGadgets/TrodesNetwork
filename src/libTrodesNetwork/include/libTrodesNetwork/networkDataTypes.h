/* 
 * File:   networkDataTypes.h
 * Author: Maris_Kali
 *
 * Created on August 28, 2017, 2:46 PM
 */

#ifndef NETWORKDATATYPES_H
#define NETWORKDATATYPES_H
// #define __USE_MINGW_ANSI_STDIO 0
// #include <czmq.h>
#include "cereal/cereal.hpp"
#include "cereal/archives/portable_binary.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/set.hpp"
#include "cereal/types/utility.hpp"
#include <vector>
#include <sstream>


class binarydata{
public:
    binarydata();
    binarydata(const char* s, size_t n);
    binarydata(const std::string &str);
    binarydata& assign(const char* s, size_t n);
    size_t size() const noexcept;
    const char* data() const noexcept;
    operator std::string() const{return str;}
private:
    std::string str;
};

/*! Abstract Base Class NetworkDataType
 * The NetworkDataType is a standard abstract base class that should be inherited by any user defined
 * structs/classes that are to be passed over the networking code.  The main purpose of the class is 
 * to provide the virtual functions encode and decode, which are called when encoding and decoding the
 * objects. The class binarydata, which is the type that is serialized to, is passed around and used
 * serialize/deserialize objects.
 *
 * !!!If you want to store networkdatatypes within networkdatatypes, you must add an additional serialize
 * function. See HighFreqDataType as an example.
 *
 */
class NetworkDataType {
public:
    NetworkDataType();
    virtual ~NetworkDataType();

    
    virtual binarydata encode() const = 0;
    virtual void decode(const binarydata & data) = 0;

    template<typename ... T>
    static binarydata serializedata(T & ... args){
        std::ostringstream stream;
        cereal::PortableBinaryOutputArchive archive(stream);
        archive(args...);
        return binarydata(stream.str());
    }

    template<typename ... T>
    static void deserializedata(const binarydata &data, T & ... args){
        std::stringstream stream(data);
        cereal::PortableBinaryInputArchive archive(stream);
        archive(args...);
    }

protected:
    
private:
};


class EventDataType : public NetworkDataType{
public:
    EventDataType(std::string n, std::string o);
    EventDataType(){}

    binarydata encode() const;
    void decode(const binarydata &data);

    size_t constgetsize() const;
    std::string getName() const;
    void setName(const std::string &value);

    std::string getOrigin() const;
    void setOrigin(const std::string &value);

private:
    std::string name;
    std::string origin;
//    std::string description;
    friend class cereal::access;
    template<typename Archive>
    void serialize(Archive &archive){
        archive(name, origin);
    }
};

//This class contains a description of high frequency data types
class HighFreqDataType : public NetworkDataType {
public:
    HighFreqDataType();
    HighFreqDataType(const HighFreqDataType &);
    HighFreqDataType(std::string n, std::string o, std::string dformat, std::string sock, int bsize);
    ~HighFreqDataType();
    bool operator==(const HighFreqDataType &obj);
    bool operator<(const HighFreqDataType obj) const;


    binarydata encode() const;
    void decode(const binarydata &data);

    void print();

    bool isValid(void) const;


    std::string getName() const;
    void setName(const std::string &value);

    std::string getOrigin() const;
    void setOrigin(const std::string &value);

    std::string getDataFormat() const;
    void setDataFormat(const std::string &value);

    std::string getSockAddr() const;
    void setSockAddr(const std::string &value);

    int getByteSize() const;
    void setByteSize(int value);

private:
    std::string name;
    std::string origin;
    std::string dataFormat;
    std::string sockAddr;
    int byteSize;
    friend class cereal::access;
    template<typename Archive>
    void serialize(Archive &archive){
        archive(name, origin, dataFormat, sockAddr, byteSize);
    }
};

class NDeviceChannel : public NetworkDataType {
public:
    NDeviceChannel();
    ~NDeviceChannel();

    bool isValid(void) const;

    NDeviceChannel* copy(void);

    binarydata encode() const;
    void decode(const binarydata &data);

    std::string getPrintStr(int indent = 0);

    std::string getId() const;
    void setId(const std::string &value);

    int getStartByte() const;
    void setStartByte(int value);

    int getStartBit() const;
    void setStartBit(int value);

    int getType() const;
    void setType(int value);

    int getInterleavedDataByte() const;
    void setInterleavedDataByte(int value);

    int getInterleavedDataBit() const;
    void setInterleavedDataBit(int value);

private:
    std::string id;
    int startByte;
    int startBit;
    int type;
    int interleavedDataByte;
    int interleavedDataBit;
    friend class cereal::access;
    template<typename Archive>
    void serialize(Archive &archive){
        archive(id, startByte, startBit, type, interleavedDataByte, interleavedDataBit);
    }
};

class NDevice : public NetworkDataType {
public:
    NDevice();
    ~NDevice();

    bool isValid(void) const;


    binarydata encode() const;
    void decode(const binarydata &data);

    void addChannel(NDeviceChannel newChan);
    NDeviceChannel getChannel(int index) const;
    void removeChannelAt(int index);

    std::string getPrintStr(int indent = 0);

    std::string getName() const;
    void setName(const std::string &value);

    int getNumBytes() const;
    void setNumBytes(int value);

    int getByteOffset() const;
    void setByteOffset(int value);

    std::vector<NDeviceChannel> getChannels() const;
    void setChannels(const std::vector<NDeviceChannel> &value);

private:
    std::string name;
    int numBytes;
    int byteOffset;
    std::vector<NDeviceChannel> channels;
    friend class cereal::access;
    template<typename Archive>
    void serialize(Archive &archive){
        archive(name, numBytes, byteOffset, channels);
    }
};

class NTrodeObj : public NetworkDataType {
public:
    NTrodeObj();
    ~NTrodeObj();

    bool isValid(void) const;

    NTrodeObj* copy(void);

    binarydata encode() const;
    void decode(const binarydata &data);

    void addHWChan(int newChan);
    int getHWChan(int index) const;
    void removeHWChanAt(int index);

    std::string getPrintStr(int indent = 0);

    std::string getId() const;
    void setId(const std::string &value);

    int getIndex() const;
    void setIndex(int value);

    std::vector<int> getHw_chans() const;
    void setHw_chans(const std::vector<int> &value);

private:
    std::string id;
    int index;
    std::vector<int> hw_chans;
    friend class cereal::access;
    template<typename Archive>
    void serialize(Archive &archive){
        archive(id, index, hw_chans);
    }
};


class TrodesConfig : public NetworkDataType {
public:
    TrodesConfig();
    TrodesConfig(const TrodesConfig &obj);
    ~TrodesConfig();

    TrodesConfig& operator= (const TrodesConfig &obj);

    bool isValid(void) const;

    void clear();
    TrodesConfig* copy(void) const;

    binarydata encode() const;
    void decode(const binarydata &data);

    void addDevice(NDevice newDevice);
    NDevice getDevice(int index) const;
    void removeDeviceAt(int index);

    void addNTrode(NTrodeObj newNTrode);
    NTrodeObj getNTrode(int index) const;
    void removeNTrodeAt(int index);

    std::string getPrintStr(int indent = 0);

    std::vector<NDevice> getDevices() const;
    void setDevices(const std::vector<NDevice> &value);

    std::vector<NTrodeObj> getNTrodes() const;
    void setNTrodes(const std::vector<NTrodeObj> &value);

private:
    std::vector<NDevice> devices;
    std::vector<NTrodeObj> nTrodes;
};


#endif /* NETWORKDATATYPES_H */

