#ifndef WHITEWATER_TRANSFERPROTOCOL_H
#define WHITEWATER_TRANSFERPROTOCOL_H

#include <Arduino.h>

class TransferProtocol {
public:

    class Response {
    public:
        enum ResponseType {
            GOT, LOST
        };

        explicit Response(const String& s);

        ResponseType getType();

    private:
        ResponseType type;
    };

    static long getControlSum(const String& s);

};


#endif //WHITEWATER_TRANSFERPROTOCOL_H
