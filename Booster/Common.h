#pragma once
// #include <ntddk.h>
// #include <ntdef.h>

#define IOCTL_Change_Process_Priority CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_WRITE_DATA)


typedef struct
{
	ULONG ThreadId;
	int Priority;
} ThreadData, *PThreadData;