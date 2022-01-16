/* Author: Noa Sendlhofer - noa.sendlhofer@wpn.ch
 * Desc: Windows API
 */

#include "windows_actions.h"

#include <cstdio>
#include <iostream>
#include <ostream>

void WindowsActions::shutdown()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return;

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,
                         &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
                          (PTOKEN_PRIVILEGES)NULL, 0);

    if (GetLastError() != ERROR_SUCCESS)
        return;

    if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE,
                       SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
                       SHTDN_REASON_MINOR_UPGRADE |
                       SHTDN_REASON_FLAG_PLANNED))
        return;

    return;
}

bool WindowsActions::get_available_COM_ports()
{
    DWORD test;
    bool gotPort = 0;
    TCHAR COMPath[5000];

    for(int i = 0; i < 255; i++)
    {
        char number_str[10];
        sprintf(number_str, "%d", i);

        char *ComName = new char[strlen("COM") + strlen(number_str) + 1];
        strcpy(ComName,"COM");
        strcat(ComName,number_str);

        test = QueryDosDevice(ComName, (LPSTR)COMPath, 5000);

        if(test != 0)
        {
            gotPort = 1;
            this->available_com_ports.push_back(ComName);
        }

        if(::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            COMPath[10000];
            continue;
        }
    }

    return gotPort;
}

void WindowsActions::send_keystroke(char key)
{
    std::cout << "Send keypress!" << std::endl;
    INPUT inputs[2] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = key;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = key;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
    if (uSent != ARRAYSIZE(inputs))
    {
        std::cout << "Keypress failed!" << std::endl;
    }
}
