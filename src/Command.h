#ifndef WHITEWATER_COMMAND_H
#define WHITEWATER_COMMAND_H

#include <Arduino.h>

class Command {
public:

    enum CommandType {
        CONNECT,
        START_MONITORING,
        UNKNOWN,
        STOP_MONITORING,
        START_DATA_TRANSFER
    };

    explicit Command(const String& s);

    CommandType getType();
private:
    CommandType type;

};


#endif //WHITEWATER_COMMAND_H
