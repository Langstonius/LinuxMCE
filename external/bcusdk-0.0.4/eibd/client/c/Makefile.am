
HEADER=eibclient-int.h
NATIVE=close.c  closesync.c  complete.c  io.c  openlocal.c  openremote.c  openurl.c  pollcomplete.c  pollfd.c

FUNCS= \
  gen/getapdu.c              gen/loadimage.c         gen/mcpropertyread.c   gen/mprogmodeoff.c              gen/opentconnection.c \
  gen/getapdusrc.c           gen/mcauthorize.c       gen/mcpropertyscan.c   gen/mprogmodeon.c               gen/opentgroup.c \
  gen/getbusmonitorpacket.c  gen/mcconnect.c         gen/mcpropertywrite.c  gen/mprogmodestatus.c           gen/opentindividual.c \
  gen/getgroupsrc.c          gen/mcgetmaskversion.c  gen/mcreadadc.c        gen/mprogmodetoggle.c           gen/openttpdu.c \
  gen/groupcacheclear.c      gen/mcgetpeitype.c      gen/mcread.c           gen/mreadindividualaddresses.c  gen/openvbusmonitor.c \
  gen/groupcachedisable.c    gen/mcprogmodeoff.c     gen/mcrestart.c        gen/mwriteindividualaddress.c   gen/openvbusmonitortext.c \
  gen/groupcacheenable.c     gen/mcprogmodeon.c      gen/mcsetkey.c         gen/openbusmonitor.c            gen/reset.c \
  gen/groupcacheread.c       gen/mcprogmodestatus.c  gen/mcwrite.c          gen/openbusmonitortext.c        gen/sendapdu.c \
  gen/groupcachereadsync.c   gen/mcprogmodetoggle.c  gen/mcwriteplain.c     gen/opengroupsocket.c           gen/sendgroup.c \
  gen/groupcacheremove.c     gen/mcpropertydesc.c    gen/mgetmaskversion.c  gen/opentbroadcast.c            gen/sendtpdu.c \
  gen/gettpdu.c

BUILT_SOURCES=$(FUNCS)
CLEANFILES=$(FUNCS)

gen/%.c : Makefile
	mkdir -p gen
	echo '#include "c/eibclient-int.h"' > $@ 
	echo '#include "def/'`basename $@|sed -e "s/\.c/.inc/g"`'"' >> $@

lib_LIBRARIES=libeibclient.a
AM_CPPFLAGS=-I$(top_srcdir)/eibd/include -I$(top_srcdir)/common -I$(top_srcdir)/eibd/client
libeibclient_a_SOURCES= $(NATIVE) $(FUNCS) $(HEADER)
