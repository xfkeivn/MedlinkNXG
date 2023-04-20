#pragma once


// TestControlDlg 对话框
#include "stdafx.h"
#include <vector>
#include "FrameRecorder.h"
using namespace std;
class VideoSourceMgr;





class TestControlDlg : public CDialogEx,public IFrameRecorderObserver
{
	DECLARE_DYNAMIC(TestControlDlg)

private:
	VideoSourceMgr *videoMgr;

public:
	TestControlDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~TestControlDlg();
	void setVideoSourceMgr(VideoSourceMgr *mgr);

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CONTROL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSpeakerView();
	afx_msg void OnBnClickedButtonGallery();
	afx_msg void OnBnClickedButtonSingle();
	afx_msg void OnBnClickedButtonSpeakerStartStopRecord();
	afx_msg void OnBnClickedButtonSpeakerReviewModeEnter();
	afx_msg void OnBnClickedButtonScreenshotSpeakerView();
	afx_msg void OnBnClickedButtonScreenshotAll();
	afx_msg LRESULT OnMessageReceived(WPARAM wParam, LPARAM lParam);
		virtual void on_frame_freeze(FreezeFrame freezeframe);
	CButton gallery_mode;
	CButton startStopRecord;
	CButton speakerView;
	CButton reviewMode;
	CButton screenShotAll;
	CListBox m_video_source_list;
	bool start_recording = FALSE;
	afx_msg void OnBnClickedButtonDoubleTb();
	afx_msg void OnBnClickedButtonDoubleLr();

	BOOL OnInitDialog();
	
	CImageList m_imageList;
	
	CListCtrl thumbnail_list;
	afx_msg void OnBnClickedButtonLoadFile();
	afx_msg void OnBnClickedButtonPlayRecord();
	afx_msg void OnBnClickedButtonStopPlaying();
	afx_msg void OnBnClickedButtonNextFrame();
	afx_msg void OnBnClickedButtonPrevFrame();
	afx_msg void OnNMCustomdrawSliderFrameSel(NMHDR *pNMHDR, LRESULT *pResult);
	CSliderCtrl m_frame_slider;
	afx_msg void OnBnClickedButtonFreezeFrameReview();
};
