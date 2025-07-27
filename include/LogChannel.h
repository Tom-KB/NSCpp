/**
 * Project NSCpp
 * @author Thomas K/BIDI
 * @version 2.0
 */


#ifndef _LOGCHANNEL_H
#define _LOGCHANNEL_H

#include "Channel.h"
#include "Serializer.h"
#include <iostream>
#include <fstream>
#include <format>
#include <chrono>

/*
* NB : If you want to log the message send to the clients you can use the logging callback
*       of this Channel, register it inside a group and send your message in that group, it will
*       be automatically register in the log file
*/

class LogChannel: public Channel {
public:
    /**
     * @brief Constructor of the LogChannel.
     * @param key Channel's key.
     * @param logFilePrefix Prefix given to the log files. (Every log file will have the .log extension)
     */
    LogChannel(const std::string& key, const std::string& logFilePrefix);

    /**
     * @brief Run method of the Channel, process the given events.
     */
    void run(std::stop_token st);

    /**
     * @brief Callback which can be used trough the group (based on this channel's key) by other channels. 
     * @details Every Channels can call the LogChannel's group and send a message which will be logged.
     * @param logEntry Message you want to be logged.
     */
    void log(const std::string& logEntry);

protected:
    /**
     * @brief Used to load/create the log file
     * @details If the log file already exist it's load and new logs will be append, otherwise it is created.
     * @details The log file have the name logFilePrefix + currentData[%Y-%M-%D] + ".log"
     * @param name Log file's name.
     */
    void getStream(const std::string& name);

    /**
     * @brief Load in the given streamstring the current time in the right format, and change the log file if needed (the date changed).
     * @param ss Stringstream in which the current time (up to the seconds) is loaded.
     */
    void loadTime(std::stringstream& ss);

    /**
     * Today's date
     */ 
    std::chrono::system_clock::time_point today;

    /**
     * Log file's prefix
     */
    std::string prefix;

    /**
     * Log file's stream
     */
    std::ofstream stream;

    /**
     * Mutex for the log callback
     */
    std::shared_mutex mtx;
};

#endif //_LOGCHANNEL_H