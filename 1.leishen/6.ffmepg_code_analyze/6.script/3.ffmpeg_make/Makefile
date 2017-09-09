# FFmpeg Main Makefile
#
# 注释：雷霄骅
# leixiaohua1020@126.com
# http://blog.csdn.net/leixiaohua1020
#
# FFmpeg的 Main Makefile。最重要。

MAIN_MAKEFILE=1
#重要：包含了configure信息
include config.mak

#config.mak中：
#SRC_PATH=.
#在SRC_PATH搜索各种类型的文件
vpath %.c    $(SRC_PATH)
vpath %.cpp  $(SRC_PATH)
vpath %.h    $(SRC_PATH)
vpath %.S    $(SRC_PATH)
vpath %.asm  $(SRC_PATH)
vpath %.v    $(SRC_PATH)
vpath %.texi $(SRC_PATH)
vpath %/fate_config.sh.template $(SRC_PATH)

#CONFIG_XXX取值为yes
#PROGS-yes= ffmpeg ffplay ffprobe
PROGS-$(CONFIG_FFMPEG)   += ffmpeg
PROGS-$(CONFIG_FFPLAY)   += ffplay
PROGS-$(CONFIG_FFPROBE)  += ffprobe
PROGS-$(CONFIG_FFSERVER) += ffserver

#config.mak中：
#EXESUF=.exe
#PROGSSUF=
#$(var:%.c=%.o)意思是把.c 为结尾的变量替换成.o。
#没有“.c”的时候，代表匹配所有
PROGS      := $(PROGS-yes:%=%$(EXESUF))
INSTPROGS   = $(PROGS-yes:%=%$(PROGSSUF)$(EXESUF))
OBJS        = $(PROGS-yes:%=%.o) cmdutils.o
TESTTOOLS   = audiogen videogen rotozoom tiny_psnr base64
HOSTPROGS  := $(TESTTOOLS:%=tests/%)
TOOLS       = qt-faststart trasher
TOOLS-$(CONFIG_ZLIB) += cws2fws

#PROGS= ffmpeg.exe ffplay.exe ffprobe.exe
#INSTPROGS= ffmpeg.exe ffplay.exe ffprobe.exe
#OBJS= ffmpeg.o ffplay.o ffprobe.o

BASENAMES   = ffmpeg ffplay ffprobe ffserver
ALLPROGS    = $(BASENAMES:%=%$(PROGSSUF)$(EXESUF))
ALLPROGS_G  = $(BASENAMES:%=%$(PROGSSUF)_g$(EXESUF))
ALLMANPAGES = $(BASENAMES:%=%.1)

#ALLPROGS= ffmpeg.exe ffplay.exe ffprobe.exe ffserver.exe
#ALLPROGS_G= ffmpeg_g.exe ffplay_g.exe ffprobe_g.exe ffserver_g.exe
#ALLMANPAGES=ffmpeg.1 ffplay.1 ffprobe.1 ffserver.1
FFLIBS-$(CONFIG_AVDEVICE) += avdevice
FFLIBS-$(CONFIG_AVFILTER) += avfilter
FFLIBS-$(CONFIG_AVFORMAT) += avformat
FFLIBS-$(CONFIG_AVCODEC)  += avcodec
FFLIBS-$(CONFIG_POSTPROC) += postproc
FFLIBS-$(CONFIG_SWRESAMPLE)+= swresample
FFLIBS-$(CONFIG_SWSCALE)  += swscale

#FFLIBS-yes= avdevice avfilter avformat avcodec postproc swresample swscale
#一定需要libavutil
FFLIBS := avutil
#让通配符在变量中展开，需要使用wildcard关键字
DATA_FILES := $(wildcard $(SRC_PATH)/presets/*.ffpreset) $(SRC_PATH)/doc/ffprobe.xsd

SKIPHEADERS = cmdutils_common_opts.h
#重要
include $(SRC_PATH)/common.mak
#依赖的类库
FF_EXTRALIBS := $(FFEXTRALIBS)
FF_DEP_LIBS  := $(DEP_LIBS)

#伪目标
#all是最关键的，生成最后的程序
#all: ffmpeg.exe ffplay.exe ffprobe.exe
#
all: $(PROGS)
#config.mak中：
#EXESUF=.exe
#PROGSSUF=
#$@是一个自动化变量。可以简单理解为目标的集合。
#$<是一个自动化变量。可以简单理解为依赖目标的集合。
#%是通配符
#两个冒号，“静态模式规则”。
#ffmpeg_g.exe生成ffmpeg.exe；ffplay_g.exe生成ffplay.exe；ffprobe_g.exe生成ffprobe.exe
#strip经常用来去除目标文件中的一些符号表、调试符号表信息,以减小程序的大小
$(PROGS): %$(EXESUF): %$(PROGSSUF)_g$(EXESUF)
    $(CP) $< $@$(PROGSSUF)
    $(STRIP) $@$(PROGSSUF)

$(TOOLS): %$(EXESUF): %.o
    $(LD) $(LDFLAGS) -o $@ $< $(ELIBS)

tools/cws2fws$(EXESUF): ELIBS = -lz

config.h: .config
.config: $(wildcard $(FFLIBS:%=$(SRC_PATH)/lib%/all*.c))
    @-tput bold 2>/dev/null
    @-printf '\nWARNING: $(?F) newer than config.h, rerun configure\n\n'
    @-tput sgr0 2>/dev/null
#给子目录中的Makefile使用的变量
SUBDIR_VARS := OBJS FFLIBS CLEANFILES DIRS TESTPROGS EXAMPLES SKIPHEADERS \
               ALTIVEC-OBJS MMX-OBJS NEON-OBJS X86-OBJS YASM-OBJS-FFT YASM-OBJS \
               HOSTPROGS BUILT_HEADERS TESTOBJS ARCH_HEADERS ARMV6-OBJS TOOLS

define RESET
$(1) :=
$(1)-yes :=
endef

#$(call <expression>,<parm1>,<parm2>,<parm3>...)
#当make执行这个函数时，<expression>参数中的变量，如$(1)，$(2)，$(3)等，会被参数
#<parm1>，<parm2>，<parm3>依次取代。而<expression>的返回值就是call函数的返回值。

#命令包===========================
#用于编译每个库
#$(1)取值为libavcodec，libavcodec等等
define DOSUBDIR
$(foreach V,$(SUBDIR_VARS),$(eval $(call RESET,$(V))))
SUBDIR := $(1)/
#每个库目录下的Makefile
include $(SRC_PATH)/$(1)/Makefile
#注：make一般情况下如果在中途检测到有执行出错的情况(返回非 0 状态)，那么就会放弃对当前规则后续命令的执行。
#在命令前面加上“-”号之后，就算执行错误了，也会继续执行下去
-include $(SRC_PATH)/$(1)/$(ARCH)/Makefile
#编译类库
include $(SRC_PATH)/library.mak
endef
#=================================

#$(foreach <var>,<list>,<text>)
#把参数<list>中的单词逐一取出放到参数<var>所指定的变量中，
#然后再执行<text>所包含的表达式。
#$(eval text)
#text的内容将作为makefile的一部分而被make解析和执行
#
#循环调用DOSUBDIR命令包
#这一步会将libavcodec，libavformat等文件夹下的Makefile包含进来。
$(foreach D,$(FFLIBS),$(eval $(call DOSUBDIR,lib$(D))))
#ffplay需要SDL
ffplay.o: CFLAGS += $(SDL_CFLAGS)
ffplay_g$(EXESUF): FF_EXTRALIBS += $(SDL_LIBS)
ffserver_g$(EXESUF): LDFLAGS += $(FFSERVERLDFLAGS)
#链接生成ffmpeg_g.exe等等
#FF_DEP_LIBS= libavcodec/libavcodec.a libavutil/libavutil.a ....
%$(PROGSSUF)_g$(EXESUF): %.o cmdutils.o $(FF_DEP_LIBS)
    $(LD) $(LDFLAGS) -o $@ $< cmdutils.o $(FF_EXTRALIBS)

OBJDIRS += tools

-include $(wildcard tools/*.d)

VERSION_SH  = $(SRC_PATH)/version.sh
GIT_LOG     = $(SRC_PATH)/.git/logs/HEAD

.version: $(wildcard $(GIT_LOG)) $(VERSION_SH) config.mak
.version: M=@

version.h .version:
	$(M)$(VERSION_SH) $(SRC_PATH) version.h $(EXTRA_VERSION)
	$(Q)touch .version

# force version.sh to run whenever version might have changed
-include .version
#安装install
#安装程序
ifdef PROGS
install: install-progs install-data
endif
#安装类库和头文件
install: install-libs install-headers
#install-libs-yes位于library.mak
install-libs: install-libs-yes

install-progs-yes:
install-progs-$(CONFIG_SHARED): install-libs

#config.mak中：
#BINDIR=$(DESTDIR)${prefix}/bin
#INSTALL=install
#cp与install区别：
#cp会先清空文件后往里写入新文件，而install则会先删除掉原先的文件然后写入新文件。
install-progs: install-progs-yes $(PROGS)
    $(Q)mkdir -p "$(BINDIR)"
    $(INSTALL) -c -m 755 $(INSTPROGS) "$(BINDIR)"

install-data: $(DATA_FILES)
    $(Q)mkdir -p "$(DATADIR)"
    $(INSTALL) -m 644 $(DATA_FILES) "$(DATADIR)"
#卸载
uninstall: uninstall-libs uninstall-headers uninstall-progs uninstall-data
#addprefix()用于加前缀
#在这里获取ffmpeg.exe等的完整路径（用于删除）
uninstall-progs:
	$(RM) $(addprefix "$(BINDIR)/", $(ALLPROGS))

uninstall-data:
	$(RM) -r "$(DATADIR)"
#清空
clean::
	$(RM) $(ALLPROGS) $(ALLPROGS_G)
	$(RM) $(CLEANSUFFIXES)
	$(RM) $(TOOLS)
	$(RM) $(CLEANSUFFIXES:%=tools/%)
	$(RM) coverage.info
	$(RM) -r coverage-html

distclean::
	$(RM) $(DISTCLEANSUFFIXES)
	$(RM) config.* .version version.h libavutil/avconfig.h

config:
	$(SRC_PATH)/configure $(value FFMPEG_CONFIGURATION)

# Without the sed genthml thinks "libavutil" and "./libavutil" are two different things
coverage.info: $(wildcard *.gcda *.gcno */*.gcda */*.gcno */*/*.gcda */*/*.gcno)
    $(Q)lcov -c -d . -b . | sed -e 's#/./#/#g' > $@

coverage-html: coverage.info
    $(Q)mkdir -p $@
    $(Q)genhtml -o $@ $<
    $(Q)touch $@

include $(SRC_PATH)/doc/Makefile
include $(SRC_PATH)/tests/Makefile

$(sort $(OBJDIRS)):
	$(Q)mkdir -p $@

# Dummy rule to stop make trying to rebuild removed or renamed headers
%.h:
	@:

# Disable suffix rules.  Most of the builtin rules are suffix rules,
# so this saves some time on slow systems.
.SUFFIXES:
#显示地指明一个目标是“伪目标”
.PHONY: all all-yes alltools *clean config examples install*
.PHONY: testprogs uninstall*

