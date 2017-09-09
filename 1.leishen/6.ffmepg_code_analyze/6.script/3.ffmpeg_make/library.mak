# FFmpeg library.mak
#
# 注释：雷霄骅
# leixiaohua1020@126.com
# http://blog.csdn.net/leixiaohua1020
#
# 编译类库(libavformat等)专用的Makefile，其中包含了编译类库的规则。

#【NAME位于每个类库的Makefile】，可以取avcodec，avformat等等
SRC_DIR := $(SRC_PATH)/lib$(NAME)

include $(SRC_PATH)/common.mak

#这些信息都位于config.mak中
#例如：
# libavformat_VERSION=53.31.100
# libavformat_VERSION_MAJOR=53

LIBVERSION := $(lib$(NAME)_VERSION)
LIBMAJOR   := $(lib$(NAME)_VERSION_MAJOR)
INCINSTDIR := $(INCDIR)/lib$(NAME)
THIS_LIB   := $(SUBDIR)$($(CONFIG_SHARED:yes=S)LIBNAME)

all-$(CONFIG_STATIC): $(SUBDIR)$(LIBNAME)
all-$(CONFIG_SHARED): $(SUBDIR)$(SLIBNAME)

$(SUBDIR)%-test.o: $(SUBDIR)%-test.c
    $(COMPILE_C)

$(SUBDIR)%-test.o: $(SUBDIR)%.c
    $(COMPILE_C)
#汇编？
$(SUBDIR)x86/%.o: $(SUBDIR)x86/%.asm
    $(YASMDEP) $(YASMFLAGS) -I $(<D)/ -M -o $@ $< > $(@:.o=.d)
    $(YASM) $(YASMFLAGS) -I $(<D)/ -o $@ $<

$(OBJS) $(OBJS:.o=.s) $(SUBDIR)%.ho $(TESTOBJS): CPPFLAGS += -DHAVE_AV_CONFIG_H
$(TESTOBJS): CPPFLAGS += -DTEST

#【OBJS来自于每个类库的Makefile】
#$@  表示规则中的目标文件集
#$^  所有的依赖目标的集合。
#生成静态库？
$(SUBDIR)$(LIBNAME): $(OBJS)
    $(RM) $@
    $(AR) rc $@ $^ $(EXTRAOBJS)
    $(RANLIB) $@
#安转头文件，根目录的Makefile调用
install-headers: install-lib$(NAME)-headers install-lib$(NAME)-pkgconfig
#install-libs-yes被install-libs（位于根目录Makefile）调用
install-libs-$(CONFIG_STATIC): install-lib$(NAME)-static
install-libs-$(CONFIG_SHARED): install-lib$(NAME)-shared

define RULES
$(EXAMPLES) $(TESTPROGS) $(TOOLS): %$(EXESUF): %.o

(LD)$(LDFLAGS)−o
@
−l$(FULLNAME)$(FFEXTRALIBS)
(ELIBS)

$(SUBDIR)$(SLIBNAME): $(SUBDIR)$(SLIBNAME_WITH_MAJOR)
    $(Q)cd ./$(SUBDIR) && $(LN_S) $(SLIBNAME_WITH_MAJOR) $(SLIBNAME)

$(SUBDIR)$(SLIBNAME_WITH_MAJOR): $(OBJS) $(SUBDIR)lib$(NAME).ver
    $(SLIB_CREATE_DEF_CMD)

(LD)$(SHFLAGS)$(LDFLAGS)−o
@
(filter
^) $(FFEXTRALIBS) $(EXTRAOBJS)
    $(SLIB_EXTRA_CMD)

#SLIBNAME_WITH_MAJOR包含了Major版本号。例如：libavformat-53.dll
ifdef SUBDIR
$(SUBDIR)$(SLIBNAME_WITH_MAJOR): $(DEP_LIBS)
endif
#清空
clean::
    $(RM) $(addprefix $(SUBDIR),*-example$(EXESUF) *-test$(EXESUF) $(CLEANFILES) $(CLEANSUFFIXES) $(LIBSUFFIXES)) \
        $(foreach dir,$(DIRS),$(CLEANSUFFIXES:%=$(SUBDIR)$(dir)/%)) \
        $(HOSTOBJS) $(HOSTPROGS)

distclean:: clean
    $(RM) $(DISTCLEANSUFFIXES:%=$(SUBDIR)%) \
        $(foreach dir,$(DIRS),$(DISTCLEANSUFFIXES:%=$(SUBDIR)$(dir)/%))
#安装库文件=====================
install-lib$(NAME)-shared: $(SUBDIR)$(SLIBNAME)
    $(Q)mkdir -p "$(SHLIBDIR)"
    $$(INSTALL) -m 755 $$< "$(SHLIBDIR)/$(SLIB_INSTALL_NAME)"
    $$(STRIP) "$(SHLIBDIR)/$(SLIB_INSTALL_NAME)"
    $(Q)$(foreach F,$(SLIB_INSTALL_LINKS),cd "$(SHLIBDIR)" && $(LN_S) $(SLIB_INSTALL_NAME) $(F);)
    $(if $(SLIB_INSTALL_EXTRA_SHLIB),$$(INSTALL) -m 644 $(SLIB_INSTALL_EXTRA_SHLIB:%=$(SUBDIR)%) "$(SHLIBDIR)")
    $(if $(SLIB_INSTALL_EXTRA_LIB),$(Q)mkdir -p "$(LIBDIR)")
    $(if $(SLIB_INSTALL_EXTRA_LIB),$$(INSTALL) -m 644 $(SLIB_INSTALL_EXTRA_LIB:%=$(SUBDIR)%) "$(LIBDIR)")

install-lib$(NAME)-static: $(SUBDIR)$(LIBNAME)
    $(Q)mkdir -p "$(LIBDIR)"
    $$(INSTALL) -m 644 $$< "$(LIBDIR)"
    $(LIB_INSTALL_EXTRA_CMD)
#安装头文件=====================
#-m
#权限：644,755,777
#644 rw-r--r--
#755 rwxr-xr-x
#777 rwxrwxrwx
#从左至右，1-3位数字代表文件所有者的权限，4-6位数字代表同组用户的权限，7-9数字代表其他用户的权限。
#通过4、2、1的组合，得到以下几种权限：0（没有权限）；4（读取权限）；5（4+1 | 读取+执行）；6（4+2 | 读取+写入）；7（4+2+1 | 读取+写入+执行）
#addprefix()
#$(addprefix src/,foo bar)返回值是“src/foo src/bar”。

#【HEADERS来自于每个类库的Makefile】
#例如libavformat中HEADERS = avformat.h avio.h version.h
install-lib$(NAME)-headers: $(addprefix $(SUBDIR),$(HEADERS) $(BUILT_HEADERS))
    $(Q)mkdir -p "$(INCINSTDIR)"
    $$(INSTALL) -m 644 $$^ "$(INCINSTDIR)"

install-lib$(NAME)-pkgconfig: $(SUBDIR)lib$(NAME).pc
    $(Q)mkdir -p "$(LIBDIR)/pkgconfig"
    $$(INSTALL) -m 644 $$^ "$(LIBDIR)/pkgconfig"

#卸载
uninstall-libs::
    -$(RM) "$(SHLIBDIR)/$(SLIBNAME_WITH_MAJOR)" \
           "$(SHLIBDIR)/$(SLIBNAME)"            \
           "$(SHLIBDIR)/$(SLIBNAME_WITH_VERSION)"
    -$(RM) $(SLIB_INSTALL_EXTRA_SHLIB:%="$(SHLIBDIR)"%)
    -$(RM) $(SLIB_INSTALL_EXTRA_LIB:%="$(LIBDIR)"%)
    -$(RM) "$(LIBDIR)/$(LIBNAME)"

uninstall-headers::
    $(RM) $(addprefix "$(INCINSTDIR)/",$(HEADERS)) $(addprefix "$(INCINSTDIR)/",$(BUILT_HEADERS))
    $(RM) "$(LIBDIR)/pkgconfig/lib$(NAME).pc"
    -rmdir "$(INCINSTDIR)"
endef

$(eval $(RULES))

$(EXAMPLES) $(TESTPROGS) $(TOOLS): $(THIS_LIB) $(DEP_LIBS)
$(TESTPROGS): $(SUBDIR)$(LIBNAME)

examples: $(EXAMPLES)
testprogs: $(TESTPROGS)
