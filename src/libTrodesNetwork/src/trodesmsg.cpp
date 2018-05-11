#include "libTrodesNetwork/trodesmsg.h"
#include <czmq.h>
TrodesMsg::TrodesMsg()
{
    msg = zmsg_new();
}

TrodesMsg::TrodesMsg(std::string f, zmsg_t *content)
    : format(f), msg(content)
{

}

TrodesMsg::TrodesMsg(const TrodesMsg &t) {
    //TODO: find out if msg can be safely deallocated IF it cannot be guarenteed that it is already a NULL ptr
    format = t.format;
    msg = zmsg_dup(t.msg);
}

TrodesMsg::TrodesMsg(TrodesMsg&& t) {
    //TODO: find out if msg can be safely deallocated IF it cannot be guarenteed that it is already a NULL ptr
    format = t.format;
    msg = t.msg;
    t.msg = NULL;
}

TrodesMsg::~TrodesMsg(){
    if(zmsg_is(msg))
        zmsg_destroy(&msg);
}

TrodesMsg TrodesMsg::copy(){
    return TrodesMsg(format, zmsg_dup(msg));
}

zmsg_t* TrodesMsg::getzmsg(){
    return msg;
}

bool TrodesMsg::isvalid(){
    return zmsg_is(msg);
}
std::string TrodesMsg::getformat() const
{
    return format;
}

void TrodesMsg::setformat(const std::string &value)
{
    format = value;
}

size_t TrodesMsg::numContents() const{
    return zmsg_size(msg);
}

size_t TrodesMsg::size(){
    return zmsg_content_size(msg);
}

std::string TrodesMsg::popstr(){
    if(format[0] != 's') {
        std::cout << "Error in TrodesMsg::popstr: First frame of msg not a string. Got " << format[0] << std::endl;
    }
    char *str = zmsg_popstr(msg);
    std::string s(str);
    freen(str);
    format.erase(1,1);
    return s;
}

void TrodesMsg::addstr(std::string str){
    zmsg_addstr(msg, str.c_str());
    format.append("s");
}

#define PROCESSERROR    "Could not read message. Expected different type, got "
#define APPENDERROR     "Could not append to msg. Expected a different type, got "
void TrodesMsg::processArg(const char c, int &a){
    if(c != 'i') throw std::string(PROCESSERROR "int");
    char *str = zmsg_popstr(msg);
    a = str ? atoi(str) : 0;
    freen(str);
}
void TrodesMsg::processArg(const char c, uint8_t &a){
    if(c != '1') throw std::string(PROCESSERROR "uint8_t");
    char *str = zmsg_popstr(msg);
    a = str ? (uint8_t)atoi(str) : 0;
    freen(str);
}
void TrodesMsg::processArg(const char c, uint16_t &a){
    if(c != '2') throw std::string(PROCESSERROR "uint16_t");
    char *str = zmsg_popstr(msg);
    a = str ? (uint16_t)atol(str) : 0;
    freen(str);
}
void TrodesMsg::processArg(const char c, uint32_t &a){
    if(c != '4') throw std::string(PROCESSERROR "uint32_t");
    char *str = zmsg_popstr(msg);
    a = str ? (uint32_t) strtoul(str, NULL, 10) : 0;
    freen(str);
}
void TrodesMsg::processArg(const char c, uint64_t &a){
    if(c != '8') throw std::string(PROCESSERROR "uint64_t");
    char *str = zmsg_popstr(msg);
    a = str ? (uint64_t) strtoull(str, NULL, 10) : 0;
    freen(str);
}
void TrodesMsg::processArg(const char c, double &a) {
    if (c != 'd') throw std::string(PROCESSERROR "double");
    char *str = zmsg_popstr(msg);
    a = str ? (double) strtod(str, NULL) : 0;
    freen(str);
}

void TrodesMsg::processArg(const char c, std::string &a){
    if(c != 's') throw std::string(PROCESSERROR "string");
    char *str = zmsg_popstr(msg);
    a.clear();
    a.assign(str);
    freen(str);
}
void TrodesMsg::processArg(const char c, std::vector<byte> &a){
    if(c != 'b') throw std::string(PROCESSERROR "byte vector");
    zframe_t *fr = zmsg_pop(msg);
    a.clear();
    if(fr){
        if(a.size() != zframe_size(fr))
            a.resize(zframe_size(fr));
        memcpy(a.data(), zframe_data(fr), zframe_size(fr));
    }
    zframe_destroy(&fr);
}
void TrodesMsg::processArg(const char c, binarydata &a){
    if(c != 'n') throw std::string(PROCESSERROR "NDT");
    CZHelp::zmsg_popNDT(msg, a);
}


void TrodesMsg::appendArg (const char c, const int a){
    if(c != 'i') throw std::string(APPENDERROR "int");
    zmsg_addstrf (msg, "%d", a);
}
void TrodesMsg::appendArg (const char c, const uint8_t a){
    if(c != '1') throw std::string(APPENDERROR "uint8");
    zmsg_addstrf (msg, "%" PRIu8, a);
}
void TrodesMsg::appendArg (const char c, const uint16_t a){
    if(c != '2') throw std::string(APPENDERROR "uint16");
    zmsg_addstrf (msg, "%" PRIu16, a);
}
void TrodesMsg::appendArg (const char c, const uint32_t a){
    if(c != '4') throw std::string(APPENDERROR "uint32");
    zmsg_addstrf (msg, "%" PRIu32, a);
}
void TrodesMsg::appendArg (const char c, const uint64_t a){
    if(c != '8') throw std::string(APPENDERROR "uint64");
    zmsg_addstrf (msg, "%" PRIu64, a);
}
void TrodesMsg::appendArg (const char c, const double a) {
    if (c != 'd') throw std::string(APPENDERROR "double");
    zmsg_addstr(msg, std::to_string(a).c_str());
}

void TrodesMsg::appendArg (const char c, const std::string &a){
    if(c != 's') throw std::string(APPENDERROR "string");
    zmsg_addstr(msg, a.c_str());
}
void TrodesMsg::appendArg (const char c, const std::vector<byte> &a){
    if(c != 'b') throw std::string(APPENDERROR "byte vector");
    zmsg_addmem(msg, a.data(), a.size());
}
void TrodesMsg::appendArg (const char c, const binarydata &a){
    if(c != 'n') throw std::string(APPENDERROR "NDT");
    CZHelp::zmsg_addNDT(msg, a);
}
