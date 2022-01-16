// Author: Noa Sendlhofer

#include <iostream>
#include <csignal>
#include <thread>
#include <cstdio>

#include "arduino_serial.h"

std::atomic<bool> interrupt{true};
ArduinoSerial *arduino_serial;

void sigint_handler(int s){
    printf("Caught signal %d\n",s);
    interrupt = false;
}

void hearbeat_sender()
{
    while(interrupt.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        if (!arduino_serial->heartbeat_ack.load())
        {
            std::cout << "Heartbeat failed!" << std::endl;
            //TODO: Reload program - retry connection!
        }
        arduino_serial->send_heartbeat_message();
        std::cout << "Heartbeat success!" << std::endl;
    }
}

int main(int argc, const char* argv[])
{
    /*
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = sigint_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, nullptr);
     */
    signal (SIGINT, sigint_handler);

    arduino_serial = new ArduinoSerial();

    if (arduino_serial->g_status != STATUS_SUCCESS)
    {
        //TODO: Retry connection!
        return 0;
    }

    std::thread heartbeat_thread = std::thread(hearbeat_sender);

    while (interrupt.load())
    {
        arduino_serial->update();
    }

    return 0;
}