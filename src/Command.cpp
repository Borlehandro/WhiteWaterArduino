#include "Command.h"

Command::Command(const String& s) {
    if (s.startsWith("Connect")) {
        type = CONNECT;
    } else if (s.startsWith("StartMonitoring")) {
        type = START_MONITORING;
    } else if (s.startsWith("StopMonitoring")) {
        type = STOP_MONITORING;
    } else if (s.startsWith("StartDataTransfer")) {
        type = START_DATA_TRANSFER;
    } else {
        type = UNKNOWN;
    }
}

Command::CommandType Command::getType() {
    return this->type;
}