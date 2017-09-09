# FFmpeg common.mak
#
# 注释：雷霄骅
# leixiaohua1020@126.com
# http://blog.csdn.net/leixiaohua1020
#
# 通用的Makefile，其中包含了通用的编译规则。
#
# common bits used by all libraries
#

# first so "all" becomes default target
all: all-yes

ifndef SUBDIR
#在控制台打印信息
ifndef V
Q      = @
#输出
ECHO   = printf "$(1)\t%s\n" $(2)
BRIEF  = CC CXX AS YASM AR LD HOSTCC STRIP CP
SILENT = DEPCC YASMDEP RM RANLIB
MSG    = $@
M      = @$(call ECHO,$(TAG),$@);
$(foreach VAR,$(BRIEF), \
    $(eval override $(VAR) = @
(callECHO,$(VAR),
(MSG)); $($(VAR))))
$(foreach VAR,$(SILENT),$(eval override $(VAR) = @$($(VAR))))
$(eval INSTALL = @$(call ECHO,INSTALL,$$(^:$(SRC_DIR)/%=%)); $(INSTALL))
endif
#所有的lib
ALLFFLIBS = avcodec avdevice avfilter avformat avutil postproc swscale swresample

# NASM requires -I path terminated with /
#各种Flag
#SRC_PATH=.
IFLAGS     := -I. -I$(SRC_PATH)/
CPPFLAGS   := $(IFLAGS) $(CPPFLAGS)
CFLAGS     += $(ECFLAGS)
CCFLAGS     = $(CFLAGS)
CXXFLAGS   := $(CFLAGS) $(CXXFLAGS)
YASMFLAGS  += $(IFLAGS) -I$(SRC_PATH)/libavutil/x86/ -Pconfig.asm
HOSTCFLAGS += $(IFLAGS)
#avcodec处理后成为-Llibavcodec
#config.mak文件中：
#LDFLAGS= -Wl,--as-needed -Wl,--warn-common -Wl,
#-rpath-link=libpostproc:libswresample:libswscale:libavfilter:libavdevice:libavformat:libavcodec:libavutil
LDFLAGS    := $(ALLFFLIBS:%=-Llib%) $(LDFLAGS)

#命令包
#具体编译命令
#
#$(1)可以取CC、CXX等
#例如取$(1)取CC
#config.mak文件中：
#SRC_PATH=.
#CC=gcc
#
#CCFLAGS=$(CFLAGS)
#CFLAGS=   -std=c99 -fno-common -fomit-frame-pointer -I/include/SDL -D_GNU_SOURCE=1 -Dmain=SDL_main
# -g -Wdeclaration-after-statement -Wall -Wno-parentheses -Wno-switch -Wno-format-zero-length
# -Wdisabled-optimization -Wpointer-arith -Wredundant-decls -Wno-pointer-sign -Wcast-qual -Wwrite-strings
# -Wtype-limits -Wundef -Wmissing-prototypes -Wno-pointer-to-int-cast -Wstrict-prototypes
# -O3 -fno-math-errno -fno-signed-zeros -fno-tree-vectorize -Werror=implicit-function-declaration -Werror=missing-prototypes
#
#CPPFLAGS= -D_ISOC99_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -U__STRICT_ANSI__
#CC_O=-o $@
#CC_DEPFLAGS=-MMD -MF $(@:.o=.d) -MT $@
#举例：
#gcc -I. -Itest/ -c -o $@ $<
#再例如$(1)取CXX
#CXXFLAGS=  -D__STDC_CONSTANT_MACROS

define COMPILE
       $($(1)DEP)
       $($(1)) $(CPPFLAGS) $($(1)FLAGS) $($(1)_DEPFLAGS) -c $($(1)_O) $<
endef

#编译命令
#$(call <expression>,<parm1>,<parm2>,<parm3>...)
#当make执行这个函数时，<expression>参数中的变量，如$(1)，$(2)，$(3)等，会被参数
#<parm1>，<parm2>，<parm3>依次取代。而<expression>的返回值就是call函数的返回值。
COMPILE_C = $(call COMPILE,CC)
COMPILE_CXX = $(call COMPILE,CXX)
COMPILE_S = $(call COMPILE,AS)

#COMPILE_C为：
#$(CC DEP)
#$($(CC) $(CPPFLAGS) $($(1)FLAGS) $($(1)_DEPFLAGS) -c $($(1)_O) $<

#依赖关系
#C语言
%.o: %.c
#编译
    $(COMPILE_C)

#C++
%.o: %.cpp
    $(COMPILE_CXX)

%.s: %.c
    $(CC) $(CPPFLAGS) $(CFLAGS) -S -o $@ $<

%.o: %.S
    $(COMPILE_S)

%.ho: %.h
    $(CC) $(CPPFLAGS) $(CFLAGS) -Wno-unused -c -o $@ -x c $<

%.ver: %.v
    $(Q)sed 's/$$MAJOR/$($(basename $(@F))_VERSION_MAJOR)/' $^ > $@

%.c %.h: TAG = GEN

# Dummy rule to stop make trying to rebuild removed or renamed headers
%.h:
	@:

# Disable suffix rules.  Most of the builtin rules are suffix rules,
# so this saves some time on slow systems.
.SUFFIXES:

# Do not delete intermediate files from chains of implicit rules
$(OBJS):
endif

OBJS-$(HAVE_MMX) +=  $(MMX-OBJS-yes)

#源自Makefile
#OBJS：该类库必须的目标文件
#OBJS-yes：该类库可配置的目标文件
OBJS      += $(OBJS-yes)
#FFLIBS：必须的类库
#FFLIBS-yes：可选的类库
#FFLIBS = avcodec avutil ....
FFLIBS    := $(FFLIBS-yes) $(FFLIBS)
TESTPROGS += $(TESTPROGS-yes)

FFEXTRALIBS := $(FFLIBS:%=-l%$(BUILDSUF)) $(EXTRALIBS)

EXAMPLES  := $(EXAMPLES:%=$(SUBDIR)%-example$(EXESUF))
#排序？
OBJS      := $(sort $(OBJS:%=$(SUBDIR)%))
TESTOBJS  := $(TESTOBJS:%=$(SUBDIR)%) $(TESTPROGS:%=$(SUBDIR)%-test.o)
TESTPROGS := $(TESTPROGS:%=$(SUBDIR)%-test$(EXESUF))
HOSTOBJS  := $(HOSTPROGS:%=$(SUBDIR)%.o)
HOSTPROGS := $(HOSTPROGS:%=$(SUBDIR)%$(HOSTEXESUF))
TOOLS     += $(TOOLS-yes)
TOOLOBJS  := $(TOOLS:%=tools/%.o)
TOOLS     := $(TOOLS:%=tools/%$(EXESUF))

#DEP_LIBS= libavcodec/libavcodec.a libavutil/libavutil.a ....
DEP_LIBS := $(foreach NAME,$(FFLIBS),lib$(NAME)/$($(CONFIG_SHARED:yes=S)LIBNAME))

ALLHEADERS := $(subst $(SRC_DIR)/,$(SUBDIR),$(wildcard $(SRC_DIR)/*.h $(SRC_DIR)/$(ARCH)/*.h))
SKIPHEADERS += $(ARCH_HEADERS:%=$(ARCH)/%) $(SKIPHEADERS-)
SKIPHEADERS := $(SKIPHEADERS:%=$(SUBDIR)%)
checkheaders: $(filter-out $(SKIPHEADERS:.h=.ho),$(ALLHEADERS:.h=.ho))

alltools: $(TOOLS)

$(HOSTOBJS): %.o: %.c
    $(HOSTCC) $(HOSTCFLAGS) -c -o $@ $<

$(HOSTPROGS): %$(HOSTEXESUF): %.o
    $(HOSTCC) $(HOSTLDFLAGS) -o $@ $< $(HOSTLIBS)

$(OBJS):     | $(sort $(dir $(OBJS)))
$(HOSTOBJS): | $(sort $(dir $(HOSTOBJS)))
$(TESTOBJS): | $(sort $(dir $(TESTOBJS)))
$(TOOLOBJS): | tools

OBJDIRS := $(OBJDIRS) $(dir $(OBJS) $(HOSTOBJS) $(TESTOBJS))

CLEANSUFFIXES     = *.d *.o *~ *.ho *.map *.ver *.gcno *.gcda
DISTCLEANSUFFIXES = *.pc
LIBSUFFIXES       = *.a *.lib *.so *.so.* *.dylib *.dll *.def *.dll.a *.exp

#依赖文件.d（dependence）
-include $(wildcard $(OBJS:.o=.d) $(TESTOBJS:.o=.d))  :w

