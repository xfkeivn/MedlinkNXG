#include "VideoConsumer.h"
#include <windows.h>
#include <iostream>




bool VideoConsumer::init()
{

	// Open the shared memory object
	HANDLE shm = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, shm_name);

	if (shm == NULL)
	{
		std::cout << "Error opening shared memory: " << GetLastError() << std::endl;
		return false;
	}

	// Map the shared memory object to the local address space
	shm_address = MapViewOfFile(shm, FILE_MAP_ALL_ACCESS, 0, 0, shm_size);

	if (shm_address == NULL)
	{
		std::cout << "Error mapping shared memory: " << GetLastError() << std::endl;
		CloseHandle(shm);
		return 1;
	}
}

void VideoConsumer::read()
{


		while (true)
		{
		
		}

		// Unmap the shared memory object
		UnmapViewOfFile(shm_address);
		CloseHandle(shm);

	
	

}