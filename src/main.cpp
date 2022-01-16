// Author: Noa Sendlhofer

#include <iostream>
#include <csignal>
#include <thread>
#include <cstdio>

#include "arduino_serial.h"

#define HEARTBEAT_RATE 5000

std::atomic<bool> interrupt{true};
std::atomic<bool> heartbeat{true};
ArduinoSerial *arduino_serial;

void sigint_handler(int s){
    printf("Caught signal %d\n",s);
    interrupt = false;
    heartbeat = false;
}

void hearbeat_sender()
{
    while(heartbeat.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(HEARTBEAT_RATE));
        if (!arduino_serial->heartbeat_ack.load())
        {
            std::cout << "Heartbeat failed!" << std::endl;
            //TODO: Reload program - retry connection!
        }
        arduino_serial->heartbeat_ack = false;
        arduino_serial->send_heartbeat_message();
        std::cout << "Heartbeat success!" << std::endl;
    }
    std::cout << "Exit heartbeat thread" << std::endl;
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

    while (interrupt.load())
    {
        arduino_serial = new ArduinoSerial();

        if (arduino_serial->g_status != STATUS_SUCCESS) { continue; }

        heartbeat = true;
        std::thread heartbeat_thread = std::thread(hearbeat_sender);

        while (interrupt.load())
        {
            if (!arduino_serial->update())
            {
                heartbeat = false;
                break;
            }
        }

        heartbeat_thread.join();
    }

    std::cout << "Exit main thread" << std::endl;

    return 0;
}