#include "TransferProtocol.h"

long TransferProtocol::getControlSum(const String& s) {
    return s.length();
}

TransferProtocol::Response::Response(const String& s) {
    if(s.startsWith("got")) {
        type = GOT;
    } else if(s.startsWith("lost")) {
        type = LOST;
    }
}

TransferProtocol::Response::ResponseType TransferProtocol::Response::getType() {
    return type;
}