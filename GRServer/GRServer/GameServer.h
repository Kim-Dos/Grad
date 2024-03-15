#pragma once
#include "TCPDevice.h"
#include <atomic>

std::atomic_int roomNumber = 0;

int GetRoomNumver() { return roomNumber++; }


class GameTCP : public TCPDevice {

public:

	void StartSession();

private:

};