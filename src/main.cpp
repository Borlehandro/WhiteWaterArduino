#include <Arduino.h>
#include "Command.h"
#include "TransferProtocol.h"

bool monitoringStarted, dataTransferStarted;
char test[1025] = {
        "Lorem ipsum dolor sit amet, consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi.Lorem ipsum dolor sit amet, consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent lupta"};
const int DATA_SIZE = 1025;

const int TRANSFER_BUFFER_SIZE = 512;

void waitingForInput();
void transferData();
void sendMonitoringData();

void setup() {
    Serial.begin(9600);
    Serial3.begin(115200);
    monitoringStarted = false;
    dataTransferStarted = false;
}

void loop() {

    if (Serial3.available()) {
        auto *command = new Command(Serial3.readString());
        switch (command->getType()) {
            case Command::CONNECT: {
                Serial3.println(String("$Connection ") +
                                "{" +
                                "status:" + "\"start\""
                                + "}");
                monitoringStarted = false;
                dataTransferStarted = false;
                break;
            }
            case Command::START_MONITORING: {
                monitoringStarted = true;
                Serial3.println(String("$Monitoring ") +
                                "{" +
                                "status:" + "\"start\""
                                + "}");
                break;
            }
            case Command::STOP_MONITORING: {
                monitoringStarted = false;

                Serial3.println(String("$Monitoring ") +
                                "{" +
                                "status:" + "\"stop\""
                                + "}");
                break;
            }
            case Command::START_DATA_TRANSFER: {
                dataTransferStarted = true;
                break;
            }
            case Command::UNKNOWN: {
                Serial3.print("unknown command");
                break;
            }
        }
    }

    if (monitoringStarted) {
        sendMonitoringData();
    }

    if (dataTransferStarted) {
        transferData();
    }

    delay(200);
}

void waitingForInput() {
    while (!Serial3.available()) {}
}

void sendMonitoringData() {
    long a = random(1000);

    Serial3.println(String("$Monitoring ") +
                    "{" +
                    "status:" + "\"monitoring\"" + ","
                                                   "data:" +
                    " {"
                    + "test:" + a
                    + "}"
                    + "}");
}

void transferData() {

    int prePosition = 0, position = 0;
    int bufferSize;
    TransferProtocol::Response response = TransferProtocol::Response(String());

    // Check if got fileSize
    do {
        Serial3.println(String("$Transfer ")
                        + "{"
                        + "status:" + "\"prepare\"" + ","
                        + "fileSize:" + DATA_SIZE
                        + "}");

        waitingForInput();
        response = TransferProtocol::Response(Serial3.readString());
    } while (response.getType() != TransferProtocol::Response::GOT
             || response.getType() != TransferProtocol::Response::REFRESH);

    if(response.getType() == TransferProtocol::Response::REFRESH) {
        dataTransferStarted = false;
        Serial3.println(String("$Connection ") +
                        "{" +
                        "status:" + "\"start\""
                        + "}");
        return;
    }

    bool wasLost = false;
    int end = 0;

    while (position < DATA_SIZE && dataTransferStarted) {

        bufferSize = (position + TRANSFER_BUFFER_SIZE) < DATA_SIZE ? TRANSFER_BUFFER_SIZE
                                                                   : TRANSFER_BUFFER_SIZE -
                                                                     ((position + TRANSFER_BUFFER_SIZE) %
                                                                      DATA_SIZE);

        if (!wasLost) {
            end = position + bufferSize;

            if (end >= DATA_SIZE)
                bufferSize--;

            response = TransferProtocol::Response(String());

            // Check if got checksum
            do {
                Serial3.println(String("$Transfer ")
                                + "{"
                                + "status:" + "\"transfer\"" + ","
                                + "checksum:" + String(bufferSize)
                                + "}");

                waitingForInput();
                response = TransferProtocol::Response(Serial3.readString());
            } while (response.getType() != TransferProtocol::Response::GOT
                     || response.getType() != TransferProtocol::Response::REFRESH);

            if(response.getType() == TransferProtocol::Response::REFRESH) {
                dataTransferStarted = false;
                Serial3.println(String("$Connection ") +
                                "{" +
                                "status:" + "\"start\""
                                + "}");
                break;
            }

        }

        // Send data
        prePosition = position;
        char buffer[bufferSize];

        int i = 0;
        while (position < end) {
            buffer[i] = test[position];
            position++;
            i++;
        }

        Serial3.print(buffer);

        // Check if get data
        digitalWrite(13, 1);
        waitingForInput();
        response = TransferProtocol::Response(Serial3.readString());
        digitalWrite(13, 0);

        switch (response.getType()) {

            case TransferProtocol::Response::REFRESH: {
                dataTransferStarted = false;
                Serial3.println(String("$Connection ") +
                                "{" +
                                "status:" + "\"start\""
                                + "}");
                return;
            }

            case TransferProtocol::Response::LOST: {
                wasLost = true;
                end = position;
                position = prePosition;
                break;
            }

            case TransferProtocol::Response::GOT: {
                wasLost = false;
                break;
            }
        }
    }

    dataTransferStarted = false;
}
