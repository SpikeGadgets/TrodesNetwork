/* 
 * File:   CZHelp.cpp
 * Author: Maris_Kali
 * 
 * Created on August 28, 2017, 2:40 PM
 */
// #define __USE_MINGW_ANSI_STDIO 0
#include "libTrodesNetwork/CZHelp.h"
#include <sstream>


// ****** CZHELP CLASS ******
//higher lvl wrapper of helper functions for zmq & czmq

//! This function adds any NetworkDataType derived class to a zmsg
//! Stores encoded data as a memory block in zmsg
int CZHelp::zmsg_addNDT(zmsg_t* self, const binarydata &ndtdata) {
    zmsg_addmem(self, ndtdata.data(), ndtdata.size());
    return(0);
}

//! Function pops off any NDT in a zmsg and adds it to ndt
//! Required to create or allocate NDT before calling function.
int CZHelp::zmsg_popNDT(zmsg_t *self, binarydata &ndtdata) {
    //Format not needed in new implementation, NDT's readByteArr should know how to deal with memory block
    if (self == NULL)
        return(0);
    zframe_t *f = zmsg_pop(self);
    ndtdata.assign((char*)zframe_data(f), zframe_size(f));
    zframe_destroy(&f);
    return(0);
}

int64_t CZHelp::systemTimeMSecs(){
    return zclock_time();
}

int64_t CZHelp::monotimer_msecs(){
    return zclock_mono();
}

int64_t CZHelp::monotimer_usecs(){
    return zclock_usecs();
}

int CZHelp::getADConversionFactor(){
    return 12780;
}

void CZHelp::ltrim(std::string &s){
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}
void CZHelp::rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim whitespace from both ends (in place)
void CZHelp::trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

// trim whitespace from start (copying)
std::string CZHelp::ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}

// trim whitespace from end (copying)
std::string CZHelp::rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}

// trim whitespace from both ends (copying)
std::string CZHelp::trim_copy(std::string s) {
    trim(s);
    return s;
}

//template<typename T>
//bool CZHelp::serialize_object(const T &obj, std::string &data){
//    try{
//        std::ostringstream stream;
//        cereal::PortableBinaryOutputArchive archive(stream);
//        archive(obj);
//        data = stream.str();
//    }
//    catch(cereal::Exception &e){
//        std::cerr << "Error when serializing object: " << e.what() << std::endl;
//        return false;
//    }
//    return true;
//}

//template<typename T>
//bool CZHelp::deserialize_object(T &obj, const std::string &data){
//    try{
//        std::stringstream stream(data);
//        cereal::PortableBinaryInputArchive archive(stream);
//        archive(obj);
//    }
//    catch(cereal::Exception &e){
//        std::cerr << "Error when deserializing object: " << e.what() << std::endl;
//        return false;
//    }
//    return true;
//}
