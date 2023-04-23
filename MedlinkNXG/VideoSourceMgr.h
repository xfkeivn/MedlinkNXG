#pragma once
#include "stdafx.h"
#include "AGVideoWnd.h"
#include "TestControlDlg.h"
#include "AGDShowVideoCapture.h"
#include <string>
class CWnd;
class CAGDShowVideoCapture;
using namespace std;
#include < map>
#include <string>
#include <vector>

// The layout is speaker view layout, with right and bottom area can be the placeholder of the window.
// ONE_VIEW is the main area of speaker has only one window, 
// TWO_VIEW is the main area has 2 windows
// GALLERY_VIEW is the main area has 4 slitted windows

typedef enum 
{
	SPEAKER_ONE_VIEW,
	SPEAKER_TWO_VIEW,
	SPEAKER_FOUR_VIEW,
}VideoMode;

typedef enum 
{
	SUBVIEWS_HIDE,
	SUBVIEWS_SHOW
} SUBVIEWS_MODE;
typedef enum 
{
	MAIN_VIEW,
	SUB_VIEW,
}ViewType;



typedef enum _CaptureType
{
	HDMI_CAP,
	USB_CAP,
}CaptureType;

class CAGVideoWnd;


class VideoSource
{


		
private:

	ViewType m_viewType = SUB_VIEW;
	int m_agora_user_id;
	VideoSourceMgr * m_video_source_mgr;
protected:
	CAGVideoWnd * m_wnd;
public:
	VideoSource(VideoSourceMgr * mgr, int agora_usr_id){
		m_agora_user_id = agora_usr_id;
		m_video_source_mgr = mgr;
		
	}

	void drop_to_main(CRect rect);
	
	CAGVideoWnd * getVideoWindow() { return m_wnd; }
	// the name of the video source 
	string m_video_source_id;
	// whether show the ODS on the left top corner
	string m_show_name;
	// whether show the border of the window
	bool m_show_border;
	// source width
	int m_source_width;
	// source height
	int m_source_height;
	bool m_hidden;
	int m_main_view_index;

	void set_main_view_index(int main_index) { m_main_view_index = main_index; }
	int get_main_view_index() { return m_main_view_index; }

	int get_agora_user_id() { return m_agora_user_id;}
	virtual void MoveWindow(int posx, int posy, int width, int heigth);
	virtual void ShowWindow(bool show);
	CRect GetWndRect();
	virtual ~VideoSource();
	BOOL is_custom_capture_vieo_source() { return m_agora_user_id >= CUSTOM_CAPTURE_USER_ID_START; }
	ViewType getViewType() const { return m_viewType; }
	void setViewType(ViewType val) { m_viewType = val; }
	virtual bool canDragAndDrop();
	VideoSourceMgr * getVideoSourceMgr() {
		return m_video_source_mgr;
	}
	void set_showable(BOOL bShow) { this->m_hidden = !bShow; }

};

class CapturedSourceVideoWnd;
class CAGDShowVideoCapture;

class  CapturedVideoSource: public VideoSource
{
private:
	CString  m_devicePath;
	// priority 1 HDMI cap ports, 2 USB cap ports 
	CaptureType m_captureType;
	CAGDShowVideoCapture *m_captureMgr;
	int m_device_index;
	
public:
	CapturedVideoSource(VideoSourceMgr * mgr, int agora_user_id,CString devicePath, CAGDShowVideoCapture* captureMgr, int deviceIndex);
	void UpdatePreview();
	virtual void MoveWindow(int posx, int posy, int width, int heigth);
	CString getDevicePath() { return m_devicePath; }
	VIDEOINFOHEADER videoInfoHeader;
	FrameRecorder * getFrameRecorder() { return m_captureMgr->GetFrameRecorder(m_devicePath); }
	virtual bool canInteractive() { return TRUE; }
};




class AgoraRemoteVideoSource :public VideoSource
{
private:
	int remote_user_id;
public:
	AgoraRemoteVideoSource(VideoSourceMgr * mgr, int remoteUserId);
	virtual bool canInteractive() { return remote_user_id >= CUSTOM_CAPTURE_USER_ID_START; }
	
};


class AgoraLocalVideoSource :public VideoSource
{
private:
	int local_user_id;
public:
	AgoraLocalVideoSource(VideoSourceMgr * mgr,int localUserId):VideoSource(mgr,localUserId) {
		local_user_id = localUserId;
		m_wnd->SetVideoSource(this);
	}

};

// this can be used for those free frame list dialog to update the thumbnail
class IVideoMgrObserver
{
public:
	virtual void on_freeze_frame_add(FreezeFrame &freezeframe) = 0;
	virtual void on_record_saved(string record_file) = 0;
	virtual void on_start_recording(string record_file) = 0;
	virtual void on_stop_recording(string record_file) = 0;
};


class VideoSourceMgr
{
public:
	VideoSourceMgr(CWnd* parentWndPtr);
	~VideoSourceMgr();
	void init(string agora_app_id, string agora_channel_id);
	void addObservers(IVideoMgrObserver *observer) { m_window_mgr_observers.push_back(observer); }
	
	VideoSource* add_agora_remote_video_source(int remoteUserId);
	void remove_agora_remote_video_source(int remoteUserId);

	VideoSource* create_agora_local_video_source(int localUserId);
	void destory_agora_local_video_source();

	void init_custom_capture_video_source();
	
	void onParentWndSize();
	void addVideoSource(VideoSource * Ptr);
	VideoMode getViewMode();
	void hideVideoView(VideoSource *);
	//One view
	void showVideoView(VideoSource *);
	//Set Views
	
	void swap_2_views(VideoSource *source, VideoSource* dest);
	void push_back_to_subview(VideoSource* videosource);
	void drop_to_main_view(VideoSource* videsource,CRect* windowRectToDrop = nullptr);
	bool set_interactive_state(InteractiveState viewState);
	bool start_Recording();
	
	bool freeze_one_frame();
	//vector<FreezeFrame>  get_FreezeFrames();
	
	void switch_view_mode(VideoMode newVideoMode);
	void start_Playing_Record(int frame_index = 0);
	void pause_Playing_Record();
	int get_total_frames();
	int get_current_frame();
	void next_Frame();
	void prev_Frame();
	void goto_Frame(int frame_index);
	void draw_main_rect(CPaintDC &dc);
	void show_sub_views(bool show = TRUE);
	CWnd* getParentWindow() { return m_parentWndPtr; }
	InteractiveState get_interactive_state() { return m_interactive_state; }
	VideoSource* get_main_video_source_in_review();
private:
	void close();
	CAGDShowVideoCapture *m_captureMgr;
	void hideAllWindows();
	VideoSource* getDefaultVideoSource();
	void remove_video_source(VideoSource* videosource);
	void pack_windows();
	bool isHost();


	int get_current_main_view_count() {
		return std::count_if(m_video_sources.begin(), m_video_sources.end(), [](VideoSource* vs) { return vs->getViewType() == MAIN_VIEW; });
	}
	int get_current_sub_view_count() {
		return std::count_if(m_video_sources.begin(), m_video_sources.end(), [](VideoSource* vs) { return vs->getViewType() == SUB_VIEW; });
	}

	vector<VideoSource *> get_main_views() {
		vector<VideoSource*> dest;
		std::copy_if(m_video_sources.begin(), m_video_sources.end(), std::back_inserter(dest), [](VideoSource* vs) { return vs->getViewType() == MAIN_VIEW; });
		return dest;
	}
	vector<VideoSource *> get_sub_views() {
		vector<VideoSource*> dest;
		std::copy_if(m_video_sources.begin(), m_video_sources.end(), std::back_inserter(dest), [](VideoSource* vs) { return vs->getViewType() == SUB_VIEW; });
		return dest;
	}
private:
	vector < VideoSource*> m_video_sources;
	vector < IVideoMgrObserver *> m_window_mgr_observers;
	CWnd* m_parentWndPtr;
	VideoMode m_viewMode;
	CVideoPlayerWnd m_review_window;
	FrameRecorder *m_frame_recorder;
	//could be 0,1,2,3,4, according to current speaker view mode.
	map<string, vector<VIDEOINFOHEADER>> device_video_caps;
	InteractiveState m_interactive_state = NO_INTERACTIVE;
	VideoSource * m_current_interactive_source = nullptr;
	vector<CRect> m_main_window_rects;
	//only for the client camera video
	AgoraLocalVideoSource *m_local_video_source;
	TestControlDlg test_dlg;
	double h_percentage = 0.85;
	int sub_view_max_width = 200;
	int rect_line_offset = 5;
	string m_agora_app_id;
	string m_agora_meeting_channel;
	bool sub_view_showable = TRUE;

};





