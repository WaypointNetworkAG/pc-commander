/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2021, Noa Sendlhofer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Author: Noa Sendlhofer

#include "arduino_serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

#define SERIAL_PORT "COM9"

ArduinoSerial::ArduinoSerial()
{
    serialib serial;

    get_available_COM_ports();
    /*
    char errorOpening = serial.openDevice(SERIAL_PORT, 115200);
    if (errorOpening!=1)
        std::cout << "Error Opening" << std::endl;
    else
    {
        printf ("Successful connection to %s\n",SERIAL_PORT);

        char* buf = new char[100];

        while(true)
        {
            serial.readString(buf, '\n', 100);
            std::cout << buf << std::endl;
            if (strcmp(buf, "return\n") == 0)
            {
                shutdown();
                break;
            }
        }
    }
     */
}

void ArduinoSerial::shutdown()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    // Get a token for this process.

    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return;

    // Get the LUID for the shutdown privilege.

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,
                         &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;  // one privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Get the shutdown privilege for this process.

    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
                          (PTOKEN_PRIVILEGES)NULL, 0);

    if (GetLastError() != ERROR_SUCCESS)
        return;

    // Shut down the system and force all applications to close.

    if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE,
                       SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
                       SHTDN_REASON_MINOR_UPGRADE |
                       SHTDN_REASON_FLAG_PLANNED))
        return;

    //shutdown was successful
    return;
}

bool ArduinoSerial::get_available_COM_ports()
{
    DWORD test;
    bool gotPort = 0; // in case the port is not found

    for(int i = 0; i < 255; i++) // checking ports from COM0 to COM255
    {
        char number_str[10];
        sprintf(number_str, "%d", i);

        char *ComName = new char[strlen("COM") + strlen(number_str) + 1];
        strcpy(ComName,"COM");
        strcat(ComName,number_str);

        test = QueryDosDevice(ComName, (LPSTR)this->COMPaths, 5000);

        // Test the return value and error if any
        if(test != 0) //QueryDosDevice returns zero if it didn't find an object
        {
            std::cout << ComName << std::endl;
            gotPort = 1; // found port
        }

        if(::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            this->COMPaths[10000]; // in case the buffer got filled, increase size of the buffer.
            continue;
        }
    }

    return gotPort;
}
