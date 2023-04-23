#pragma once


// TestControlDlg 对话框
#include "stdafx.h"
#include <vector>
#include "AGVideoWnd.h"
using namespace std;
class VideoSourceMgr;





class TestControlDlg : public CDialogEx,public IVideoPlayerObserver
{
	DECLARE_DYNAMIC(TestControlDlg)

private:
	VideoSourceMgr *videoMgr;
	int frame_num = 0;
	bool show_sub_view = TRUE;
public:
	TestControlDlg(CWnd* pParent = nullptr);   
	virtual ~TestControlDlg();
	void setVideoSourceMgr(VideoSourceMgr *mgr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CONTROL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);   

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBackToNoInteractive();
	afx_msg void on_4view();
	afx_msg void on_1view();
	afx_msg void OnStartRecording();
	
	afx_msg void OnFreezeFrame();
	
	//virtual void on_frame_freezed(FreezeFrame freezeframe);
	//virtual void on_video_recorded(int framenumber);

	bool start_recording = FALSE;

	afx_msg void On_2view();

	BOOL OnInitDialog();
	afx_msg void on_play_record();
	afx_msg void on_stop_record_play();
	afx_msg void on_next_frame();
	afx_msg void OnBnClickedButtonPrevFrame();
	afx_msg void OnNMCustomdrawSliderFrameSel(NMHDR *pNMHDR, LRESULT *pResult);
//	CSliderCtrl m_frame_slider;
	afx_msg void OnBnClickedButtonFreezeFrameReview();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButton1();


	virtual void on_frame_freezed(FreezeFrame freezeframe);
	virtual void on_video_record_loaded(int frame_number);
	virtual void on_frame_changed(int frame_index);
	virtual void on_video_play_to_end(int frame_index);
	CSliderCtrl m_frame_slider;
};
