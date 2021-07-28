
#include "lame.h"
#include <stdlib.h>

#pragma comment (lib,"lib/libmp3lame-static.lib")
#pragma comment (lib,"lib/libmpghip-static.lib")

int encode_open();
int encode_close();
int encode_data();

#define INBUFSIZE 4096
#define MP3BUFSIZE (int) (1.25 * INBUFSIZE) + 7200

struct MP3ENC_CTX {
	lame_global_flags*	lame_fg;
	int					sample_rate;
	int					channgle_num;
	int					format;//0£º¸¡µãÊý£¬1£ºS16
	int					bit_rate;
	FILE*				fp;
	short*				input_buffer;
	int					input_samples;
	unsigned char*		mp3_buffer;
	int					opened = 0;
};




MP3ENC_CTX * mp3_encode_open(wchar_t * name,int rate,int ch,int format,int bit_rate)
{
	lame_global_flags*	lame_fg = lame_init();
	if (lame_fg == NULL){
		return NULL;
	}
	lame_set_in_samplerate(lame_fg, rate);
	lame_set_num_channels(lame_fg, ch);
	lame_set_out_samplerate(lame_fg, rate);
	lame_set_brate(lame_fg, bit_rate/1000);
	if(ch == 2)
		lame_set_mode(lame_fg, (MPEG_mode)STEREO);
	else
		lame_set_mode(lame_fg, (MPEG_mode)MONO);
	lame_set_quality(lame_fg, 2);

	int ret_code = lame_init_params(lame_fg);
	if (ret_code < 0){
		lame_close(lame_fg);
		return NULL;
	}

	FILE * fp = NULL;
	fp = _wfopen(name, L"wb");
	if (fp == NULL)
		return NULL;

	unsigned char * mp3_buffer = (unsigned char *)malloc(MP3BUFSIZE);
	if (mp3_buffer == NULL)
		return NULL;

	MP3ENC_CTX * ctx = (MP3ENC_CTX *)malloc(sizeof(MP3ENC_CTX));
	ctx->lame_fg = lame_fg;
	ctx->sample_rate = rate;
	ctx->channgle_num = ch;
	ctx->bit_rate = bit_rate;
	ctx->fp = fp;
	ctx->mp3_buffer = mp3_buffer;
	ctx->opened = 1;

	return ctx;
}

int mp3_encode_close(MP3ENC_CTX * ctx)
{
	ctx->opened = 0;
	int mp3_bytes = lame_encode_flush(ctx->lame_fg, ctx->mp3_buffer, MP3BUFSIZE);
	if (mp3_bytes > 0)
	{
		fwrite(ctx->mp3_buffer, 1, mp3_bytes, ctx->fp);
	}
	lame_close(ctx->lame_fg);
	fclose(ctx->fp);

	free(ctx->mp3_buffer);
	free(ctx);
	return 0;
}

int mp3_encode_do(MP3ENC_CTX * ctx,float * samples,int sample_num)
{
	if (ctx->opened != 1)
		return -1;
	int mp3_bytes = lame_encode_buffer_interleaved_ieee_float(ctx->lame_fg, samples, sample_num, ctx->mp3_buffer, MP3BUFSIZE);
	//fprintf(stderr, "mp3_bytes is %d./n", mp3_bytes);
	if (mp3_bytes < 0)
	{
		return -1;
	}
	else if (mp3_bytes > 0){
		fwrite(ctx->mp3_buffer, 1, mp3_bytes, ctx->fp);
	}
	return 0;
}