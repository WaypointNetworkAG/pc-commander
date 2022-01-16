/* Author: Noa Sendlhofer - noa.sendlhofer@wpn.ch
 * Desc: PCCommander main
 */

#include <iostream>
#include <csignal>
#include <thread>
#include <cstdio>

#include "arduino_serial.h"

#define HEARTBEAT_RATE 5000

std::atomic<bool> interrupt{true};
ArduinoSerial *arduino_serial;

void sigint_handler(int s){
    printf("Caught signal %d\n",s);
    interrupt = false;
}

void hearbeat_sender()
{
    while(interrupt.load() && arduino_serial->connection_status == STATUS_INITIALIZED)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(HEARTBEAT_RATE));
        if (!arduino_serial->heartbeat_ack.load())
        {
            std::cout << "Heartbeat failed!" << std::endl;
            arduino_serial->connection_status = STATUS_ERROR;
            break;
        }
        arduino_serial->heartbeat_ack = false;
        arduino_serial->send_heartbeat_message();
        std::cout << "Heartbeat success!" << std::endl;
    }
    std::cout << "Exit heartbeat thread" << std::endl;
}

int main(int argc, const char* argv[])
{
    signal (SIGINT, sigint_handler);

    ::ShowWindow(::GetConsoleWindow(), SW_MINIMIZE);

    while (interrupt.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        arduino_serial = new ArduinoSerial();

        if (arduino_serial->g_status != STATUS_SUCCESS) { continue; }

        std::thread heartbeat_thread = std::thread(hearbeat_sender);

        while (interrupt.load() && arduino_serial->connection_status == STATUS_INITIALIZED)
        {
            if (!arduino_serial->update())
            {
                arduino_serial->connection_status = STATUS_ERROR;
                break;
            }
        }

        heartbeat_thread.join();
    }

    return 0;
}