#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"

struct FDualSenseDeviceCandidate
{
	FInputDeviceId DeviceId;
	FName InputDeviceName;
	bool bConnected = false;
};

struct FDualSenseDeviceRoute
{
	FInputDeviceId DeviceId = INPUTDEVICEID_NONE;
	int32 ControllerId = INDEX_NONE;
	bool IsValid() const { return DeviceId.IsValid() && ControllerId != INDEX_NONE; }
};

/** Select a connected Apple DualSense and remap that exact device, never the latest input. */
inline FDualSenseDeviceRoute ResolveDualSenseDevice(
	TConstArrayView<FDualSenseDeviceCandidate> Devices,
	TFunctionRef<bool(FInputDeviceId, int32&)> Remap)
{
	for (const FDualSenseDeviceCandidate& Device : Devices)
	{
		if (!Device.bConnected || !Device.DeviceId.IsValid() || Device.InputDeviceName != FName("PS5 Wireless Controller"))
		{
			continue;
		}
		FDualSenseDeviceRoute Route;
		Route.DeviceId = Device.DeviceId;
		if (Remap(Device.DeviceId, Route.ControllerId) && Route.IsValid())
		{
			return Route;
		}
	}
	return {};
}
