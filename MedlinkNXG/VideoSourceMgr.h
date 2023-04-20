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

typedef enum 
{
	ONE_VIEW,
	TWO_H_VIEW,
	TWO_V_VIEW,
	GALLERY_VIEW,
	//this is the view that show a main window on the left and right is the sub captured 
	//if there is some none-host camera,show on the bottom 
	SPEAKER_VIEW,
}VideoMode;


#define CUSTOM_CAPTURE_USER_ID_START		10000




typedef enum _CaptureType
{
	HDMI_CAP,
	USB_CAP,
}CaptureType;

class CAGVideoWnd;


class VideoSource
{
	
		
private:
	int m_agora_user_id;
	VideoSourceMgr * m_video_source_mgr;
protected:
	CAGVideoWnd * m_wnd;
public:
	VideoSource(VideoSourceMgr * mgr, int agora_usr_id){
		m_agora_user_id = agora_usr_id;
		m_video_source_mgr = mgr;
	}
	
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
	int get_agora_user_id() { return m_agora_user_id;}
	virtual void MoveWindow(int posx, int posy, int width, int heigth);
	virtual void ShowWindow(bool show);
	CRect GetWndRect();
	virtual ~VideoSource();
	BOOL is_custom_capture_vieo_source() { return m_agora_user_id >= CUSTOM_CAPTURE_USER_ID_START; }
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
};




class AgoraRemoteVideoSource :public VideoSource
{
private:
	int remote_user_id;
public:
	AgoraRemoteVideoSource(VideoSourceMgr * mgr, int remoteUserId);
	
};


class AgoraLocalVideoSource :public VideoSource
{
private:
	int local_user_id;
public:
	AgoraLocalVideoSource(VideoSourceMgr * mgr,int localUserId):VideoSource(mgr,localUserId) {
		local_user_id = localUserId;
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

/*
1. Host will be fully responsible for the UI layout logic, and the client will only follow the host UI layout.
2. Host will first init its own capture resource layout and then listen on the channel
   if there is a user login, it will get the remote user ID and create a remote video source with window and put it at default location 
   if there is a user leave, it will also remove the corresponding video source and window. 
3. Host can broadcast the layout by 1 second interval.
	The client will update it layout to align with the host if there is difference.
4. Host can set the layout to Single mode, then drag and drop the window to main view, 
4. Host can grant the layout control right to a specific client and then the client and do send a layout request to host.
   1. when client is grant the right, he can ignore the host layout broadcast and adjust its layout. 
   2. client can click the sync button to send the layout to host 
   3. host will adjust it layout and send new layout update by 1 second interval  
*/


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
	//Two view
	void setOneViewAndSource(VideoSource *);
	void setTwoViewHAndSource(VideoSource *, VideoSource *);
	void setTwoViewVAndSource(VideoSource *, VideoSource *);
	//Gallery
	void setGalleryViewAndSource(vector<VideoSource *> sorted_video_sources);
	//speaker view
	void setSpeakerViewAndSource(VideoSource *mainSource, vector<VideoSource *> sorted_video_sources);
	void swapVideoSource(VideoSource *source, VideoSource* dest);

	void setNormalViewState();
	void setMainViewState(MainViewState viewState);

	void start_Recording(string record_file);
	void stop_Recording();

	void freeze_one_frame();
	vector<FreezeFrame>  get_FreezeFrames();
	void display_freeze_frame(FreezeFrame *freezeframe);


	int load_Review_Record(string record_file);
	void start_Playing_Record(int frame_index = 0);
	void pause_Playing_Record();
	void next_Frame();
	void prev_Frame();
	void goto_Frame(int frame_index);

	CWnd* getParentWindow() { return m_parentWndPtr; }

	vector < VideoSource*> m_video_sources;
	vector < IVideoMgrObserver *> m_window_mgr_observers;
	VideoSource * m_main_view_source = nullptr;
private:
	void close();
	CAGDShowVideoCapture *m_captureMgr;
	void hideAllWindows();
	VideoSource* getDefaultVideoSource();
	void remove_video_source(VideoSource* videosource);
	void pack_windows();
	CWnd* m_parentWndPtr;
	VideoMode m_viewMode;
	CVideoPlayerWnd m_review_window;
	bool isHost();
	FrameRecorder *m_frame_recorder;
	
	VideoSource * m_lastOneViewSource = nullptr;
	VideoSource * m_lastTwoViewLeftSource = nullptr;
	VideoSource * m_lastTwoViewRightSource = nullptr;
	VideoSource * m_lastTwoViewTopSource = nullptr;
	VideoSource * m_lastTwoViewBottomSource = nullptr;

	vector<VideoSource*> m_lastSortedGalleryViewVideoSource;
	VideoSource * m_lastSpeakerViewMainSource;
	vector<VideoSource*> m_lastSortedSpeakerViewVideoSource;
	map<string, vector<VIDEOINFOHEADER>> device_video_caps;

	MainViewState m_main_view_state = MAIN_VIEW_NORMAL_STATE;

	TestControlDlg test_dlg;
	double h_percentage = 0.8;
	double v_percentage = 0.8;
	string m_agora_app_id;
	string m_agora_meeting_channel;

};





