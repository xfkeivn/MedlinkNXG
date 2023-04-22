// AGVideoWnd.cpp : implement file
#include "pch.h"
#include "AGVideoWnd.h"
#include <Gdiplus.h>
#include "VideoSourceMgr.h"
#include "FrameRecorder.h"
using namespace std;
#include <vector>
#define IDD_PLAY_TIMER 1001010

IMPLEMENT_DYNAMIC(CAGInfoWnd, CWnd)

CAGInfoWnd::CAGInfoWnd()
	: m_bShowTip(TRUE)
	, m_nWidth(0)
	, m_nHeight(0)
	, m_nFps(0)
	, m_nVideoBitrate(0)
{
	m_brBack.CreateSolidBrush(RGB(0x58, 0x58, 0x58));
}

CAGInfoWnd::~CAGInfoWnd()
{
	m_brBack.DeleteObject();
}


BEGIN_MESSAGE_MAP(CAGInfoWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void CAGInfoWnd::ShowTips(BOOL bShow, BOOL bIsRemote)
{
	if (m_bShowTip != bShow) {
		if (bShow)
			ShowWindow(SW_SHOW);
		else
			ShowWindow(SW_HIDE);
	}
	
	m_bShowTip = bShow;
	m_isRemote = bIsRemote;
	Invalidate(TRUE);
}

void CAGInfoWnd::SetVideoResolution(UINT nWidth, UINT nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	if (m_bShowTip) {
		Invalidate(TRUE);
	}
}

void CAGInfoWnd::SetFrameRateInfo(UINT nFPS)
{
	m_nFps = nFPS;

	if (m_bShowTip) {
		Invalidate(TRUE);
	}
}

void CAGInfoWnd::SetVideoBitrate(UINT nBitrate)
{
	m_nVideoBitrate = nBitrate;

	if (m_bShowTip) {
		Invalidate(TRUE);
	}
}


void CAGInfoWnd::SetAudioBitrate(UINT bitrate)
{
	m_nAudioBitrate = bitrate;

	if (m_bShowTip) {
		Invalidate(TRUE);
	}
}


void CAGInfoWnd::SetVideoLossRate(UINT lossRate)
{
	m_nVideoLossRate = lossRate;

	if (m_bShowTip) {
		Invalidate(TRUE);
	}
}


void CAGInfoWnd::SetAudioLossRate(UINT lossRate)
{
	m_nAudioLossRate = lossRate;

	if (m_bShowTip) {
		Invalidate(TRUE);
	}
}


void CAGInfoWnd::SetAudioDelay(UINT delay)
{
	m_nAudioDelay = delay;
	if (m_bShowTip) {
		Invalidate(TRUE);
	}
}


void CAGInfoWnd::SetVideoDelay(UINT delay)
{
	m_nVideoDelay = delay;
	if (m_bShowTip) {
		Invalidate(TRUE);
	}
}


void CAGInfoWnd::Reset()
{
	m_nWidth = m_nHeight = m_nFps = m_nVideoBitrate
		= m_nVideoLossRate = m_nVideoDelay
		= m_nAudioBitrate = m_nAudioLossRate
		= m_nAudioDelay = 0;
	if (m_bShowTip) {
		Invalidate(TRUE);
	}
}

void CAGInfoWnd::OnPaint()
{
	CPaintDC dc(this);
	CRect	rcClient;
	CString strTip;

	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(RGB(0xFF, 0xFF, 0xFF));

	if (m_bShowTip) {
		// 640x480,15fps,400k
		GetClientRect(&rcClient);
		rcClient.top += 4;
		if (m_isRemote) {
			strTip.Format(_T("%dx%d, %dfps\nVRecv: %dkbps\nVLossRate: %d��\nVDelay: %dms\nARecv: %dkbps\nALossRate: %d��\nADelay: %dms"),
				m_nWidth, m_nHeight, m_nFps, m_nVideoBitrate, m_nVideoLossRate, m_nVideoDelay, m_nAudioBitrate, m_nAudioLossRate, m_nAudioDelay);
		}
		else {
			strTip.Format(_T("%dx%d, %dfps\nVSent: %dkbps\nVLossRate: %d��\nASent: %dkbps\nALossRate: %d��"),
				m_nWidth, m_nHeight, m_nFps, m_nVideoBitrate, m_nVideoLossRate, m_nAudioBitrate, m_nAudioLossRate);
		}
		
		dc.DrawText(strTip, &rcClient, DT_VCENTER | DT_CENTER);
	}
}

BOOL CAGInfoWnd::OnEraseBkgnd(CDC* pDC)
{

	// TODO:   add message handle code and /or call defalut values here
	CRect rcClient;

	GetClientRect(&rcClient);
	pDC->FillRect(&rcClient, &m_brBack);

	return TRUE;
}

// CAGVideoWnd

IMPLEMENT_DYNAMIC(CAGVideoWnd, CWnd)


CAGVideoWnd::CAGVideoWnd()
	: m_nUID(0)
	, m_crBackColor(RGB(0x58, 0x58, 0x58))
{

}

CAGVideoWnd::~CAGVideoWnd()
{
	m_imgBackGround.DeleteImageList();
}
#define ON_REGISTERED_MESSAGE(message, memberFxn) \
    { message, 0, 0, 0, AfxSig_lwl, \
        (AFX_PMSG)(AFX_PMSGW)(LRESULT (AFX_MSG_CALL CWnd::*)(WPARAM, LPARAM))(memberFxn) },

BEGIN_MESSAGE_MAP(CAGVideoWnd, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_NCACTIVATE()
END_MESSAGE_MAP()



// CAGVideoWnd message handle
BOOL CAGVideoWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO:   add message handle code and /or call default values here
	CRect		rcClient;
	CPoint		ptDraw;
	//IMAGEINFO	imgInfo;

	GetClientRect(&rcClient);

	pDC->FillSolidRect(&rcClient, m_crBackColor);
	return TRUE;
}

void CAGVideoWnd::SetUID(UINT nUID)
{
	m_nUID = nUID;
	//m_wndInfo.ShowWindow(SW_SHOW);
}

UINT CAGVideoWnd::GetUID()
{
	return m_nUID;
}

BOOL CAGVideoWnd::IsWndFree()
{
	return m_nUID == 0 ? TRUE : FALSE;
}

BOOL CAGVideoWnd::SetBackImage(UINT nID, UINT nWidth, UINT nHeight, COLORREF crMask)
{
	CBitmap bmBackImage;

	if (!bmBackImage.LoadBitmap(nID))
		return FALSE;

	m_imgBackGround.DeleteImageList();

	m_imgBackGround.Create(nWidth, nHeight, ILC_COLOR24 | ILC_MASK, 1, 1);
	m_imgBackGround.Add(&bmBackImage, crMask);
	bmBackImage.DeleteObject();

	Invalidate(TRUE);

	return TRUE;
}


void CAGVideoWnd::SetVideoStatsInfo(UINT nWidth, UINT nHeight, UINT nFps, UINT nBitrate, UINT lossRate, UINT delay)
{
	m_wndInfo.SetVideoResolution(nWidth, nHeight);
	m_wndInfo.SetFrameRateInfo(nFps);
	m_wndInfo.SetVideoBitrate(nBitrate);
	m_wndInfo.SetVideoLossRate(lossRate);
	m_wndInfo.SetVideoDelay(delay);
}


void CAGVideoWnd::SetAudioStatsInfo(UINT nBitrate, UINT lossRate, UINT delay)
{
	m_wndInfo.SetAudioBitrate(nBitrate);
	m_wndInfo.SetAudioLossRate(lossRate);
	m_wndInfo.SetAudioDelay(delay);
}

void CAGVideoWnd::SetFaceColor(COLORREF crBackColor)
{
	m_crBackColor = crBackColor;

	Invalidate(TRUE);
}


void CAGVideoWnd::SetVideoSource(VideoSource *vs)
{
	m_videoSource = vs;
}
VideoSource* CAGVideoWnd::GetVideoSource()
{
	return m_videoSource;
}

void CAGVideoWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO:  add message handle code and /or call default values here

	::SendMessage(GetParent()->GetSafeHwnd(), WM_LBUTTON_DOWN_WND, (WPARAM)point.x, (LPARAM)point.y);
	CWnd::OnLButtonDown(nFlags, point);
	setActive(TRUE);
	
	m_bDragging = TRUE;
	m_ptDragStart = point;

	SetCapture();
	SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);


	
}


void CAGVideoWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// The user is moving the mouse on the window
	if (m_bDragging)
	{
		// Calculate the new position of the window
		
		
		CPoint ptOffset = point - m_ptDragStart;
		CRect rectWnd;
		GetWindowRect(&rectWnd);
		GetParent()->ScreenToClient(&rectWnd);

		rectWnd.OffsetRect(ptOffset);
		CRect rectParent;
		GetParent()->GetClientRect(&rectParent);

		if (rectWnd.left < rectParent.left)
		{
			rectWnd.OffsetRect(rectParent.left - rectWnd.left, 0);
		}
		if (rectWnd.right > rectParent.right)
		{
			rectWnd.OffsetRect(rectParent.right - rectWnd.right, 0);
		}
		if (rectWnd.top < rectParent.top)
		{
			rectWnd.OffsetRect(0, rectParent.top - rectWnd.top);
		}
		if (rectWnd.bottom > rectParent.bottom)
		{
			rectWnd.OffsetRect(0, rectParent.bottom - rectWnd.bottom);
		}

		SetWindowPos(NULL, rectWnd.left, rectWnd.top, rectWnd.Width(), rectWnd.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

		//GetParent()->Invalidate();
	


	}
}


void CAGVideoWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO:  add message handle code and /or call default values here

	::SendMessage(GetParent()->GetSafeHwnd(), WM_LBUTTON_UP_WND, (WPARAM)point.x, (LPARAM)point.y);
	CWnd::OnLButtonDown(nFlags, point);
	setActive(FALSE);
	if (m_bDragging)
	{
		// Move the window to the new position
		ReleaseCapture();
		m_bDragging = FALSE;
		
		CRect rectWnd;
		GetWindowRect(&rectWnd);
		GetParent()->ScreenToClient(&rectWnd);

		m_videoSource->drop_to_main(rectWnd);
		

	}


	
}

void CAGVideoWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO:  add message handle code and /or call default values here
	::SendMessage(GetParent()->GetSafeHwnd(), WM_SHOWMODECHANGED, (WPARAM)this, (LPARAM)m_nUID);

	CWnd::OnRButtonDown(nFlags, point);
}


int CAGVideoWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  add you own creation code here
	m_wndInfo.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CRect(0, 0, WND_INFO_WIDTH, WND_INFO_HEIGHT), this, IDC_STATIC);
	m_wndInfo.ShowTips(FALSE);
	return 0;
}


void CAGVideoWnd::ShowStatsInfo(BOOL bShow, BOOL bIsRemote)
{
	m_wndInfo.ShowTips(bShow, bIsRemote);
	if (!bIsRemote) {
		m_wndInfo.MoveWindow(0, 0, WND_INFO_WIDTH, WND_INFO_HEIGHT - 25);
	}
	
}


void CAGVideoWnd::Reset()
{
	m_wndInfo.Reset();
	Invalidate(TRUE);
}

void CAGVideoWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
}

void CAGVideoWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO:  add message handle code and /or call default values here
	::SendMessage(GetParent()->GetSafeHwnd(), WM_SHOWMODECHANGED, (WPARAM)this, (LPARAM)m_nUID);

	CWnd::OnLButtonDblClk(nFlags, point);
}


BOOL CAGVideoWnd::OnNcActivate(BOOL bActive)
{
	setActive(bActive);
	return TRUE;
}


void CAGVideoWnd::setActive(BOOL bActive)
{

	if (bActive)
	{
		// highlight the window
		ModifyStyleEx(0, WS_EX_WINDOWEDGE);
	}
	else
	{
		// remove the highlight
		ModifyStyleEx(WS_EX_WINDOWEDGE, 0);
	}
}



















void CapturedSourceVideoWnd::SetLocalVideoSource(CapturedVideoSource * localvideosource)
{
	m_local_video_source = localvideosource;
}

IMPLEMENT_DYNAMIC(CVideoPlayerWnd, CWnd)

BEGIN_MESSAGE_MAP (CVideoPlayerWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_TIMER()


END_MESSAGE_MAP()

void CVideoPlayerWnd::OnSize(UINT id, int w, int h)
{
	CWnd::OnSize(id, w, h);
}

void CVideoPlayerWnd::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == IDD_PLAY_TIMER)
	{
		next_Frame();
	}

	
}

int CVideoPlayerWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	BOOL result = CWnd::OnCreate(lpCreateStruct);
	return result;
}

void CVideoPlayerWnd::display_freeze_frame(FreezeFrame *freezeframe)
{
	m_current_freeze_frame = freezeframe;
	Invalidate();

}
	void CVideoPlayerWnd::OnPaint() {
		CWnd::OnPaint();
		if (m_main_view_state == VIDEO_REVIEW_STATE)
		{
			m_current_capture_main_source->getFrameRecorder()->display_frame(current_display_frame_index, this);
		}
		if (m_main_view_state == FREEZE_FRAME_STATE && m_current_freeze_frame!=nullptr)
		{

			m_current_capture_main_source->getFrameRecorder()->display_frame(m_current_freeze_frame, this);
		}
		
		
	}

	 void CVideoPlayerWnd::start_Recording(string record_file)
	 {
		 if (m_current_capture_main_source != nullptr)
		 {
			 FileHeader header;
			 header.frame_width = m_current_capture_main_source->videoInfoHeader.bmiHeader.biWidth;
			 header.frame_heigth = m_current_capture_main_source->videoInfoHeader.bmiHeader.biHeight;
			 m_current_capture_main_source->getFrameRecorder()->start_recording(header, record_file);
		 }
	 }
	 void CVideoPlayerWnd::stop_Recording()
	 {
		 if (m_current_capture_main_source != nullptr)
		 {
			 m_current_capture_main_source->getFrameRecorder()->stop_recording();
		 }
	 }
	 int  CVideoPlayerWnd::load_Review_Record(string record_file)
	 {
		 int frames_in_file = 0;
		 if (m_current_capture_main_source != nullptr)
		 {
			 frames_in_file = m_current_capture_main_source->getFrameRecorder()->load_file(record_file);
			 current_display_video_total_frame_number = frames_in_file;
		 }
		 return frames_in_file;
	 }
	 void CVideoPlayerWnd::start_Playing_Record(int frame_index)
	 {
			 if (m_isPaused) {
				
				 int average_fs_ms = m_current_capture_main_source->videoInfoHeader.AvgTimePerFrame / 10000;
				 // start playing from index
				 if (frame_index <= current_display_video_total_frame_number - 1 && frame_index >= 0)
				 {
					 current_display_frame_index = frame_index;
					 SetTimer(IDD_PLAY_TIMER, average_fs_ms, NULL);
					 m_isPlaying = true;
					 m_isPaused = false;
				 }
					
				 
			 }
		
		
	 }

	 void CVideoPlayerWnd::pause_Playing_Record()
	 {
		 if (m_isPlaying) {
			 // Pause the playback
			 m_isPlaying = false;
			 m_isPaused = true;
			 KillTimer(IDD_PLAY_TIMER);

		 }
	 }


	 void CVideoPlayerWnd::next_Frame()
	 {
		
		 if (current_display_frame_index < current_display_video_total_frame_number - 1)
		 {
			 current_display_frame_index++;
			 Invalidate();
		 }
			 
		 

	 }
	 void CVideoPlayerWnd::prev_Frame()
	 {
		
		 if (current_display_frame_index > 0)
		 {
			 current_display_frame_index--;
			 Invalidate();
		 }
	 }
	 void CVideoPlayerWnd::goto_Frame(int frame_index)
	 {
		 
		 if (frame_index <= current_display_video_total_frame_number - 1 && frame_index >= 0)
		 {
			 current_display_frame_index = frame_index;
			 Invalidate();
		 }
	 }


	 void CVideoPlayerWnd::freeze_one_frame()
	 {
		 if (m_current_capture_main_source != nullptr)
		 {
			 m_current_capture_main_source->getFrameRecorder()->freeze_frame();
		 }
	 }
	 vector<FreezeFrame>  CVideoPlayerWnd::get_FreezeFrames()
	 {
		 if (m_current_capture_main_source != nullptr)
		 {
			 return m_current_capture_main_source->getFrameRecorder()->get_FreezeFrames();
		 }
		 vector<FreezeFrame> noneframes;
		 return noneframes;
		
	 }


	 void CVideoPlayerWnd::set_Interactive_State(InteractiveState state)
	 {
		 m_main_view_state = state;

	 }