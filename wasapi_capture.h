#pragma once

#include <vector>
//using namespace std;

struct WasapiDevice {
	int is_input;
	int is_default;
	wchar_t deviceID[1024];
	wchar_t deviceName[1024];
	wchar_t descText[1024];
};

struct AUDIO_CAPTURE_CTX;
int audio_capture_creat(AUDIO_CAPTURE_CTX ** pctx, wchar_t * dev_id, int input);
int audio_capture_start(AUDIO_CAPTURE_CTX *ctx);
int audio_capture_stop(AUDIO_CAPTURE_CTX *ctx);
int audio_capture_setcallback(AUDIO_CAPTURE_CTX *ctx, int(*cb)(void *, void *, int), void * data);
int audio_capture_getFormat(AUDIO_CAPTURE_CTX *ctx, int * sample_rate, int * channel_num);
int audio_capture_free(AUDIO_CAPTURE_CTX **pctx);

