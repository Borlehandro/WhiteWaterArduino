#include <Arduino.h>
#include "Command.h"
#include "TransferProtocol.h"

bool monitoringStarted, dataTransferStarted;

char test[1054] = {"Transfer test 1.\n"
                   "Transfer test 2.\n"
                   "Transfer test 3.\n"
                   "Transfer test 4.\n"
                   "Transfer test 5.\n"
                   "Transfer test 6.\n"
                   "Transfer test 7.\n"
                   "Transfer test 8.\n"
                   "Transfer test 9.\n"
                   "Transfer test 10.\n"
                   "Transfer test 11.\n"
                   "Transfer test 12.\n"
                   "Transfer test 13.\n"
                   "Transfer test 14.\n"
                   "Transfer test 15.\n"
                   "Transfer test 16.\n"
                   "Transfer test 17.\n"
                   "Transfer test 18.\n"
                   "Transfer test 19.\n"
                   "Transfer test 20.\n"
                   "Transfer test 21.\n"
                   "Transfer test 22.\n"
                   "Transfer test 23.\n"
                   "Transfer test 24.\n"
                   "Transfer test 25.\n"
                   "Transfer test 26.\n"
                   "Transfer test 27.\n"
                   "Transfer test 28.\n"
                   "Transfer test 29.\n"
                   "Transfer test 30.\n"
                   "Transfer test 31.\n"
                   "Transfer test 32.\n"
                   "Transfer test 33.\n"
                   "Transfer test 34.\n"
                   "Transfer test 35.\n"
                   "Transfer test 36.\n"
                   "Transfer test 37.\n"
                   "Transfer test 38.\n"
                   "Transfer test 39.\n"
                   "Transfer test 40.\n"
                   "Transfer test 41.\n"
                   "Transfer test 42.\n"
                   "Transfer test 43.\n"
                   "Transfer test 44.\n"
                   "Transfer test 45.\n"
                   "Transfer test 46.\n"
                   "Transfer test 47.\n"
                   "Transfer test 48.\n"
                   "Transfer test 49.\n"
                   "Transfer test 50.\n"
                   "Transfer test 51.\n"
                   "Transfer test 52.\n"
                   "Transfer test 53.\n"
                   "Transfer test 54.\n"
                   "Transfer test 55.\n"
                   "Transfer test 56.\n"
                   "Transfer test 57.\n"
                   "Transfer test 58.\n"
                   "Transfer test 59.\n"};

const int DATA_SIZE = 1054;

const int TRANSFER_BUFFER_SIZE = 1024;

void waitingForInput() {
    while (!Serial3.available()) {}
}

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
                Serial.println(String("$Connection ") +
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
                Serial.println(String("$Monitoring ") +
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
                Serial.println(String("$Monitoring ") +
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
                Serial.print("unknown command");
                break;
            }
        }
    }

    if (monitoringStarted) {
        long a = random(1000);

        Serial3.println(String("$Monitoring ") +
                        "{" +
                        "status:" + "\"monitoring\"" + ","
                                                       "data:" +
                        " {"
                        + "test:" + a
                        + "}"
                        + "}");

        Serial.println(String("$Monitoring ") +
                       "{" +
                       "status:" + "\"monitoring\"" + ","
                                                      "data:" +
                       " {"
                       + "test:" + a
                       + "}"
                       + "}");
    }

    if (dataTransferStarted) {

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

            Serial.println(String("$Transfer ")
                           + "{"
                           + "status:" + "\"prepare\"" + ","
                           + "fileSize:" + DATA_SIZE
                           + "}");

            waitingForInput();
            response = TransferProtocol::Response(Serial3.readString());
        } while (response.getType() != TransferProtocol::Response::GOT);

        bool wasLost = false;
        int end = 0;

        while (position < DATA_SIZE && dataTransferStarted) {

            bufferSize = (position + TRANSFER_BUFFER_SIZE) < DATA_SIZE ? TRANSFER_BUFFER_SIZE
                                                                       : TRANSFER_BUFFER_SIZE -
                                                                         ((position + TRANSFER_BUFFER_SIZE) %
                                                                          DATA_SIZE);
            if (!wasLost) {

                // TODO Write normal loop exit
                if (bufferSize == 0)
                    break;

                if (end < DATA_SIZE) {
                    bufferSize--;
                }

                end = position + bufferSize;

                response = TransferProtocol::Response(String());

                bufferSize += 2;

                // Check if got checksum
                do {
                    Serial3.println(String("$Transfer ")
                                    + "{"
                                    + "status:" + "\"transfer\"" + ","
                                    + "checksum:" + String(bufferSize)
                                    + "}");

                    Serial.println(String("$Transfer ")
                                   + "{"
                                   + "status:" + "\"transfer\"" + ","
                                   + "checksum:" + String(bufferSize)
                                   + "}");

                    waitingForInput();
                    response = TransferProtocol::Response(Serial3.readString());
                } while (response.getType() != TransferProtocol::Response::GOT
                         && response.getType() != TransferProtocol::Response::REFRESH);

            }

            // Send data
            prePosition = position;
            char buffer[bufferSize];

            int i = 1;
            buffer[0] = '#';
            while (position < end) {
                buffer[i] = test[position];
                position++;
                i++;
            }
            Serial.println(String("Try to write \'#\' into ") + i);
            buffer[i] = '#';

            Serial3.print(buffer);
            Serial.print(buffer);

            digitalWrite(13, 1);
            // Check if get data
            Serial.println("Waiting for input...");
            waitingForInput();
            response = TransferProtocol::Response(Serial3.readString());
            digitalWrite(13, 0);

            switch (response.getType()) {

                case TransferProtocol::Response::REFRESH: {
                    dataTransferStarted = false;
                    break;
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

    delay(200);
}