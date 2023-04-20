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
	DDX_Control(pDX, IDC_BUTTON_GALLERY, gallery_mode);
	DDX_Control(pDX, IDC_BUTTON_RECORD, startStopRecord);
	DDX_Control(pDX, IDC_BUTTON_SPEAKER_VIEW, speakerView);
	DDX_Control(pDX, IDC_BUTTON_REVIEWMODE, reviewMode);
	DDX_Control(pDX, IDC_BUTTON_SCREENSHOT_ALL, screenShotAll);
	DDX_Control(pDX, IDC_SOURCE_LIST, m_video_source_list);
	DDX_Control(pDX, IDC_LIST3, thumbnail_list);
	DDX_Control(pDX, IDC_SLIDER_FRAME_SEL, m_frame_slider);
}


BEGIN_MESSAGE_MAP(TestControlDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SPEAKER_VIEW, &TestControlDlg::OnBnClickedButtonSpeakerView)
	ON_BN_CLICKED(IDC_BUTTON_RECORD, &TestControlDlg::OnBnClickedButtonSpeakerStartStopRecord)
	ON_BN_CLICKED(IDC_BUTTON_REVIEWMODE, &TestControlDlg::OnBnClickedButtonSpeakerReviewModeEnter)
	ON_BN_CLICKED(IDC_BUTTON_SCREENSHOT_SPEAKER_VIEW, &TestControlDlg::OnBnClickedButtonScreenshotSpeakerView)
	ON_BN_CLICKED(IDC_BUTTON_GALLERY, &TestControlDlg::OnBnClickedButtonGallery)
	ON_BN_CLICKED(IDC_BUTTON_SINGLE, &TestControlDlg::OnBnClickedButtonSingle)
	ON_BN_CLICKED(IDC_BUTTON_SCREENSHOT_ALL, &TestControlDlg::OnBnClickedButtonScreenshotAll)
	ON_BN_CLICKED(IDC_BUTTON_DOUBLE_TB, &TestControlDlg::OnBnClickedButtonDoubleTb)
	ON_BN_CLICKED(IDC_BUTTON_DOUBLE_LR, &TestControlDlg::OnBnClickedButtonDoubleLr)
	ON_MESSAGE(WM_ON_FREEZE_FRAME, OnMessageReceived)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_FILE, &TestControlDlg::OnBnClickedButtonLoadFile)
	ON_BN_CLICKED(IDC_BUTTON_PLAY_RECORD, &TestControlDlg::OnBnClickedButtonPlayRecord)
	ON_BN_CLICKED(IDC_BUTTON_STOP_PLAYING, &TestControlDlg::OnBnClickedButtonStopPlaying)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_FRAME, &TestControlDlg::OnBnClickedButtonNextFrame)
	ON_BN_CLICKED(IDC_BUTTON_PREV_FRAME, &TestControlDlg::OnBnClickedButtonPrevFrame)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_FRAME_SEL, &TestControlDlg::OnNMCustomdrawSliderFrameSel)
	ON_BN_CLICKED(IDC_BUTTON_FREEZE_FRAME_REVIEW, &TestControlDlg::OnBnClickedButtonFreezeFrameReview)
END_MESSAGE_MAP()

void TestControlDlg::setVideoSourceMgr(VideoSourceMgr *mgr)
{
	videoMgr = mgr;
	vector<VideoSource*>::iterator itor = videoMgr->m_video_sources.begin();
	while (itor != videoMgr->m_video_sources.end())
	{
		VideoSource *s = *itor;

		m_video_source_list.AddString((LPCTSTR)s->m_show_name.c_str());
		itor++;

	}
	
}



void TestControlDlg::OnBnClickedButtonSpeakerView()
{
	videoMgr->setNormalViewState();
	
	vector<VideoSource*> sub_views = videoMgr->m_video_sources;
	int index = m_video_source_list.GetCurSel();
	
	VideoSource *vs = nullptr;
	if (index != -1)
	{
		vector<VideoSource*>::iterator begin = sub_views.begin();
		VideoSource *main_video = *(begin + index);
		sub_views.erase(begin+index);

		videoMgr->setSpeakerViewAndSource(main_video,sub_views);


	}


	
	
}


void TestControlDlg::OnBnClickedButtonSpeakerStartStopRecord()
{
	if (start_recording == FALSE)
	{
		videoMgr->start_Recording("test_record_1.bin");
		startStopRecord.SetWindowTextW(L"Stop Recording");
		start_recording = TRUE;
	
	}
	else
	{
		start_recording = FALSE;
		videoMgr->stop_Recording();
		startStopRecord.SetWindowTextW(L"Start Recording");
	}
	
}


BOOL TestControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	thumbnail_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	thumbnail_list.SetIconSpacing(CSize(140, 130));     //set pictures spacing

	//wide of picture is 120,height is 90

	m_imageList.Create(100, 100, ILC_COLORDDB | ILC_COLOR32, 1, 1);

	thumbnail_list.SetImageList(&m_imageList, LVSIL_NORMAL);     //CImageList associates CListCtrl


	
	return TRUE;
}




void TestControlDlg::OnBnClickedButtonScreenshotSpeakerView()
{
	videoMgr->freeze_one_frame();


	
}


void TestControlDlg::OnBnClickedButtonGallery()
{
	videoMgr->setGalleryViewAndSource(videoMgr->m_video_sources);
}


void TestControlDlg::OnBnClickedButtonSingle()
{
	int index= m_video_source_list.GetCurSel();
	if (index != -1)
	{
		videoMgr->setOneViewAndSource(videoMgr->m_video_sources.at(index));
	}
}


void TestControlDlg::OnBnClickedButtonSpeakerReviewModeEnter()
{
	videoMgr->setMainViewState(MAIN_VIEW_RECORD_REVIEW_STATE);
}

void TestControlDlg::OnBnClickedButtonScreenshotAll()
{
	
}



void TestControlDlg::OnBnClickedButtonDoubleTb()
{
	if (m_video_source_list.GetCount() >= 2)
	{
		vector<VideoSource*>::iterator begin = videoMgr->m_video_sources.begin();
		VideoSource *v1 = *(begin + 0);
		VideoSource *v2 = *(begin + 1);
		videoMgr->setTwoViewHAndSource(v1, v2);
	}
	
}


void TestControlDlg::OnBnClickedButtonDoubleLr()
{
	if (m_video_source_list.GetCount() >= 2)
	{
		vector<VideoSource*>::iterator begin = videoMgr->m_video_sources.begin();
		VideoSource *v1 = *(begin + 0);
		VideoSource *v2 = *(begin + 1);
		videoMgr->setTwoViewVAndSource(v1, v2);
	}
}
void TestControlDlg::on_frame_freeze(FreezeFrame freezeframe) {

	msg_freeze_frame = freezeframe;
	videoMgr->display_freeze_frame(&msg_freeze_frame);
	PostMessage(WM_ON_FREEZE_FRAME, WM_ON_FREEZE_FRAME, (LPARAM)&msg_freeze_frame);
}

afx_msg LRESULT TestControlDlg::OnMessageReceived(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case WM_ON_FREEZE_FRAME:
	{
		FreezeFrame *frame = (FreezeFrame *)lParam;
		int framesize = frame->heigth*frame->width;
		const uint8_t* yBuffer = frame->frame_buffer;
		const uint8_t* uBuffer = yBuffer + framesize;
		const uint8_t* vBuffer = uBuffer + framesize/4;
		int width = frame->width;
		int height = frame->heigth;
		BITMAPINFO bitmapInfo = { 0 };
		bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapInfo.bmiHeader.biWidth = width;
		bitmapInfo.bmiHeader.biHeight = -height; // negative height for top-down bitmap
		bitmapInfo.bmiHeader.biPlanes = 1;
		bitmapInfo.bmiHeader.biBitCount = 24;
		bitmapInfo.bmiHeader.biCompression = BI_RGB;
		CBitmap originalBitmap;
		void* bitmapData;
		HBITMAP hBitmap = CreateDIBSection(NULL, &bitmapInfo, DIB_RGB_COLORS, &bitmapData, NULL, 0);
		originalBitmap.Attach(hBitmap);

		libyuv::I420ToRGB24(yBuffer, width,
			uBuffer, width / 2,
			vBuffer, width / 2,
			static_cast<uint8_t*>(bitmapData), width * 3,
			width, height);

		
		// Get the device context of the original bitmap
		CDC originalDC;
		originalDC.CreateCompatibleDC(NULL);
		originalDC.SelectObject(&originalBitmap);

		// Create a new bitmap with the desired size
		CBitmap thumbnailBitmap;
		CDC thumbnailDC;
		thumbnailDC.CreateCompatibleDC(&originalDC);
		thumbnailBitmap.CreateCompatibleBitmap(&originalDC, 100, 100);
		thumbnailDC.SelectObject(&thumbnailBitmap);

		// Copy the original bitmap to the new bitmap, scaled to fit
		thumbnailDC.SetStretchBltMode(HALFTONE);
		thumbnailDC.StretchBlt(0, 0, 100, 100, &originalDC, 0, 0, width, height, SRCCOPY);








		









		//int itemIndex = thumbnail_list.InsertItem(LVIF_IMAGE, index, _T("Item text"), 0, 0, 0, 0);
		//thumbnail_list.SetItem(itemIndex, 1, LVIF_TEXT, _T("Item text"), 0, 0, 0, 0);
		// Update the UI with the data from the worker thread
		CImage image;
		image.Attach(thumbnailBitmap);


		int a = m_imageList.Add(&thumbnailBitmap, RGB(0, 0, 0));

		thumbnail_list.InsertItem(a, _T(""), a);



		CString filePath = _T("file.bmp");
		HRESULT hr = image.Save(filePath, Gdiplus::ImageFormatBMP);
		break;
	}
	default:
		break;
	}

	return 0;
}

void TestControlDlg::OnBnClickedButtonLoadFile()
{
	int frame_num = videoMgr->load_Review_Record("test_record_1.bin");
	m_frame_slider.SetRange(0, frame_num);

}


void TestControlDlg::OnBnClickedButtonPlayRecord()
{
	videoMgr->start_Playing_Record(0);
}


void TestControlDlg::OnBnClickedButtonStopPlaying()
{
	videoMgr->pause_Playing_Record();
}


void TestControlDlg::OnBnClickedButtonNextFrame()
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
	videoMgr->setMainViewState(MAIN_VIEW_REVIEW_FREEZE_FRAME_STATE);
}
