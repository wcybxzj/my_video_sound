#gcc -o 1.test 1.test.c -lavutil -lavformat -lavcodec -lz -lavutil -lm -lrt -liconv -ldl

CC=gcc
CFLAGS1+=  -lavfilter  -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

CFLAGS2+= -lavfilter -lavformat -lavcodec -lavutil  -lavdevice -lpostproc  -lswresample -lswscale  -lSDL2  -lm -lz -lpthread -ldl -lrt -liconv -lmp3lame -lx264 -lx265 -lfdk-aac -llzma -lbz2 -lnuma

all:1.create_test_pic

1.create_test_pic:1.create_test_pic.c
	$(CC) 1.create_test_pic.c  -o $@ $(CFLAGS1)

clean:
	$(RM) -r a.out 1.create_test_pic allcolor_xr_yg_zb_256x256_rgb24.rgb allcolor_xu_yv_zy_256x256_yuv444p.yuv colorbar_1280x720_rgb24.bmp colorbar_1280x720_rgb24.rgb graybar_1280x720_yuv420p.yuv rgbgradientbar_1280x720_rgb24.rgb rgbstripe_1280x720_rgb24.rgb yuvgradientbar_1280x720_yuv420p.yuv

