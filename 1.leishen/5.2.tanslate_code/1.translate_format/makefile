CC=g++
CFLAGS2+= -lavfilter -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL2  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

all:1.ffmpeg_mp4_to_flv \
	2.ffmpeg_simple_demuxer_flv_to_h264_and_mp3 \
	3.ffmpeg_demuxer_mpeg2ts_to_h264_and_aac \
	4.ffmpeg_muxer_h264_and_aac_to_mpeg2ts

1.ffmpeg_mp4_to_flv: 1.ffmpeg_mp4_to_flv.cpp
	$(CC) 1.ffmpeg_mp4_to_flv.cpp -o $@ $(CFLAGS2)

2.ffmpeg_simple_demuxer_flv_to_h264_and_mp3: 2.ffmpeg_simple_demuxer_flv_to_h264_and_mp3.cpp
	$(CC) 2.ffmpeg_simple_demuxer_flv_to_h264_and_mp3.cpp -o $@ $(CFLAGS2)

3.ffmpeg_demuxer_mpeg2ts_to_h264_and_aac:
	$(CC) 3.ffmpeg_demuxer_mpeg2ts_to_h264_and_aac.cpp -o $@ $(CFLAGS2)

4.ffmpeg_muxer_h264_and_aac_to_mpeg2ts: 4.ffmpeg_muxer_h264_and_aac_to_mpeg2ts.cpp
	$(CC) 4.ffmpeg_muxer_h264_and_aac_to_mpeg2ts.cpp -o $@ $(CFLAGS2)

clean:
	rm -rf 1.ffmpeg_mp4_to_flv
	rm -rf 2.ffmpeg_simple_demuxer_flv_to_h264_and_mp3
	rm -rf 3.ffmpeg_demuxer_mpeg2ts_to_h264_and_aac
	rm -rf 4.ffmpeg_muxer_h264_and_aac_to_mpeg2ts
	rm -rf h288_w512.flv 2.mp3 2.h264 3.aac 3.h264
	rm -rf 4.mp4

