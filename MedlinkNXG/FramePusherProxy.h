#pragma once
#include "stdafx.h"
#include <string>
using namespace std;
class FramePusherProxy
{
private:
	int m_width;
	int m_height;
	string m_mutext_name;
	string m_shm_name;
	string m_channel_id;
	int m_user_id;
	string m_app_id;
	int user_id;
	void* m_shm_address;
	HANDLE hMutex;
	HANDLE shm;
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
public:
	FramePusherProxy();
	bool init(int with, int height, string mutex_name, string shm_name, string app_id, string channelid,string videosourcename,int user_id);
	void push_frame(BYTE* buffer, int bufsize, int ts);
	void deinit();
};

