#pragma once
#include <Windows.h>
class VideoConsumer
{
private:
	// Define the shared memory name and size


	void read();
public:
	bool init();
};

