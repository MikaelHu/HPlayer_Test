// HPlayer_Test.cpp : 定义控制台应用程序的入口点。
#include "stdafx.h"
#include <Windows.h>

#include "../HPlayer/Com/Common.h"
#include "SDLAudioPlayer.h"
#include "SDLVideoPlayer.h"


#ifdef _DEBUG
#pragma comment(lib, "HPlayer_d.lib")
#else
#pragma comment(lib, "HPlayer.lib")
#endif // _DEBUG


//SDL 重新定义了main: #define main SDL_main
#undef main


int test_queue_audio_1();
int test_nonqueue_audio_1();
int test_audio_wav_1();
int test_audio_pcm_1();

int test_video_yuv420p_1();
int test_video_bgr24_1();


//int main(int argc, char** argv)
//{
//	//test_queue_audio_1();
//	//test_nonqueue_audio_1();
//	//test_audio_wav_1();
//	//test_audio_pcm_1();
//
//	//test_video_yuv420p_1();
//	test_video_bgr24_1();
//
//    return 0;
//}


int test_queue_audio_1() {
	printf("########################test_queue_audio_1#################################\n");
	printf("###queue audio play\n");
	printf("###open pcm file and display!\n");

	//open pcm file
	char* pcm_file = "F:/Projs/WorkDir/Projects/Nico_Face_Reco/audio_data/sample-15s_nc_1_16000.pcm";
	FILE* pcm_fd = fopen(pcm_file, "rb");
	if (!pcm_fd) {
		fprintf(stderr, "Failed to open pcm file!\n");
		return -1;
	}

	SDLAudioOpt_t sdl_audio_opt = { 16000, 1, 1024, AUDIO_S16LSB };
	SDLQueueAudioPlayer sdl_audio_player(sdl_audio_opt);
	sdl_audio_player.Start();
	int buf_size = sdl_audio_opt.samples * sdl_audio_opt.channels * 2;
	char* buf = new char[buf_size];
	memset(buf, 0, buf_size);

	do {
		//read data from pcm file
		int nret = fread(buf, 1, buf_size, pcm_fd);

		nret = sdl_audio_player.Write(buf, nret);
		sdl_audio_player.Delay(6);
	} while (feof(pcm_fd) == 0);

	Sleep(1000 * 30);
	sdl_audio_player.Stop();
	fclose(pcm_fd);
	SAFE_DEL_A(buf);

	return 0;
}

int test_nonqueue_audio_1() {
	printf("########################test_nonqueue_audio_1#################################\n");
	printf("###nonqueue audio play\n");
	printf("###open pcm file and display!\n");

	//open pcm file
	char* pcm_file = "F:/Projs/WorkDir/Projects/Nico_Face_Reco/audio_data/sample-15s_nc_2_44100.pcm";
	FILE* pcm_fd = fopen(pcm_file, "rb");
	if (!pcm_fd) {
		fprintf(stderr, "Failed to open pcm file!\n");
		return -1;
	}

	SDLAudioOpt_t sdl_audio_opt = { 44100, 2, 1024, AUDIO_S16LSB };
	SDLNonQueueAudioPlayer sdl_audio_player(sdl_audio_opt);
	sdl_audio_player.NewChannel("Left");
	//sdl_speaker.NewChannel("Right");

	sdl_audio_player.Start();
	int buf_size = sdl_audio_opt.samples * sdl_audio_opt.channels * 2;
	char* buf = new char[buf_size];
	memset(buf, 0, buf_size);

	do {
		//read data from pcm file
		int nret = fread(buf, 1, buf_size, pcm_fd);
		//int size = buf_size / 2;

		do {
			nret = sdl_audio_player.Write(buf, nret, "Left");
			//nret = sdl_speaker.Write(buf, nret, "Right");
			sdl_audio_player.Delay(6);
		} while (nret == -2);
		if (nret == -1 || nret == -3)
			break;
	} while (feof(pcm_fd) == 0);

	Sleep(1000 * 30);
	sdl_audio_player.Stop();
	fclose(pcm_fd);
	SAFE_DEL_A(buf);

	return 0;
}

int test_audio_wav_1() {
	printf("########################test_audio_wav_1#################################\n");
	printf("###queue audio play\n");
	printf("###open wav file, parse it and play!\n");

	char* wav_file = "F:/Projs/WorkDir/Projects/Nico_Face_Reco/audio_data/sample-15s.wav";
	SDLAudio sdl_audio;

	SDLAudioOpt_t src_opt;
	Uint8 *audio_buf = nullptr;
	Uint32 audio_len;
	int nret = sdl_audio.LoadWav(wav_file, &src_opt, &audio_buf, &audio_len);
	if (!nret) {
		SDLAudioOpt_t dst_opt;
		dst_opt.format = AUDIO_S16LSB;
		dst_opt.channels = 1;
		dst_opt.freq = 16000;
		dst_opt.samples = 1024;

		int stream_id = sdl_audio.NewStream(src_opt, dst_opt);
		if (stream_id >= 0) {
			long LEN = audio_len;
			int in_size = src_opt.samples / src_opt.channels;
			int out_size = dst_opt.samples * dst_opt.channels * 2;
			Uint8 *out_buf = new Uint8[out_size];

			SDLQueueAudioPlayer sdl_audio_player(dst_opt);
			sdl_audio_player.Start();

			do {
				int pos = audio_len - LEN;
				nret = sdl_audio.StreamPut(stream_id, (void*)(audio_buf + pos), in_size);
				if (nret != 0) {
					printf("SDLAudio::StreamPut err!\n");
					break;
				}
				LEN -= in_size;

				nret = sdl_audio.StreamFlush(stream_id);

				memset(out_buf, 0, sizeof(Uint8) * out_size);
				nret = sdl_audio.StreamGet(stream_id, (void*)out_buf, out_size);
				if (nret < 0) {
					printf("SDLAudio::StreamGet err!\n");
					break;
				}
				if (nret == 0) {
					continue;
				}

				nret = sdl_audio_player.Write(out_buf, nret);
				if (nret < 0) {
					printf("SDLAudioPlayer::Write err!\n");
					break;
				}
				sdl_audio_player.Delay(6);
			} while (LEN > 0);

			// 等待音频播放完。
			sdl_audio_player.Delay(1000 * 30);
			sdl_audio_player.Stop();
		}
		sdl_audio.FreeStream(stream_id);
	}

	sdl_audio.FreeWav(audio_buf);


	return 0;
}

int test_audio_pcm_1() {
	printf("########################test_audio_wav_1#################################\n");
	printf("###queue audio play\n");
	printf("###open pcm file and play it!\n");

	char* pcm_file = "F:/Projs/WorkDir/Projects/Nico_Face_Reco/audio_data/sample-15s_nc_2_44100.pcm";

	FILE *fp = NULL;
	fp = fopen(pcm_file, "rb+");
	if (fp == NULL) {
		printf("cannot open this audio file: %s\n", pcm_file);
		return -1;
	}

	SDLAudio sdl_audio;

	SDLAudioOpt_t dst_opt;
	dst_opt.format = AUDIO_S16LSB;
	dst_opt.channels = 2;
	dst_opt.freq = 44100;
	dst_opt.samples = 1024;

	SDLQueueAudioPlayer sdl_audio_player(dst_opt);
	if (sdl_audio_player.Start()) {
		fclose(fp);
		printf("cannot open sdl audio player err: %s\n", sdl_audio_player.GetLastErr());
		return -1;
	}

	const int BUFSIZE = dst_opt.samples * dst_opt.channels * 2;
	uint8_t *pcm_buf = NULL;
	pcm_buf = new uint8_t[BUFSIZE];

	while (feof(fp) == 0) {
		memset(pcm_buf, 0, BUFSIZE);
		// 读取一帧数数据到缓冲区
		int nret = fread(pcm_buf, 1, BUFSIZE, fp);
		if (nret > 0) {
			nret = sdl_audio_player.Write(pcm_buf, nret);
			if (nret < 0) {
				printf("SDLAudioPlayer::Write err!\n");
				break;
			}
			sdl_audio_player.Delay(6);
		}
	}

	// 等待音频播放完。
	sdl_audio_player.Delay(1000 * 30);
	sdl_audio_player.Stop();

	// 关闭文件和释放资源
	fclose(fp);
	SAFE_DEL_A(pcm_buf);

	return 0;
}


int test_video_yuv420p_1() {
	printf("########################test_video_yuv420_1#################################\n");
	printf("###video play\n");
	printf("###open yuv420 file, and display!\n");

	const char* yuv420p = "F:/Projs/WorkDir/Projects/Nico_Face_Reco/video_data/VID_20240906_204935_hdmx.yuv420";

	FILE *fp = NULL;
	fp = fopen(yuv420p, "rb+");
	if (fp == NULL) {
		printf("cannot open this video file: %s\n", yuv420p);
		return -1;
	}

	SDLVideoPlayer sdl_video_player;
	if (sdl_video_player.Open()) {
		fclose(fp);
		printf("cannot open sdl video player err: %s\n", sdl_video_player.GetLastErr());
		return -1;
	}

	const int width = 960;
	const int height = 540;
	const int BUFSIZE = width * height * 3 >> 1;
	uint8_t *yuv_buf = NULL;
	yuv_buf = new uint8_t[BUFSIZE];

	while (feof(fp) == 0) {
		memset(yuv_buf, 0, BUFSIZE);
		// 读取一帧数数据到缓冲区
		int ret = fread(yuv_buf, 1, BUFSIZE, fp);
		if (ret > 0) {
			if (sdl_video_player.Show(yuv_buf, ret)) {
				printf("sdl video player show err: %s\n", sdl_video_player.GetLastErr());
				break;
			}

			sdl_video_player.Delay(25);
		}
	}

	sdl_video_player.Close();

	// 关闭文件和释放资源
	fclose(fp);
	SAFE_DEL_A(yuv_buf);

	return 0;
}

int test_video_bgr24_1() {
	printf("########################test_video_bgr24_1#################################\n");
	printf("###video play\n");
	printf("###open bgr24 file, and display!\n");

	const char* bgr24 = "F:/Projs/WorkDir/Projects/Nico_Face_Reco/video_data/VID_20240906_204935_hcvt.bgr24";

	FILE *fp = NULL;
	fp = fopen(bgr24, "rb+");
	if (fp == NULL) {
		printf("cannot open this video file: %s\n", bgr24);
		return -1;
	}

	SDLVideoPlayer sdl_video_player;
	pixelSpec_t pixel_spec = { SDL_PIXELFORMAT_BGR24, 960, 540 };
	if (sdl_video_player.Open(pixel_spec)) {
		fclose(fp);
		printf("cannot open sdl video player err: %s\n", sdl_video_player.GetLastErr());
		return -1;
	}

	const int width = 960;
	const int height = 540;
	const int BUFSIZE = width * height * 3;
	uint8_t *yuv_buf = NULL;
	yuv_buf = new uint8_t[BUFSIZE];

	while (feof(fp) == 0) {
		memset(yuv_buf, 0, BUFSIZE);
		// 读取一帧数数据到缓冲区
		int ret = fread(yuv_buf, 1, BUFSIZE, fp);
		if (ret > 0) {
			if (sdl_video_player.Show(yuv_buf, ret)) {
				printf("sdl video player show err: %s\n", sdl_video_player.GetLastErr());
				break;
			}

			sdl_video_player.Delay(25);
		}
	}

	sdl_video_player.Close();

	// 关闭文件和释放资源
	fclose(fp);
	SAFE_DEL_A(yuv_buf);

	return 0;
}
