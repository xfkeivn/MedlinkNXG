#include "stdafx.h"
#include "CAgoraFramePusher.h"
#include "AgVideoBuffer.h"
#include "CAgoraEngineEventHanlder.h"
#include "spdlog/sinks/rotating_file_sink.h"
using namespace std;
using namespace agora::rtc;
extern std::shared_ptr<spdlog::logger> logger;
CAgoraFramePusher::CAgoraFramePusher(string app_id,string shm_name, int width, int height, string channel_id,string mutex_name,int user_id):m_app_id(app_id),m_shm_name(shm_name),m_width(width),m_height(height), m_channel_id(channel_id),m_mutex_name(mutex_name)
{
	m_user_id = user_id;
	m_shareMem = new SharedMem();
	
	eventhandler = new CAgoraEngineEventHanlder();
}


bool CAgoraFramePusher::InitAgora()
{
	logger->debug("InitAgora");
	m_rtcEngine = createAgoraRtcEngine();
	if (!m_rtcEngine) {
	
		logger->error("createAgoraRtcEngine failed");
		return false;
	}
	

	RtcEngineContext context;
	std::string strAppID = m_app_id;
	context.appId = strAppID.c_str();
	context.eventHandler = eventhandler;
	//set channel profile in the engine to the CHANNEL_PROFILE_LIVE_BROADCASTING.
	context.channelProfile = CHANNEL_PROFILE_LIVE_BROADCASTING;
	//initialize the Agora RTC engine context.

	int ret = m_rtcEngine->initialize(context);
	if (ret != 0) {
		m_initialize = false;
		logger->error("rtc engine initialize failed");
		return false;
	}
	else
		m_initialize = true;
	//set client role in the engine to the CLIENT_ROLE_BROADCASTER.
	m_rtcEngine->setClientRole(CLIENT_ROLE_BROADCASTER);
	logger->debug("InitAgora Succeed");
	return true;
}

/*
	stop and release agora rtc engine.
*/
bool CAgoraFramePusher::UnInitAgora()
{
	logger->debug("start UnInitAgora");
	if (m_rtcEngine) {
		if (m_joinChannel)
			m_joinChannel = !m_rtcEngine->leaveChannel();
		EnableExtendVideoCapture(FALSE);
		EnableCaputre(FALSE);
	
		//stop preview in the engine.
		//m_rtcEngine->stopPreview();
		
		//disable video in the engine.
		m_rtcEngine->disableVideo();
	
		//release engine.
		m_rtcEngine->release(true);
		
		m_rtcEngine = NULL;
	}
	return true;
}


BOOL CAgoraFramePusher::EnableExtendVideoCapture(BOOL bEnable)
{
	agora::util::AutoPtr<agora::media::IMediaEngine> mediaEngine;
	//query interface agora::AGORA_IID_MEDIA_ENGINE in the engine.
	bool result = mediaEngine.queryInterface(m_rtcEngine, agora::rtc::AGORA_IID_MEDIA_ENGINE);
	int nRet = 0;
	agora::base::AParameter apm(m_rtcEngine);
	if (mediaEngine.get() == NULL)
	{
		logger->error("query media engine failed");
		return FALSE;

	}
		
	if (bEnable) {
		//mediaEngine->setExternalVideoSource(false, false);
		//set local video camera index.
		//nRet = apm->setParameters("{\"che.video.local.camera_index\":1024}");
		//register agora video frame observer.
		//(*lpDeviceManager)->setDevice("1024");
		nRet = mediaEngine->setExternalVideoSource(true, false);

		if (nRet != 0)
		{
			logger->error("setExternalVideoSource = True failed");
			return FALSE;
		}
		//nRet = mediaEngine->registerVideoFrameObserver(&m_extVideoFrameObserver);
	}
	else {

		//nRet = apm->setParameters("{\"che.video.local.camera_index\":0}");
		nRet = mediaEngine->setExternalVideoSource(false, false);
		if (nRet != 0)
		{
			logger->error("setExternalVideoSource = False failed");
			return FALSE;
		}

		//unregister agora video frame observer.
		//nRet = mediaEngine->registerVideoFrameObserver(NULL);
	}
	return nRet == 0 ? TRUE : FALSE;
}



// start or stop capture.
// if bEnable is true start capture otherwise stop capture.
bool CAgoraFramePusher::EnableCaputre(BOOL bEnable)
{
	if (bEnable)
	{
		//select video capture type.
	
		
		VideoEncoderConfiguration config;
		config.dimensions.width =m_width;
		config.dimensions.height = m_height;
		m_videoFrame.stride = m_width;
		m_videoFrame.height = m_height;
		m_videoFrame.rotation = 0;
		m_videoFrame.cropBottom = 0;
		m_videoFrame.cropLeft = 0;
		m_videoFrame.cropRight = 0;
		m_videoFrame.cropTop = 0;
		m_videoFrame.format = agora::media::base::VIDEO_PIXEL_I420;
		m_videoFrame.type = agora::media::base::ExternalVideoFrame::VIDEO_BUFFER_TYPE::VIDEO_BUFFER_RAW_DATA;
		//set video encoder configuration.
		int result = m_rtcEngine->setVideoEncoderConfiguration(config);
		if (result != 0)
		{
			logger->error("setVideoEncoderConfiguration failed");
			return false;
		}
		//start video capture.
		//enable video in the engine.
		result = m_rtcEngine->enableVideo();
		if (result != 0)
		{
			logger->error("enableVideo failed");
			return false;
		}
		result = m_rtcEngine->disableAudio();
		if (result != 0)
		{
			logger->error("disableAudio failed");
			return false;
		}
		result = m_rtcEngine->enableLocalAudio(false);
		if (result != 0)
		{
			logger->error("enableLocalAudio = False failed");
			return false;
		}

	}
	else {
		//video capture stop.

		if (m_rtcEngine)
		{
			//disable video in the engine.
			m_rtcEngine->disableVideo();
			//stop preview in the engine.
			
		}
	}

	return true;
}


/*
	start or stop capture,register or unregister video frame observer. 
*/
bool CAgoraFramePusher::StartCaputre()
{
	bool result = false;
	result = InitAgora();
	if (!result)
		return false;
	result = Joinchannel(m_channel_id,m_user_id);
	if (!result)
		return false;

	logger->debug("EnableExtendVideoCapture = TRUE start");
	bool nRet = EnableExtendVideoCapture(TRUE);
	if (!nRet)
	{
		logger->debug("EnableExtendVideoCapture = TRUE failed");
		return false;
	}
	logger->debug("EnableExtendVideoCapture = TRUE success");
	//register agora video frame observer.
	nRet = EnableCaputre(TRUE);
	if (!nRet)
	{
		logger->debug("EnableCaputre failed");
		return false;
	}
	
	return true;
}

bool CAgoraFramePusher::Joinchannel(string channel_id, agora::rtc::uid_t user_id)
{
	logger->debug("Join channel started");
	if (!m_rtcEngine || !m_initialize)
		return false;

	if (!m_joinChannel) {

		ChannelMediaOptions options;
		options.channelProfile = CHANNEL_PROFILE_LIVE_BROADCASTING;
		options.clientRoleType = CLIENT_ROLE_BROADCASTER;
		options.autoSubscribeAudio = FALSE;
		options.autoSubscribeVideo = FALSE;
		//join channel in the engine.
		if (0 == m_rtcEngine->joinChannel(NULL, channel_id.c_str(), user_id, options)) {
			logger->debug("Join channel success ");

		}
	}
	else {
		//leave channel in the engine.
		if (0 == m_rtcEngine->leaveChannel()) {
			logger->debug("Leave channel success");
		}
	}
	return true;
}



CAgoraFramePusher::~CAgoraFramePusher()
{
	UnInitAgora();
	delete eventhandler;
	delete m_shareMem;

}

void CAgoraFramePusher::StartShareMemoryCapture()
{

	HANDLE hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, m_mutex_name.c_str()); // Open the named mutex
	if (hMutex == NULL)
	{
		
		DWORD errorcode = GetLastError();
		logger->error("open mutex failed errorcode = {}",errorcode);
		return ;
	}
	logger->info("Open MUTEXT name is {} ", m_mutex_name.c_str());
	
	// Open the shared memory object
	HANDLE shm = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, m_shm_name.c_str());

	if (shm == NULL)
	{
		logger->error("open shared memory failed");
		return ;
	}


	


	// Map the shared memory object to the local address space
	m_shm_address = MapViewOfFile(shm, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMem));

	if (m_shm_address == nullptr)
	{
		logger->error("mapping shared memory failed");
		return;
	}
	logger->info("start capture ");
	StartCaputre();
	agora::util::AutoPtr<agora::media::IMediaEngine> mediaEngine;
	bool result = mediaEngine.queryInterface(this->m_rtcEngine, agora::rtc::AGORA_IID_MEDIA_ENGINE);
	if (result)
	{
		logger->debug("media engine is ok, start pushing ");
	}
	else
	{
		logger->debug("media engine is not ok, exit");
		return;
	}
	
	int bufSize = this->m_videoFrame.stride * this->m_videoFrame.height * 3 / 2;
	DWORD last_push_frame_ts = 0;
	while (true)
	{
		if (this->m_videoFrame.format == 24)
		{
			logger->info("format = 24 exit ");
			return;
		}
		else if (this->m_videoFrame.format == agora::media::base::VIDEO_PIXEL_I420) {
			
			int timestamp = GetTickCount();
			DWORD result = WaitForSingleObject(hMutex, INFINITE);
			if (result == WAIT_OBJECT_0)
			{
				m_shareMem = (SharedMem*) m_shm_address;
				//memcpy_s(m_shareMem, sizeof(SharedMem), m_shm_address,bufSize+sizeof(Command)+sizeof(DWORD));
				this->m_videoFrame.timestamp = timestamp;
				

				if (m_shareMem->command == EXIT)

				{
					ReleaseMutex(hMutex);
					logger->debug("command is exit received");
					break;
				}
				else if (m_shareMem->command == PUSH && last_push_frame_ts !=m_shareMem->tickcount)
				{
					last_push_frame_ts = m_shareMem->tickcount;
					logger->debug("frame pushed");
					m_videoFrame.buffer = m_shareMem->frameBuffer;
					mediaEngine->pushVideoFrame(&this->m_videoFrame);
					ReleaseMutex(hMutex);

				}
				else if (m_shareMem->command == WAIT)
				{
					ReleaseMutex(hMutex);
					logger->debug("command is wait received");
					Sleep(15);
				
				}
				else
				{
					ReleaseMutex(hMutex);
					Sleep(1);
				}


			}
				
			
			else
			{
				Sleep(1);
				continue;
			}

		
			
		}
	}
	// Unmap the shared memory object
	if (m_shm_address != nullptr)
	{
		UnmapViewOfFile(m_shm_address);
		CloseHandle(shm);
		shm = nullptr;
	}
	if ( shm != NULL)
	{
		CloseHandle(shm);

	}
	return;



}