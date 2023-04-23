#pragma once

#define WM_SHOWMODECHANGED			WM_USER+300
#define WM_LBUTTON_DOWN_WND			WM_USER+301
#define WM_LBUTTON_UP_WND			WM_USER+302
#define WND_VIDEO_WIDTH     192
#define WND_VIDEO_HEIGHT    144

#define WND_INFO_WIDTH      130
#define WND_INFO_HEIGHT     130

using namespace std;
#include <string>
#include <vector>
#include "FrameRecorder.h"
class CAGInfoWnd : public CWnd
{
	DECLARE_DYNAMIC(CAGInfoWnd)

public:
	CAGInfoWnd();
	virtual ~CAGInfoWnd();

	void ShowTips(BOOL bShow = TRUE, BOOL bIsRemote = TRUE);
	void SetVideoResolution(UINT nWidth, UINT nHeight);
	void SetFrameRateInfo(UINT nFPS);
	void SetVideoBitrate(UINT nBitrate);
	void SetAudioBitrate(UINT bitrate);
	void SetVideoLossRate(UINT lossRate);
	void SetAudioLossRate(UINT lossRate);
	void SetAudioDelay(UINT delay);
	void SetVideoDelay(UINT delay);

	void Reset();

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()

private:
	BOOL		m_bShowTip;

	COLORREF	m_crBackColor;

	UINT		m_nWidth = 0;
	UINT		m_nHeight = 0;
	UINT		m_nFps = 0;
	UINT		m_nVideoBitrate = 0;
	UINT		m_nAudioBitrate = 0;
	UINT		m_nVideoLossRate = 0;
	UINT		m_nAudioLossRate = 0;
	UINT		m_nAudioDelay = 0;
	UINT		m_nVideoDelay = 0;
	BOOL		m_isRemote;
	CBrush		m_brBack;
};

class VideoSource;
class CAGVideoWnd : public CWnd
{
	DECLARE_DYNAMIC(CAGVideoWnd)

public:
	CAGVideoWnd();
	virtual ~CAGVideoWnd();

	void SetUID(UINT dwUID);

	UINT GetUID();
	BOOL IsWndFree();

	void SetFaceColor(COLORREF crBackColor);
	BOOL SetBackImage(UINT nID, UINT nWidth, UINT nHeight, COLORREF crMask = RGB(0xFF, 0xff, 0xFF));


	void SetVideoStatsInfo(UINT nWidth, UINT nHeight, UINT nFps, UINT nBitrate, UINT nLossRate, UINT delay = 0);
	void SetAudioStatsInfo(UINT nBitrate, UINT lossRate, UINT delay = 0);
	void ShowStatsInfo(BOOL bShow, BOOL bIsRemote = TRUE);
	void SetVideoSource(VideoSource *vs);
	VideoSource* GetVideoSource();
	void Reset();
	void setActive(BOOL bActive);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	DECLARE_MESSAGE_MAP()

private:
	CImageList		m_imgBackGround;
	COLORREF		m_crBackColor;
	CAGInfoWnd		m_wndInfo;
	UINT			m_nUID;
	VideoSource *m_videoSource;

private:
	BOOL m_bDragging;
	CPoint m_ptDragStart;
};

class CapturedVideoSource;
class CapturedSourceVideoWnd :public CAGVideoWnd
{
private:
	CapturedVideoSource * m_local_video_source;
public:
	void SetLocalVideoSource(CapturedVideoSource * localvideosource);

};
class FrameRecorder;
class CapturedVideoSource;
class VideoSourceMgr;



class IVideoPlayerObserver
{
public:
	virtual void on_frame_freezed(FreezeFrame freezeframe) = 0;
	virtual void on_video_record_loaded(int frame_number) = 0;
	virtual void on_frame_changed(int frame_index) = 0;
	virtual void on_video_play_to_end(int frame_index) = 0;
};


class CVideoPlayerWnd : public CWnd {

	DECLARE_DYNAMIC(CVideoPlayerWnd)
public:
	CVideoPlayerWnd() {};
	
	// can be called at normal state
	void set_Capture_Video_Source(CapturedVideoSource* vs) { m_current_capture_main_source = vs; }
	void start_Recording(string record_file);
	void stop_Recording();
	void freeze_one_frame();
	
	//can only be called at review state

	void start_Playing_Record(int frame_index = 0);
	void pause_Playing_Record();
	void next_Frame();
	void prev_Frame();
	void goto_Frame(int frame_index);
	void display_freeze_frame(FreezeFrame *freezeframe);
	void on_frame_freezed(FreezeFrame *freezeframe);
	void setVideoSourceMgr(VideoSourceMgr *mgr);
	int get_current_record_frames() { return m_current_record_frame_number; }
	int get_get_current_review_frame_index() {return current_display_frame_index;
	}
	void add_observer(IVideoPlayerObserver* observer) { m_observers.push_back(observer); }
protected:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT id, int w, int h);
	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT on_video_player_notification(WPARAM wParam, LPARAM lParam);
	int  load_Review_Record(string record_file);
private:
	vector<IVideoPlayerObserver*> m_observers;
	int  current_display_frame_index = 0;
	int  current_display_video_total_frame_number = 0;
	bool m_isPlaying = FALSE;
	VideoSourceMgr *m_videoSourcceMgr;
	bool m_isPaused = TRUE;
	CapturedVideoSource* m_current_capture_main_source=nullptr;
	FreezeFrame *m_current_freeze_frame = nullptr;
	int m_current_record_frame_number = 0;
	DECLARE_MESSAGE_MAP()
};