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

#ifndef TEST_ARDUINO_SERIAL_H_
#define TEST_ARDUINO_SERIAL_H_

#include "serialib.h"
#include <windows.h>
#include <vector>
#include <base64_rfc4648.hpp>
#include <CRC.h>
#include <atomic>

using base64 = cppcodec::base64_rfc4648;

class ArduinoSerial
{
public:
    ArduinoSerial();
    void shutdown();
    void update();

private:
    serialib *serial;

    bool get_available_COM_ports();
    std::vector<char*> available_com_ports;

    void device_handshake();

    unsigned char *decode(char *data);
    char *encode(char *data);
    bool verify_checksum(unsigned char *msg);

    char * __insert_initial_char(char *message) const;

    const char msg_end = 38;
    const int msg_length_encoded = 16;
    const int msg_length_decoded = 12;

    const char *host_key      = "T6z$}~B{";
    const char *device_key    = "GXB)5jbS";
    const char *success_msg   = "SUCCESS!";
    const char *error_msg     = "ERROR!--";
    const char *heartbeat_msg = "STATUS--";

    std::atomic<bool> connected{false};
    std::atomic<bool> try_update{false};
};

#endif //TEST_ARDUINO_SERIAL_H_
