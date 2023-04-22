// TestControlDlg.cpp: 实现文件
//

#include "pch.h"
#include "MedlinkNXG.h"
#include "TestControlDlg.h"
#include "afxdialogex.h"
#include "VideoSourceMgr.h"
#include "AGVideoWnd.h"
#include "libyuv.h"
#include "CustomMessages.h"
// TestControlDlg 对话框
#define MY_TIMER_ID 0
IMPLEMENT_DYNAMIC(TestControlDlg, CDialogEx)

FreezeFrame msg_freeze_frame;
TestControlDlg::TestControlDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CONTROL, pParent)
{
	
}

TestControlDlg::~TestControlDlg()
{
}

void TestControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

}


BEGIN_MESSAGE_MAP(TestControlDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_NO_INTERACTIVE_VIEW, &TestControlDlg::OnBackToNoInteractive)
	ON_BN_CLICKED(IDC_BUTTON_INTERACTIVE_RECORD, &TestControlDlg::OnStartRecording)
	ON_BN_CLICKED(IDC_BUTTON_INTERACTIVE_FREEZEFRAME, &TestControlDlg::OnFreezeFrame)
	ON_BN_CLICKED(IDC_BUTTON_4_VIEW, &TestControlDlg::on_4view)
	ON_BN_CLICKED(IDC_BUTTON_SINGLE, &TestControlDlg::on_1view)
	ON_BN_CLICKED(IDC_BUTTON_DOUBLE, &TestControlDlg::On_2view)
	ON_BN_CLICKED(IDC_BUTTON_INTERACTIVE_RECORD_REVIEW_PLAY, &TestControlDlg::on_play_record)
	ON_BN_CLICKED(INTERACTIVE_RECORD_REVIEW_NEXT, &TestControlDlg::on_stop_record_play)
	ON_BN_CLICKED(INTERACTIVE_RECORD_REVIEW_PREV, &TestControlDlg::on_next_frame)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_FRAME_SEL, &TestControlDlg::OnNMCustomdrawSliderFrameSel)
	ON_WM_TIMER()
END_MESSAGE_MAP()

void TestControlDlg::setVideoSourceMgr(VideoSourceMgr *mgr)
{
	videoMgr = mgr;
	
	
}



void TestControlDlg::OnBackToNoInteractive()
{
	videoMgr->set_interactive_state(NO_INTERACTIVE);	
}


void TestControlDlg::OnStartRecording()
{

		videoMgr->start_Recording("test_record_1.bin");
		SetTimer(MY_TIMER_ID, 5000,NULL);

}


BOOL TestControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	return TRUE;
}




void TestControlDlg::OnFreezeFrame()
{
	videoMgr->freeze_one_frame();


	
}


void TestControlDlg::on_4view()
{
	videoMgr->switch_view_mode(SPEAKER_FOUR_VIEW);
}


void TestControlDlg::on_1view()
{
	videoMgr->switch_view_mode(SPEAKER_ONE_VIEW);
}


void TestControlDlg::On_2view()
{
	videoMgr->switch_view_mode(SPEAKER_TWO_VIEW);
}
void TestControlDlg::on_frame_freeze(FreezeFrame freezeframe) {

	msg_freeze_frame = freezeframe;
	videoMgr->display_freeze_frame(&msg_freeze_frame);
	PostMessage(WM_ON_FREEZE_FRAME, WM_ON_FREEZE_FRAME, (LPARAM)&msg_freeze_frame);
}


void TestControlDlg::on_play_record()
{
	
	
	videoMgr->start_Playing_Record(0);
}


void TestControlDlg::on_stop_record_play()
{
	videoMgr->pause_Playing_Record();
}


void TestControlDlg::on_next_frame()
{
	videoMgr->next_Frame();
}


void TestControlDlg::OnBnClickedButtonPrevFrame()
{
	videoMgr->prev_Frame();
}

void TestControlDlg::OnNMCustomdrawSliderFrameSel(NMHDR *pNMHDR, LRESULT *pResult)
{
	
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	switch (pNMCD->dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
	{
		if (pNMCD->dwItemSpec == TBCD_THUMB)
		{
			CSliderCtrl* pSlider = static_cast<CSliderCtrl*>(CWnd::FromHandle(pNMHDR->hwndFrom));
			int value = pSlider->GetPos();
			videoMgr->goto_Frame(value);
			
		}
		*pResult = CDRF_DODEFAULT;
		break;
	}

	default:
		*pResult = CDRF_DODEFAULT;
		break;
	}
}


void TestControlDlg::OnBnClickedButtonFreezeFrameReview()
{
	videoMgr->set_interactive_state(FREEZE_FRAME_STATE);
}

void TestControlDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == MY_TIMER_ID)
	{
		// perform your one-shot task here
		videoMgr->stop_Recording();
		frame_num = videoMgr->load_Review_Record("test_record_1.bin");
		m_frame_slider.SetRange(0, frame_num);
		KillTimer(MY_TIMER_ID); // stop the timer
	}

	CDialogEx::OnTimer(nIDEvent);
}