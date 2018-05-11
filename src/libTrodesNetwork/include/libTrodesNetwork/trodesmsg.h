#ifndef TRODESMSG_H
#define TRODESMSG_H

#include <string>
#include <vector>
#include <iostream>
#include "networkDataTypes.h"
#include "CZHelp.h"

typedef unsigned char byte;

/*!
 * \brief TrodesMsg is a class that encapsulates zmsg_t and other related properties
 *
 * Usage:
 * Creating a new TrodesMsg to send: pass in the format string, and the variables you wish to encode into the msg
 *  ex: TrodesMsg message("ssi", "string1", "string2", 42);
 *
 * Decoding a received TrodesMsg: Pass the format string, and the variables you wish to fill with the contents
 *  ex: receivedMessage.popcontents("ssi", stringVar1, stringVar2, intVar);
 *
 * Additional functions:
 * popstr(): pop the first part of the message off as a string
 * The current supported formats follow the czmq/zmq picture format paradigm and are as follows:
 *          * i = int
 *          * 1 = uint8_t
 *          * 2 = uint16_t
 *          * 4 = uint32_t
 *          * 8 = uint64_t
 *          * d = double
 *          * s = std::string
 *          * p = vector<byte>
 *          * n = network data types
 *
 *
 * Reasons for making this class:
 * 1. Abstract away zmsg_t and low level zmsg functions
 * 2. Use C++11 functionality to provide type safety and remove need for manual deallocations by the user
 * 3. Compile time checking if they pass in an unsupported type
 * 4. Not using va_arg and void*'s means we can check during runtime if passed format matches order of their args
 * 5. Also not using va_arg and void*'s means we can use references instead of pointers to avoid accidental
 *      null's and bad allocations resulting in segfaults (ideally, still testing to verify)
 * 6. Allow the wrapper to let users interact with std::strings instead of char*'s.
 */
class TrodesMsg
{
public:
    //! Create empty message
    TrodesMsg();

    //! Read message from format and zmsg_t*
    TrodesMsg(std::string f, zmsg_t *content);

    //! Create message from scratch
    template <typename H, typename... T>
    TrodesMsg(std::string f, H head, T... tail) : TrodesMsg(){
        addcontents(f, head, tail...);
    }
    //! Copy constructor
    TrodesMsg(const TrodesMsg &t);

    //! Move Constructor
    TrodesMsg(TrodesMsg &&t);

    //! Destructor: deallocates msg
    ~TrodesMsg();

    //! Returns a copy of this TrodesMsg
    TrodesMsg copy();

    //! Gets the pointer to the underlying zmsg_t*
    zmsg_t* getzmsg();

    bool isvalid();

    //! Gets the format of the underlying zmsg_t*
    std::string getformat() const;

    //! Sets the format
    void setformat(const std::string &value);

    //! Gets the size (number of frames) of the zmsg_t
    size_t numContents(void) const ;

    //! Gets the total byte size of the contents of the zmsg_t
    size_t size();

    //! Add a string to the end of the message.
    void addstr(std::string str);

    //! Pop the next item in the string as a string. only use if you know it will be a string
    std::string popstr();

    //! Read msg into passed variables using format string.
    //! USAGE: Do not pass in pointers/addresses with the exception of NDT's. The rest are all
    //! passed by reference, no need to use potentially unsafe raw pointers here.
    template <typename H, typename... T>
    void popcontents(std::string f, H &head, T & ... tail){
        try{
            if(f.empty())
                throw std::string("TrodesMsg Error: Mismatched format string and variables");
            processArg(f[0], head);
            format.erase(0, 1);
            popcontents(f.substr(1), tail...);
        } catch(std::string e){
            std::cout << "[TrodesMsg::popcontents Error] " << e << "\n";
        } catch(...){
            std::cout << "[TrodesMsg::popcontents Error] Unknown exception thrown!\n";
        }
    }

    //! Create message with passed variables using format string
    //! USAGE: Pass in format and variables by value (string, byte vector, and NDT are references to be faster).
    //!
    template <typename H, typename... T>
    void addcontents(std::string f, H head, T... tail){
        try{
            if(f.empty())
                throw std::string("TrodesMsg Error: Mismatched format string and variables");
            appendArg(f[0], head);
            format.push_back(f[0]);
            addcontents(f.substr(1), tail...);
        } catch(std::string e){
            std::cerr << "[TrodesMsg::addcontents Error] " << e << "\n";
        } catch(...){
            std::cout << "[TrodesMsg::addcontents Error] Unknown exception thrown!\n";
        }
    }

protected:
    std::string format;
    zmsg_t *msg;


    //*******************Variadic template stuff
    void popcontents(std::string f){}                  //!< Stop condition for variadic templates
    void addcontents(std::string f){}                    //!< Stop condition for variadic templates
    void processArg(const char c, int &a);                  //!< Checks for int and assigns value from msg to 'a'
    void processArg(const char c, uint8_t &a);              //!< Checks for uint8_t and assigns value from msg to 'a'
    void processArg(const char c, uint16_t &a);             //!< Checks for uint16_t and assigns value from msg to 'a'
    void processArg(const char c, uint32_t &a);             //!< Checks for uint32_t and assigns value from msg to 'a'
    void processArg(const char c, uint64_t &a);             //!< Checks for uint64_t and assigns value from msg to 'a'
    void processArg(const char c, double &a);               //!< Checks for double and assigns value from msg to 'a'
    void processArg(const char c, std::string &a);          //!< Checks for string and assigns value from msg to 'a'
    void processArg(const char c, std::vector<byte> &a);    //!< Checks for byte vector and assigns value from msg to 'a'
    void processArg(const char c, binarydata &a);           //!< Checks for NDT and assigns value from msg to 'a'
    void appendArg (const char c, const int a);             //!< Appends int to msg
    void appendArg (const char c, const uint8_t a);         //!< Appends uint8_t to msg
    void appendArg (const char c, const uint16_t a);        //!< Appends uint16_t to msg
    void appendArg (const char c, const uint32_t a);        //!< Appends uint32_t to msg
    void appendArg (const char c, const uint64_t a);        //!< Appends uint64_t to msg
    void appendArg (const char c, const double a);          //!< Appends double to msg
    void appendArg (const char c, const std::string &a);    //!< Appends string to msg
    void appendArg (const char c, const std::vector<byte> &a);//!< Appends byte vector to msg
    void appendArg (const char c, const binarydata &a);      //!< Appends NetworkDataType to msg
};

#endif // TRODESMSG_H
