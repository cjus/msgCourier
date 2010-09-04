/* pluginmanager.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

pluginmanager.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

pluginmanager.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with pluginmanager.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file pluginmanager.h
 @brief External plugin manager
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <string>
#include <map>
#include "threadsync.h"
#include "msgCourierPlugin.h"
#include "commandhandlerfactory.h"
#include "msgqueue.h"
#include "service.h"
#include "isysinfo.h"
#include "ilogger.h"

typedef void* PLUGINHANDLE;

class cPlugin
{
public:
	PLUGINHANDLE libraryHandle;
	cICommandHandler* handler;
	bool localAccess;
	bool remoteAccess;
};

/**
 @class cPluginManager
 @brief External plugin manager
*/
class cPluginManager
{
public:
	cPluginManager(cCommandHandlerFactory *pFactory, cService *pService, cMsgQueue *pMsgQueue, cISysInfo *pSysInfo, cILogger *pLogger);
	virtual ~cPluginManager();

	void SetPluginDirectory(const char *pDirectory);
	void LoadPlugin(const char *pPluginName, std::string &localAccess, std::string &remoteAccess);
private:
	cCommandHandlerFactory *m_pCommandHandlerFactory;
	cMsgQueue *m_pMsgQueue;
	cISysInfo *m_pSysInfo;
	cILogger  *m_pLogger;
	cThreadSync m_ThreadSync;
	cService *m_pService;

	std::string m_PluginSourceDirectory;
	std::map<std::string,cPlugin*> m_Map;
};

#endif // PLUGIN_MANAGER_H

