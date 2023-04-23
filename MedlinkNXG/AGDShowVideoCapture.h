#pragma once
#include <afx.h>
#include <atlbase.h>
#include <atlcoll.h>
#include <DShow.h>

#include "capture-filter.hpp"
#include <vector>
#include <tchar.h>
#include <windows.h>
#include <map>
#pragma comment(lib, "strmbase.lib")
using namespace std;

typedef struct _AGORA_DEVICE_INFO
{
	GUID	guidDevice;
	CString	szDeviceName;
	CString	szDevicePath;
	CString	szDeviceDesc;

} AGORA_DEVICE_INFO, *PAGORA_DEVICE_INFO, *LPAGORA_DEVICE_INFO;

enum {
	DEVICE_START = 0,			// start capture
	DEVICE_STOP,				// stop capture
};


using namespace std;
class FramePusherProxy;
class FrameRecorder;

class VideoCaptureSource
{
public:
	VideoCaptureSource();
	virtual ~VideoCaptureSource();
	AGORA_DEVICE_INFO       m_device_info;
	CComPtr<IBaseFilter>	pCaptureFilter = nullptr;
	CComPtr<IBaseFilter>    pSplitterFilter = nullptr;
	CComPtr<IBaseFilter>	pRenderFilter = nullptr;
	CComPtr<CaptureFilter>  videoCapture = nullptr;
	CComPtr <IVideoWindow>  pWindow;
	CWnd *pPreviewWnd = nullptr;
	AM_MEDIA_TYPE                   curMT;
	BITMAPINFOHEADER                bmiHeader;
	bool                            active;
	string     VideoIndexName;
	int			deviceIndex;
	int  m_nCapSelected;
	VIDEO_STREAM_CONFIG_CAPS	m_vscStreamCfgCaps;
	FramePusherProxy *m_framePusherProxy = nullptr;
	FrameRecorder *m_frameRecorder = nullptr;
	LPBYTE		m_lpYUVBuffer = nullptr;
	LPBYTE		m_lpY = nullptr;
	LPBYTE		m_lpU = nullptr;
	LPBYTE		m_lpV = nullptr;
};



class CAGDShowVideoCapture
{
public:
	CAGDShowVideoCapture();
	virtual ~CAGDShowVideoCapture();



	virtual vector<AGORA_DEVICE_INFO> GetDeviceList();
	virtual BOOL OpenDevice(CString devicePath);
	virtual map<int, VIDEOINFOHEADER> get_Device_I420_Support_Config(CString devicePath);
	
	virtual void CloseDevice(CString devicePath);

	virtual int  GetMediaCapCount(CString devicePath);
	virtual BOOL GetMediaCap(CString devicePath,int nIndex, AM_MEDIA_TYPE **ppMediaType, LPVOID lpMediaStreamConfigCaps, SIZE_T nSize);
	virtual BOOL SelectMediaCap(CString devicePath,int nIndex);
	virtual BOOL GetSelectedMediaCap(CString devicePath, AM_MEDIA_TYPE **pMediaType);

    virtual BOOL GetVideoCap(CString devicePath,int nIndex, VIDEOINFOHEADER *lpVideoInfo);
    virtual BOOL GetSelectedVideoCap(CString devicePath,VIDEOINFOHEADER *lpVideoInfo);

	virtual void SetPreviewWnd(CString devicePath,CWnd *wnd);
	virtual void UpdatePreview(CString devicePath);
	virtual bool StartPusherProcess(CString devicePath, string agora_app_id, string agora_channel_id, int agora_user_id);
	
	virtual FrameRecorder * GetFrameRecorder(CString devicePath);

    virtual BOOL Start();
    virtual void Stop();
   

private:
	virtual void EnumDeviceList();
	virtual BOOL Create();
	virtual void Close();
	virtual BOOL ConnectFilters(CString devicePath);
	virtual BOOL RemoveCaptureFilter(CString devicePath);
	virtual BOOL CreateCaptureFilters(CString devicePath);
    void Receive(CString devicePath,bool video, IMediaSample *sample);

private:
	
    CComPtr<IGraphBuilder>     		 m_ptrGraphBuilder;//filter graph
    CComPtr<ICaptureGraphBuilder2>   m_ptrCaptureGraphBuilder2;//filter graph manager
    CComPtr<IMediaControl>           control;


private:
	map<CString, VideoCaptureSource*> m_mapVideoSources;
	bool active = false;
};

