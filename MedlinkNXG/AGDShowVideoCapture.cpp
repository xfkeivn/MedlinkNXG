#define HAVE_JPEG
#include "stdafx.h"
#include "AGDShowVideoCapture.h"
#include "DShowHelper.h"
#include "AgVideoBuffer.h"
#include "libyuv.h"
#include <Dvdmedia.h>
#include "FramePusherProxy.h"
#include "FrameRecorder.h"
#include <Dshow.h>
#include <sstream>
#ifdef DEBUG
#pragma comment(lib, "yuv.lib")
#pragma comment(lib, "jpeg-static.lib")
#else
#pragma comment(lib, "yuv.lib")
#pragma comment(lib, "jpeg-static.lib")
#endif

using namespace libyuv;
#define MAX_VIDEO_BUFFER_SIZE (4*1920*1080) //4K RGBA max size


#include "spdlog/sinks/rotating_file_sink.h"

extern shared_ptr<spdlog::logger> logger;
VideoCaptureSource::VideoCaptureSource()
{
	m_lpYUVBuffer = new BYTE[MAX_VIDEO_BUFFER_SIZE];
	VideoIndexName = "";
	m_framePusherProxy = new FramePusherProxy();
	m_frameRecorder = new FrameRecorder();
}

VideoCaptureSource::~VideoCaptureSource()
{
	m_framePusherProxy->deinit();
	if (m_lpYUVBuffer) {
		delete[] m_lpYUVBuffer;
		m_lpYUVBuffer = nullptr;
	}
	delete m_framePusherProxy;

	delete m_frameRecorder;

}


CAGDShowVideoCapture::CAGDShowVideoCapture(): m_ptrGraphBuilder(nullptr), m_ptrCaptureGraphBuilder2(nullptr)
{
	EnumDeviceList();
	Create();
 
}


CAGDShowVideoCapture::~CAGDShowVideoCapture()
{
	map<CString, VideoCaptureSource*>::iterator itor = m_mapVideoSources.begin();
	while (itor != m_mapVideoSources.end())
	{
		VideoCaptureSource *vsc = itor->second;
		CloseDevice(vsc->m_device_info.szDevicePath);
		delete vsc;
		itor++;
	}
	
    Close();
   
}

BOOL CAGDShowVideoCapture::Create()
{
	HRESULT			hResult = S_OK;
    BOOL bRet = FALSE;
    do {
        if (S_OK != CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER
            , IID_IFilterGraph, (void**)&m_ptrGraphBuilder))
            break;
       
        if (S_OK != CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER
            , IID_ICaptureGraphBuilder2, (void**)&m_ptrCaptureGraphBuilder2))
            break;

        if (S_OK != m_ptrCaptureGraphBuilder2->SetFiltergraph(m_ptrGraphBuilder))
            break;

        if (S_OK != m_ptrGraphBuilder->QueryInterface(IID_IMediaControl, (void**)&control))
            break;

        bRet = TRUE;
    } while (false);
	return bRet;
}

void CAGDShowVideoCapture::Close()
{
    CComPtr<IEnumFilters>     filterEnum = nullptr;
    HRESULT                  hr;

    if (!m_ptrGraphBuilder)
        return;

    hr = m_ptrGraphBuilder->EnumFilters(&filterEnum);
    if (FAILED(hr))
        return;

    CComPtr<IBaseFilter> filter = nullptr;
    while (filterEnum->Next(1, &filter, nullptr) == S_OK) {
        m_ptrGraphBuilder->RemoveFilter(filter);
        filterEnum->Reset();
        filter.Release();
    }

    m_ptrGraphBuilder.Release();
    m_ptrCaptureGraphBuilder2.Release();
  
}


void CAGDShowVideoCapture::EnumDeviceList()
{
	HRESULT		hResult = S_OK;

	CComVariant		var;
	WCHAR			*wszDevicePath = nullptr;

	CComPtr<ICreateDevEnum>     ptrCreateDevEnum = nullptr;
	CComPtr<IEnumMoniker>       ptrEnumMoniker = nullptr;
	CComPtr<IMoniker>    	    ptrMoniker = nullptr;



	m_mapVideoSources.clear();

	hResult = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&ptrCreateDevEnum);
	if (FAILED(hResult))
		return ;

	hResult = ptrCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &ptrEnumMoniker, 0);
	if (FAILED(hResult))
		return ;
	
	do {
		
		AGORA_DEVICE_INFO			agDeviceInfo;
		hResult = ptrEnumMoniker->Next(1, &ptrMoniker, nullptr);
		if (hResult != S_OK)
			break;
		IBaseFilter*  filter;
		if (SUCCEEDED(ptrMoniker->BindToObject(NULL, 0, IID_IBaseFilter,
			(void**)&filter))) {
			CComPtr<IPropertyBag>       ptrPropertyBag = nullptr;

			hResult = ptrMoniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag, (void**)(&ptrPropertyBag));
			if (hResult != S_OK)
				break;


			var.Clear();
			hResult = ptrPropertyBag->Read(L"FriendlyName", &var, nullptr);


			TCHAR temp[200];
			if (SUCCEEDED(hResult)) {

#ifdef UNICODE

				_tcscpy_s(temp, var.bstrVal);

#else
				::WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, temp, MAX_PATH, nullptr, nullptr);

#endif
				agDeviceInfo.szDeviceName.Format(_T("%s"), temp);
			}
			var.Clear();
			hResult = ptrPropertyBag->Read(_T("DevicePath"), &var, nullptr);
			if (SUCCEEDED(hResult)) {
				_tcscpy_s(temp, var.bstrVal);
				agDeviceInfo.szDevicePath.Format(L"%s", temp);
			}

			if (agDeviceInfo.szDevicePath != "")
			{
				VideoCaptureSource *videoCaptureSource = new VideoCaptureSource();
				videoCaptureSource->m_device_info = agDeviceInfo;
				m_mapVideoSources[agDeviceInfo.szDevicePath] = videoCaptureSource;
				std::stringstream ss;
				ss << "VideoSource_" << m_mapVideoSources.size();
				videoCaptureSource->VideoIndexName = ss.str();
			}


			
			
		}
		if (ptrMoniker)
			ptrMoniker.Release();

	} while (TRUE);

}


vector<AGORA_DEVICE_INFO> CAGDShowVideoCapture::GetDeviceList()
{
	HRESULT		hResult = S_OK;

	CComVariant		var;
	WCHAR			*wszDevicePath = nullptr;

    CComPtr<ICreateDevEnum>     ptrCreateDevEnum = nullptr;
    CComPtr<IEnumMoniker>       ptrEnumMoniker   = nullptr;
    CComPtr<IMoniker>    	    ptrMoniker       = nullptr;
   
	
	vector<AGORA_DEVICE_INFO> m_listDeviceInfo;
	m_listDeviceInfo.clear();

	hResult = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
        CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&ptrCreateDevEnum);
	if (FAILED(hResult))
		return m_listDeviceInfo;

	hResult = ptrCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &ptrEnumMoniker, 0);
	if (FAILED(hResult))
		return m_listDeviceInfo;

    do {
		AGORA_DEVICE_INFO			agDeviceInfo;
        hResult = ptrEnumMoniker->Next(1, &ptrMoniker, nullptr);
        if (hResult != S_OK)
            break;
        IBaseFilter*  filter;
        if (SUCCEEDED(ptrMoniker->BindToObject(NULL, 0, IID_IBaseFilter,
            (void**)&filter))) {
            CComPtr<IPropertyBag>       ptrPropertyBag = nullptr;

            hResult = ptrMoniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag, (void**)(&ptrPropertyBag));
            if (hResult != S_OK)
                break;

           
            var.Clear();
            hResult = ptrPropertyBag->Read(L"FriendlyName", &var, nullptr);


			TCHAR temp[200];
            if (SUCCEEDED(hResult)) {
				
#ifdef UNICODE
				
                _tcscpy_s(temp, var.bstrVal);
				
#else
                ::WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1,temp , MAX_PATH, nullptr, nullptr);
				
#endif
				agDeviceInfo.szDeviceName.Format(_T("%s"), temp);
            }
            var.Clear();
            hResult = ptrPropertyBag->Read(_T("DevicePath"), &var, nullptr);
            if (SUCCEEDED(hResult)) {
                _tcscpy_s(temp, var.bstrVal);
				agDeviceInfo.szDevicePath.Format(L"%s", temp);
            }

			if (agDeviceInfo.szDevicePath != "")
			{
				m_listDeviceInfo.push_back(agDeviceInfo);
			}

            
        }
        if (ptrMoniker)
            ptrMoniker.Release();

    } while (TRUE);

	return m_listDeviceInfo;
}



BOOL CAGDShowVideoCapture::OpenDevice(CString devicePath)
{
	AGORA_DEVICE_INFO lpDeviceInfo;
	
	
    HRESULT		         hResult = S_OK;
    IBaseFilter*         filter  = nullptr;

	map<CString,VideoCaptureSource*>::iterator itor = m_mapVideoSources.find(devicePath);
	if (itor == m_mapVideoSources.end())
	{
		return FALSE;
	}
	VideoCaptureSource* vsc = itor->second;

    if (CDShowHelper::GetDeviceFilter(CLSID_VideoInputDeviceCategory, vsc->m_device_info.szDeviceName.GetBuffer(), devicePath.GetBuffer(),&vsc->pCaptureFilter)) {


        hResult = m_ptrGraphBuilder->AddFilter(vsc->pCaptureFilter, nullptr);
        ATLASSERT(SUCCEEDED(hResult));
        if (hResult != S_OK)
            return FALSE;


		map<int, VIDEOINFOHEADER> index_map = get_Device_I420_Support_Config(devicePath);
	
		if (index_map.size() != 0)
		{
			int index = index_map.begin()->first;
			SelectMediaCap(devicePath, index);
		}
		else
			SelectMediaCap(devicePath,0);
	
    }

	if (CreateCaptureFilters(devicePath) == FALSE)
		return FALSE;
	return ConnectFilters(devicePath);
}


void CAGDShowVideoCapture::CloseDevice(CString devicePath)
{
    HRESULT               hResult = S_OK;
  
	map<CString, VideoCaptureSource*>::iterator itor = m_mapVideoSources.find(devicePath);
	if (itor == m_mapVideoSources.end())
	{
		return ;
	}
	VideoCaptureSource* vsc = itor->second;
    m_ptrGraphBuilder->RemoveFilter(vsc->pCaptureFilter);
    
	vsc->active = false;
}

map<int, VIDEOINFOHEADER> CAGDShowVideoCapture::get_Device_I420_Support_Config(CString devicePath)
{
	int mediaCapCount = GetMediaCapCount(devicePath);
	map<int, VIDEOINFOHEADER> index_config_map;
	for (int i = 0; i < mediaCapCount; i++)
	{
		VIDEOINFOHEADER header;
		GetVideoCap(devicePath, i, &header);
	
		if (header.bmiHeader.biCompression == MAKEFOURCC('I', '4', '2', '0') && header.AvgTimePerFrame == 333333 && header.bmiHeader.biWidth == 1920 && header.bmiHeader.biHeight == 1080)
		{
			index_config_map[i] = header;
		}
	}
	return index_config_map;
}


int CAGDShowVideoCapture::GetMediaCapCount(CString devicePath)
{
	int		nCount  = 0;
	int		nSize   = 0;
	HRESULT hResult = S_OK;

    CComPtr<IAMStreamConfig>		ptrStreamConfig  = nullptr;

	map<CString, VideoCaptureSource*>::iterator itor = m_mapVideoSources.find(devicePath);
	if (itor == m_mapVideoSources.end())
	{
		return 0;
	}
	VideoCaptureSource* vsc = itor->second;

	hResult = m_ptrCaptureGraphBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, vsc->pCaptureFilter, IID_IAMStreamConfig, (void**)&ptrStreamConfig);
	ATLASSERT(SUCCEEDED(hResult));
	if (FAILED(hResult))
		return 0;

	hResult = ptrStreamConfig->GetNumberOfCapabilities(&nCount, &nSize);
	ATLASSERT(SUCCEEDED(hResult));
	if (FAILED(hResult))
		return 0;
  
	return nCount;
}

BOOL CAGDShowVideoCapture::GetMediaCap(CString devicePath,int nIndex, AM_MEDIA_TYPE **ppMediaType, LPVOID lpMediaStreamConfigCaps, SIZE_T nSize)
{
	int		nCount = 0;
	int		nCapSize = 0;
	HRESULT hResult = S_OK;

    CComPtr<IBaseFilter>			ptrCaptureFilter = nullptr;
    CComPtr<IAMStreamConfig>		ptrStreamConfig  = nullptr;


	map<CString, VideoCaptureSource*>::iterator itor = m_mapVideoSources.find(devicePath);
	if (itor == m_mapVideoSources.end())
	{
		return FALSE;
	}
	VideoCaptureSource* vsc = itor->second;


	hResult = m_ptrCaptureGraphBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, vsc->pCaptureFilter, IID_IAMStreamConfig, (void**)&ptrStreamConfig);
	ATLASSERT(SUCCEEDED(hResult));
	if (FAILED(hResult))
		return FALSE;

	hResult = ptrStreamConfig->GetNumberOfCapabilities(&nCount, &nCapSize);
	ATLASSERT(SUCCEEDED(hResult));
	if (FAILED(hResult))
		return FALSE;

	ATLASSERT(nCapSize <= static_cast<int>(nSize));
	if (nCapSize > static_cast<int>(nSize))
		return FALSE;

	hResult = ptrStreamConfig->GetStreamCaps(nIndex, ppMediaType, reinterpret_cast<BYTE*>(lpMediaStreamConfigCaps));
	ATLASSERT(SUCCEEDED(hResult));
	if (FAILED(hResult))
		return FALSE;
   
	return TRUE;
}


BOOL CAGDShowVideoCapture::SelectMediaCap(CString devicePath,int nIndex)
{
    int		nCount = 0;
    int		nSize = 0;
    HRESULT hResult = S_OK;

    AM_MEDIA_TYPE	*lpMediaType = NULL;

  
    CComPtr<IAMStreamConfig>		ptrStreamConfig  = nullptr;

	map<CString, VideoCaptureSource*>::iterator itor = m_mapVideoSources.find(devicePath);
	if (itor == m_mapVideoSources.end())
	{
		return FALSE;
	}
	VideoCaptureSource* vsc = itor->second;

    hResult = m_ptrCaptureGraphBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, vsc->pCaptureFilter, IID_IAMStreamConfig, (void**)&ptrStreamConfig);
    ATLASSERT(SUCCEEDED(hResult));
    if (FAILED(hResult))
        return FALSE;

    hResult = ptrStreamConfig->GetNumberOfCapabilities(&nCount, &nSize);
    ATLASSERT(SUCCEEDED(hResult));
    if (FAILED(hResult))
        return FALSE;

    ATLASSERT(nIndex >= 0 && nIndex < nCount);
    if (nIndex < 0 || nIndex >= nCount)
        nIndex = 0;

    ATLASSERT(nSize <= sizeof(VIDEO_STREAM_CONFIG_CAPS));

    do {
        hResult = ptrStreamConfig->GetStreamCaps(nIndex, &lpMediaType, reinterpret_cast<BYTE*>(&vsc->m_vscStreamCfgCaps));
        ATLASSERT(SUCCEEDED(hResult));
        if (FAILED(hResult))
            break;

		if (lpMediaType->formattype == FORMAT_VideoInfo) {
			VIDEOINFOHEADER* pVideoInfo = reinterpret_cast<VIDEOINFOHEADER*>(lpMediaType->pbFormat);
			//pVideoInfo->bmiHeader.biCompression = MAKEFOURCC('I', '4', '2', '0');
		}
		else if (lpMediaType->formattype == FORMAT_VideoInfo2) {
			VIDEOINFOHEADER2* pVideoInfo2 = reinterpret_cast<VIDEOINFOHEADER2*>(lpMediaType->pbFormat);
			//pVideoInfo2->bmiHeader.biCompression = MAKEFOURCC('I', '4', '2', '0');
		}


        hResult = ptrStreamConfig->SetFormat(lpMediaType);
        ATLASSERT(SUCCEEDED(hResult));
        if (FAILED(hResult))
            break;

    } while (FALSE);

    CDShowHelper::FreeMediaType(lpMediaType);
    
    return SUCCEEDED(hResult);
}

BOOL CAGDShowVideoCapture::GetVideoCap(CString devicePath,int nIndex, VIDEOINFOHEADER *lpVideoInfo)
{
	int		nCount = 0;
	int		nSize = 0;

	AM_MEDIA_TYPE *lpAMMediaType = NULL;
	VIDEO_STREAM_CONFIG_CAPS videoStreamCfgCaps;

	BOOL bSuccess = GetMediaCap(devicePath,nIndex, &lpAMMediaType, &videoStreamCfgCaps, sizeof(VIDEO_STREAM_CONFIG_CAPS));

	if (lpAMMediaType->formattype == FORMAT_VideoInfo) {
		VIDEOINFOHEADER* pVideoInfo = reinterpret_cast<VIDEOINFOHEADER*>(lpAMMediaType->pbFormat);
		memcpy_s(lpVideoInfo, sizeof(VIDEOINFOHEADER), pVideoInfo, sizeof(VIDEOINFOHEADER));

		bSuccess = TRUE;
	}
	else if (lpAMMediaType->formattype == FORMAT_VideoInfo2) {
		VIDEOINFOHEADER2* pVideoInfo2 = reinterpret_cast< VIDEOINFOHEADER2*>(lpAMMediaType->pbFormat);
		memcpy_s(&lpVideoInfo->bmiHeader, sizeof(BITMAPINFOHEADER), &pVideoInfo2->bmiHeader, sizeof(BITMAPINFOHEADER));
		lpVideoInfo->AvgTimePerFrame = pVideoInfo2->AvgTimePerFrame;
		lpVideoInfo->dwBitErrorRate = pVideoInfo2->dwBitErrorRate;
		lpVideoInfo->dwBitRate = pVideoInfo2->dwBitRate;
		bSuccess = TRUE;
	}
	else
		bSuccess = FALSE;

    CDShowHelper::FreeMediaType(lpAMMediaType);

	return bSuccess;
}

BOOL CAGDShowVideoCapture::GetSelectedVideoCap(CString devicePath, VIDEOINFOHEADER *lpVideoInfo)
{
	BOOL			bSuccess = FALSE;
	HRESULT			hResult = S_OK;
	AM_MEDIA_TYPE	*lpAMMediaType = NULL;

  
    CComPtr<IAMStreamConfig>		ptrStreamConfig  = nullptr;

	map<CString, VideoCaptureSource*>::iterator itor = m_mapVideoSources.find(devicePath);
	if (itor == m_mapVideoSources.end())
	{
		return FALSE;
	}
	VideoCaptureSource* vsc = itor->second;

	hResult = m_ptrCaptureGraphBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, vsc->pCaptureFilter, IID_IAMStreamConfig, (void**)&ptrStreamConfig);
	ATLASSERT(SUCCEEDED(hResult));
	if (FAILED(hResult))
		return FALSE;

	hResult = ptrStreamConfig->GetFormat(&lpAMMediaType);
	ATLASSERT(SUCCEEDED(hResult));
	if (FAILED(hResult))
		return FALSE;

	if (lpAMMediaType->formattype == FORMAT_VideoInfo) {
		VIDEOINFOHEADER* pVideoInfo = reinterpret_cast<VIDEOINFOHEADER*>(lpAMMediaType->pbFormat);
		memcpy_s(lpVideoInfo, sizeof(VIDEOINFOHEADER), pVideoInfo, sizeof(VIDEOINFOHEADER));

		bSuccess = TRUE;
	}
	else if (lpAMMediaType->formattype == FORMAT_VideoInfo2) {
		VIDEOINFOHEADER2* pVideoInfo2 = reinterpret_cast< VIDEOINFOHEADER2*>(lpAMMediaType->pbFormat);
		memcpy_s(&lpVideoInfo->bmiHeader, sizeof(BITMAPINFOHEADER), &pVideoInfo2->bmiHeader, sizeof(BITMAPINFOHEADER));
		lpVideoInfo->AvgTimePerFrame = pVideoInfo2->AvgTimePerFrame;
		lpVideoInfo->dwBitErrorRate = pVideoInfo2->dwBitErrorRate;
		lpVideoInfo->dwBitRate = pVideoInfo2->dwBitRate;
		bSuccess = TRUE;
	}
	else
		bSuccess = FALSE;

    CDShowHelper::FreeMediaType(lpAMMediaType);

	return bSuccess;
}

BOOL CAGDShowVideoCapture::RemoveCaptureFilter(CString devicePath)
{
	map<CString, VideoCaptureSource*>::iterator itor = m_mapVideoSources.find(devicePath);
	if (itor == m_mapVideoSources.end())
	{
		return FALSE;
	}
	VideoCaptureSource* vsc = itor->second;
    if (vsc->videoCapture) {
        m_ptrGraphBuilder->RemoveFilter(vsc->videoCapture);
		vsc->videoCapture.Release();
        return TRUE;
    }
    return FALSE;
}

void CAGDShowVideoCapture::SetPreviewWnd(CString devicePath,CWnd *wnd)
{
	map<CString, VideoCaptureSource*>::iterator itor = m_mapVideoSources.find(devicePath);
	if (itor == m_mapVideoSources.end())
	{
		return ;
	}
	VideoCaptureSource* vsc = itor->second;
	vsc->pPreviewWnd = wnd;

	if (vsc->pPreviewWnd != nullptr)
	{
		HWND hWnd = vsc->pPreviewWnd->m_hWnd;

		HRESULT hr = vsc->pWindow->put_Owner((OAHWND)hWnd);
		if (FAILED(hr))
		{
			return;
		}
		hr = vsc->pWindow->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
		if (FAILED(hr))
		{
			return;
		}
		hr = vsc->pWindow->put_MessageDrain((OAHWND)hWnd);
		if (FAILED(hr))
		{
			return;
		}
	}
}

void CAGDShowVideoCapture::UpdatePreview(CString devicePath)
{
	map<CString, VideoCaptureSource*>::iterator itor = m_mapVideoSources.find(devicePath);
	if (itor == m_mapVideoSources.end())
	{
		return;
	}
	VideoCaptureSource* vsc = itor->second;
	RECT rcClient;
	GetClientRect(vsc->pPreviewWnd->m_hWnd, &rcClient);
	VIDEOINFOHEADER header;
	GetSelectedVideoCap(devicePath,&header);
	long renderWidth = rcClient.right - rcClient.left;
	long renderHeight = rcClient.bottom - rcClient.top;
	float widthRatio = (float)renderWidth / (float)header.bmiHeader.biWidth;
	float heightRatio = (float)renderHeight / (float)header.bmiHeader.biHeight;
	if (widthRatio > heightRatio) {
		renderWidth = (long)(header.bmiHeader.biWidth * heightRatio);
	}
	else {
		renderHeight = (long)(header.bmiHeader.biHeight * widthRatio);
	}
	long posX = (rcClient.right - rcClient.left - renderWidth) / 2;
	long posY = (rcClient.bottom - rcClient.top - renderHeight) / 2;
	HRESULT hr = vsc->pWindow->SetWindowPosition(posX, posY, renderWidth, renderHeight);
	if (FAILED(hr))
	{
		return;
	}
	hr = vsc->pWindow->put_Visible(OATRUE);
	if (FAILED(hr))
	{
		return;
	}
}

BOOL CAGDShowVideoCapture::CreateCaptureFilters(CString devicePath)
{

	map<CString, VideoCaptureSource*>::iterator itor = m_mapVideoSources.find(devicePath);
	if (itor == m_mapVideoSources.end())
	{
		return FALSE;
	}
	VideoCaptureSource* vsc = itor->second;

    if (vsc->videoCapture) {
        m_ptrGraphBuilder->RemoveFilter(vsc->videoCapture);
		vsc->videoCapture.Release();
		delete vsc->videoCapture;
    }

    AM_MEDIA_TYPE*  mt = nullptr;
	HRESULT hResult = S_OK;
	CString devicepath = vsc->m_device_info.szDevicePath;
    if (GetSelectedMediaCap(devicePath,&mt)) {
        PinCaptureInfo info;
        info.callback = [this, devicepath](IMediaSample *s) {Receive(devicepath,true, s); };
        info.expectedMajorType = mt->majortype;
        info.expectedSubType   = mt->subtype;
        vsc->videoCapture = new CaptureFilter(info);

        vsc->bmiHeader = *CDShowHelper::GetBitmapInfoHeader(*mt);
       // CVideoPackageQueue::GetInstance()->SetVideoFormat(bmiHeader);
        HRESULT hr = m_ptrGraphBuilder->AddFilter(vsc->videoCapture, nullptr);
       
		if (S_OK != CoCreateInstance(CLSID_SmartTee, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&vsc->pSplitterFilter))
		{
			return FALSE;
		}

		// Add the splitter filter to the graph
		hResult = m_ptrGraphBuilder->AddFilter(vsc->pSplitterFilter, nullptr);
		ATLASSERT(SUCCEEDED(hResult));
		if (hResult != S_OK)
			return FALSE;

		
		if (S_OK != CoCreateInstance(CLSID_VideoRenderer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&vsc->pRenderFilter))
		{
			return FALSE;
		}
		hResult = m_ptrGraphBuilder->AddFilter(vsc->pRenderFilter, nullptr);
		ATLASSERT(SUCCEEDED(hResult));
		if (hResult != S_OK)
			return FALSE;
		
		hResult = vsc->pRenderFilter->QueryInterface(IID_IVideoWindow, (void **)&vsc->pWindow);
		ATLASSERT(SUCCEEDED(hResult));
		if (hResult != S_OK)
			return FALSE;

        CDShowHelper::FreeMediaType(mt);
    }
    return TRUE;
}

bool CAGDShowVideoCapture::StartPusherProcess(CString devicePath,string agora_app_id,string agora_channel_id,int agora_user_id)
{
	map<CString, VideoCaptureSource*>::iterator itor = m_mapVideoSources.find(devicePath);
	if (itor == m_mapVideoSources.end())
	{
		return false;
	}
	VideoCaptureSource* vsc = itor->second;


	VIDEOINFOHEADER header;
	GetSelectedVideoCap(devicePath,&header);
	string mutex_name = "Mutex_" + vsc->VideoIndexName;
	string shm_name = "Shm_" + vsc->VideoIndexName;
	//string appid = "b158865ce08b43f788e909aceda9fb7f";
	string appid = agora_app_id;
	string channel_id = agora_channel_id;
	//user id cannot be 0, otherwise there will be error
	int user_id = agora_user_id;
	return  vsc->m_framePusherProxy->init(header.bmiHeader.biWidth, header.bmiHeader.biHeight, mutex_name, shm_name, appid, channel_id,vsc->VideoIndexName, user_id);

}

BOOL CAGDShowVideoCapture::Start()
{
	if (!active)
	{
		HRESULT	result = control->Run();
		if (FAILED(result))
		{
			return FALSE;

		}
		active = true;
		return TRUE;
	}
	
    
 
}

void CAGDShowVideoCapture::Stop()
{
    if (active) {
        control->Stop();
        active = false;
    }
       
}



BOOL CAGDShowVideoCapture::ConnectFilters(CString devicePath)
{
	HRESULT hr = S_OK;
	CComPtr<IPin>   filterPin = nullptr;
	CComPtr<IPin>   capturePin = nullptr;
	CComPtr<IPin>   splitterInPin = nullptr;
	CComPtr<IPin>   splitterOutPinCapture = nullptr;
	CComPtr<IPin>   splitterOutPinPreview = nullptr;
	CComPtr<IPin>   renderInPin = nullptr;
	map<CString, VideoCaptureSource*>::iterator itor = m_mapVideoSources.find(devicePath);
	if (itor == m_mapVideoSources.end())
	{
		return FALSE;
	}
	VideoCaptureSource* vsc = itor->second;

	if (vsc->pCaptureFilter && vsc->videoCapture&&vsc->pSplitterFilter &&vsc->pRenderFilter) {

		// get DS capture filter out pin
		if (!CDShowHelper::GetFilterPin(vsc->pCaptureFilter, MEDIATYPE_Video, PIN_CATEGORY_CAPTURE, PINDIR_OUTPUT, &filterPin)) {
			OutputDebugString(L"Failed to find pin");
			return FALSE;
		}
		// get capture splitter  pin
		if (!CDShowHelper::GetPinByName(vsc->pSplitterFilter, PINDIR_INPUT, nullptr, &splitterInPin)) {
			OutputDebugString(L"Failed to find pin");
			return FALSE;
		}
		// get splitter capture out pin

		hr = vsc->pSplitterFilter->FindPin(L"Preview", &splitterOutPinPreview);

		if (!SUCCEEDED(hr))
		{
			return FALSE;
		}

		hr = vsc->pSplitterFilter->FindPin(L"Capture", &splitterOutPinCapture);

		if (!SUCCEEDED(hr))
		{
			return FALSE;
		}

		// get the agora capture filter in pin 
		if (!CDShowHelper::GetPinByName(vsc->videoCapture, PINDIR_INPUT, nullptr, &capturePin)) {
			OutputDebugString(L"Failed to find capture pin");
			return FALSE;
		}
		// get the render in pin 
		hr = vsc->pRenderFilter->FindPin(L"In", &renderInPin);
		if (FAILED(hr))
		{
			OutputDebugString(L"Failed to find capture pin");
			return FALSE;
		}

		//connect the capture filter to the splitter in
		hr = m_ptrGraphBuilder->ConnectDirect(filterPin, splitterInPin, nullptr);
		if (FAILED(hr)) {
			OutputDebugString(L"failed to connect pins");
			return FALSE;
		}


		// Splitter Capture output connect to capture Pin for pushing to other process
		hr = m_ptrGraphBuilder->ConnectDirect(splitterOutPinCapture, capturePin, nullptr);
		if (FAILED(hr)) {
			OutputDebugString(L"failed to connect pins");
			return FALSE;
		}

		// Splitter Preview output connect to render input
		hr = m_ptrGraphBuilder->Connect(splitterOutPinPreview, renderInPin);
		if (FAILED(hr)) {
			OutputDebugString(L"failed to connect pins");
			return FALSE;
		}
	}

    return TRUE;
}


FrameRecorder * CAGDShowVideoCapture::GetFrameRecorder(CString devicePath)
{
	return m_mapVideoSources.at(devicePath)->m_frameRecorder;

}

BOOL CAGDShowVideoCapture::GetSelectedMediaCap(CString devicePath,AM_MEDIA_TYPE **lpAMMediaType)
{
    BOOL			bSuccess = FALSE;
    HRESULT			hResult = S_OK;

  
    CComPtr<IAMStreamConfig>		ptrStreamConfig = nullptr;

	map<CString, VideoCaptureSource*>::iterator itor = m_mapVideoSources.find(devicePath);
	if (itor == m_mapVideoSources.end())
	{
		return FALSE;
	}
	VideoCaptureSource* vsc = itor->second;

    hResult = m_ptrCaptureGraphBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, vsc->pCaptureFilter, IID_IAMStreamConfig, (void**)&ptrStreamConfig);
    ATLASSERT(SUCCEEDED(hResult));
    if (FAILED(hResult))
        return FALSE;

    hResult = ptrStreamConfig->GetFormat(lpAMMediaType);
    ATLASSERT(SUCCEEDED(hResult));
    if (FAILED(hResult))
        return FALSE;

    return TRUE;
}

void CAGDShowVideoCapture::Receive(CString devicePath,bool video, IMediaSample *sample)
{
    BYTE *pBuffer;
    if (!sample)
        return;
   
    int size = sample->GetActualDataLength();
    if (!size)
        return;

    if (FAILED(sample->GetPointer(&pBuffer)))
        return;
    long long startTime, stopTime;
    bool hasTime = SUCCEEDED(sample->GetTime(&startTime, &stopTime));
   



	map<CString, VideoCaptureSource*>::iterator itor = m_mapVideoSources.find(devicePath);
	if (itor == m_mapVideoSources.end())
	{
		return ;
	}


	VideoCaptureSource* vsc = itor->second;


	logger->debug(string("received frame") + vsc->VideoIndexName);


	BITMAPINFOHEADER   bmiHeader = vsc->bmiHeader;

	LPBYTE		m_lpYUVBuffer = vsc->m_lpYUVBuffer;
	LPBYTE		m_lpY = nullptr;
	LPBYTE		m_lpU = nullptr;
	LPBYTE		m_lpV = nullptr;



	string source_name = vsc->VideoIndexName;
	string path = string("d:\\pictest\\") + source_name;
	string name;
#ifdef DEBUG_TEST
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD  dwBytesWritten = 0;

	



    switch (bmiHeader.biCompression)
    {
    case 0x00000000:	// RGB24
		name = path + (".rgb24");
        hFile = ::CreateFileA(name.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        break;
    case MAKEFOURCC('I', '4', '2', '0'):	// I420
		name = path + (".i420");
        hFile = ::CreateFileA(name.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        break;
    case MAKEFOURCC('Y', 'U', 'Y', '2'):	// YUY2
		name = path + (".yuy2");
        hFile = ::CreateFileA(name.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        break;
    case MAKEFOURCC('M', 'J', 'P', 'G'):	// MJPEG
		name = path + (".jpeg");
        hFile = ::CreateFileA(name.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        break;
    case MAKEFOURCC('U', 'Y', 'V', 'Y'):	// UYVY
		name = path + (".uyvy");
        hFile = ::CreateFileA(name.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        break;
    default:
        break;
    }

    if (hFile != INVALID_HANDLE_VALUE) {
        ::WriteFile(hFile, pBuffer, size, &dwBytesWritten, NULL);
        ::CloseHandle(hFile);
    }
#endif
    m_lpY = m_lpYUVBuffer;
    m_lpU = m_lpY + bmiHeader.biWidth*bmiHeader.biHeight;
    m_lpV = m_lpU + bmiHeader.biWidth*bmiHeader.biHeight / 4;
    switch (bmiHeader.biCompression)
    {
    case 0x00000000:	// RGB24
		RGB24ToI420(pBuffer, bmiHeader.biWidth * 3, 
            m_lpY, bmiHeader.biWidth,
            m_lpU, bmiHeader.biWidth / 2,
            m_lpV, bmiHeader.biWidth / 2,
            bmiHeader.biWidth, bmiHeader.biHeight);
        break;
    case MAKEFOURCC('I', '4', '2', '0'):	// I420
		m_lpYUVBuffer = pBuffer;
        //memcpy_s(m_lpYUVBuffer, 0x800000, pBuffer, size);
        break;
    case MAKEFOURCC('Y', 'U', 'Y', '2'):	// YUY2
        YUY2ToI420(pBuffer, bmiHeader.biWidth * 2,
            m_lpY, bmiHeader.biWidth,
            m_lpU, bmiHeader.biWidth / 2,
            m_lpV, bmiHeader.biWidth / 2,
            bmiHeader.biWidth, bmiHeader.biHeight);
        break;
    case MAKEFOURCC('M', 'J', 'P', 'G'):	// MJPEG
         MJPGToI420(pBuffer, size,
            m_lpY, bmiHeader.biWidth,
            m_lpU, bmiHeader.biWidth / 2,
            m_lpV, bmiHeader.biWidth / 2,
            bmiHeader.biWidth, bmiHeader.biHeight,
            bmiHeader.biWidth, bmiHeader.biHeight);
        break;
    case MAKEFOURCC('U', 'Y', 'V', 'Y'):	// UYVY
       UYVYToI420(pBuffer, bmiHeader.biWidth,
            m_lpY, bmiHeader.biWidth,
            m_lpU, bmiHeader.biWidth / 2,
            m_lpV, bmiHeader.biWidth / 2,
            bmiHeader.biWidth, bmiHeader.biHeight);
        break;
    default:
        ATLASSERT(FALSE);
        break;
    }
    SIZE_T nYUVSize = bmiHeader.biWidth*bmiHeader.biHeight * 3 / 2;

    //if (!CAgVideoBuffer::GetInstance()->writeBuffer(m_lpYUVBuffer, nYUVSize, GetTickCount())) {
    //    OutputDebugString(L"CAgVideoBuffer::GetInstance()->writeBuffer failed.");
    //    return;
    //}

	//if the recorder is started, the frame will be write to the buffer.
	DWORD ts = GetTickCount();
	vsc->m_frameRecorder->record_frame((char*)m_lpYUVBuffer,nYUVSize, bmiHeader.biWidth, bmiHeader.biHeight, ts);
	// push the the other process
	if (vsc->m_frameRecorder->is_push_recorded_frame())
	{
		BYTE* buffer = vsc->m_frameRecorder->get_current_review_buffer();
		vsc->m_framePusherProxy->push_frame(buffer, nYUVSize, ts);
	}
	else
		vsc->m_framePusherProxy->push_frame(m_lpYUVBuffer, nYUVSize, ts);
	
	
	return;


#ifdef DEBUG_TEST
    //hFile = ::CreateFile(_T("d:\\pictest\\trans.i420"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        ::WriteFile(hFile, m_lpYUVBuffer, nYUVSize, &dwBytesWritten, NULL);
        ::CloseHandle(hFile);
    }

#endif
}
