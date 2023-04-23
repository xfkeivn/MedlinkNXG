#include "FrameRecorder.h"
#include <iostream>
#include <cstdio>
#include "libyuv.h"
#include "AGVideoWnd.h"
#include "VideoSourceMgr.h"
#define MAX_VIDEO_BUFFER_SIZE (4*1920*1080)
using namespace std;


bool fileExists(const std::string& fileName) {
	std::ifstream infile(fileName);
	return infile.good();
}

bool Yuv420iToRgb24(const uint8_t* yuv_buffer, int width, int height, uint8_t* rgb_buffer) {
	int y_size = width * height;
	int uv_size = y_size / 4;

	const uint8_t* y_plane = yuv_buffer;
	const uint8_t* u_plane = yuv_buffer + y_size;
	const uint8_t* v_plane = yuv_buffer + y_size + uv_size;

	int y_stride = width;
	int u_stride = width / 2;
	int v_stride = u_stride;

	int rgb_stride = width * 3;

	// Convert YUV to RGB24
	libyuv::I420ToRGB24(y_plane, y_stride,
		u_plane, u_stride,
		v_plane, v_stride,
		rgb_buffer, rgb_stride,
		width, height);

	return true;
}

// Convert YUV420I to CBitmap
CBitmap* Yuv420iToCBitmap(CBitmap* pBitmap,const uint8_t* yuv_buffer, int width, int height) {
	// Allocate memory for RGB buffer
	int rgb_size = width * height * 3;
	uint8_t* rgb_buffer = new uint8_t[rgb_size];

	// Convert YUV420I to RGB24
	Yuv420iToRgb24(yuv_buffer, width, height, rgb_buffer);

	// Create compatible DC
	CDC* pDC = CDC::FromHandle(::GetDC(NULL));
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	// Create compatible bitmap
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;


	pBitmap->CreateCompatibleBitmap(pDC, width, height);

	// Select bitmap into DC
	CBitmap* pOldBitmap = memDC.SelectObject(pBitmap);

	// Set stretch mode
	memDC.SetStretchBltMode(COLORONCOLOR);

	// Copy RGB buffer to bitmap
	::SetDIBitsToDevice(memDC.GetSafeHdc(), 0, 0, width, height, 0, 0, 0, height, rgb_buffer, &bmi, DIB_RGB_COLORS);

	// Cleanup
	memDC.SelectObject(pOldBitmap);
	::ReleaseDC(NULL, pDC->GetSafeHdc());
	delete[] rgb_buffer;

	return pBitmap;
}

FrameRecorder::FrameRecorder()
{
	bitmapData = new unsigned char[MAX_VIDEO_BUFFER_SIZE];
	display_buffer = new char[MAX_VIDEO_BUFFER_SIZE];
	freeze_frame_buffer = new unsigned char[MAX_VIDEO_BUFFER_SIZE];
	m_freeze_frame = { 0,freeze_frame_buffer,MAX_VIDEO_BUFFER_SIZE,0,0 };
	InitializeCriticalSection(&m_cs);
}
void FrameRecorder::start_recording(FileHeader infileHeader, string file_name)
{
	stop_recording();

	EnterCriticalSection(&m_cs);
	fileheader = infileHeader;
	
	if (fileExists(file_name))
	{
		remove(file_name.c_str());
	}
	file_name = file_name;
	if (m_infile.is_open()) {
		m_infile.close();
	}
	if (m_outfile.is_open()) {
		m_infile.close();
	}
	
	m_outfile.open(file_name, std::ios::out | std::ios::binary);
	//m_outfile.seekp(0);
	m_outfile.write((const char*)&fileheader, sizeof(FileHeader));
	is_recording = TRUE;
	LeaveCriticalSection(&m_cs);
}
void FrameRecorder::stop_recording()
{
	EnterCriticalSection(&m_cs);
	is_recording = FALSE;
	//write the frame number to the file
	if (m_outfile.is_open())
	{
		m_outfile.seekp(0);
		m_outfile.write((const char*)&fileheader, sizeof(FileHeader));
		m_outfile.close();
	}
	LeaveCriticalSection(&m_cs);
}


void FrameRecorder::record_frame(char* ybuffer,int ybuffersize, int width, int height,DWORD tick)
{
	if (is_free_frame == TRUE)
	{
		freeze_one_frame(ybuffer, ybuffersize,width,height, tick);
		is_free_frame = FALSE;
	}
	if (is_recording == TRUE)
	{
		EnterCriticalSection(&m_cs);
		if (m_outfile.is_open())
		{
			fileheader.frames++;
			//int framesize = fileheader.frame_width*fileheader.frame_heigth * 2 / 3;
			m_outfile.write((const char*)ybuffer, ybuffersize);
		}
		LeaveCriticalSection(&m_cs);
	}

}

void FrameRecorder::display_frame(int frame_index, CWnd* pWnd)
{
	if (m_infile.is_open())
	{
		int ysize = fileheader.frame_width*fileheader.frame_heigth;
		int uvsize = fileheader.frame_width*fileheader.frame_heigth / 4;
		int framesize = ysize + uvsize * 2;
		int offset = frame_index * framesize + sizeof(FileHeader);

		m_infile.seekg(offset, std::ios::beg);
		m_infile.read(display_buffer, framesize);
		display_yuv_on_window(pWnd, display_buffer, display_buffer + ysize, display_buffer + ysize + uvsize,fileheader.frame_width,fileheader.frame_heigth);
	}
}

void  FrameRecorder::display_frame(FreezeFrame *freezeframe, CWnd* pWnd)
{
	int ysize = freezeframe->width*freezeframe->heigth;
	int uvsize = ysize / 4;
	int framesize = ysize + uvsize * 2;
	display_yuv_on_window(pWnd, (char*)freezeframe->frame_buffer, (char*)freezeframe->frame_buffer + ysize, (char*)freezeframe->frame_buffer + ysize + uvsize,freezeframe->width,freezeframe->heigth);
}




int  FrameRecorder::load_file(string file_name)
{
	FileHeader header;
	header.frames = 0;
	EnterCriticalSection(&m_cs);
	if (fileExists(file_name) && is_recording == FALSE)
	{
		m_infile.open(file_name, std::ios::binary);
		m_infile.seekg(0, std::ios::beg);
		m_infile.read((char*)&header, sizeof(FileHeader));
	}
	LeaveCriticalSection(&m_cs);
	return  header.frames;

}

void FrameRecorder::display_yuv_on_window(CWnd* pWnd,  char* yPlane,  char* uPlane,  char* vPlane,int frame_width,int frame_height) {
	// Calculate the size of a single YUV420 frame

	CBitmap bitmap;
	Yuv420iToCBitmap(&bitmap,(const uint8*)yPlane, frame_width, frame_height);

	BITMAP bm;
	bitmap.GetBitmap(&bm);

	HDC hdc = ::GetDC(pWnd->GetSafeHwnd());
	HDC memDC = ::CreateCompatibleDC(hdc);
	HBITMAP oldBmp = (HBITMAP)::SelectObject(memDC, bitmap);

	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;

	RECT rect;
	::GetClientRect(pWnd->GetSafeHwnd(), &rect);
	int nClientWidth = rect.right - rect.left;
	int nClientHeight = rect.bottom - rect.top;
	double dbAspectRatio = (double)bm.bmWidth / (double)bm.bmHeight;
	double dbClientAspectRatio = (double)nClientWidth / (double)nClientHeight;

	if (dbClientAspectRatio >= dbAspectRatio) {
		// Stretch horizontally
		h = nClientHeight;
		w = nClientHeight * dbAspectRatio;
		x = (nClientWidth - w) / 2;
	}
	else {
		w = nClientWidth;
		h = nClientWidth / dbAspectRatio;
		y = (nClientHeight -h) / 2;
		
	}

	::SetStretchBltMode(hdc, HALFTONE);
	::StretchBlt(hdc, x, y, w, h, memDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	::SelectObject(memDC, oldBmp);
	::DeleteDC(memDC);
	::ReleaseDC(pWnd->GetSafeHwnd(), hdc);
}

void FrameRecorder::freeze_one_frame(char* ybuffer, int ybuffersize, int width, int height, DWORD tick)
{
	//m_freeze_frames.clear();
	memcpy_s(freeze_frame_buffer, MAX_VIDEO_BUFFER_SIZE, ybuffer, ybuffersize);
	m_freeze_frame.width = width;
	m_freeze_frame.heigth = height;
	m_freeze_frame.time_tick = tick;
	((CVideoPlayerWnd*)m_preview_wnd)->on_frame_freezed(&m_freeze_frame);
}

FrameRecorder::~FrameRecorder()
{
	reset();
	delete[] bitmapData;
	delete[] freeze_frame_buffer;
	delete[] display_buffer;
	DeleteCriticalSection(&m_cs);

	/*
	vector<FreezeFrame>::iterator itor = m_freeze_frames.begin();
	while (itor != m_freeze_frames.end())
	{
		FreezeFrame &f = *itor;
		if (f.frame_buffer != nullptr)
		{
			delete[] f.frame_buffer;
		}
		itor++;
	}
	*/
}

void FrameRecorder::freeze_frame()
{
	is_free_frame = TRUE;
}

bool FrameRecorder::is_push_recorded_frame()
{
	VideoSource* vs = m_videoSource->getVideoSourceMgr()->get_main_video_source_in_review();
	if (vs == m_videoSource)
	{
		return TRUE;
	}
	return FALSE;
}

void FrameRecorder::setVideoSource(VideoSource * vs)
{
	m_videoSource = vs;
}
BYTE* FrameRecorder::get_current_review_buffer()
{
	InteractiveState state = m_videoSource->getVideoSourceMgr()->get_interactive_state();
	if (state == FREEZE_FRAME_REVIEW_STATE)
	{
		return freeze_frame_buffer;
	}
	if (state == VIDEO_RECORD_REVIEW_STATE)
	{
		return (BYTE*)display_buffer;
	}
	return nullptr;
}
void FrameRecorder::reset()
{
	if (m_infile.is_open()) {
		m_infile.close();
	}
	if (m_outfile.is_open())
	{
		m_outfile.close();
	}
}