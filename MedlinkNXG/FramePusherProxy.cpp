#include "FramePusherProxy.h"
#include "ShareMem.h"
#include "AGDShowVideoCapture.h"
#include <shlwapi.h>
#include <sstream>
#include "spdlog/sinks/rotating_file_sink.h"
extern std::shared_ptr<spdlog::logger> logger;
FramePusherProxy::FramePusherProxy()
{

}
void FramePusherProxy::deinit()
{
	DWORD result = WaitForSingleObject(hMutex, INFINITE);
	if (result == WAIT_OBJECT_0)
	{
		// Access the shared resource
		Command cmd = EXIT;
		memcpy_s((BYTE*)m_shm_address, sizeof(Command), &cmd, sizeof(Command));
		// Release ownership of the mutex
		ReleaseMutex(hMutex);
	}
	WaitForSingleObject(pi.hProcess,200);
	CloseHandle(pi.hProcess);
	UnmapViewOfFile(m_shm_address);
	CloseHandle(shm);
	CloseHandle(hMutex);
}
bool FramePusherProxy::init(int with, int height, string mutex_name, string shm_name, string app_id, string channelid, string videosourcename, int user_id)
{
	m_width = with;
	m_height = height;
	m_mutext_name = mutex_name;
	m_shm_name = shm_name;
	m_app_id = app_id;
	m_channel_id = channelid;
	m_user_id = user_id;
	// Create the shared memory object
	shm = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedMem), m_shm_name.c_str());

	if (shm == NULL)
	{
		
		return false;
	}

	// Map the shared memory object to the local address space
	m_shm_address = MapViewOfFile(shm, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMem));

	if (m_shm_address == NULL)
	{
	
		CloseHandle(shm);
		return false;
	}

	//init the command
	Command wait = Command::WAIT;
	memcpy_s((BYTE*)m_shm_address, sizeof(Command), &wait, sizeof(Command));



	hMutex = CreateMutexA(NULL, FALSE, m_mutext_name.c_str()); // Create a named mutex
	if (hMutex == NULL)
	{
		
		return false;
	}


	

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	ZeroMemory(&pi, sizeof(pi));

	// Set up the command line and parameters
	/*
	char* app_id = argv[1];
	char* shm_name = argv[2];
	const char* width = argv[3];
	const char* height = argv[4];
	char* channel_id = argv[5];
	char* mutex_name = argv[6];
	*/
	std::stringstream ss;

	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	PathRemoveFileSpecA(buffer);

	ss <<buffer << "\\FramePusher.exe" << " " << m_app_id<<" "<<m_shm_name<<" "<<m_width<<" "<<m_height<<" "<<m_channel_id<<" "<<m_mutext_name<<" "<<videosourcename<<" "<<m_user_id;
	
	
	if (!CreateProcessA(NULL, (LPSTR)(ss.str().c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		std::stringstream err;
		err<< "CreateProcessA failed: " << GetLastError() << std::endl;
		logger->error(err.str().c_str());
		return 1;
	}

	


	
}
void FramePusherProxy::push_frame(BYTE* buffer, int bufsize, int ts)
{
	// Wait for ownership of the mutex
	DWORD result = WaitForSingleObject(hMutex, INFINITE);
	if (result == WAIT_OBJECT_0)
	{
		logger->debug("push frame");
		// Access the shared resource
		Command cmd = PUSH;
		memcpy_s((BYTE*)m_shm_address, sizeof(Command), &cmd, sizeof(Command));
		memcpy_s((BYTE*)m_shm_address + sizeof(Command), sizeof(DWORD) , &ts, sizeof(DWORD));
		memcpy_s((BYTE*)m_shm_address + sizeof(Command)+ sizeof(DWORD), MAX_VIDEO_BUFFER_SIZE, buffer, bufsize);
		// Release ownership of the mutex
		ReleaseMutex(hMutex);
	}
	// Unmap the shared memory object

	return ;
}
