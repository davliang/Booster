// Boost.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include "..\Booster\Common.h";

int main(int argc, const char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: Boost <threadid> <priority>\n";
        return 1;
    }

    int tId = atoi(argv[1]);
    int priority = atoi(argv[2]);

    HANDLE hDevice = CreateFile(L"\\\\.\\Booster", GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hDevice == INVALID_HANDLE_VALUE)
    {
        std::cout << "Failed to open device (error=" << GetLastError() << ")\n";
        return 1;
    }

    ThreadData data;
    data.ThreadId = tId;
    data.Priority = priority;

    DWORD returned;
    BOOL success = DeviceIoControl(hDevice, IOCTL_Change_Process_Priority, &data, sizeof(data), nullptr, 0, &returned , nullptr);
    if (!success)
    {
        std::cout << "Priority change failed! (error=" << GetLastError() << ")\n";
        return 1;
    }

    std::cout << "Priority change succeeded!\n";

    CloseHandle(hDevice);
}