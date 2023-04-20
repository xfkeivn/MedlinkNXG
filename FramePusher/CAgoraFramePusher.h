#pragma once
#include "AgoraBase.h"
#include "AgoraMediaBase.h"
#include "IAgoraRtcEngine.h"
#include "ShareMem.h"
using namespace std;
using namespace agora::rtc;
#include <string>
class CAgoraEngineEventHanlder;
class CAgoraFramePusher
{
	
public:
	CAgoraFramePusher(string app_id,string shm_name,int width,int height,string channel_id, string mutex_name,int user_id);
	virtual ~CAgoraFramePusher();
	bool StartCaputre();
	void StartShareMemoryCapture();
private:
	//Initialize the Agora SDK
	bool InitAgora();
	//UnInitialize the Agora SDK
	bool UnInitAgora();
	//set control text from config.
	BOOL EnableExtendVideoCapture(BOOL bEnable);
	// if bEnable is true start capture otherwise stop capture.
	bool EnableCaputre(BOOL bEnable);
	bool Joinchannel(string channel_id,agora::rtc::uid_t user_id=0);
private:
	agora::media::base::ExternalVideoFrame m_videoFrame;
	IRtcEngine* m_rtcEngine =nullptr;
	int m_user_id;
	int m_width;
	int m_height;
	BOOL m_initialize = false;
	int m_joinChannel;
	string m_channel_id;
	string m_shm_name;
	string m_mutex_name;
	string m_app_id;
	void* m_shm_address =nullptr;
	HANDLE shm;
	SharedMem * m_shareMem = nullptr;
	CAgoraEngineEventHanlder *eventhandler = nullptr;
};
