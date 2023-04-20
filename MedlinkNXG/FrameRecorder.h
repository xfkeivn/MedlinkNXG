#pragma once
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;
typedef struct _FileHeader
{
public:
	int frames = 0;
	int frame_width = 0;
	int frame_heigth = 0;

}FileHeader;
class CWnd;

typedef struct __FreezeFrame
{
	DWORD time_tick;
	BYTE *frame_buffer;
	int frame_size;
	int width;
	int heigth;
}FreezeFrame;


class IFrameRecorderObserver
{
public:
	virtual void on_frame_freeze(FreezeFrame freezeframe) = 0;
};



class FrameRecorder
{

private:
	CRITICAL_SECTION  m_cs;
	vector<IFrameRecorderObserver*> m_observers;
	vector<FreezeFrame> m_freeze_frames;

	std::ofstream m_outfile;
	std::ifstream m_infile;
	FileHeader fileheader;
	CWnd *m_preview_wnd;
	unsigned char* bitmapData = NULL;
	bool is_recording = FALSE;
	bool is_free_frame = FALSE;
	void display_yuv_on_window(CWnd* pWnd,  char* yPlane,  char* uPlane,  char* vPlane, int frame_width, int frame_height);
	void freeze_one_frame(char* ybuffer, int ybuffersize, int width, int height, DWORD tick);
public:
	FrameRecorder();
	void add_observer(IFrameRecorderObserver* observer) { m_observers.push_back(observer); }
	void start_recording(FileHeader fileHeader,string record_to_file_name);
	vector<FreezeFrame> & get_FreezeFrames() { return m_freeze_frames; }
	void freeze_frame();
	void record_frame(char* ybuffer, int ybuffersize, int width, int height,DWORD tick);

	void stop_recording();
	int  load_file(string recorded_file_name);
	void display_frame(int frame_index, CWnd* pWnd);
	void display_frame(FreezeFrame *freezeframe, CWnd* pWnd);
	virtual ~FrameRecorder();
};


