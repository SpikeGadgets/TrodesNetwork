/* 
 * File:   CZHelp.h
 * Author: Maris_Kali
 *
 * Created on August 28, 2017, 2:40 PM
 */

#ifndef CZHELP_H
#define CZHELP_H
// #define __USE_MINGW_ANSI_STDIO 0
#include <iostream>
#include <cstdlib>
#include <vector>
#include <type_traits>
#include <algorithm>
#include <cctype>
#include <locale>
#include "networkDataTypes.h"
struct _zmsg_t; typedef _zmsg_t zmsg_t;

// //Fix for freeing char*'s on windows. 
// #if (defined (__WINDOWS__))
// #define freen(x) do {zstr_free(&x); x = NULL;} while(0)
// #endif
class binarydata;
//class NetworkDataType;

/*!
 * \class CZHelp
 * CZHelp is a class used to facilitate using ZMQ and CZMQ
 *
 * This class wraps several helper functions in the CZHelp namespace (it is not ment to be initiated and used
 * as an object, though you can if you want).  Primarily, CZHelp's purpose is to simplify constructing zmsgs
 * and can be used to encode/decode NetworkDataType derivatives enabling them to be sent over the network.
 */

//typedef std::string binarydata;

class CZHelp {
public:
    CZHelp(void) {}
    ~CZHelp() {}

    /*!
     * \brief systemtimer_realtime returns the real time system date time in milliseconds.
     */
    static int64_t systemTimeMSecs();

    /*!
     * \brief monotimer_msecs returns time in milliseconds for timing/latencies
     * \return time in milliseconds
     */
    static int64_t monotimer_msecs();

    /*!
     * \brief monotimer_usecs returns time in microseconds used for latency testing
     * \return time in microseconds
     */
    static int64_t monotimer_usecs();

    //! returns the ad conversion factor
    static int getADConversionFactor();

    /*! zmsg_addNDT
     * Adds a NetworkDataType derived class to the zmsg_t self.
     * This function adds any NetworkDataType derived class to the input zmsg_t.  Returns 0 if susccessful
     * and -1 if unsuccessful.
     */
    static int zmsg_addNDT(zmsg_t *self, const binarydata &ndtdata);
    /*! zmsg_popNDT
     * Pops a NetworkDataType derived class from the input zmsg_t self.
     * This function pops any NetworkDataType derived class from the input zmsg_t self.  Returns 0 if
     * successful and -1 if unsuccessful.
     */
    static int zmsg_popNDT(zmsg_t *self, binarydata &ndtdata);

    //! \brief Serializes an object using Cereal serialization library
    //! \return false if an exception was caught, else returns true
//    template<typename T>
//    static bool serialize_object(const T &obj, std::string &data);

    //! \brief De-serializes an object using Cereal serialization library
    //! \return false if an exception was caught, else returns true
//    template<typename T>
//    static bool deserialize_object(T &obj, const std::string &data);

    //! trim whitespace from start (in place)
    static void ltrim(std::string &s);

    //! trim whitespace from end (in place)
    static void rtrim(std::string &s);

    //! trim whitespace from both ends (in place)
    static void trim(std::string &s);

    //! trim whitespace from start (copying)
    static std::string ltrim_copy(std::string s);

    //! trim whitespace from end (copying)
    static std::string rtrim_copy(std::string s);

    //! trim whitespace from both ends (copying)
    static std::string trim_copy(std::string s);
};

#endif /* CZHELP_H */

