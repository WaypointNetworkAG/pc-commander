/* Author: Noa Sendlhofer - noa.sendlhofer@wpn.ch
 * Desc: Windows API
 */

#ifndef PC_COMMANDER_WINDOWS_ACTIONS_H_
#define PC_COMMANDER_WINDOWS_ACTIONS_H_

#include <vector>
#include <windows.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

class WindowsActions
{
protected:
    void shutdown();
    void send_keystroke(char key);
    bool get_available_COM_ports();
    std::vector<char*> available_com_ports;
};

#endif //PC_COMMANDER_WINDOWS_ACTIONS_H_
