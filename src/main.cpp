#include <Arduino.h>
#include "Command.h"
#include "TransferProtocol.h"

bool monitoringStarted, dataTransferStarted;
char test[1000] = {"TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT"};

const int TRANSFER_BUFFER_SIZE = 256;

void waitingForInput() {
    while (!Serial3.available()) {}
}

void setup() {
    Serial.begin(9600);
    Serial3.begin(9600);
    monitoringStarted = false;
    dataTransferStarted = false;
}

void loop() {

    if (Serial3.available()) {
        auto *command = new Command(Serial3.readString());
        switch (command->getType()) {
            case Command::CONNECT: {
                Serial3.println("ok");
                break;
            }
            case Command::START_MONITORING: {
                monitoringStarted = true;
                Serial3.println(String("$Monitoring") +
                                " {" +
                                "status:" + "start" + ";"
                                + "}");
                break;
            }
            case Command::STOP_MONITORING: {
                Serial3.println(String("$Monitoring") +
                                " {" +
                                "status:" + "stop" + ";"
                                + "}");

                monitoringStarted = false;
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
        long a = random(1000);

        Serial3.println(String("$Monitoring") +
                        " {" +
                        "status:" + "monitoring" + ";"
                        "data:" +
                        " {"
                        + "test:" + a
                        + "}"
                        + "}");
    }

    if (dataTransferStarted) {

        int prePosition = 0, position = 0;
        int bufferSize;

        Serial3.println(String("$Transfer")
        + " {"
        + "status:" + "prepare" + ";"
        + "fileSize:" + 1000 + ";"
        + "}");

        while (position < 1000) {
            bufferSize = (position + TRANSFER_BUFFER_SIZE) < 1000 ? TRANSFER_BUFFER_SIZE
                                                                  : TRANSFER_BUFFER_SIZE -
                                                                    ((position + TRANSFER_BUFFER_SIZE) % 1000);
            int end = position + bufferSize;

            Serial3.println(String("$Transfer")
                            + " {"
                            + "status:" + "transfer" + ";"
                            + "checksum:" + String(bufferSize)
                            + "}");

            waitingForInput();
            TransferProtocol::Response response = TransferProtocol::Response(Serial3.readString());
            switch (response.getType()) {
                case TransferProtocol::Response::LOST: {
                    end = position;
                    position = prePosition;
                }

                    // Todo Fix!
                label :
                case TransferProtocol::Response::GOT: {

                    prePosition = position;

                    while (position < end) {
                        Serial3.print(test[position]);
                        position++;
                    }

                    Serial3.flush();
                    Serial3.println();

                    if (position >= 1000) {

                        Serial3.println(String("$Transfer")
                                        + " {"
                                        + "status:" + "check" + ";"
                                        + "}");

                        waitingForInput();
                        TransferProtocol::Response lastResponse = TransferProtocol::Response(Serial3.readString());
                        if (lastResponse.getType() == TransferProtocol::Response::LOST) {
                            position = prePosition;
                            // Todo Fix!
                            goto label;
                        }
                    }
                    break;
                }
            }
        }
        Serial3.println(String("$Transfer")
                        + " {"
                        + "status:" + "finish" + ";"
                        + "}");

        dataTransferStarted = false;
    }

    delay(1000);
}