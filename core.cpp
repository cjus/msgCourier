/* core.cpp
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

core.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

core.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with core.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file core.cpp
 @brief The cCore class is used to group application wide shared data, and to
 provide easy system wide access.
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include <stdio.h>
#include <stdlib.h>

#ifdef _PLATFORM_WIN32
	#include <windows.h>
	#include <direct.h>
#endif //_PLATFORM_WIN32

#ifndef _PLATFORM_WIN32
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/dir.h>
#endif //!_PLATFORM_WIN32

#include <string>
using namespace std;

#include "uniqueidprovider.h"
#include "msgCourierPlugin.h"

#include "master.h"
#include "core.h"
#include "addonhandlers.h"
#include "umachid.h"

cCore *cCore::m_pInstance = 0;

cCore::cCore(const char *pszModuleName)
: m_ModuleName(pszModuleName)
, m_ifUsingDatabase(false)
, m_pConnectionQueue(0)
, m_pWebServer(0)
, m_pPluginManager(0)
, m_pMsgQueue(0)
, m_bZeroConfig(false)
, m_server_max_cache_size("")
{
	try
	{
		char dirbuf[_MAX_PATH];
		#ifdef _PLATFORM_WIN32
			m_ServerWorkingDirectory = _getcwd(dirbuf, _MAX_PATH);
		#endif //_PLATFORM_WIN32
		#ifdef _PLATFORM_LINUX
			m_ServerWorkingDirectory = getcwd(dirbuf, _MAX_PATH);
		#endif //_PLATFORM_LINUX
	}
	catch (exception const &e)
	{
		LOG(e.what());
	}
}

cCore::~cCore()
{
	Deinit();
}

int cCore::Create(const char *pszModuleName)
{
	int iRet = HRC_CORE_OK;
    if (m_pInstance == 0)
    {
        MC_NEW(m_pInstance, cCore(pszModuleName));
		m_pInstance->Init();
    }
	return iRet;
}

int cCore::Destroy()
{
	int iRet = HRC_CORE_OK;
    if (m_pInstance != 0)
    {
        delete m_pInstance;
        m_pInstance = 0;
    }
	return iRet;
}

cIMsgQueue *cCore::GetMSGQueue()
{
	return (cIMsgQueue*)m_pMsgQueue;
}

void cCore::AppMessage(const char *pFormat, ...)
{
    char buf[_MAX_PATH];
    va_list ap;
    va_start(ap, pFormat);
    vsprintf(buf, pFormat, ap);
    va_end(ap);
#ifdef _PLATFORM_WIN32
	::MessageBox(NULL, buf, (LPCTSTR)APP_NAME, MB_OK | MB_ICONINFORMATION);
#else
    printf("cCore: @@@ %s", buf);
#endif //_PLATFORM_WIN32
}

void cCore::Init()
{	
	try
	{
		m_ServerName = m_NetInterfaces.GetHostName();
		m_ServerDetectedIP = m_NetInterfaces.GetPrimaryIP();
		m_MACAddress = m_NetInterfaces.GetPrimaryMac();
		//m_UniqueServerID = m_Presence.BuildKey(m_ServerDetectedIP.c_str(), m_ServerName.c_str(), m_MACAddress.c_str());

		// Start unique ID provider
		//
		cUniqueIDProvider::Create();
		cUniqueIDProvider::GetInstance()->GetID(m_ServerInstanceID);

		// Create MsgQueue
		//
		MC_NEW(m_pMsgQueue, cMsgQueue(&m_CommandHandlerFactory, &m_MsgRouterRulesEngine, &m_Service));

		// Create SysInfo
		MC_NEW(m_pSysInfo, cSysInfo());
		cUMachID uniqueMachineID;
		m_UniqueServerID.append(m_ModuleName);
		m_UniqueServerID.append(":");
		m_UniqueServerID.append(m_ServerName);
		m_UniqueServerID.append(":");
		m_UniqueServerID.append(uniqueMachineID.GetID());
		m_pSysInfo->SetSysInfo("server.uuid", m_UniqueServerID.c_str());

		// Create Logger
		//
		MC_NEW(m_pLogger, cLogger());

		LoadConfig();

		// Start connection queue
		int iConnectionTimeout = atoi(m_ConnectionTimeout.c_str());
		MC_NEW(m_pConnectionQueue, cConnectionQueue(m_pMsgQueue, &m_PeerRoutingMap, iConnectionTimeout));
		m_pConnectionQueue->SetSysMetrics(&m_SysMetrics);
		m_pConnectionQueue->SetAccessLog(&m_AccessLog);

		// Start MsgQueue
		m_pMsgQueue->SetSysMetrics(&m_SysMetrics);
		m_pMsgQueue->SetConnectionQueue(m_pConnectionQueue);

		// Init local database
		//
		if (m_ifUsingDatabase)
		{
			char dirbuf[_MAX_PATH];
			string sDBPath;
			#ifdef _PLATFORM_WIN32
				sDBPath = _getcwd(dirbuf, _MAX_PATH);
				sDBPath += "\\" + m_DatabaseName;
			#endif //_PLATFORM_WIN32
			#ifdef _PLATFORM_LINUX
				sDBPath = getcwd(dirbuf, _MAX_PATH);
				sDBPath += "/" + m_DatabaseName;
			#endif //_PLATFORM_LINUX

			int rc = m_pMsgQueue->UseDatabase((char*)sDBPath.c_str(), atoi(m_DatabaseFlushInterval.c_str()));
			if (rc != HRC_MSG_QUEUE_OK)
			{
				THROW("Unable to create local database");
			}
		}

		// Setup TCL scripting engine
		//
		MC_NEW(m_pScriptEngine, cScriptEngine());
		m_pScriptEngine->SetCommandHandlerFactory(&m_CommandHandlerFactory);
		m_pScriptEngine->SetMsgQueue(m_pMsgQueue);
		m_pScriptEngine->SetSysMetrics(&m_SysMetrics);
		m_pScriptEngine->SetAccessLog(&m_AccessLog);

		// Init servers
		//
		cTCPServer* pTCPServer;
		m_TCPServerList = "";
		map<int,cTCPServer*>::iterator itTCPServer;
		for (itTCPServer = m_TCPServers.begin(); itTCPServer != m_TCPServers.end(); itTCPServer++)
		{
			MC_NEW(pTCPServer, cTCPServer(m_pConnectionQueue, m_server_listen_addr.c_str(), (*itTCPServer).first, &m_IPAccess));
			(*itTCPServer).second = pTCPServer;
			pTCPServer->SetSysMetrics(&m_SysMetrics);
			pTCPServer->SetAccessLog(&m_AccessLog);
			pTCPServer->Start();
			ostringstream oPort;
			if ((oPort << (*itTCPServer).first))
			{
				if (itTCPServer == m_TCPServers.begin())
					m_TCPPort = oPort.str();
				m_TCPServerList.append(oPort.str());
				m_TCPServerList.append(" ");
			}
		}

		cUDPServer* pUDPServer;
		m_UDPServerList = "";
		map<int,cUDPServer*>::iterator itUDPServer;
		for (itUDPServer = m_UDPServers.begin(); itUDPServer != m_UDPServers.end(); itUDPServer++)
		{
			MC_NEW(pUDPServer, cUDPServer(m_pMsgQueue, m_server_listen_addr.c_str(), (*itUDPServer).first, &m_IPAccess));
			(*itUDPServer).second = pUDPServer;
			pUDPServer->SetSysMetrics(&m_SysMetrics);
			pUDPServer->SetAccessLog(&m_AccessLog);
			pUDPServer->Start();
			ostringstream oPort;
			if ((oPort << (*itUDPServer).first))
			{
				if (itUDPServer == m_UDPServers.begin())
					m_UDPPort = oPort.str();
				m_UDPServerList.append(oPort.str());
				m_UDPServerList.append(" ");
			}
		}

		//LOGINFO("Starting msgCourier");
		LOG("TCP Server(s) listening to port(s): %s", m_TCPServerList.c_str());
		LOG("UDP Server(s) listening to port(s): %s", m_UDPServerList.c_str());
		LOG("Binding to %s, listening on %s, %s and %s",
			m_server_listen_addr.c_str(), m_server_listen_addr.c_str(), m_server_primary_addr.c_str(), m_server_secondary_addr.c_str());
		if (m_ifUsingDatabase)
			LOG("Using database %s", m_DatabaseName.c_str());
		else
			LOG("Not using local database");


		// set file cache size
		int fileCacheSize = atoi(m_server_max_cache_size.c_str()) * 1024;		
		m_FileCache.SetCacheMaxSize(fileCacheSize);		
		LOG("Max file cache set to: %d", fileCacheSize);
		
		// now that system is running, start presence thread
		// Init Presence handler
		//
		if (m_bZeroConfig == false && m_DotPath.length() != 0)
			m_Presence.SetDot(m_DotPath.c_str(), m_DotFilesPath.c_str());
		m_Presence.SetHostNode(m_ServerDetectedIP.c_str(), m_UDPPort.c_str(), m_TCPPort.c_str(),
								m_ServerName.c_str(), m_MACAddress.c_str());

		// start app threads
		m_pConnectionQueue->Start();
		m_pMsgQueue->Start();
		m_SysMetrics.Start();
		m_AccessLog.Start();
		m_Presence.Start();

        // Add additional system info items
		m_pSysInfo->SetSysInfo("server.directory", m_ServerWorkingDirectory.c_str());
		m_pSysInfo->SetSysInfo("server.name", m_ServerName.c_str());
		m_pSysInfo->SetSysInfo("server.module.name", m_ModuleName.c_str());
		m_pSysInfo->SetSysInfo("server.ip", m_server_primary_addr.c_str()); //m_ServerDetectedIP.c_str());
		m_pSysInfo->SetSysInfo("server.mac.addr", m_MACAddress.c_str());
		m_pSysInfo->SetSysInfo("server.version", APP_VERSION);

		///
		/// Add internal handlers
		///
		m_CommandHandlerFactory.RegisterCommandHandler(m_MsgCourierInternalCommandHandler.GetCommandName(),
														(cICommandHandler*)&m_MsgCourierInternalCommandHandler,
														(cISysInfo*)m_pSysInfo);
		cICommandHandler *pMsgCourier = m_CommandHandlerFactory.GetCommandHandler("MC");
		pMsgCourier->OnStartup((cIMsgQueue*)m_pMsgQueue, (cISysInfo*)m_pSysInfo, (cILogger*)m_pLogger);
		m_Service.RegisterLocalService(pMsgCourier, true, true);

		MC_NEW(m_pWebServer, cWebServer(&m_VirtualDirectoryMap, atoi(m_MaxWebServerThreads.c_str())));
		m_pWebServer->SetScriptEngine(m_pScriptEngine);
		m_CommandHandlerFactory.RegisterCommandHandler(m_pWebServer->GetCommandName(),
														(cICommandHandler*)m_pWebServer,
														(cISysInfo*)m_pSysInfo);
		cICommandHandler *pWebServer = m_CommandHandlerFactory.GetCommandHandler("MCWS");
		pWebServer->OnStartup((cIMsgQueue*)m_pMsgQueue, (cISysInfo*)m_pSysInfo, (cILogger*)m_pLogger);

/*cg*/
		MC_NEW(m_pTSZScraperServer, cTSZScraper());
		m_CommandHandlerFactory.RegisterCommandHandler(m_pTSZScraperServer->GetCommandName(),
														(cICommandHandler*)m_pTSZScraperServer,
														(cISysInfo*)m_pSysInfo);
		cICommandHandler *pTSZScraperServer = m_CommandHandlerFactory.GetCommandHandler("TSZSCRAPER");
		pTSZScraperServer->OnStartup((cIMsgQueue*)m_pMsgQueue, (cISysInfo*)m_pSysInfo, (cILogger*)m_pLogger);
/*cg*/

		MC_NEW(m_pDownloadManager, cDownloadManager(atoi(m_MaxWebServerThreads.c_str())));
		m_CommandHandlerFactory.RegisterCommandHandler(m_pDownloadManager->GetCommandName(),
														(cICommandHandler*)m_pDownloadManager,
														(cISysInfo*)m_pSysInfo);
		cICommandHandler *m_pDownloadManager = m_CommandHandlerFactory.GetCommandHandler("DWNLDM");
		m_pDownloadManager->OnStartup((cIMsgQueue*)m_pMsgQueue, (cISysInfo*)m_pSysInfo, (cILogger*)m_pLogger);

		MC_NEW(m_pAutoUpdateManager, cAutoUpdate());
		m_CommandHandlerFactory.RegisterCommandHandler(m_pAutoUpdateManager->GetCommandName(),
														(cICommandHandler*)m_pAutoUpdateManager,
														(cISysInfo*)m_pSysInfo);
		cICommandHandler *pAutoUpdateManager =  m_CommandHandlerFactory.GetCommandHandler("AUTOUPDATE");
		pAutoUpdateManager->OnStartup((cIMsgQueue*)m_pMsgQueue, (cISysInfo*)m_pSysInfo, (cILogger*)m_pLogger);

		// Register add-on handlers
		//
		cAddOnHandlers addonHandlers(m_pMsgQueue, &m_CommandHandlerFactory, (cISysInfo*)m_pSysInfo);
		addonHandlers.Register();

		DoPostProcesses();

		// send startup message to log server
		/*
		string sMessage;
		sMessage = "Starting server ";
		sMessage += GetServerName();
		sMessage += "@";
		sMessage += GetServerPrimaryAddr();
		m_pMsgQueue->SendToLogger("MC", sMessage.c_str());
		*/
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (cException &e)
	{
		LOGALL("cException()");
	}
}


void cCore::Deinit()
{
	try
	{
		// Stop Presence thread
		m_Presence.Stop();

		//
    	// Stop PluginManager
		//
		if (m_pPluginManager)
			delete m_pPluginManager;

		//
	    // Stop Servers
		//
		cTCPServer* pTCPServer;
		map<int,cTCPServer*>::iterator itTCPServer;
		for (itTCPServer = m_TCPServers.begin();itTCPServer != m_TCPServers.end(); itTCPServer++)
		{
			pTCPServer = (*itTCPServer).second;
			pTCPServer->Stop();
			delete pTCPServer;
			pTCPServer = 0;
		}
		m_TCPServers.erase(m_TCPServers.begin(), m_TCPServers.end());

		cUDPServer* pUDPServer;
		map<int,cUDPServer*>::iterator itUDPServer;
		for (itUDPServer = m_UDPServers.begin();itUDPServer != m_UDPServers.end(); itUDPServer++)
		{
			pUDPServer = (*itUDPServer).second;
			pUDPServer->Stop();
			delete pUDPServer;
			pUDPServer = 0;
		}
		m_UDPServers.erase(m_UDPServers.begin(), m_UDPServers.end());

		//
    	// Stop connection Queue
		//
		if (m_pConnectionQueue)
		{
			m_pConnectionQueue->Stop();
			delete m_pConnectionQueue;
		}

		//
    	// Stop WebServer
		//
		if (m_pWebServer)
			delete m_pWebServer;

		//
	    // Stop Messsage Queue
		//
		if (m_pMsgQueue)
		{
			m_pMsgQueue->Stop();
			delete m_pMsgQueue;
		}

		// stop script engine
		if (m_pScriptEngine)
			delete m_pScriptEngine;

		//
    	// Stop Unique ID provider
		//
		cUniqueIDProvider::Destroy();

		m_AccessLog.Stop();
		m_SysMetrics.Stop();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

void cCore::LoadConfig()
{
	cXMLLiteParser xml;
	string sXML;
	FILE *fp;
	int iread;
	char buffer[1025];

	m_pSysInfo->SetSysInfo("server.mode", "MASTER");

	string sFileName = m_ModuleName + ".xml";
	m_pSysInfo->SetSysInfo("server.config.filename", sFileName.c_str());

	fp = fopen(sFileName.c_str(), "r");
	if (!fp)
	{
		// no config file
		//*cg* remove - fclose(fp);
		SetDefaults();
		return;
	}

	while ((iread = fread(buffer,1,1024,fp)))
	{
		sXML.append(buffer, iread);
	}
	fclose(fp);

	char *pData = NULL;
	cXMLLiteElement *pElm;
	xml.Parse(sXML.c_str());
	xml.MoveHead();

	try
	{
		pElm = xml.FindTag("server_listen_addr");
		if (pElm)
			m_server_listen_addr = pElm->GetElementValue();
		else
			m_server_listen_addr = "0.0.0.0";

		pElm = xml.FindTag("server_primary_addr");
		if (pElm)
			m_server_primary_addr = pElm->GetElementValue();
		else
		{
			m_ServerDetectedIP = m_NetInterfaces.GetPrimaryIP();
			m_server_primary_addr = m_ServerDetectedIP;
		}

		pElm = xml.FindTag("server_secondary_addr");
		if (pElm)
			m_server_secondary_addr = pElm->GetElementValue();
		else
		{
			m_ServerDetectedIP = m_NetInterfaces.GetPrimaryIP();
			m_server_secondary_addr = m_ServerDetectedIP;
		}

		pElm = xml.FindTag("server_max_cache_size");
		if (pElm)
			m_server_max_cache_size = pElm->GetElementValue();
		else
			m_server_max_cache_size = "10000"; // 10MB

		
		string sPortAddr;
		string sDirectory;
		pElm = xml.FindTag("tcp_servers");
		if (pElm)
		{
			while ((pElm = xml.FindNextTag("host","/web")) != 0)
			{
				sPortAddr = "host_";
				sPortAddr += pElm->GetAttributeValue("addr");
				sDirectory = pElm->GetAttributeValue("directory");
				if (sDirectory != "")
					m_VirtualDirectoryMap.MapVirtualDirectory(sPortAddr, sDirectory);
			}
			pElm = xml.FindTag("tcp_servers");
			while ((pElm = xml.FindNextTag("port","/web")) != 0)
			{
				sPortAddr = "port_";
				sPortAddr += pElm->GetAttributeValue("addr");
				m_TCPServers.insert(pair<int,cTCPServer*>(atoi(pElm->GetAttributeValue("addr")), 0));
				sDirectory = pElm->GetAttributeValue("directory");
				if (sDirectory != "")
					m_VirtualDirectoryMap.MapVirtualDirectory(sPortAddr, sDirectory);
			}
		}
		else
		{
			m_ServerDetectedIP = m_NetInterfaces.GetPrimaryIP();
			m_server_secondary_addr = m_ServerDetectedIP;
			m_TCPServers.insert(pair<int,cTCPServer*>(3400, 0));
		}

		pElm = xml.FindTag("udp_servers");
		if (pElm)
		{
			while ((pElm = xml.FindNextTag("port","/udp_servers")) != 0)
			{
				sPortAddr = pElm->GetAttributeValue("addr");
				m_UDPServers.insert(pair<int,cUDPServer*>(atoi(sPortAddr.c_str()), 0));
			}
		}
		else
		{
			m_UDPServers.insert(pair<int,cUDPServer*>(3401, 0));
		}

		pElm = xml.FindTag("connectiontimeout");
		if (pElm)
			m_ConnectionTimeout = pElm->GetElementValue();
		else
			m_ConnectionTimeout = "30";

		pElm = xml.FindTag("max_webserver_threads");
		if (pElm)
			m_MaxWebServerThreads = pElm->GetElementValue();
		else
			m_MaxWebServerThreads = "16";

		pElm = xml.FindTag("db_name");
		if (pElm)
		{
			m_DatabaseName = pElm->GetElementValue();
			m_ifUsingDatabase = (m_DatabaseName.length() == 0) ? false : true;
			pElm = xml.FindTag("db_flush_interval");
			if (pElm)
				m_DatabaseFlushInterval = pElm->GetElementValue();
			if (m_ifUsingDatabase && m_DatabaseFlushInterval.length() == 0)
				THROW("Unable to locate 'db_flush_interval' entry in msgCourier.xml file.");
		}

		pElm = xml.FindTag("peers");
		if (pElm)
		{
			string sIP;
			string sMask;
			string sTCP;
			string sHTTP;
			bool bTCP, bHTTP;
			while ((pElm = xml.FindNextTag("broadcast","/peers")) != 0)
			{
				sIP = pElm->GetAttributeValue("ip");
				sPortAddr = pElm->GetAttributeValue("port");
				sMask = pElm->GetAttributeValue("mask");
				sTCP = pElm->GetAttributeValue("transport");
				sHTTP = pElm->GetAttributeValue("use-http");
				bTCP = (sTCP == "tcp") ? true : false;
				bHTTP = (sHTTP == "yes") ? true : false;
				if (bTCP == false && sMask.length() == 0)
					THROW("Use of UDP broadcast requires mask= parameter in msgCourier.xml file.");
				if (bTCP == false)
					sIP = m_NetInterfaces.GetBroadcastAddress(sIP.c_str(), sMask.c_str());
				m_Presence.AddBroadcastAddress(sIP.c_str(), sPortAddr.c_str(), bTCP, bHTTP);
			}
		}
		else
		{
			m_ServerDetectedIP = m_NetInterfaces.GetPrimaryIP();
			m_server_primary_addr = m_ServerDetectedIP;
			string sBroadcast = m_server_primary_addr;

			string port = "3401";
			string::size_type pos;
			pos = sBroadcast.find_last_of(".");
			int len = sBroadcast.length();
			sBroadcast.erase(pos+1, len-pos);
			sBroadcast.append("255");
			m_Presence.AddBroadcastAddress(sBroadcast.c_str(), port.c_str(), false, false);
		}
		
		pElm = xml.FindTag("graphviz");
		if (pElm)
		{
			pElm = xml.FindNextTag("directory");
			if (pElm)
				m_DotPath = pElm->GetElementValue();
			else
				m_DotPath = "";
			pElm = xml.FindNextTag("output");
			if (pElm)
				m_DotFilesPath = pElm->GetElementValue();
			else
				m_DotFilesPath = "";
		}
		else
		{
			m_DotPath = "";
			m_DotFilesPath = "";
		}

		LOG("Starting %s server (version %s) on %s", m_ModuleName.c_str(), APP_VERSION, m_ServerName.c_str());
		LOG("ProcessID = %ld", getpid());
		char processID[30];
		sprintf(processID, "%ld", getpid());
		m_pSysInfo->SetSysInfo("server.process.id", (const char *)processID);

		LOG("ServerID = %s", m_UniqueServerID.c_str());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (cException &ex)
	{
		throw ex;
	}
}

void cCore::DoPostProcesses()
{
	cXMLLiteParser xml;
	string sXML;
	FILE *fp;
	int iread;
	char buffer[1025];

	string sFileName = m_ModuleName + ".xml";
	m_pSysInfo->SetSysInfo("server.config.filename", sFileName.c_str());

	fp = fopen(sFileName.c_str(), "r");
	if (!fp)
	{
		//*cg* remove fclose(fp);
		return;
	}

	while ((iread = fread(buffer,1,1024,fp)))
	{
		sXML.append(buffer, iread);
	}
	fclose(fp);

	char *pData = NULL;
	cXMLLiteElement *pElm;
	xml.Parse(sXML.c_str());
	xml.MoveHead();

	try
	{
		pElm = xml.FindTag("handlers");
		ProcessIPRules(&xml, pElm);
		ProcessHandlers(&xml, pElm);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (cException &ex)
	{
		throw ex;
	}
}


void cCore::ProcessIPRules(cXMLLiteParser *pXML, cXMLLiteElement *pElm)
{
	try
	{
		// setup IPAccess
		pElm = pXML->FindTag("connections");
		if (pElm)
		{
			string sRule;

			// do allow rules
			pElm = pXML->FindNextTag("allow");
			if (pElm)
			{
				while ((pElm = pXML->FindNextTag("addr","/allow")) != 0)
				{
					sRule = "+";
					sRule.append(pElm->GetElementValue());
					m_IPAccess.Add((char*)sRule.c_str());
				}
			}

			// do deny rules
			pElm = pXML->FindNextTag("block");
			if (pElm)
			{
				while ((pElm = pXML->FindNextTag("addr","/block")) != 0)
				{
					sRule = "-";
					sRule.append(pElm->GetElementValue());
					m_IPAccess.Add((char*)sRule.c_str());
				}
			}
		}
		else
		{
			// only allow connections on network subnet
			m_ServerDetectedIP = m_NetInterfaces.GetPrimaryIP();
			m_server_primary_addr = m_ServerDetectedIP;

			string sBroadcast = m_server_primary_addr;
			string port = "3401";
			string::size_type pos;

			string subnet;
			subnet = "+";
			subnet += sBroadcast;
			pos = subnet.find_last_of(".");
			int len = subnet.length();
			subnet.erase(pos+1, len-pos);
			subnet.append("*");
			m_IPAccess.Add((char*)subnet.c_str());
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

void cCore::ProcessHandlers(cXMLLiteParser *pXML, cXMLLiteElement *pElm)
{
	string name;
	string to;
	string op;
	string pattern;
	string variance;
	string location;
	string value;

	try
	{
		// Add message courier rules first to ensure fastest routing
		name = "MC";
		to = "MC";
		pattern = "";
		value = "MC ";
		m_MsgRouterRulesEngine.AddRule(name, to, cMsgRouterRule::MSG_RTL_STATUS, cMsgRouterRule::MSG_RTO_CONTAINS, pattern, value);

		pElm = pXML->FindTag("handlers");
		if (pElm)
		{
			pElm = pXML->FindNextTag("directory");
			if (pElm)
				m_PluginDirectory = pElm->GetElementValue();
			else
				m_PluginDirectory = "";

			MC_NEW(m_pPluginManager, cPluginManager(&m_CommandHandlerFactory, &m_Service, m_pMsgQueue, (cISysInfo*)m_pSysInfo, (cILogger*)m_pLogger));
			m_pPluginManager->SetPluginDirectory(m_PluginDirectory.c_str());

			string modulename;
			while ((pElm = pXML->FindNextTag("handler","/handlers")) != 0)
			{
				string local, remote;
				local = pElm->GetAttributeValue("local-access");
				remote = pElm->GetAttributeValue("remote-access");
				m_pPluginManager->LoadPlugin(pElm->GetAttributeValue("module"), local, remote);
			}

			char *pData = 0;
			cMsgRouterRule::eMsgRouterOperationType OperationType = cMsgRouterRule::MSG_RTO_NOTSET;
			cMsgRouterRule::eMsgRouterLocationType Location = cMsgRouterRule::MSG_RTL_NOTSET;

			pElm = pXML->FindTag("handlers");
			while ((pElm = pXML->FindNextTag("route","/handlers")) != 0)
			{
				name = pElm->GetAttributeValue("name");
				to = pElm->GetAttributeValue("to");
				if (name.length() == 0 ||
					to.length() == 0)
				{
					THROW("route rule is missing 'name' or 'to' field.");
				}
				while ((pElm = pXML->FindNextTag("if","/route")) != 0)
				{
					op = pElm->GetAttributeValue("op");
					if (op.length()==0)
						THROW("route rule is missing 'op' field.");

					value = pElm->GetAttributeValue("value");
					//if (value.length()==0)
					//	THROW("route rule is missing 'value' field.");

					variance = pElm->GetAttributeValue("variance");
					pattern = pElm->GetAttributeValue("pattern");

					location = pElm->GetAttributeValue("location");
					if (location.length()==0)
						THROW("route rule is missing 'location' field.");
					if (location == "status")
						Location = cMsgRouterRule::MSG_RTL_STATUS;
					else if (location == "header")
						Location = cMsgRouterRule::MSG_RTL_HEADER;
					else if (location == "payload")
						Location = cMsgRouterRule::MSG_RTL_PAYLOAD;

					if (op == "match" && variance.length())
						OperationType = cMsgRouterRule::MSG_RTO_LIKE;
					else if (op == "match")
						OperationType = cMsgRouterRule::MSG_RTO_MATCH;
					else if (op == "contains")
						OperationType = cMsgRouterRule::MSG_RTO_CONTAINS;

					m_MsgRouterRulesEngine.AddRule(name, to, Location, OperationType, pattern, value);
				}
			}
		}
		else
		{
			cMsgRouterRule::eMsgRouterOperationType OperationType = cMsgRouterRule::MSG_RTO_CONTAINS;
			cMsgRouterRule::eMsgRouterLocationType Location = cMsgRouterRule::MSG_RTL_STATUS;
			string name = "MC";
			string to = "MC";
			string pattern = "";
			string value = "MC ";
			m_MsgRouterRulesEngine.AddRule(name, to, Location, cMsgRouterRule::MSG_RTO_CONTAINS, pattern, value);

			name = "HTTP_GET";
			to = "MCWS";
			value = "GET ";
			OperationType = cMsgRouterRule::MSG_RTO_CONTAINS;
			m_MsgRouterRulesEngine.AddRule(name, to, Location, OperationType, pattern, value);

			name = "HTTP_POST";
			to = "MCWS";
			value = "POST ";
			OperationType = cMsgRouterRule::MSG_RTO_CONTAINS;
			m_MsgRouterRulesEngine.AddRule(name, to, Location, OperationType, pattern, value);
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

void cCore::SetDefaults()
{
	m_pSysInfo->SetSysInfo("server.mode", "SERVANT");
	try
	{
		m_ServerName = m_NetInterfaces.GetHostName();
		m_ServerDetectedIP = m_NetInterfaces.GetPrimaryIP();
		m_MACAddress = m_NetInterfaces.GetPrimaryMac();
		//m_UniqueServerID = m_Presence.BuildKey(m_ServerDetectedIP.c_str(), m_ServerName.c_str(), m_MACAddress.c_str());
		string BroadcastAddress = m_NetInterfaces.GetBroadcastIP();

		m_server_listen_addr = "0.0.0.0";
		m_server_primary_addr = m_ServerDetectedIP;
		m_server_secondary_addr = m_ServerDetectedIP;

		m_server_max_cache_size = "10000"; // 10MB

		m_TCPServers.insert(pair<int,cTCPServer*>(3400, 0));
		m_UDPServers.insert(pair<int,cUDPServer*>(3401, 0));

		m_ConnectionTimeout = "300";
		m_MaxWebServerThreads = "16";
		string port = "3401";
		m_Presence.AddBroadcastAddress(BroadcastAddress.c_str(), port.c_str(), false, false);

		cMsgRouterRule::eMsgRouterOperationType OperationType = cMsgRouterRule::MSG_RTO_CONTAINS;
		cMsgRouterRule::eMsgRouterLocationType Location = cMsgRouterRule::MSG_RTL_STATUS;
		string name = "MC";
		string to = "MC";
		string pattern = "";
		string value = "MC ";
		m_MsgRouterRulesEngine.AddRule(name, to, Location, cMsgRouterRule::MSG_RTO_CONTAINS, pattern, value);

		name = "HTTP_GET";
		to = "MCWS";
		value = "GET ";
		OperationType = cMsgRouterRule::MSG_RTO_CONTAINS;
		m_MsgRouterRulesEngine.AddRule(name, to, Location, OperationType, pattern, value);

		name = "HTTP_POST";
		to = "MCWS";
		value = "POST ";
		OperationType = cMsgRouterRule::MSG_RTO_CONTAINS;
		m_MsgRouterRulesEngine.AddRule(name, to, Location, OperationType, pattern, value);

		LOG("Starting %s server (version %s) on %s", m_ModuleName.c_str(), APP_VERSION, m_ServerName.c_str());
		LOG("ProcessID = %ld", getpid());
		LOG("ServerID = %s", m_UniqueServerID.c_str());

		//disabled for CIG06 demo
		//m_IPAccess.Add("+127.0.0.1");

        //enabled for CIG06 demo
        m_IPAccess.Add("+*.*.*.*");

        /* disabled for CIG06 demo
		string::size_type pos;
		string subnet;
		subnet = "+";
		subnet += BroadcastAddress;
		pos = subnet.find_last_of(".");
		int len = subnet.length();
		subnet.erase(pos+1, len-pos);
		subnet.append("*");
		m_IPAccess.Add((char*)subnet.c_str());
		*/

		string local = "true";
		string remote = "false";
#ifdef _PLATFORM_WIN32
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		hFind = FindFirstFile("*.dll", &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			m_PluginDirectory = ".";
			MC_NEW(m_pPluginManager, cPluginManager(&m_CommandHandlerFactory, &m_Service, m_pMsgQueue, (cISysInfo*)m_pSysInfo, (cILogger*)m_pLogger));
			m_pPluginManager->SetPluginDirectory(m_PluginDirectory.c_str());
			m_pPluginManager->LoadPlugin(FindFileData.cFileName, local, remote);
			while (FindNextFile(hFind, &FindFileData) != 0)
			{
				m_pPluginManager->LoadPlugin(FindFileData.cFileName, local, remote);
			}
			FindClose(hFind);
		}
#endif //_PLATFORM_WIN32

#ifdef _PLATFORM_LINUX
		m_PluginDirectory = ".";
		MC_NEW(m_pPluginManager, cPluginManager(&m_CommandHandlerFactory, &m_Service, m_pMsgQueue, (cISysInfo*)m_pSysInfo, (cILogger*)m_pLogger));
		m_pPluginManager->SetPluginDirectory(m_PluginDirectory.c_str());

		struct direct *DirEntryPtr;
		DIR *DirPtr = opendir(".");
		while (1)
		{
			DirEntryPtr = readdir(DirPtr);
			if (DirEntryPtr == 0)
				break;

			if (strcmp(DirEntryPtr->d_name,".") != 0 && strcmp(DirEntryPtr->d_name,"..") != 0)
			{
				if (strstr(DirEntryPtr->d_name,".so") != 0)
				{
					//printf("%s",DirEntryPtr->d_name);
					//printf("\n");
					m_pPluginManager->LoadPlugin(DirEntryPtr->d_name, local, remote);
				}
			}
		}
#endif //_PLATFORM_LINUX
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	m_bZeroConfig = true;
}

void cCore::SendToLogger(const char *pLogMessage)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	//m_pMsgQueue->SendToLogger("MC", pLogMessage);
}


