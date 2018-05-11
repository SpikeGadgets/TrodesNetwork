/* 
 * File:   networkDataTypes.cpp
 * Author: Maris_Kali
 * 
 * Created on August 28, 2017, 2:46 PM
 */
// #define __USE_MINGW_ANSI_STDIO 0
#include "libTrodesNetwork/networkDataTypes.h"
#include <czmq.h>
#include <ctime>
#include <sstream>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
binarydata::binarydata()
    : str(){

}
binarydata::binarydata(const char *s, size_t n)
    : str(s, n+1){

}

binarydata::binarydata(const std::string &str)
    : str(str){

}

binarydata& binarydata::assign(const char *s, size_t n){
    str.assign(s, n+1);
    return *this;
}

size_t binarydata::size() const noexcept{
    return str.size();
}

const char* binarydata::data() const noexcept{
    return str.data();
}
// ****** NetworkDataType CLASS ******

NetworkDataType::NetworkDataType(void) {
}

NetworkDataType::~NetworkDataType(void) {
}

NDeviceChannel::NDeviceChannel() : NetworkDataType() {
    id = "";
    startByte = -1;
    startBit = -1;
    type = -1;
    interleavedDataByte = -1;
    interleavedDataBit = -1;
}

NDeviceChannel::~NDeviceChannel() {

}

bool NDeviceChannel::isValid() const {
    if (id.empty() || startByte == -1 || type == -1 || startBit == -1)
        return(false); //a device must have an id, a startByte and a type to be valid
    return(true);
}

NDeviceChannel* NDeviceChannel::copy() {
    NDeviceChannel *newObj = new NDeviceChannel();
    newObj->setId(id);
    newObj->setStartByte(startByte);
    newObj->setStartBit(startBit);
    newObj->setType(type);
    newObj->setInterleavedDataBit(interleavedDataBit);
    newObj->setInterleavedDataByte(interleavedDataByte);
    return(newObj);
}

binarydata NDeviceChannel::encode() const{
    return NetworkDataType::serializedata(id, startByte, startBit, type, interleavedDataByte, interleavedDataBit);
}

void NDeviceChannel::decode(const binarydata &data){
    NetworkDataType::deserializedata(data, id, startByte, startBit, type, interleavedDataByte, interleavedDataBit);
}

std::string NDeviceChannel::getPrintStr(int indent) {
    std::ostringstream oss;
    std::string indentStr = "    ";
    for (int i = 0; i < indent; i++) { oss << indentStr; }
    oss << "Printing 'NDevice Channel'\n";
    for (int i = 0; i < indent; i++) { oss << indentStr; }
    oss << "    -id                  : " << id << "\n";
    for (int i = 0; i < indent; i++) { oss << indentStr; }
    oss << "    -startByte           : " << startByte << "\n";
    for (int i = 0; i < indent; i++) { oss << indentStr; }
    oss << "    -startBit           : " << startBit << "\n";
    for (int i = 0; i < indent; i++) { oss << indentStr; }
    oss << "    -type                : " << type << "\n";
    for (int i = 0; i < indent; i++) { oss << indentStr; }
    oss << "    -interleavedDataByte : " << interleavedDataByte << "\n";
    for (int i = 0; i < indent; i++) { oss << indentStr; }
    oss << "    -interleavedDataBit  : " << interleavedDataBit << "\n";
    return(oss.str());
}

std::string NDeviceChannel::getId() const
{
    return id;
}

void NDeviceChannel::setId(const std::string &value)
{
    id = value;
}

int NDeviceChannel::getStartByte() const
{
    return startByte;
}

void NDeviceChannel::setStartByte(int value)
{
    startByte = value;
}

int NDeviceChannel::getStartBit() const
{
    return startBit;
}

void NDeviceChannel::setStartBit(int value)
{
    startBit = value;
}

int NDeviceChannel::getType() const
{
    return type;
}

void NDeviceChannel::setType(int value)
{
    type = value;
}

int NDeviceChannel::getInterleavedDataByte() const
{
    return interleavedDataByte;
}

void NDeviceChannel::setInterleavedDataByte(int value)
{
    interleavedDataByte = value;
}

int NDeviceChannel::getInterleavedDataBit() const
{
    return interleavedDataBit;
}

void NDeviceChannel::setInterleavedDataBit(int value)
{
    interleavedDataBit = value;
}

NDevice::NDevice() : NetworkDataType() {
    name = "";
    numBytes = -1;
    byteOffset = -1;

}

NDevice::~NDevice() {
    channels.clear();
}

bool NDevice::isValid() const {
    if (name.empty() || numBytes == -1 || byteOffset == -1 || channels.empty())
        return(false); // a device must have a name, a set numBytes, a set byteOffset, and at least one channel
    for (int i = 0; i < (int)channels.size(); i++) {
        if (!channels.at(i).isValid()) //make sure all channels are valid
            return(false);
    }
    return(true);
}


binarydata NDevice::encode() const{
    return NetworkDataType::serializedata(name, numBytes, byteOffset, channels);
}

void NDevice::decode(const binarydata &data){
    NetworkDataType::deserializedata(data, name, numBytes, byteOffset, channels);
}


std::string NDevice::getPrintStr(int indent) {
    std::ostringstream oss;
    std::string indentStr = "    ";
    for (int k = 0; k < indent; k++) { oss << indentStr; }
    oss << "Printing 'NDevice'\n";
    for (int k = 0; k < indent; k++) { oss << indentStr; }
    oss << "    -name       : " << name << "\n";
    for (int k = 0; k < indent; k++) { oss << indentStr; }
    oss << "    -numBytes   : " << numBytes << "\n";
    for (int k = 0; k < indent; k++) { oss << indentStr; }
    oss << "    -byteOffset : " << byteOffset << "\n";
    for (int k = 0; k < indent; k++) { oss << indentStr; }
    oss << "    -Channels vector size[" << channels.size() << "]:\n";
    for (int i = 0; i < (int)channels.size(); i++) {
        for (int k = 0; k < indent; k++) { oss << indentStr; }
        oss << "        - i[" << i << "] : \n" << channels.at(i).getPrintStr(indent+2) << "\n";
    }
    return(oss.str());
}

std::string NDevice::getName() const
{
    return name;
}

void NDevice::setName(const std::string &value)
{
    name = value;
}

int NDevice::getNumBytes() const
{
    return numBytes;
}

void NDevice::setNumBytes(int value)
{
    numBytes = value;
}

int NDevice::getByteOffset() const
{
    return byteOffset;
}

void NDevice::setByteOffset(int value)
{
    byteOffset = value;
}

std::vector<NDeviceChannel> NDevice::getChannels() const
{
    return channels;
}

void NDevice::setChannels(const std::vector<NDeviceChannel> &value)
{
    channels = value;
}

void NDevice::addChannel(NDeviceChannel newChan){
    channels.push_back(newChan);
}

NDeviceChannel NDevice::getChannel(int index) const{
    if (index >= (int)channels.size()) {
        return NDeviceChannel();
    }
    return channels[index];
}

void NDevice::removeChannelAt(int index){
     if (index >= (int)channels.size()) {
         return;
     }
     channels.erase(channels.begin() + index);
}

NTrodeObj::NTrodeObj() : NetworkDataType() {
    id = "";
}

NTrodeObj::~NTrodeObj() {

}

bool NTrodeObj::isValid() const {
    if (id.empty()) //each nTrode must be assigned an ID
        return(false);
    if (hw_chans.empty()) //each nTrode must have a hw channel
        return(false);
    return(true);
}

NTrodeObj* NTrodeObj::copy() {
    NTrodeObj* newObj = new NTrodeObj();
    newObj->setId(id);
    for (int i = 0; i < (int)hw_chans.size(); i++) {
        newObj->addHWChan(hw_chans.at(i));
    }
    return(newObj);
}

binarydata NTrodeObj::encode() const{
    return NetworkDataType::serializedata(id, hw_chans);
}

void NTrodeObj::decode(const binarydata &data){
    NetworkDataType::deserializedata(data, id, hw_chans);
}

std::string NTrodeObj::getPrintStr(int indent) {
    std::ostringstream oss;
    std::string indentStr = "    ";
    for (int k = 0; k < indent; k++) { oss << indentStr; }
    oss << "Printing 'NTrodeObj'\n";
    for (int k = 0; k < indent; k++) { oss << indentStr; }
    oss << "    -id : " << id << "\n";
    for (int k = 0; k < indent; k++) { oss << indentStr; }
    oss << "    -hw_chans vector size[" << hw_chans.size() << "]:\n";
    for (int i = 0; i < (int)hw_chans.size(); i++) {
        for (int k = 0; k < indent; k++) { oss << indentStr; }
        oss << "        - i[" << i << "] : " << hw_chans.at(i) << "\n";
    }
    return(oss.str());
}

std::string NTrodeObj::getId() const
{
    return id;
}

void NTrodeObj::setId(const std::string &value)
{
    id = value;
}

int NTrodeObj::getIndex() const
{
    return index;
}

void NTrodeObj::setIndex(int value)
{
    index = value;
}

std::vector<int> NTrodeObj::getHw_chans() const
{
    return hw_chans;
}

void NTrodeObj::setHw_chans(const std::vector<int> &value)
{
    hw_chans = value;
}

void NTrodeObj::addHWChan(int newChan){
    hw_chans.push_back(newChan);
}

int NTrodeObj::getHWChan(int index) const{
    if (index >= (int)hw_chans.size()) {
        return(-1);
    }
    return hw_chans[index];
}

void NTrodeObj::removeHWChanAt(int index){
    if (index >= (int)hw_chans.size()) {
        return;
    }
    hw_chans.erase(hw_chans.begin() + index);
}
TrodesConfig::TrodesConfig() : NetworkDataType() {
}

TrodesConfig::TrodesConfig(const TrodesConfig &obj) {
    clear();
//    std::cerr << "COPYING [" << &obj << "] into this [" << this << "]\n";

    TrodesConfig *objCpy = obj.copy(); //create a new memory copy of the target TrodesConfig object and assign it to self
    devices = objCpy->getDevices();
    nTrodes = objCpy->getNTrodes();
}

TrodesConfig::~TrodesConfig() {
//    std::cerr << "DESTROY TrodesConfig() " << this << "\n";
    clear(); //TODO: find out why clear() calls are causing crashes
}

TrodesConfig& TrodesConfig::operator = (const TrodesConfig &obj) {
    TrodesConfig *objCpy = obj.copy();
    devices = objCpy->getDevices();
    nTrodes = objCpy->getNTrodes();
    return(*this);
}

bool TrodesConfig::isValid() const {
//    std::cerr << "ISVALID check on TrodesConfig() " << this << "\n";
    if (devices.empty()) //there must always at least be an MCU
        return(false);

    for (int i = 0; i < (int)devices.size(); i++) {
        if (!devices.at(i).isValid()) //check that all devices are valid
            return(false);
    }

    for (int i = 0; i < (int)nTrodes.size(); i++) {
        if (!nTrodes.at(i).isValid()) //check that all added nTrodes are valid
            return(false);
    }
    return(true);
}

// TODO: find out why clear() calls are causing crashes.  This should be safe
void TrodesConfig::clear() { //deletes from memory all devices and nTrodes
    devices.clear();
    nTrodes.clear();
}

TrodesConfig* TrodesConfig::copy() const {
    TrodesConfig* newObj = new TrodesConfig();
//    TrodesConfig newObj

    for (int i = 0; i < (int)devices.size(); i++) {
        newObj->addDevice(devices.at(i));
    }
    for (int i = 0; i < (int)nTrodes.size(); i++) {
        newObj->addNTrode(nTrodes.at(i));
    }
    return(newObj);
}

binarydata TrodesConfig::encode() const{
    return NetworkDataType::serializedata(devices, nTrodes);
}

void TrodesConfig::decode(const binarydata &data){
    NetworkDataType::deserializedata(data, devices, nTrodes);
}

std::string TrodesConfig::getPrintStr(int indent) {
    std::ostringstream oss;
    std::string indentStr = "\t";
    for (int k = 0; k < indent; k++) { oss << indentStr; }
    oss << "Printing 'TrodesConfig'\n";

    for (int k = 0; k < indent; k++) { oss << indentStr; }
    oss << "    -devices vector size[" << devices.size() << "]:\n";

    for (int i = 0; i < (int)devices.size(); i++) {
        for (int k = 0; k < indent; k++) { oss << indentStr; }
        oss << "        - i[" << i << "] : \n" << devices.at(i).getPrintStr(indent+2) << "\n";
    }

    for (int k = 0; k < indent; k++) { oss << indentStr; }
    oss << "    -nTrodes vector size[" << nTrodes.size() << "]:\n";

    for (int i = 0; i < (int)nTrodes.size(); i++) {
        for (int k = 0; k < indent; k++) { oss << indentStr; }
        oss << "        - i[" << i << "] : \n" << nTrodes.at(i).getPrintStr(indent+2) << "\n";
    }
    return(oss.str());
}

void TrodesConfig::addDevice(NDevice newDevice){
    devices.push_back(newDevice);
}

NDevice TrodesConfig::getDevice(int index) const{
    if (index >= (int)devices.size()) {
        return NDevice();
    }
    return devices[index];
}

void TrodesConfig::removeDeviceAt(int index){
    if (index >= (int)devices.size()) {
        return;
    }
    devices.erase(devices.begin() + index);
}
std::vector<NDevice> TrodesConfig::getDevices() const
{
    return devices;
}

void TrodesConfig::setDevices(const std::vector<NDevice> &value)
{
    devices = value;
}

void TrodesConfig::addNTrode(NTrodeObj newNTrode){
    nTrodes.push_back(newNTrode);
}

NTrodeObj TrodesConfig::getNTrode(int index) const{
    if (index >= (int)nTrodes.size()) {
        return NTrodeObj();
    }
    return(nTrodes[index]);
}

void TrodesConfig::removeNTrodeAt(int index){
    if (index >= (int)nTrodes.size()) {
        return;
    }
    nTrodes.erase(nTrodes.begin() + index);
}

std::vector<NTrodeObj> TrodesConfig::getNTrodes() const
{
    return nTrodes;
}

void TrodesConfig::setNTrodes(const std::vector<NTrodeObj> &value)
{
    nTrodes = value;
}

HighFreqDataType::HighFreqDataType() : NetworkDataType() {
    name = "";
    origin = "";
    dataFormat = "";
    sockAddr = "";
    byteSize = 0;
}

HighFreqDataType::HighFreqDataType(const HighFreqDataType &obj) {
    name = obj.name;
    origin = obj.origin;
    dataFormat = obj.dataFormat;
    sockAddr = obj.sockAddr;
    byteSize = obj.byteSize;
}

HighFreqDataType::HighFreqDataType(std::string n, std::string o, std::string dformat, std::string sock, int bsize){
    name = n;
    origin = o;
    dataFormat = dformat;
    sockAddr = sock;
    byteSize = bsize;
}

HighFreqDataType::~HighFreqDataType() {
}

bool HighFreqDataType::operator ==(const HighFreqDataType &obj) {
    bool retval = true;
    if (name != obj.name)
        retval = false;
    if (origin != obj.origin)
        retval = false;
    if (sockAddr != obj.sockAddr)
        retval = false;
    if (byteSize != obj.byteSize)
        retval = false;

    return(retval);
}

bool HighFreqDataType::operator <(const HighFreqDataType obj) const {
    bool retval = true;
    if (name > obj.name)
        retval = false;
    if (origin > obj.origin)
        retval = false;

    return(retval);
}

binarydata HighFreqDataType::encode() const{
    return NetworkDataType::serializedata(name, origin, dataFormat, sockAddr, byteSize);
}

void HighFreqDataType::decode(const binarydata &data){
    NetworkDataType::deserializedata(data, name, origin, dataFormat, sockAddr, byteSize);
}

void HighFreqDataType::print() {
    std::cout << "-Printing HighFreqDataType:\n";
    std::cout << "  name: " << name << "\n  origin: " << origin << "\n  dataFormat: " << dataFormat << std::endl;
    std::cout << "  SockAddress: " << sockAddr;
    std::cout << std::endl;
}


bool HighFreqDataType::isValid() const {
//    std::cout << "HFDT: Checking Validity\n";
    bool isValid = true;
    if (name.empty())
        isValid = false;
//    if (dataFormat.empty())
//        isValid = false;
    if (sockAddr.empty())
        isValid = false;
    if (byteSize == 0)
        isValid = false;
    return(isValid);
}

std::string HighFreqDataType::getName() const
{
    return name;
}

void HighFreqDataType::setName(const std::string &value)
{
    name = value;
}

std::string HighFreqDataType::getOrigin() const
{
    return origin;
}

void HighFreqDataType::setOrigin(const std::string &value)
{
    origin = value;
}

std::string HighFreqDataType::getDataFormat() const
{
    return dataFormat;
}

void HighFreqDataType::setDataFormat(const std::string &value)
{
    dataFormat = value;
}

std::string HighFreqDataType::getSockAddr() const
{
    return sockAddr;
}

void HighFreqDataType::setSockAddr(const std::string &value)
{
    sockAddr = value;
}

int HighFreqDataType::getByteSize() const
{
    return byteSize;
}

void HighFreqDataType::setByteSize(int value)
{
    byteSize = value;
}

EventDataType::EventDataType(std::string n, std::string o)
    : name(n), origin(o)
{
}


binarydata EventDataType::encode() const{
    return NetworkDataType::serializedata(name, origin);
}

void EventDataType::decode(const binarydata &data){
    NetworkDataType::deserializedata(data, name, origin);
}

size_t EventDataType::constgetsize() const {
    return name.length() + 1 + origin.length() + 1;
}
std::string EventDataType::getName() const{
    return name;
}

void EventDataType::setName(const std::string &value){
    name = value;
}

std::string EventDataType::getOrigin() const{
    return origin;
}

void EventDataType::setOrigin(const std::string &value){
    origin = value;
}
