//
#include "stdafx.h"
#include <string.h>

#include <mmdeviceapi.h>
#include <propkeydef.h>//must include before functiondiscoverykeys_devpkey
#include <functiondiscoverykeys_devpkey.h>
#include <wrl/client.h>
#include <devicetopology.h>
#include <propsys.h>
#include <AudioClient.h>
#include <AudioPolicy.h>

#include <initguid.h>//must define in a cpp file only once

#include "wasapi_capture.h"

using namespace std;

#define SaveRelease(x) if(x!=NULL){x->Release();x=NULL;}
#define SaveCloseHandle(x) if(x!=0){CloseHandle(x);x=0;}

struct AUDIO_CAPTURE_CTX {
	//IMMDeviceEnumerator*	enumerator;
	//IMMDevice *				device;
	IAudioClient*			client_capture;
	IAudioClient*			client_render;
	IAudioCaptureClient*	capture_service;
	IAudioRenderClient*		render_service;
	//IMMNotificationClient*	notify;
	HANDLE ready_event;
	HANDLE stop_event;
	HANDLE hThread;

	wchar_t device_id[1024];
	wchar_t device_name[1024];
	wchar_t last_error[1024];


	int is_input;	//是micphone还是电脑输出
	int is_default;	//是不是默认设备
	int stop_flag;
	int active;
	int(*audio_callback)(void *, void *, int) = NULL;
	void * callback_data = NULL;

	int sampleRate = 0;
	int channelNum = 0;

};

static int ProcessCaptureData(AUDIO_CAPTURE_CTX *ctx)
{
	HRESULT res;
	LPBYTE buffer;
	UINT32 frames;
	DWORD flags;
	UINT64 pos, ts;
	UINT captureSize = 0;

	while (true) {
		res = ctx->capture_service->GetNextPacketSize(&captureSize);
		if (FAILED(res))
			return -1;
		if (!captureSize)
			break;
		res = ctx->capture_service->GetBuffer(&buffer, &frames, &flags, &pos, &ts);
		if (FAILED(res))
			return -1;
		if (ctx->audio_callback != NULL) {
			ctx->audio_callback(ctx->callback_data, buffer, frames * 4 * ctx->channelNum);
		}
		ctx->capture_service->ReleaseBuffer(frames);
	}

	return 0;
}

#define RECONNECT_INTERVAL 3000
static DWORD WINAPI CaptureThread(LPVOID param)
{
	AUDIO_CAPTURE_CTX *ctx = (AUDIO_CAPTURE_CTX *)param;
	DWORD dur = 100;// the_obj->is_input ? RECONNECT_INTERVAL : 10;
	DWORD ret = 0;
	HANDLE sigs[2] = { ctx->ready_event, ctx->stop_event };

	while (ctx->stop_flag == 0) {
		ret = WaitForMultipleObjects(2, sigs, false, dur);
		if (ret == WAIT_OBJECT_0)//收到录制信号
		{
			ProcessCaptureData(ctx);
		}
		else if (ret == WAIT_OBJECT_0 + 1) //收到停止信号
		{
			break;
		}

	}
	ctx->active = 0;
	return 0;
}

int audio_capture_creat(AUDIO_CAPTURE_CTX ** pctx, wchar_t * dev_id, int input)
{
	AUDIO_CAPTURE_CTX* ctx = (AUDIO_CAPTURE_CTX*)malloc(sizeof(AUDIO_CAPTURE_CTX));
	memset(ctx, 0, sizeof(AUDIO_CAPTURE_CTX));
	if (dev_id == NULL) 
		ctx->is_default = 1;
	else
		wcscpy(ctx->device_id, dev_id);
	ctx->is_input = input;

	HRESULT res;
	IMMDeviceEnumerator*	enumerator;
	IMMDevice *				device;
	res = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,CLSCTX_ALL,
		__uuidof(IMMDeviceEnumerator),(void **)&enumerator);
	if (FAILED(res)) {
		wcscpy(ctx->last_error,L"Failed to create enumerator");
		goto error;
	}
	if (ctx->is_default) {
		if (ctx->is_input)
			res = enumerator->GetDefaultAudioEndpoint(eCapture, eCommunications, &device);
		else
			res = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
		if (FAILED(res)) {
			wcscpy(ctx->last_error, L"Failed to GetDefaultAudioEndpoint");
			goto error;
		}
	}
	else {
		res = enumerator->GetDevice(ctx->device_id, &device);
	}
	enumerator->Release();
	enumerator = NULL;
	
	device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&ctx->client_capture);
	WAVEFORMATEX *_wfex;
	ctx->client_capture->GetMixFormat(&_wfex);

	ctx->channelNum = _wfex->nChannels;
	ctx->sampleRate = _wfex->nSamplesPerSec;

	DWORD flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;
	if (ctx->is_input == 0)
		flags |= AUDCLNT_STREAMFLAGS_LOOPBACK;

#define NS_PER_SEC 1000000000
#define REFTIMES_PER_SEC  NS_PER_SEC/100            //100ns per buffer unit
	res = ctx->client_capture->Initialize(AUDCLNT_SHAREMODE_SHARED, flags, REFTIMES_PER_SEC, 0, _wfex, NULL);

	res = ctx->client_capture->GetService(__uuidof(IAudioCaptureClient), (void **)&ctx->capture_service);
	if (FAILED(res)) {
		wcscpy(ctx->last_error, L"Failed to client_capture->GetService");
		goto error;
	}

	ctx->stop_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	ctx->ready_event = CreateEvent(NULL, FALSE, FALSE, NULL);

	res = ctx->client_capture->SetEventHandle(ctx->ready_event);
	if (FAILED(res)) {
		wcscpy(ctx->last_error, L"Failed to SetEventHandle");
		goto error;
	}

	if (!ctx->is_input) {
		WAVEFORMATEX * wfex;
		LPBYTE buffer;
		UINT32 frames;
		#define BUFFER_TIME_100NS (5 * 10000000)
		res = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void **)&ctx->client_render);
		if (FAILED(res))
			goto error;
		res = ctx->client_render->GetMixFormat(&wfex);
		if (FAILED(res))
			goto error;
		res = ctx->client_render->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, BUFFER_TIME_100NS, 0, wfex, nullptr);
		CoTaskMemFree(wfex);
		if (FAILED(res))
			goto error;
		res = ctx->client_render->GetBufferSize(&frames);
		if (FAILED(res))
			goto error;

		res = ctx->client_render->GetService(__uuidof(IAudioRenderClient), (void **)&ctx->render_service);
		if (FAILED(res))
			goto error;
		res = ctx->render_service->GetBuffer(frames, &buffer);
		if (FAILED(res))
			goto error;
		memset(buffer, 0, frames * wfex->nBlockAlign);
		ctx->render_service->ReleaseBuffer(frames, 0);
		ctx->client_render->Start();
	}
	*pctx = ctx;
	device->Release();
	return 0;
error:
	SaveRelease(ctx->client_render);
	SaveRelease(ctx->client_capture);
	SaveRelease(ctx->render_service);
	SaveRelease(ctx->capture_service);
	SaveCloseHandle(ctx->ready_event);
	SaveCloseHandle(ctx->stop_event);
	free(ctx);
	*pctx = NULL;
	return -1;
}
int audio_capture_start(AUDIO_CAPTURE_CTX *ctx)
{
	if (ctx != NULL && ctx->client_capture != NULL) {
		ctx->stop_flag = 0;
		ctx->hThread = CreateThread(nullptr, 0, CaptureThread, ctx, 0, nullptr);
		if (!ctx->hThread) {
			wcscpy(ctx->last_error, L"Failed to CreateThread");
			return -1;
		};
		ctx->client_capture->Start();
		ctx->active = 1;
	}
	return 0;
}
int audio_capture_stop(AUDIO_CAPTURE_CTX *ctx)
{
	if (ctx == NULL)
		return -1;
	ctx->stop_flag = 1;
	Sleep(100);
	if (ctx->active) {
		SetEvent(ctx->stop_event);
		WaitForSingleObject(ctx->hThread, INFINITE);
	}
	if(ctx->client_capture)
		ctx->client_capture->Stop();
	if (ctx->client_render != NULL)
		ctx->client_render->Stop();

	ctx->active = 0;
	return 0;
}
int audio_capture_setcallback(AUDIO_CAPTURE_CTX *ctx, int(*cb)(void *, void *, int), void * data)
{
	if (ctx != NULL) {
		ctx->audio_callback = cb;
		ctx->callback_data = data;
	}
	return 0;
}
int audio_capture_getFormat(AUDIO_CAPTURE_CTX *ctx, int * sample_rate, int * channel_num)
{
	*sample_rate = ctx->sampleRate;
	*channel_num = ctx->channelNum;
	return 0;
}

int audio_capture_free(AUDIO_CAPTURE_CTX **pctx)
{
	if (pctx == NULL)
		return -1;
	AUDIO_CAPTURE_CTX *ctx = *pctx;
	if (ctx == NULL)
		return -1;
	SaveRelease(ctx->client_render);
	SaveRelease(ctx->client_capture);
	SaveRelease(ctx->render_service);
	SaveRelease(ctx->capture_service);
	SaveCloseHandle(ctx->ready_event);
	SaveCloseHandle(ctx->stop_event);

	free(ctx);
	*pctx = NULL;
	return 0;
}

static int GetDeviceName(IMMDevice *device,wchar_t * device_name)
{
	IPropertyStore * store;
	HRESULT res;
	if (SUCCEEDED(device->OpenPropertyStore(STGM_READ, &store))) {
		PROPVARIANT nameVar;
		PropVariantInit(&nameVar);
		res = store->GetValue(PKEY_Device_FriendlyName, &nameVar);
		if (SUCCEEDED(res) && nameVar.pwszVal && *nameVar.pwszVal) {
			wcscpy(device_name,nameVar.pwszVal);
		}
		store->Release();
	}
	return 0;
}

static int GetWASAPIAudioDevices_(vector<WasapiDevice> &devicesList, bool input)
{
	IMMDeviceEnumerator* enumerator;
	IMMDeviceCollection* collection;
	UINT count;
	HRESULT res;

	res = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void **)&enumerator);
	if (FAILED(res))
		return -1;

	res = enumerator->EnumAudioEndpoints(input ? eCapture : eRender, DEVICE_STATE_ACTIVE, &collection);
	if (FAILED(res))
		return -1;

	res = collection->GetCount(&count);
	if (FAILED(res))
		return -1;
	int n = 2;
	for (UINT i = 0; i < count; i++) {
		IMMDevice* device;
		wchar_t *w_id;

		size_t len, size;
		res = collection->Item(i, &device);
		if (FAILED(res))
			continue;

		res = device->GetId(&w_id);
		if (FAILED(res) || !w_id || !*w_id)
			continue;

		WasapiDevice dev;
		dev.is_input = input;
		dev.is_default = 0;
		wcscpy(dev.deviceID , w_id);
		GetDeviceName(device, dev.deviceName);
		if (!input)
			wsprintf(dev.descText, L"电脑输出   ：%s", dev.deviceName);
		else
			wsprintf(dev.descText, L"麦克风输入 ：%s", dev.deviceName);

		devicesList.push_back(dev);
		device->Release();
	}
	enumerator->Release();
	collection->Release();
	return 0;
}

int GetWASAPIAudioDevices(vector<WasapiDevice> &devicesList)
{
	GetWASAPIAudioDevices_(devicesList, 0);
	GetWASAPIAudioDevices_(devicesList, 1);
	return 0;
}
