#include <ntifs.h>
// #include <ntddk.h> included by ntifs.h
#include "Common.h"


void UnloadRoutine(_In_ PDRIVER_OBJECT DriverObject);

NTSTATUS CreateCloseRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NTSTATUS DeviceIoControlRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath) 
{
	UNREFERENCED_PARAMETER(RegistryPath);

	// Setting Unload Function
	DriverObject->DriverUnload = UnloadRoutine;

	// Supported Dispatch Routines
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateCloseRoutine;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CreateCloseRoutine;

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceIoControlRoutine;

	// Creating Device
	UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\Booster");

	PDEVICE_OBJECT DeviceObject;
	NTSTATUS status = IoCreateDevice(
		DriverObject,
		0,
		&deviceName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&DeviceObject);

	if (!NT_SUCCESS(status))
	{
		KdPrint(("Failed to create device object (0x%08X)\n", status));
		return status;
	}

	// Creating Symbolic Link
	UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\Booster");
	status = IoCreateSymbolicLink(&symbolicLink, &deviceName);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Failed to create symbolic link (0x%08X)\n", status));
		IoDeleteDevice(DeviceObject);
		return status;
	}
	
	// Driver is ready to accept requests
	KdPrint(("Driver is ready to accept requests.\n"));
	return STATUS_SUCCESS;
}

NTSTATUS CreateCloseRoutine(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	KdPrint(("Called CreateCloseRoutine."));
	
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS DeviceIoControlRoutine(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	KdPrint(("Called DeviceIoControlRoutine"));

	NTSTATUS status = STATUS_SUCCESS;
	ULONG_PTR information = 0;

	PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
	do
	{
		if (irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ThreadData))
		{
			KdPrint(("Buffer too small (%d < %I64u).\n", irpSp->Parameters.DeviceIoControl.InputBufferLength, sizeof(ThreadData)));
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		PThreadData data = (PThreadData)(Irp->AssociatedIrp.SystemBuffer);
		if (data == NULL || data->Priority < 1 || data->Priority > 31)
		{
			KdPrint(("Invalid input parameter.\n"));
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		PETHREAD thread;
		status = PsLookupThreadByThreadId(ULongToHandle(data->ThreadId), &thread);
		if (!NT_SUCCESS(status))
		{
			KdPrint(("Unable to find thread by threadid.\n"));
			break;
		}

		KPRIORITY oldPriority = KeSetPriorityThread(thread, data->Priority);
		KdPrint(("Priority change for thread %u from %d to %d succeeded!\n", data->ThreadId, oldPriority, data->Priority));

		ObDereferenceObject(thread);

		information = sizeof(data);
	} while (FALSE);

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = information;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

void UnloadRoutine(
	_In_ PDRIVER_OBJECT DriverObject)
{
	// Delete Symlink
	UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\??\\Booster");
	IoDeleteSymbolicLink(&symbolicLink);

	// Delete Device Object
	IoDeleteDevice(DriverObject->DeviceObject);

	KdPrint(("Driver has been unloaded.\n"));
}