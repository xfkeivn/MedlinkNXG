#pragma once


class IDShowCaptureDevice
{
public:

	virtual BOOL Create() = 0;
	virtual void Close() = 0;

	virtual vector<AGORA_DEVICE_INFO> EnumDeviceList() = 0;
	virtual BOOL GetDeviceInfo(int nIndex, LPAGORA_DEVICE_INFO lpDeviceInfo) = 0;

	virtual BOOL OpenDevice(int nIndex) = 0;
	
	virtual int GetCurrentDevice(LPTSTR lpDevicePath, SIZE_T *nDevicePathLen) = 0;
	virtual void CloseDevice() = 0;

	virtual int GetMediaCapCount() = 0;
	virtual BOOL GetMediaCap(int nIndex, AM_MEDIA_TYPE **ppMediaType, LPVOID lpMediaStreamConfigCaps, SIZE_T nSize) = 0;
	virtual BOOL SelectMediaCap(int nIndex) = 0;
	virtual int GetSelectedMediaCap() = 0;
	
	virtual BOOL GetVideoCap(int nIndex, VIDEOINFOHEADER *lpVideoInfo) = 0;
	virtual BOOL GetCurrentVideoCap(VIDEOINFOHEADER *lpVideoInfo) = 0;
	virtual BOOL GetCurrentMediaType(AM_MEDIA_TYPE **pMediaType) = 0;
	virtual BOOL CreateCaptureFilters() = 0;
	virtual BOOL Start() = 0;
	virtual void Stop() = 0;
	virtual BOOL GetAudioCap(int nIndex, WAVEFORMATEX *lpWaveInfo) = 0;
	virtual BOOL GetCurrentAudioCap(WAVEFORMATEX *lpWaveInfo) = 0;
};