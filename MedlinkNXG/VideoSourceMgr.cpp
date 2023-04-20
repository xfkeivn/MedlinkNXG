#include "pch.h"
#include "VideoSourceMgr.h"
#include "AGVideoWnd.h"
#include "AGDShowVideoCapture.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <typeinfo.h>
#include "resource.h"
#include "FrameRecorder.h"
extern std::shared_ptr<spdlog::logger> logger;


AgoraRemoteVideoSource::AgoraRemoteVideoSource(VideoSourceMgr * mgr, int remoteUserId):VideoSource(mgr,remoteUserId)
{ 
	m_wnd = new CAGVideoWnd();
	m_wnd->Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CRect(0, 0, 1, 1), mgr->getParentWindow(), remoteUserId);
}

VideoSourceMgr::VideoSourceMgr(CWnd* parentWndPtr)
{
	//ASSERT(parentWndPtr == NULL);
	this->m_parentWndPtr = parentWndPtr;
	m_captureMgr = new CAGDShowVideoCapture();
	m_review_window.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CRect(0, 0, 1, 1), this->m_parentWndPtr,0);
	//Default mode is SPEAKER_VIEW, But if there is only one it will fall back to one view mode.
	m_viewMode = SPEAKER_VIEW;
	init("b158865ce08b43f788e909aceda9fb7f", "100");
}

void VideoSourceMgr::init(string agora_app_id, string agora_channel_id)
{
	m_agora_app_id = agora_app_id;
	m_agora_meeting_channel = agora_channel_id;
}


VideoSource* VideoSourceMgr::add_agora_remote_video_source(int remoteUserId)
{
	vector<VideoSource*>::iterator itor = m_video_sources.begin();
	while (itor != m_video_sources.end())
	{
		VideoSource* source = *itor;
		AgoraRemoteVideoSource *arvs = dynamic_cast<AgoraRemoteVideoSource*>(source);
		if (arvs != nullptr)
		{
			if (arvs->get_agora_user_id() == remoteUserId)
			{
				return arvs;
			}
		}
		itor++;
	}
	//create a new window for remote user and show it at current mode
	AgoraRemoteVideoSource * remote_video_source = new AgoraRemoteVideoSource(this,remoteUserId);
	this->m_video_sources.push_back(remote_video_source);
	//if there is no video source yet
	if (this->m_video_sources.size() == 1)
	{
		m_lastOneViewSource = remote_video_source;
		m_viewMode = ONE_VIEW;
	}
	// if there is already video source
	else
	{	//if it is ONE_VIEW 
		if (m_viewMode == ONE_VIEW)
		{
			if (m_lastOneViewSource == nullptr)
				m_lastOneViewSource = remote_video_source;
		}
		else if (m_viewMode == TWO_H_VIEW)
		{
			if (m_lastTwoViewLeftSource == nullptr)
			{
				m_lastTwoViewLeftSource = remote_video_source;
			}
			else if (m_lastTwoViewRightSource == nullptr)
			{
				m_lastTwoViewRightSource = remote_video_source;
			}
			else
			{
				//No nothing
			}
			
		}
		else if (m_viewMode == TWO_V_VIEW)
		{
			if (m_lastTwoViewTopSource == nullptr)
			{
				m_lastTwoViewTopSource = remote_video_source;
			}
			else if (m_lastTwoViewBottomSource == nullptr)
			{
				m_lastTwoViewBottomSource = remote_video_source;
			}
			else
			{
				//No nothing
			}

		}
		else if (m_viewMode == GALLERY_VIEW)
		{
			m_lastSortedGalleryViewVideoSource.push_back(remote_video_source);
		}
		else if (m_viewMode == SPEAKER_VIEW)
		{
			m_lastSortedSpeakerViewVideoSource.push_back(remote_video_source);
		}
	}

	pack_windows();
	return remote_video_source;
}



void VideoSourceMgr::remove_agora_remote_video_source(int remoteUserId)
{
	vector<VideoSource*>::iterator itor = m_video_sources.begin();
	while (itor != m_video_sources.end())
	{
		VideoSource* source = *itor;
		AgoraRemoteVideoSource *arvs = dynamic_cast<AgoraRemoteVideoSource*>(source);
		if (arvs != nullptr)
		{
			if (arvs->get_agora_user_id() == remoteUserId)
			{
				return remove_video_source(arvs);
				
			}
		}
		itor++;
	}
}

void VideoSourceMgr::remove_video_source(VideoSource* videosource)
{
	// first hide the window of this source and destroy the window

	videosource->ShowWindow(FALSE);
	m_video_sources.erase(std::remove(m_video_sources.begin(), m_video_sources.end(), videosource), m_video_sources.end());
	m_lastSortedSpeakerViewVideoSource.erase(std::remove(m_lastSortedSpeakerViewVideoSource.begin(), m_lastSortedSpeakerViewVideoSource.end(), videosource), m_lastSortedSpeakerViewVideoSource.end());
	m_lastSortedGalleryViewVideoSource.erase(std::remove(m_lastSortedGalleryViewVideoSource.begin(), m_lastSortedGalleryViewVideoSource.end(), videosource), m_lastSortedGalleryViewVideoSource.end());

	if (m_lastTwoViewBottomSource == videosource)
		m_lastTwoViewBottomSource = nullptr;
	if (m_lastOneViewSource == videosource)
		m_lastOneViewSource = nullptr;
	if (m_lastTwoViewTopSource == videosource)
		m_lastTwoViewTopSource = nullptr;
	if (m_lastTwoViewLeftSource == videosource)
		m_lastTwoViewLeftSource = nullptr;
	if (m_lastTwoViewRightSource == videosource)
		m_lastTwoViewRightSource = nullptr;
	if (m_lastSpeakerViewMainSource == videosource)
		m_lastSpeakerViewMainSource = nullptr;
	delete videosource;
	pack_windows();

}

VideoSource* VideoSourceMgr::create_agora_local_video_source(int localUserId)
{
	AgoraLocalVideoSource * remote_video_source = new AgoraLocalVideoSource(this, localUserId);
	this->m_video_sources.push_back(remote_video_source);
	return remote_video_source;
}
void VideoSourceMgr::destory_agora_local_video_source()
{

}


void VideoSourceMgr::init_custom_capture_video_source()
{
	//Get all the video capture source from host
	vector<AGORA_DEVICE_INFO> device_info_list = m_captureMgr->GetDeviceList();
	vector<AGORA_DEVICE_INFO>::iterator itor = device_info_list.begin();
	int device_index = 0;
	int agora_user_id = CUSTOM_CAPTURE_USER_ID_START;
	while (itor != device_info_list.end())
	{
		AGORA_DEVICE_INFO &info = *itor;
		m_captureMgr->OpenDevice(info.szDevicePath);
		CString indexstr;
		indexstr.Format(_T("_%d"), device_index);
		string device_show_name = CW2A(info.szDeviceName + indexstr);
		int mediaCapCount = m_captureMgr->GetMediaCapCount(info.szDevicePath);
		for (int i = 0; i < mediaCapCount; i++)
		{
			VIDEOINFOHEADER header;
			m_captureMgr->GetVideoCap(info.szDevicePath, i, &header);
			if (device_video_caps.find(device_show_name) == device_video_caps.end())
			{
				device_video_caps[device_show_name] = vector<VIDEOINFOHEADER>();

			}
			if (header.bmiHeader.biCompression == MAKEFOURCC('I', '4', '2', '0'))
			{
				device_video_caps[device_show_name].push_back(header);

				if (header.bmiHeader.biWidth == 1920 && header.bmiHeader.biHeight == 1080 && header.AvgTimePerFrame == 333333)
				{
					//m_captureMgr->SelectMediaCap(info.szDevicePath, i);
					break;
				}

			}
			
		}
		
		logger->debug("OpenDevice {}",info.szDevicePath);
		m_captureMgr->StartPusherProcess(info.szDevicePath,m_agora_app_id,m_agora_meeting_channel, agora_user_id);
		CapturedVideoSource * capturedVideoSource = new CapturedVideoSource(this,agora_user_id,info.szDevicePath, m_captureMgr, device_index);
		m_captureMgr->SetPreviewWnd(info.szDevicePath, capturedVideoSource->getVideoWindow());
		//Create the capture video source 
		VIDEOINFOHEADER video_info_header;
		m_captureMgr->GetSelectedVideoCap(info.szDevicePath, &video_info_header);
		
		capturedVideoSource->getFrameRecorder()->add_observer(&test_dlg);
		capturedVideoSource->videoInfoHeader = video_info_header;
		capturedVideoSource->m_show_name = device_show_name;
		m_video_sources.push_back(capturedVideoSource);
		
		device_index++;
		agora_user_id++;
		itor++;
		
	}
	//start the capturing
	m_captureMgr->Start();
	// the default layout is the speaker mode, but if there is only one, fall back to one view mode.
	// in one view mode and speaker view, you can change to the recording and review state, other view mode cannot change to those states.
	
	m_main_view_state = MAIN_VIEW_NORMAL_STATE;
	if (this->m_video_sources.size() == 1)
	{
		setOneViewAndSource(this->m_video_sources[0]);
	}	
	if (this->m_video_sources.size() >= 2) {
		//default is speaker view
		vector<VideoSource*> sub_views(this->m_video_sources.begin() + 1, this->m_video_sources.end());
		
		setSpeakerViewAndSource(this->m_video_sources[0], sub_views);
	}

	// This is the test dialog for debug, in the release mode, it will be the commands from the remote or the control from local
	test_dlg.Create(IDD_DIALOG_CONTROL, this->m_parentWndPtr);
	test_dlg.setVideoSourceMgr(this);
	test_dlg.ShowWindow(TRUE);

}

void  VideoSourceMgr::close()
{
	m_captureMgr->Stop();
	delete m_captureMgr;
}

VideoSourceMgr::~VideoSourceMgr()
{
	close();
	for (unsigned int i = 0; i < m_video_sources.size(); i++)
	{
		delete m_video_sources[i];
	}
}


void VideoSourceMgr::addVideoSource(VideoSource *vsPtr)
{
	m_video_sources.push_back(vsPtr);
	
}



VideoMode VideoSourceMgr::getViewMode()
{
	return m_viewMode;
}

void VideoSourceMgr::pack_windows()
{
	// when there is new video source added in all removed, this function will be called to adjust the window
	RECT rect;
	if (m_parentWndPtr != nullptr)
	{
		m_parentWndPtr->GetClientRect(&rect);

		if (getViewMode() == ONE_VIEW)
		{
			setOneViewAndSource(this->m_lastOneViewSource);
		}
		if (getViewMode() == TWO_H_VIEW)
		{
			setTwoViewHAndSource(this->m_lastTwoViewLeftSource, this->m_lastTwoViewRightSource);
		}
		if (getViewMode() == TWO_V_VIEW)
		{
			setTwoViewVAndSource(this->m_lastTwoViewTopSource, this->m_lastTwoViewBottomSource);
		}
		if (getViewMode() == GALLERY_VIEW)
		{
			setGalleryViewAndSource(m_lastSortedGalleryViewVideoSource);
		}
		if (getViewMode() == SPEAKER_VIEW)
		{
			setSpeakerViewAndSource(m_lastSpeakerViewMainSource, m_lastSortedSpeakerViewVideoSource);
		}
	}
}
bool VideoSourceMgr::isHost()
{
	return true;
}

void VideoSourceMgr::showVideoView(VideoSource * video_source_id)
{

}
//Two view
void VideoSourceMgr::setOneViewAndSource(VideoSource * video_source)
{
	if (video_source == nullptr)
	{
		return;
	}
	this->m_viewMode = ONE_VIEW;
	RECT clientRect;
	m_parentWndPtr->GetClientRect(&clientRect);
	hideAllWindows();

	if (m_main_view_state == MAIN_VIEW_NORMAL_STATE)
	{
		video_source->MoveWindow(0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
		video_source->ShowWindow(TRUE);
	}
	if (m_main_view_state != MAIN_VIEW_NORMAL_STATE)
	{
		m_review_window.MoveWindow(0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
		m_review_window.ShowWindow(TRUE);
	}

	this->m_lastOneViewSource = video_source;

	//Get the recording or freeze frame source.
	CapturedVideoSource* cvs = dynamic_cast<CapturedVideoSource*>(m_lastOneViewSource);
	if (cvs != nullptr)
	{
		
		m_review_window.set_Capture_Video_Source(cvs);
	}

}
void VideoSourceMgr::setTwoViewHAndSource(VideoSource * video_source_left, VideoSource * video_source_right)
{
	hideAllWindows();
	this->m_viewMode = TWO_H_VIEW;
	RECT clientRect;
	m_parentWndPtr->GetClientRect(&clientRect);
	int width = clientRect.right - clientRect.left;
	if (video_source_left != nullptr)
	{
		this->m_lastTwoViewLeftSource = video_source_left;
		video_source_left->MoveWindow(0, 0, width/2-1, clientRect.bottom - clientRect.top);
		video_source_left->ShowWindow(TRUE);
	}
	if (video_source_right != nullptr)
	{
		this->m_lastTwoViewRightSource = video_source_right;
		video_source_right->MoveWindow(width/2+1, 0, width / 2 - 1, clientRect.bottom - clientRect.top);
		video_source_right->ShowWindow(TRUE);
	}
	
	
}
void VideoSourceMgr::setTwoViewVAndSource(VideoSource * video_source_top, VideoSource * video_source_bottom)
{
	hideAllWindows();
	this->m_viewMode = TWO_V_VIEW;
	RECT clientRect;
	m_parentWndPtr->GetClientRect(&clientRect);
	int heigth = clientRect.bottom - clientRect.top;
	if (video_source_top != nullptr)
	{
		this->m_lastTwoViewTopSource = video_source_top;
		video_source_top->MoveWindow(0, 0, clientRect.right - clientRect.left, heigth/2);
		video_source_top->ShowWindow(TRUE);
	}
	if (video_source_bottom != nullptr)
	{
		this->m_lastTwoViewBottomSource = video_source_bottom;
		video_source_bottom->MoveWindow(0, heigth / 2 - 1, clientRect.right - clientRect.left,heigth/2);
		video_source_bottom->ShowWindow(TRUE);
	}

}

//Gallery
void VideoSourceMgr::setGalleryViewAndSource(vector<VideoSource *> sorted_video_sources_on_gallery)
{
	int total_video_source_num = sorted_video_sources_on_gallery.size();
	RECT clientRect;
	m_parentWndPtr->GetClientRect(&clientRect);
	int width = clientRect.right - clientRect.left;
	int heigth = clientRect.bottom - clientRect.top;
	if (total_video_source_num == 0)
	{
		return;
	}

	if (total_video_source_num == 1)
	{
		setOneViewAndSource(sorted_video_sources_on_gallery[0]);
		return;
	}
	if (total_video_source_num == 2)
	{
		setTwoViewHAndSource(sorted_video_sources_on_gallery[0], sorted_video_sources_on_gallery[1]);
		return;
	}

	this->hideAllWindows();
	this->m_viewMode = GALLERY_VIEW;
	this->m_lastSortedGalleryViewVideoSource = sorted_video_sources_on_gallery;
	if (total_video_source_num == 3 || total_video_source_num == 4)
	{
		// 4 
		
		for (int i = 0; i < total_video_source_num; i++)
		{
			VideoSource *vs = sorted_video_sources_on_gallery[i];
			int col = i % 2;
			int row = i / 2;
			
			int posx = col * width / 2;
			int posy = row * heigth / 2;
			vs->MoveWindow(posx, posy, width/2,heigth/2);
			vs->ShowWindow(TRUE);
		}
		
		return;
	}
	if (total_video_source_num == 5 || total_video_source_num == 6)
	{
		// 6 
	
		for (int i = 0; i < total_video_source_num; i++)
		{
			VideoSource *vs = sorted_video_sources_on_gallery[i];
			int col = i % 3;
			int row = i / 3;

			int posx = col * width / 3;
			int posy = row * heigth / 2;
			vs->MoveWindow(posx, posy, width / 3, heigth / 2);
			vs->ShowWindow(TRUE);
		}
		return;
	}
	if (total_video_source_num == 7 || total_video_source_num == 8 || total_video_source_num == 9)
	{
		// 9 
		for (int i = 0; i < total_video_source_num; i++)
		{
			VideoSource *vs = sorted_video_sources_on_gallery[i];
			int col = i % 3;
			int row = i / 3;
			int posx = col * width / 3;
			int posy = row * heigth / 3;
			vs->MoveWindow(posx, posy, width / 3, heigth / 3);
			vs->ShowWindow(TRUE);
		}
		return;
	}
	if (total_video_source_num == 10 || total_video_source_num == 11 || total_video_source_num == 12)
	{
		// 12 
	
		RECT clientRect;
		m_parentWndPtr->GetClientRect(&clientRect);
		int width = clientRect.right - clientRect.left;
		int heigth = clientRect.bottom - clientRect.top;
		for (int i = 0; i < total_video_source_num; i++)
		{
			VideoSource *vs = sorted_video_sources_on_gallery[i];
			int col = i % 4;
			int row = i / 4;
			int posx = col * width / 4;
			int posy = row * heigth / 3;
			vs->MoveWindow(posx, posy, width / 4, heigth / 3);
			vs->ShowWindow(TRUE);
		}
	
	}
	if (total_video_source_num>=12)
	{	// 16 
	
		for (int i = 0; i < min(total_video_source_num,16); i++)
		{
			VideoSource *vs = sorted_video_sources_on_gallery[i];
			int col = i % 4;
			int row = i / 4;

			int posx = col * width / 4;
			int posy = row * heigth / 4;
			vs->MoveWindow(posx, posy, width / 4, heigth / 4);
			vs->ShowWindow(TRUE);
		}

	}

}
//speaker view
void VideoSourceMgr::setSpeakerViewAndSource(VideoSource *video_source_main, vector<VideoSource *> orgsorted_other_video_sources)
{
	RECT clientRect;
	m_parentWndPtr->GetClientRect(&clientRect);
	int width = (clientRect.right - clientRect.left);
	int height = (clientRect.bottom - clientRect.top);
	int main_width = 0;
	int main_height = 0;
	vector<VideoSource *> sorted_video_sources;
	// max to show 1+ 9 windows , more than 9 sliced the others.
	if (orgsorted_other_video_sources.size() > 9)
	{
		vector<VideoSource *> sliced(orgsorted_other_video_sources.begin(), orgsorted_other_video_sources.begin() + 9);
		sorted_video_sources = sliced;
	}
	else
	{
		sorted_video_sources = orgsorted_other_video_sources;
	}

	
	int size_view = sorted_video_sources.size();
	//less than 4, then 
	if (sorted_video_sources.size() ==0  )
	{
		setOneViewAndSource(video_source_main);
		return;
	}
	m_lastSpeakerViewMainSource = video_source_main;
	m_lastSortedSpeakerViewVideoSource = orgsorted_other_video_sources;
	this->m_viewMode = SPEAKER_VIEW;
	hideAllWindows();
	if (size_view <= 4)
	{
		//show all the others on the right, the capture source at the top
		int y = 0;
		for (int i = 0; i < size_view; i++)
		{
			
			if (sorted_video_sources[i] != video_source_main)
			{
				int x = (int)(h_percentage*width);
				sorted_video_sources[i]->MoveWindow(x, y, (int)(width*(1- h_percentage)), (int)(height*0.2));
				sorted_video_sources[i]->ShowWindow(TRUE);
				y += int(height*(0.2));
			}
		}


		m_lastSpeakerViewMainSource = video_source_main;
		main_width = int(width*h_percentage);
		main_height = int(height);
		
	
	}
	if (size_view >= 5)
	{
		int x = 0;
		int y = 0;
		for (int i = 0; i < size_view; i++)
		{
			
			if ( i < 4 )
			{
				
				int x0 = int(h_percentage*width);
				sorted_video_sources[i]->MoveWindow(x0, y, int(width*0.2), int(height*0.2));
				sorted_video_sources[i]->ShowWindow(TRUE);
				y += int(height * 0.2);
			}
			else
			{
				int y0 = int(0.8*height);
				sorted_video_sources[i]->MoveWindow(x, y0, int(width*0.2), int(height*0.2));
				sorted_video_sources[i]->ShowWindow(TRUE);
				x += int(width * 0.2);
			}

		}

		//set the main window
		m_lastSpeakerViewMainSource = video_source_main;
		main_width = int(width*h_percentage);
		main_height = int(height*v_percentage);
		
		CapturedVideoSource* cvs = dynamic_cast<CapturedVideoSource*>(m_lastSpeakerViewMainSource);
		if (cvs != nullptr)
		{
			
			m_review_window.set_Capture_Video_Source(cvs);
		}

	}

	if (video_source_main != nullptr && m_main_view_state == MAIN_VIEW_NORMAL_STATE)
	{
		video_source_main->MoveWindow(0, 0, main_width, main_height);
		video_source_main->ShowWindow(TRUE);

	}

	// for the review mode, also resize the window
	if (m_main_view_state != MAIN_VIEW_NORMAL_STATE)
	{
		m_review_window.MoveWindow(0, 0, main_width, main_height);
		m_review_window.ShowWindow(TRUE);
	}
		




}

CRect VideoSource::GetWndRect()
{
	CRect rect;
	m_wnd->GetClientRect(&rect);
	return rect;
}
void VideoSourceMgr::swapVideoSource(VideoSource *source, VideoSource* dest)
{
	CRect source_rect = source->GetWndRect();
	CRect target_rect = dest->GetWndRect();
	source->MoveWindow(target_rect.left, target_rect.top, target_rect.Width(), target_rect.Height());
	  dest->MoveWindow(source_rect.left, source_rect.top, source_rect.Width(), source_rect.Height());
}

void VideoSourceMgr::hideAllWindows()
{
	vector<VideoSource*>::iterator itor = m_video_sources.begin();
	while (itor != m_video_sources.end())
	{
		VideoSource *vs = *itor;
		vs->ShowWindow(false);
		itor++;
	}
	//also hide the review window
	m_review_window.ShowWindow(FALSE);
}

void VideoSourceMgr::onParentWndSize()
{
	
	pack_windows();
}


void VideoSourceMgr::hideVideoView(VideoSource * video_source)
{
	video_source->ShowWindow(false);
}

VideoSource* VideoSourceMgr::getDefaultVideoSource()
{
	vector<VideoSource*>::iterator p = m_video_sources.begin();
	if (p != m_video_sources.end())
		return *p;
	return NULL;
}
void  VideoSource::ShowWindow(bool show)
{
	m_wnd->ShowWindow(show);
}


VideoSource::~VideoSource()
{
	m_wnd->DestroyWindow();
	delete m_wnd;
}

void VideoSource::MoveWindow(int posx, int posy, int width, int heigth)
{
	
	if (m_wnd != nullptr)
	{
		//this->ShowWindow(TRUE);

		m_wnd->MoveWindow(posx, posy, width, heigth);
	}
}

void CapturedVideoSource::MoveWindow(int posx, int posy, int width, int heigth)
{
	VideoSource::MoveWindow(posx, posy, width, heigth);
	UpdatePreview();

}



void CapturedVideoSource::UpdatePreview()
{
	m_captureMgr->UpdatePreview(this->m_devicePath);
}

CapturedVideoSource::CapturedVideoSource(VideoSourceMgr * mgr, int agora_user_id,CString devicePath, CAGDShowVideoCapture* captureMgr, int deviceIndex):VideoSource(mgr,agora_user_id),m_devicePath(devicePath),m_captureMgr(captureMgr), m_device_index(deviceIndex)
{
	m_wnd = new CapturedSourceVideoWnd();
	m_wnd->Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CRect(0, 0, 1, 1), mgr->getParentWindow(), deviceIndex);

}




void VideoSourceMgr::setMainViewState(MainViewState viewState)
{

	m_review_window.set_Main_View_State(viewState);


	if (viewState == MAIN_VIEW_NORMAL_STATE)
	{
		setNormalViewState();
		return;
	}
	
	// if it is the freeze frame review state or the record review state

	if (getViewMode() == SPEAKER_VIEW)
	{
		m_lastSpeakerViewMainSource->ShowWindow(FALSE);
		CRect source_rect = m_lastSpeakerViewMainSource->GetWndRect();
		m_review_window.MoveWindow(source_rect.left, source_rect.top, source_rect.Width(), source_rect.Height());
		m_review_window.ShowWindow(TRUE);
		m_main_view_state = viewState;
		m_main_view_source = m_lastSpeakerViewMainSource;
		// bind the frame recorder to the interactive window 
		CapturedVideoSource* cvs = dynamic_cast<CapturedVideoSource*>(m_lastSpeakerViewMainSource);
		if (cvs != nullptr)
		{
			
			m_review_window.set_Capture_Video_Source(cvs);
		}
		


	}
	if (getViewMode() == ONE_VIEW)
	{
		m_lastOneViewSource->ShowWindow(FALSE);
		CRect source_rect = m_lastOneViewSource->GetWndRect();
		m_review_window.MoveWindow(source_rect.left, source_rect.top, source_rect.Width(), source_rect.Height());
		m_review_window.ShowWindow(TRUE);
		m_main_view_state = viewState;
		m_main_view_source = m_lastOneViewSource;
		// bind the frame recorder to the interactive window 
		CapturedVideoSource* cvs = dynamic_cast<CapturedVideoSource*>(m_lastOneViewSource);
		if (cvs != nullptr)
		{
			
			m_review_window.set_Capture_Video_Source(cvs);

		}
	}

}
void VideoSourceMgr::setNormalViewState()
{
	m_main_view_state = MAIN_VIEW_NORMAL_STATE;
	m_review_window.ShowWindow(FALSE);
	m_main_view_source = nullptr;
	if (getViewMode() == SPEAKER_VIEW)
	{
		setSpeakerViewAndSource(m_lastSpeakerViewMainSource, m_lastSortedSpeakerViewVideoSource);
	}
	if (getViewMode() == ONE_VIEW)
	{
		setOneViewAndSource(this->m_lastOneViewSource);
	}
	
	m_review_window.set_Capture_Video_Source(nullptr);
}

//proxy the interactive window to working on review and play stuff
void VideoSourceMgr::start_Recording(string record_file)
{
	m_review_window.start_Recording(record_file);

}
void VideoSourceMgr::stop_Recording()
{
	m_review_window.stop_Recording();
}
int  VideoSourceMgr::load_Review_Record(string record_file)
{
	return m_review_window.load_Review_Record(record_file);
}
void VideoSourceMgr::start_Playing_Record(int frame_index)
{
	m_review_window.start_Playing_Record(frame_index);
}
void VideoSourceMgr::pause_Playing_Record()
{
	m_review_window.pause_Playing_Record();
}
void VideoSourceMgr::next_Frame()
{
	m_review_window.next_Frame();
}
void VideoSourceMgr::prev_Frame()
{
	m_review_window.prev_Frame();
}
void VideoSourceMgr::goto_Frame(int frame_index)
{
	m_review_window.goto_Frame(frame_index);
}

void VideoSourceMgr::freeze_one_frame()
{
	m_review_window.freeze_one_frame();

}
vector<FreezeFrame>  VideoSourceMgr::get_FreezeFrames()
{
	return m_review_window.get_FreezeFrames();
}

void  VideoSourceMgr::display_freeze_frame(FreezeFrame *freezeframe)
{
	m_review_window.display_freeze_frame(freezeframe);
}