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
	m_wnd->SetVideoSource(this);
	
}

VideoSourceMgr::VideoSourceMgr(CWnd* parentWndPtr)
{
	//ASSERT(parentWndPtr == NULL);
	this->m_parentWndPtr = parentWndPtr;
	m_captureMgr = new CAGDShowVideoCapture();
	m_review_window.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CRect(0, 0, 1, 1), this->m_parentWndPtr,0);
	//Default mode is SPEAKER_VIEW, But if there is only one it will fall back to one view mode.
	m_viewMode = SPEAKER_ONE_VIEW;
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
	addVideoSource(remote_video_source);
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
	delete videosource;

}

VideoSource* VideoSourceMgr::create_agora_local_video_source(int localUserId)
{
	m_local_video_source = new AgoraLocalVideoSource(this, localUserId);
	addVideoSource(m_local_video_source);
	return m_local_video_source;
}
void VideoSourceMgr::destory_agora_local_video_source()
{
	if(m_local_video_source!=nullptr)
		remove_video_source(m_local_video_source);
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
		addVideoSource(capturedVideoSource);
		device_index++;
		agora_user_id++;
		itor++;
		
	}
	//start the capturing
	m_captureMgr->Start();
	// the default layout is the speaker mode, but if there is only one, fall back to one view mode.
	// in one view mode and speaker view, you can change to the recording and review state, other view mode cannot change to those states.
	interactive_state = NO_INTERACTIVE;
	pack_windows();
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
	auto itor = std::find(m_video_sources.begin(), m_video_sources.end(), vsPtr);
	if (itor == m_video_sources.end())
	{
		m_video_sources.push_back(vsPtr);
	}
}

void VideoSourceMgr::push_back_to_subview(VideoSource* videosource)
{
	
	videosource->setViewType(SUB_VIEW);

}
void VideoSourceMgr::drop_to_main_view(VideoSource* videsource, CRect* windowRectToDrop)
{
	// if there is no target window, it should be the initial status main window setting
	if (windowRectToDrop != nullptr)
	{
		CRect rect = *windowRectToDrop;
		vector<CRect>::iterator itor = m_main_window_rects.begin();
		int target_index = 0;
		bool hitted = false;
		//For the main to main, the size of the window is the same so to make it dropable, we will deflat it
		int deflat_x = rect.Width() / 2;
		int deflat_y = rect.Height() / 2;
		if(videsource->getViewType() == MAIN_VIEW)
			rect.DeflateRect(deflat_x, deflat_y);
		//Whether it is drop to a rect of main window
		while (itor != m_main_window_rects.end())
		{
			CRect mainRect = *itor;
			if (mainRect.left <= rect.left && mainRect.right >= rect.right && mainRect.top <= rect.top && mainRect.bottom >= rect.bottom)
			{
				hitted = true;
				break;
			}
			target_index++;
				itor++;
		}
		// if it is in a rect
		if (hitted)
		{
			// set the index of this rect
			vector<VideoSource*> main_views = get_main_views();
			for (int i = 0; i < main_views.size(); i++)
			{
				VideoSource* vs = main_views[i];
				
				if (vs != videsource )
				{
					
					if (vs->get_main_view_index() == target_index)
					{
						swap_2_views(vs, videsource);
						videsource->set_main_view_index(target_index);
						pack_windows();
						return;
					}
				}
				
				else
				{
					
				}
			}
			videsource->set_main_view_index(target_index);
			videsource->setViewType(MAIN_VIEW);
			pack_windows();
		}
		else
		{
			pack_windows();
		}
	
	}
}

void VideoSourceMgr::swap_2_views(VideoSource *source, VideoSource* dest)
{
	//adjust the main and sub view vectors 
	ViewType src_type = source->getViewType();
	ViewType dest_type = dest->getViewType();
	if (src_type == MAIN_VIEW && dest_type == MAIN_VIEW)
	{
		int src_index = source->get_main_view_index();
		int dst_index = dest->get_main_view_index();
		source->set_main_view_index(dst_index);
		dest->set_main_view_index(src_index);
	}


	source->setViewType(dest_type);
	dest->setViewType(src_type);
	auto itor_src = std::find(m_video_sources.begin(), m_video_sources.end(), source);
	auto itor_dst = std::find(m_video_sources.begin(), m_video_sources.end(), dest);
	std::swap(itor_src, itor_dst);

	
}


VideoMode VideoSourceMgr::getViewMode()
{
	return m_viewMode;
}

void VideoSourceMgr::pack_windows()
{
	// when there is new video source added in all removed, this function will be called to adjust the window
	
	if (m_parentWndPtr != nullptr)
	{
		RECT clientRect;
		m_parentWndPtr->GetClientRect(&clientRect);

		int totoal_width = (clientRect.right - clientRect.left);
		int total_height = (clientRect.bottom - clientRect.top);
		int main_width = totoal_width;
		int main_height = total_height;
		// max to show 1+ 9 windows , more than 9 sliced the others.
		hideAllWindows();
		int sub_view_count = get_current_sub_view_count();
		if (sub_view_count == 0)
			sub_view_count = 1;
		int sub_view_avg_width = totoal_width / sub_view_count;
		int sub_view_width = sub_view_avg_width;
		if (sub_view_avg_width >= this->sub_view_max_width)
		{
			sub_view_width = this->sub_view_max_width;
		}

		int video_source_count = this->m_video_sources.size();
		int x = 0;
		int y = 0;
		//int main_view_index = 0;
		int sub_view_index = 0;

		
		for (int i = 0; i < video_source_count; i++)
		{
			VideoSource * vs = m_video_sources[i];
			ViewType vt = vs->getViewType();

			if (vt == MAIN_VIEW)
			{
				if (m_viewMode == SPEAKER_ONE_VIEW)
				{
					vs->MoveWindow(0, 0, main_width, main_height*h_percentage);
					vs->ShowWindow(TRUE);
				}
				else if (m_viewMode == SPEAKER_TWO_VIEW)
				{
					
					vs->MoveWindow(vs->get_main_view_index()*ceil(main_width / 2.0), 0, floor(main_width / 2.0), main_height*h_percentage);
					vs->ShowWindow(TRUE);
				}
				else if (m_viewMode == SPEAKER_FOUR_VIEW)
				{
					int main_view_index = vs->get_main_view_index();
					if (main_view_index % 2 == 0)
					{
						x = 0;
					}
					else
					{
						x = ceil(main_width / 2.0);
					}
					if (main_view_index < 2)
					{
						y = 0;
					}
					else
					{
						y = ceil(main_height*h_percentage / 2.0);
					}
					vs->MoveWindow(x, y, floor(main_width / 2.0), floor(main_height*h_percentage / 2.0));
					vs->ShowWindow(TRUE);
					//main_view_index++;
				}

			}
			else
			{
				int y0 = int(h_percentage*total_height);
				int sub_view_height = int((1 - h_percentage)*total_height);
				vs->MoveWindow(sub_view_index*sub_view_width, y0, sub_view_width, sub_view_height);
				vs->ShowWindow(TRUE);
				sub_view_index++;
			}
		}

		m_main_window_rects.clear();
		if (m_viewMode == SPEAKER_ONE_VIEW)
		{
			CRect rect = clientRect;
			rect.bottom = h_percentage * rect.bottom;
			m_main_window_rects.push_back(rect);

		
		}
		else if (m_viewMode == SPEAKER_TWO_VIEW)
		{
			CRect rect1 = clientRect;
			CRect rect2 = clientRect;
			rect1.right = rect1.right / 2;
			rect1.bottom = h_percentage * rect1.bottom;

			rect2.left = rect2.right / 2;
			rect2.bottom = h_percentage * rect2.bottom;
			m_main_window_rects.push_back(rect1);
			m_main_window_rects.push_back(rect2);
		}
		else if (m_viewMode == SPEAKER_FOUR_VIEW)
		{

			CRect rect1 = clientRect;
			CRect rect2 = clientRect;
			CRect rect3 = clientRect;
			CRect rect4 = clientRect;
			rect1.right = rect1.right / 2;
			rect1.bottom = h_percentage * rect1.bottom / 2;

			rect2.left = rect2.right / 2;
			rect2.bottom = h_percentage * rect2.bottom / 2;

			rect3.right = rect3.right / 2;
			rect3.top = h_percentage * rect3.bottom / 2;
			rect3.bottom = h_percentage * rect3.bottom;

			rect4.left = rect4.right / 2;
			rect4.top = h_percentage * rect4.bottom / 2;
			rect4.bottom = h_percentage * rect4.bottom;
			m_main_window_rects.push_back(rect1);
			m_main_window_rects.push_back(rect2);
			m_main_window_rects.push_back(rect3);
			m_main_window_rects.push_back(rect4);
			
		}










		}

		m_parentWndPtr->Invalidate();
}
bool VideoSourceMgr::isHost()
{
	return true;
}

void VideoSourceMgr::showVideoView(VideoSource * video_source_id)
{

}


CRect VideoSource::GetWndRect()
{
	CRect rect;
	m_wnd->GetClientRect(&rect);
	return rect;
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

void VideoSource::drop_to_main(CRect rect)
{
	this->m_video_source_mgr->drop_to_main_view(this, &rect);
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
	m_wnd->SetVideoSource(this);
	
}




void VideoSourceMgr::set_interactive_state(InteractiveState viewState)
{
	if (viewState == NO_INTERACTIVE)
	{
		m_review_window.ShowWindow(FALSE);
		m_current_interactive_source = nullptr;
		m_review_window.set_Capture_Video_Source(nullptr);
		m_review_window.set_Interactive_State(viewState);
		return;
	}
	
	// if it is the freeze frame review state or the record review state

	if (getViewMode() == SPEAKER_ONE_VIEW)
	{

		if (get_main_views().size() > 0)
		{
			VideoSource* vs = get_main_views()[0];
			vs->ShowWindow(FALSE);
			CRect source_rect = vs->GetWndRect();
			m_review_window.MoveWindow(source_rect.left, source_rect.top, source_rect.Width(), source_rect.Height());
			m_review_window.ShowWindow(TRUE);
			m_current_interactive_source = vs;
			// bind the frame recorder to the interactive window 
			CapturedVideoSource* cvs = dynamic_cast<CapturedVideoSource*>(m_current_interactive_source);
			if (cvs != nullptr)
			{

				m_review_window.set_Capture_Video_Source(cvs);
			}

		}
		m_review_window.set_Interactive_State(viewState);
	}
	else
	{
		// not support the interactive mode if 2 view or 4 view in the main area
	}


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

void  VideoSourceMgr::switch_view_mode(VideoMode newVideoMode)
{
	if (newVideoMode == m_viewMode)
		return;

	if (interactive_state != NO_INTERACTIVE)
	{
		return;
	}

	m_viewMode = newVideoMode;
	vector<VideoSource *> main_views = get_main_views();
	CRect rect;
	m_parentWndPtr->GetClientRect(&rect);
	

	vector<VideoSource*>::iterator itor = main_views.begin();
	
	while (itor != main_views.end())
	{
		VideoSource *vs = *itor;
		push_back_to_subview(vs);
		itor++;
	}
		
	pack_windows();
	
}

void  VideoSourceMgr::draw_main_rect(CPaintDC &dc)
{
	
		// Create a pen for drawing the lines
		CPen pen(PS_SOLID, 2, RGB(255, 0, 0)); // red, 2-pixel wide pen
		CPen* pOldPen = dc.SelectObject(&pen);

		vector<CRect>::iterator itor = m_main_window_rects.begin();
		while (itor != m_main_window_rects.end())
		{
			CRect rect = *itor;
			rect.DeflateRect(1, 1);
			dc.Rectangle(rect);
			itor++;
		}
		
		/*
		CRect rect;
		m_parentWndPtr->GetClientRect(rect);








		if (SPEAKER_ONE_VIEW == this->getViewMode())
		{
		
			dc.Rectangle(rect_line_offset, rect_line_offset, rect.Width() - rect_line_offset, rect.Height()*h_percentage - rect_line_offset);
		}
		if (SPEAKER_TWO_VIEW == this->getViewMode())
		{
		
			dc.Rectangle(rect_line_offset, rect_line_offset, rect.Width() - rect_line_offset, rect.Height()*h_percentage - rect_line_offset);
			dc.MoveTo(rect.Width() / 2, rect_line_offset);
			dc.LineTo(rect.Width() / 2, rect.Height()*h_percentage - rect_line_offset);
		}
		if (SPEAKER_FOUR_VIEW == this->getViewMode())
		{
		
			dc.Rectangle(rect_line_offset, rect_line_offset, rect.Width() - rect_line_offset, rect.Height()*h_percentage - rect_line_offset);
			dc.MoveTo(rect.Width() / 2, rect_line_offset);
			dc.LineTo(rect.Width() / 2, rect.Height()*h_percentage - rect_line_offset);
			dc.MoveTo(rect_line_offset, rect.Height()*h_percentage/2);
			dc.LineTo(rect.Width() - rect_line_offset, rect.Height()*h_percentage / 2);
		}*/
	
}