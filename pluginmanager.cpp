/* pluginmanager.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

pluginmanager.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

pluginmanager.cpp was developed by Carlos Justiniano for use on the
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
 @file pluginmanager.cpp
 @brief External plugin manager
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include "exception.h"
#include "log.h"
#include "pluginmanager.h"

#ifdef _PLATFORM_LINUX
	#define _GNU_SOURCE 1
	#include <stdio.h>
	#include <sys/types.h>
	#include <dirent.h>
	#include <errno.h>
	#include <unistd.h>
	#include <dlfcn.h>
#endif //_PLATFORM_LINUX

using namespace std;

#ifdef _PLATFORM_WIN32
	#define MSGHANDLER_API __declspec(dllimport)
#endif //_PLATFORM_WIN32

#ifdef _PLATFORM_LINUX
	#define MSGHANDLER_API
#endif //_PLATFORM_LINUX

typedef MSGHANDLER_API void (*CreateProcAddr)(cICommandHandler **);
typedef MSGHANDLER_API void (*DestroyProcAddr)(cICommandHandler **);

cPluginManager::cPluginManager(cCommandHandlerFactory *pFactory, cService *pService, cMsgQueue *pMsgQueue, cISysInfo *pSysInfo, cILogger *pLogger)
: m_pCommandHandlerFactory(pFactory)
, m_pMsgQueue(pMsgQueue)
, m_pService(pService)
, m_pSysInfo(pSysInfo)
, m_pLogger(pLogger)
{
}

cPluginManager::~cPluginManager()
{
	try
	{	
		cPlugin *pPlugin;
		map<string,cPlugin*>::iterator mapIterator;
		for (mapIterator = m_Map.begin(); mapIterator != m_Map.end(); mapIterator++)
		{
			pPlugin = (*mapIterator).second;
			pPlugin->handler->OnShutdown();
		#ifdef _PLATFORM_WIN32
			DestroyProcAddr DestroyProc = (DestroyProcAddr)GetProcAddress((HINSTANCE)pPlugin->libraryHandle, "Destroy");
			DestroyProc(&pPlugin->handler);
			FreeLibrary((HMODULE)pPlugin->libraryHandle);
		#endif //_PLATFORM_WIN32
		#ifdef _PLATFORM_LINUX
			delete pPlugin->handler;
		#endif //_PLATFORM_LINUX
			delete pPlugin;
		}
		m_Map.erase(m_Map.begin(), m_Map.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

void cPluginManager::SetPluginDirectory(const char *pDirectory)
{
	try
	{
		m_PluginSourceDirectory = pDirectory;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}


#ifdef _PLATFORM_WIN32
void cPluginManager::LoadPlugin(const char *pPluginName, string &localAccess, string &remoteAccess)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);

	try
	{
		HMODULE pluginHandle;
		string path;

		path = m_PluginSourceDirectory;
		path += "\\";
		path += pPluginName;

		pluginHandle = LoadLibrary(path.c_str());
		if (pluginHandle == NULL)
		{
			LOG("Error, failed to load plugin %s", path.c_str());
		}
		else
		{
			CreateProcAddr CreateProc = (CreateProcAddr)GetProcAddress((HINSTANCE)pluginHandle, "Create");
			if (CreateProc == (CreateProcAddr)0)
			{
				LOG("Skipping %s because its not a msgCourier compatible plugin.", pPluginName);
				FreeLibrary(pluginHandle);
				return;
			}
			DestroyProcAddr DestroyProc = (DestroyProcAddr)GetProcAddress((HINSTANCE)pluginHandle, "Destroy");

			cICommandHandler* pCH = NULL;
			CreateProc(&pCH);
			string sName = pCH->GetCommandName();

			map<string,cPlugin*>::iterator mapIterator;
			mapIterator = m_Map.find(sName);
			if (mapIterator != m_Map.end())
			{
				LOG("Error, failed to register %s plugin because a message handler for message [%s] is already registered", path.c_str(), sName.c_str());
				DestroyProc(&pCH);
				FreeLibrary(pluginHandle);
			}
			else
			{
				cPlugin *pPlugin;
				MC_NEW(pPlugin, cPlugin);
				m_Map.insert(pair<string,cPlugin*>(sName, pPlugin));
	
				m_pCommandHandlerFactory->RegisterCommandHandler((char*)sName.c_str(), (cICommandHandler*)pCH, m_pSysInfo);
				pPlugin->handler = pCH;
				pPlugin->libraryHandle = pluginHandle;
				bool bLocal = false;
				bool bRemote = false;
				if (localAccess == "yes")
					bLocal = true;
				if (remoteAccess == "yes")
					bRemote = true;
				pPlugin->localAccess = bLocal;
				pPlugin->remoteAccess = bRemote;

				cICommandHandler *pIHandler = m_pCommandHandlerFactory->GetCommandHandler(sName.c_str());
				if (pIHandler)
				{
					pIHandler->OnStartup((cIMsgQueue*)m_pMsgQueue, m_pSysInfo, m_pLogger);
					m_pService->RegisterLocalService(pIHandler, bLocal, bRemote);
				}
			}
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}
#endif //_PLATFORM_WIN32

#ifdef _PLATFORM_LINUX
void cPluginManager::LoadPlugin(const char *pPluginName, string &localAccess, string &remoteAccess)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);

	try
	{
		DIR *dip;
		struct dirent *dit;
		void *handle = 0;
		const char *errmsg;

		string path;
		path = m_PluginSourceDirectory;
		path += "/";
		path += pPluginName;
		LOG("Loading plugin %s...", path.c_str());
		handle = dlopen(path.c_str(), RTLD_NOW); //RTLD_LAZY | RTLD_GLOBAL);
		if ((errmsg = dlerror()) != NULL)
			LOG("%s", errmsg);
		if (handle == NULL)
		{
			LOG("Error, failed to load plugin %s", path.c_str());
		}
		else
		{
			CreateProcAddr CreateProc = NULL;
			DestroyProcAddr DestroyProc = NULL;
			//dlerror();

			CreateProc = (CreateProcAddr)dlsym(handle, "Create");
			if ((errmsg = dlerror()) != NULL)
			{
				LOG("Skipping %s because its not a msgCourier compatible plugin.", pPluginName);
				dlclose(handle);
				return;
			}
			DestroyProc = (DestroyProcAddr)dlsym(handle, "Destroy");
			if ((errmsg = dlerror()) != NULL)
				LOG("Error can't find \"Destroy\" symbol in %s", path.c_str());

			cICommandHandler* pCH = NULL;
			CreateProc(&pCH);
			string sName = pCH->GetCommandName();
			map<string,cPlugin*>::iterator mapIterator;
			mapIterator = m_Map.find(sName);
			if (mapIterator != m_Map.end())
			{        	
				LOG("Error, failed to register %s plugin because a message handler for message [%s] is already registered", path.c_str(), sName.c_str());
				DestroyProc(&pCH);
			}
			else
			{
				cPlugin *pPlugin;
				MC_NEW(pPlugin, cPlugin);
				m_Map.insert(pair<string,cPlugin*>(sName, pPlugin));

				m_pCommandHandlerFactory->RegisterCommandHandler((char*)sName.c_str(), (cICommandHandler*)pCH, m_pSysInfo);
				pPlugin->handler = pCH;
				pPlugin->libraryHandle = handle;
				bool bLocal = false;
				bool bRemote = false;
				if (localAccess == "yes")
					bLocal = true;
				if (remoteAccess == "yes")
					bRemote = true;
				pPlugin->localAccess = bLocal;
				pPlugin->remoteAccess = bRemote;

				cICommandHandler *pIHandler = m_pCommandHandlerFactory->GetCommandHandler(sName.c_str());
				if (pIHandler)
				{
					pIHandler->OnStartup((cIMsgQueue*)m_pMsgQueue, m_pSysInfo, m_pLogger);
					m_pService->RegisterLocalService(pIHandler, bLocal, bRemote);
				}
			}
			dlclose(handle);
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}
#endif //_PLATFORM_LINUX


