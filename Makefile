# Makefile
#   Copyright (C) 2004 Carlos Justiniano
#   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net
#
# Makefile is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.
# 
# Makefile was developed by Carlos Justiniano for use on the 
# msgsrv project and the ChessBrain Project and is now distributed in
# the hope that it will be useful, but WITHOUT ANY WARRANTY; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.

# You should have received a copy of the GNU General Public License
# along with Makefile; if not, write to the Free Software 
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

###################
# Application Host
###################
APP_HOST = MSGCOURIER

###################
# Operating System
###################
PLATFORM = _PLATFORM_LINUX

#############
# Debugging?
#############
#DEBUGGING=YES
DEBUGGING=NO

ifeq ($(DEBUGGING), YES)
	COMP_FLAGS= -g3 -Wl,-export-dynamic -fexceptions
endif
ifeq ($(DEBUGGING), NO)
	COMP_FLAGS= -O2 -fexceptions -DNDEBUG 
endif

ifeq ($(PLATFORM), _PLATFORM_LINUX)
	CC = g++
	#INCLUDES = -I../uuid-1.0.0 -I../sqlite-3_2_7 -I../tcl8.4.9/generic -I./cpptcl-1.1.1 -I../boost_1_33_0 -I../cryptopp521 -I../sexpr_1.0.1/src -I../botan-1.4.12/build/include
	INCLUDES = -I../uuid-1.0.0 -I../sqlite-3_2_7 -I../tcl8.4.9/generic -I./cpptcl-1.1.1 -I../boost_1_33_0 -I../sexpr_1.0.1/src -I../botan-1.4.12/build/include
	UUID = -L../uuid-1.0.0/.libs -luuid
    BOTAN = -L../botan-1.4.12 -lbotan
	BOOST = -L../boost_1_33_0/libs
	BOOST_REGEX = -L../boost_1_33_0/libs/regex/build/gcc/ -l boost_regex-gcc-1_33
	#CRYPTOPP = -L../cryptopp521/cryptopp -lcryptopp
	TCL = -L../tcl8.4.9/unix -ltcl8.4
	SQLITE = -L../sqlite-3_2_7 -lsqlite-3_2_7
	SEXPR = -L../sexpr_1.0.1/src -lsexp
	CFLAGS = $(COMP_FLAGS) -pipe $(INCLUDES) -D_REENTRANT -D_PLATFORM_LINUX -D$(APP_HOST)
	LIBS = $(UUID) $(TCL) $(BOTAN) $(BOOST) $(BOOST_REGEX) $(SQLITE) $(SEXPR)  
endif

######################
# object modules
######################
OBJS = main.o \
	accesslog.o addonhandlers.o \
	base64.o bench.o buffer.o \
	commandhandlerfactory.o commandhandlerdecorator.o \
	connectioncache.o \
	connectionhandlerinbound.o connectionhandleroutbound.o \
	connectionqueue.o core.o cpptcl.o \
	crc32.o crypto.o \
	date.o dotengine.o \
	exception.o \
	filecache.o \
	headerextract.o ipaccess.o \
	keyvalue.o \
	levenshtein.o log.o \
	macaddr.o mcscript.o mimemap.o msgCourier.o msg.o msgpool.o msgqueue.o msgrouterrulesengine.o msgrouterruleset.o \
	netinterfaces.o \
	onepiximage.o \
	peerroutingmap.o pluginmanager.o presence.o proccom.o \
	routingtag.o \
	service.o sexprwrapper.o scriptengine.o sysinfo.o sysmetrics.o \
	tcpserver.o thread.o threadsync.o timer.o \
	udpserver.o umachid.o uniqueidprovider.o url.o \
	virtualdirectorymap.o webrequesthandler.o webserver.o \
	xmlliteparser.o

main: $(OBJS)
	$(CC) -static -o msgCourier $(OBJS) $(LIBS) -lc -lm -lnsl -lpthread -ldl
	nm msgCourier | sort > msgCourier.nm

main.o: main.cpp 
	$(CC) -c $(CFLAGS) main.cpp

accesslog.o: accesslog.cpp accesslog.h
	$(CC) -c $(CFLAGS) accesslog.cpp

addonhandlers.o: addonhandlers.cpp addonhandlers.h
	$(CC) -c $(CFLAGS) addonhandlers.cpp

base64.o: base64.cpp base64.h
	$(CC) -c $(CFLAGS) base64.cpp
	
bench.o: bench.cpp bench.h
	$(CC) -c $(CFLAGS) bench.cpp

buffer.o: buffer.cpp buffer.h
	$(CC) -c $(CFLAGS) buffer.cpp

dotengine.o: dotengine.cpp dotengine.h
	$(CC) -c $(CFLAGS) dotengine.cpp

commandhandlerfactory.o: commandhandlerfactory.cpp commandhandlerfactory.h
	$(CC) -c $(CFLAGS) commandhandlerfactory.cpp

commandhandlerdecorator.o: commandhandlerdecorator.cpp commandhandlerdecorator.h
	$(CC) -c $(CFLAGS) commandhandlerdecorator.cpp

connectioncache.o: connectioncache.cpp connectioncache.h
	$(CC) -c $(CFLAGS) connectioncache.cpp

connectionhandlerinbound.o: connectionhandlerinbound.cpp connectionhandlerinbound.h connectionhandler.h
	$(CC) -c $(CFLAGS) connectionhandlerinbound.cpp

connectionhandleroutbound.o: connectionhandleroutbound.cpp connectionhandleroutbound.h connectionhandler.h
	$(CC) -c $(CFLAGS) connectionhandleroutbound.cpp

connectionqueue.o: connectionqueue.cpp connectionqueue.h
	$(CC) -c $(CFLAGS) connectionqueue.cpp

core.o: core.cpp core.h master.h
	$(CC) -c $(CFLAGS) core.cpp

cpptcl.o: ./cpptcl-1.1.1/cpptcl/cpptcl.cc ./cpptcl-1.1.1/cpptcl/cpptcl.h
	$(CC) -c $(CFLAGS) ./cpptcl-1.1.1/cpptcl/cpptcl.cc

crc32.o: crc32.cpp crc32.h
	$(CC) -c $(CFLAGS) crc32.cpp
	
crypto.o: crypto.cpp crypto.h
	$(CC) -c $(CFLAGS) crypto.cpp

date.o: date.cpp date.h
	$(CC) -c $(CFLAGS) date.cpp

exception.o: exception.cpp exception.h
	$(CC) -c $(CFLAGS) exception.cpp

filecache.o: filecache.cpp filecache.h
	$(CC) -c $(CFLAGS) filecache.cpp

headerextract.o: headerextract.cpp headerextract.h
	$(CC) -c $(CFLAGS) headerextract.cpp

ipaccess.o: ipaccess.cpp ipaccess.h
	$(CC) -c $(CFLAGS) ipaccess.cpp

keyvalue.o: keyvalue.cpp keyvalue.h
	$(CC) -c $(CFLAGS) keyvalue.cpp

levenshtein.o: levenshtein.cpp levenshtein.h
	$(CC) -c $(CFLAGS) levenshtein.cpp

log.o: log.cpp log.h
	$(CC) -c $(CFLAGS) log.cpp

macaddr.o: macaddr.cpp macaddr.h
	$(CC) -c $(CFLAGS) macaddr.cpp

mcscript.o: mcscript.cpp mcscript.h
	$(CC) -c $(CFLAGS) mcscript.cpp

mimemap.o: mimemap.cpp mimemap.h
	$(CC) -c $(CFLAGS) mimemap.cpp

msgCourier.o: msgCourier.cpp msgCourier.h
	$(CC) -c $(CFLAGS) msgCourier.cpp

msg.o: msg.cpp msg.h
	$(CC) -c $(CFLAGS) msg.cpp

msgpool.o: msgpool.cpp msgpool.h
	$(CC) -c $(CFLAGS) msgpool.cpp

msgqueue.o: msgqueue.cpp msgqueue.h
	$(CC) -c $(CFLAGS) msgqueue.cpp

msgrouterrulesengine.o: msgrouterrulesengine.cpp msgrouterrulesengine.h
	$(CC) -c $(CFLAGS) msgrouterrulesengine.cpp

msgrouterruleset.o: msgrouterruleset.cpp msgrouterruleset.h
	$(CC) -c $(CFLAGS) msgrouterruleset.cpp

netinterfaces.o: netinterfaces.cpp netinterfaces.h
	$(CC) -c $(CFLAGS) netinterfaces.cpp

onepiximage.o: onepiximage.cpp onepiximage.h
	$(CC) -c $(CFLAGS) onepiximage.cpp
	
peerroutingmap.o: peerroutingmap.cpp peerroutingmap.h
	$(CC) -c $(CFLAGS) peerroutingmap.cpp
	
pluginmanager.o: pluginmanager.cpp pluginmanager.h
	$(CC) -c $(CFLAGS) pluginmanager.cpp

presence.o: presence.cpp presence.h
	$(CC) -c $(CFLAGS) presence.cpp

proccom.o: proccom.cpp proccom.h
	$(CC) -c $(CFLAGS) proccom.cpp

routingtag.o: routingtag.cpp routingtag.h
	$(CC) -c $(CFLAGS) routingtag.cpp
	
scriptengine.o: scriptengine.cpp scriptengine.h
	$(CC) -c $(CFLAGS) scriptengine.cpp

service.o: service.cpp service.h
	$(CC) -c $(CFLAGS) service.cpp

sexprwrapper.o: sexprwrapper.cpp sexprwrapper.h
	$(CC) -c $(CFLAGS) sexprwrapper.cpp

sysinfo.o: sysinfo.cpp sysinfo.h
	$(CC) -c $(CFLAGS) sysinfo.cpp

sysmetrics.o: sysmetrics.cpp sysmetrics.h
	$(CC) -c $(CFLAGS) sysmetrics.cpp
	
tcpserver.o: tcpserver.cpp tcpserver.h
	$(CC) -c $(CFLAGS) tcpserver.cpp

thread.o: thread.cpp thread.h
	$(CC) -c $(CFLAGS) thread.cpp

threadsync.o: threadsync.cpp threadsync.h
	$(CC) -c $(CFLAGS) threadsync.cpp

timer.o: timer.cpp timer.h
	$(CC) -c $(CFLAGS) timer.cpp

umachid.o: umachid.cpp umachid.h
	$(CC) -c $(CFLAGS) umachid.cpp

udpserver.o: udpserver.cpp udpserver.h
	$(CC) -c $(CFLAGS) udpserver.cpp

uniqueidprovider.o: uniqueidprovider.cpp uniqueidprovider.h
	$(CC) -c $(CFLAGS) uniqueidprovider.cpp

url.o: url.cpp url.h
	$(CC) -c $(CFLAGS) url.cpp

virtualdirectorymap.o: virtualdirectorymap.cpp virtualdirectorymap.h
	$(CC) -c $(CFLAGS) virtualdirectorymap.cpp

webrequesthandler.o: webrequesthandler.cpp webrequesthandler.h
	$(CC) -c $(CFLAGS) webrequesthandler.cpp

webserver.o: webserver.cpp webserver.h
	$(CC) -c $(CFLAGS) webserver.cpp

xmlliteparser.o: xmlliteparser.cpp xmlliteparser.h
	$(CC) -c $(CFLAGS) xmlliteparser.cpp	

install:
	scp msgCourier msgCourier.nm root@localhost:/usr/local/msgCourier

deploy:
	scp msgCourier root@falcon:/usr/local/msgCourier
	scp msgCourier.nm root@falcon:/usr/local/msgCourier

clean:
	@rm -f msgCourier msgCourier.nm msgCourier.pid core *.o

docs:
	@doxygen doxygen.conf

