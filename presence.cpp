/* presence.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

presence.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

presence.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with presence.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file presence.cpp
 @brief Presence managment
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include "exception.h"
#include "core.h"
#include "log.h"
#include "presence.h"

#ifdef _PLATFORM_LINUX
#include <signal.h>
	#include <sys/time.h>
	#include <sys/resource.h>
	#include <unistd.h>

	#include <netdb.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <sys/types.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/sysinfo.h>
	#include <netinet/in.h>
	#include <errno.h>
#endif //PLATFORM_LINUX

using namespace std;

#define PRESENCE_TIMOUT 60	// every minute

cPresence::cPresence()
: m_pDotEngine(0)
, m_bRender(false)
{
}

cPresence::~cPresence()
{
	try
	{
		cNodeEntry* pEntry;
		map<string,cNodeEntry*>::iterator it;
		for (it = m_PresenceMap.begin(); it != m_PresenceMap.end(); it++)
		{
			pEntry = (*it).second;
			delete pEntry;
		}
		m_PresenceMap.erase(m_PresenceMap.begin(), m_PresenceMap.end());

		cBroadcastTarget *pTarget;
		int iTot = m_PresenceBroadcast.size();
		for (int i=0; i<iTot; i++)
		{
			pTarget = m_PresenceBroadcast[i];
			delete pTarget;
		}
		m_PresenceBroadcast.erase(m_PresenceBroadcast.begin(), m_PresenceBroadcast.end());

		if (m_pDotEngine)
			delete m_pDotEngine;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

int cPresence::Start()
{
	cThread::Create();
	cThread::Start();
	return HRC_PRESENCE_OK;
}

int cPresence::Stop()
{
	cThread::Destroy();
	return HRC_PRESENCE_OK;
}

int cPresence::Run()
{
	if (m_bRender)
	{
		m_RenderTimer.SetInterval(10);
		m_RenderTimer.Start();
	}

	m_BroadcastTimer.SetInterval(5);
	m_BroadcastTimer.Start();

    while (ThreadRunning())
    {
		m_ThreadSync.Lock();

		// Update Nodes
		DoUpdateNode();

		// Do Render
		if (m_bRender)
		{
			if (m_RenderTimer.IsReady())
			{
L
				RenderMap();
				m_RenderTimer.Reset();
			}
		}

		// Do Broadcast
        //Disable for now - CIG06 Demo
        /*
		if (m_BroadcastTimer.IsReady())
		{
			DoBroadcast();
			m_BroadcastTimer.Reset();
		}
		*/

		m_ThreadSync.Unlock();

		Sleep(250);
    }
    return HRC_PRESENCE_OK;
}

void cPresence::SetDot(const char *pDotPath, const char *pDotFilesPath)
{
	try
	{
		m_DotPath = pDotPath;
		m_DotFilesPath = pDotFilesPath;
		m_bRender = true;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

void cPresence::SetHostNode(const char *pIPAddress, const char *pUDPPort, const char *pTCPPort, const char *pMachineName, const char *pMacAddress)
{
	try
	{
		MC_NEW(m_pDotEngine, cDotEngine(m_DotPath, m_DotFilesPath));
		m_pDotEngine->Start();

		m_HostIP = pIPAddress;
		m_HostUDPPort = pUDPPort;
		m_HostTCPPort = pTCPPort;
		m_HostName = pMachineName;
		m_HostMac = pMacAddress;
		m_HostKey = BuildKey(pIPAddress, pMachineName, pMacAddress);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (...)
	{
		LOGALL("exception in cPresense::SetHostName()");
	}
}

const char *cPresence::UpdateNode(const char *pIPAddress, const char *pMachineName, const char *pMacAddress, const char *pServices, ePresenceStatus status)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);

	char *pKey = NULL;

	try
	{
		//if (m_HostIP != string(pIPAddress))
		//	LOG("Presence update from %s on %s", pMachineName, pIPAddress);

		cNodeEntry* pMachineEntry;
		map<string,cNodeEntry*>::iterator it;

		string key = BuildKey(pIPAddress, pMachineName, pMacAddress);
		it = m_PresenceMap.find(key);
		if (it == m_PresenceMap.end())
		{
			string sLogMsg = "Presence adding new: ";
			sLogMsg += pIPAddress;
			sLogMsg += " ";
			sLogMsg += pMachineName;
			sLogMsg += " ";
			sLogMsg += pMacAddress;
			sLogMsg += " ";
			sLogMsg += pServices;

			LOG(sLogMsg.c_str());
			cCore::GetInstance()->SendToLogger(sLogMsg.c_str());

			// node doesn't exist, so add it.
			MC_NEW(pMachineEntry, cNodeEntry());
			pMachineEntry->m_IPAddress = pIPAddress;
			pMachineEntry->m_MachineName = pMachineName;
			pMachineEntry->m_MacAddress = pMacAddress;
			pMachineEntry->m_Services = pServices;
			pMachineEntry->m_Key = key;
			pMachineEntry->m_status = status;
			pMachineEntry->m_LastConnect = time(0);
			m_PresenceMap.insert(pair<string,cNodeEntry*>(key, pMachineEntry));
		}
		else
		{
			//LOG("Presence updating existing");
			pMachineEntry = (*it).second;
			pMachineEntry->m_status = status;
		}
		pMachineEntry->m_LastConnect = time(0);
		m_Temp = key;
		pKey = (char*)m_Temp.c_str();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (...)
	{
		LOGALL("exception in cPresense::UpdateNode()");
	}
	return pKey;
}

void cPresence::RenderMap()
{
	try
	{
		string dotScript, dotScriptTail, resultImageFile;
		dotScript =
			"digraph G {\n";
			//"size=\"7.5,10\";\n";
		dotScript += "\"";
		dotScript += m_HostName.c_str();
		dotScript += "(";
		dotScript += m_HostIP.c_str();
		dotScript += ")";
		dotScript += "\";\n";

		dotScriptTail += "\"";
		dotScriptTail += m_HostName.c_str();
		dotScriptTail += "(";
		dotScriptTail += m_HostIP.c_str();
		dotScriptTail += ")";
		dotScriptTail += "\" [label=\"";
		dotScriptTail += m_HostName.c_str();
		dotScriptTail += "\\n";
		dotScriptTail += m_HostIP.c_str();
		dotScriptTail += "\",";
		dotScriptTail += "shape=tripleoctagon,color=\"#6699CC\",style=filled,fontsize=\"14.0\"];\n";

		dotScriptTail += "\"";
		dotScriptTail += m_HostName.c_str();
		dotScriptTail += "(";
		dotScriptTail += m_HostIP.c_str();
		dotScriptTail += ")";
		dotScriptTail += "\" [URL=\"http://";
		dotScriptTail += m_HostIP.c_str();
		dotScriptTail += "\"];\n";

		cNodeEntry* pEntry;
		map<string,cNodeEntry*>::iterator it;
		for (it = m_PresenceMap.begin(); it != m_PresenceMap.end(); it++)
		{
			pEntry = (*it).second;

			if (pEntry->m_MachineName == m_HostName)
				continue;

			dotScript += "\"";
			dotScript += m_HostName;
			dotScript += "(";
			dotScript += m_HostIP;
			dotScript += ")";
			dotScript += "\"";
			dotScript += " -> ";
			dotScript += "\"";
			dotScript += pEntry->m_MachineName;
			dotScript += "(";
			dotScript += pEntry->m_IPAddress;
			dotScript += ")";
			dotScript += "\";\n";

			dotScriptTail += "\"";
			dotScriptTail += pEntry->m_MachineName;
			dotScriptTail += "(";
			dotScriptTail += pEntry->m_IPAddress;
			dotScriptTail += ")";
			dotScriptTail += "\"";
			dotScriptTail += " [label=\"";
			dotScriptTail += pEntry->m_MachineName;
			dotScriptTail += "\\n";
			dotScriptTail += pEntry->m_IPAddress;
			dotScriptTail += "\",";
			if (pEntry->m_status == PS_AWAY)
				dotScriptTail += "shape=ellipse,color=\"#666666\",style=filled,fontsize=\"14.0\"];\n";
			else if (pEntry->m_status == PS_ACTIVE)
				dotScriptTail += "shape=ellipse,color=\"#669933\",style=filled,fontsize=\"14.0\"];\n";
			else if (pEntry->m_status == PS_BUSY)
				dotScriptTail += "shape=ellipse,color=\"#993333\",style=filled,fontsize=\"14.0\"];\n";

			if (pEntry->m_status == PS_ACTIVE)
			{
				dotScriptTail += "\"";
				dotScriptTail += pEntry->m_MachineName;
				dotScriptTail += "(";
				dotScriptTail += pEntry->m_IPAddress;
				dotScriptTail += ")";
				dotScriptTail += "\" [URL=\"http://";
				dotScriptTail += pEntry->m_IPAddress;
				dotScriptTail += ":3400\"];\n";
			}
		}
		dotScript += dotScriptTail;
		dotScript += "}\n";

		resultImageFile = "netmap.png";
		m_pDotEngine->GenerateGraph(dotScript, cDotEngine::DOT_PNG, resultImageFile);

		resultImageFile = "netmap.cmapx";
		m_pDotEngine->GenerateGraph(dotScript, cDotEngine::DOT_CMAPX, resultImageFile);

		// read contents of map file
		string sPath = m_DotFilesPath;
		string sMapFile;
		FILE *fp;
		int iread;
		char buffer[1025];
		fp = fopen(m_pDotEngine->GetMapFilePath(), "r");
		if (fp)
		{
			while ((iread = fread(buffer,1,1024,fp)))
				sMapFile.append(buffer, iread);
			fclose(fp);
		}

		// write html file
		string sHTML = "<html><body>";
		sHTML += sMapFile;
		sHTML += "<img src=\"netmap.png\" USEMAP=\"#G\" border=\"0\">";
		sHTML += "</body></html>";
		string sHTMLFileName = m_DotFilesPath;
	#ifndef _PLATFORM_LINUX
		sHTMLFileName += "\\";
	#else
		sHTMLFileName += "/";
	#endif //! _PLATFORM_LINUX
		sHTMLFileName += "netmap.html";
		fp = fopen(sHTMLFileName.c_str(), "wt");
		fwrite(sHTML.c_str(), sHTML.length(), 1, fp);
		fclose(fp);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (...)
	{
		LOGALL("exception in cPresense::RenderMap()");
	}
}

const char *cPresence::GetPresenceData()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	char *pData = NULL;

	try
	{
		cNodeEntry* pEntry;
		map<string,cNodeEntry*>::iterator it;
		string message;
		m_PresenceDataBuffer = "";
		for (it = m_PresenceMap.begin(); it != m_PresenceMap.end(); it++)
		{
			pEntry = (*it).second;

			if (pEntry->m_MachineName == m_HostName)
				continue;

			if (pEntry->m_status == PS_AWAY)
			{
				m_PresenceDataBuffer += "w|";
				message = "Node is currently away";
			}
			else if (pEntry->m_status == PS_ACTIVE)
			{
				m_PresenceDataBuffer += "a|";
				message = "Node is currently active";
			}
			else if (pEntry->m_status == PS_BUSY)
			{
				m_PresenceDataBuffer += "b|";
				message = "Node is currently busy";
			}

			message += ". Services: ";
			message += pEntry->m_Services;

			time_t t = pEntry->m_LastConnect;
			tm *t2 = localtime(&t);
			m_PresenceDataBuffer += asctime(t2);
			m_PresenceDataBuffer.erase(m_PresenceDataBuffer.end() - 1);
			m_PresenceDataBuffer += "|";
			m_PresenceDataBuffer += pEntry->m_MachineName;
			m_PresenceDataBuffer += "|";
			m_PresenceDataBuffer += pEntry->m_IPAddress;
			m_PresenceDataBuffer += "|";
			m_PresenceDataBuffer += message;
			m_PresenceDataBuffer += "~";
		}
		pData = (char*)m_PresenceDataBuffer.c_str();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return pData;
}

const char *cPresence::BuildKey(const char *pIPAddress, const char *pMachineName, const char *pMacAddress)
{
	char *pKey = NULL;

	try
	{
		m_Temp = pIPAddress;
		m_Temp += pMachineName;
		m_Temp += pMacAddress;

		m_Buf.ReplaceWith((char*)m_Temp.c_str());
		unsigned int key = m_Buf.HashKey();
		m_Buf.Sprintf(32, "%X", key);
		m_Buf.MakeString();
		m_Temp.assign((const char *)m_Buf.cstr(), m_Buf.StringLen());
		pKey = (char*)m_Temp.c_str();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return pKey;
}

void cPresence::DoUpdateNode()
{
	try
	{
		cNodeEntry* pEntry;
		map<string,cNodeEntry*>::iterator it;
		int current_time = time(0);
		for (it = m_PresenceMap.begin(); it != m_PresenceMap.end(); it++)
		{
			pEntry = (*it).second;
			if ((pEntry->m_LastConnect + PRESENCE_TIMOUT) < current_time)
				pEntry->m_status = PS_AWAY;
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

void cPresence::DoBroadcast()
{
	try
	{
		cBroadcastTarget *pTarget;
		int iTot = m_PresenceBroadcast.size();

		//LOG("iTot = %d", iTot);

		for (int i=0; i<iTot; i++)
		{
			// do broadcast to this address
			//
			// TCP		HTTP		USE
			// true		false		Internal machine not using UDP and using HTTP
			// true		true		Remote machines using TCP and HTTP
			// false	false		Subnet (UDP and no HTTP)
			// false	true		UDP using HTTP (not valid!)
			//
			pTarget = m_PresenceBroadcast[i];
			if (pTarget->m_bTCP == false && pTarget->m_bUseHTTP == false)
			{
				// broadcast to subnet
				SendBroadcastToSubnet(pTarget->m_BroadcastIPAddress, pTarget->m_Port);
			}
			else if (pTarget->m_bTCP == true && pTarget->m_bUseHTTP == false)
			{
				// broadcast to remote machine using UDP and no HTTP
				SendBroadcastToMachine(pTarget->m_BroadcastIPAddress, pTarget->m_Port, true, false);
			}
			else if (pTarget->m_bTCP == true && pTarget->m_bUseHTTP == true)
			{
				// broadcast to remote machine using TCP and HTTP
			}
			else
			{
				LOG("Unable to broadcast to this invalid entry: IP: %s, Port: %s, TCP: %s, HTTP: %s",
					pTarget->m_BroadcastIPAddress.c_str(), pTarget->m_Port.c_str(),
				(pTarget->m_bTCP == true) ? "tcp" : "udp",
				(pTarget->m_bUseHTTP == true) ? "true" : "false");
			}
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

void cPresence::AddBroadcastAddress(const char *pBroadcastAddress, const char *pPort, bool bTCP, bool bUseHTTP)
{
	try
	{
		cBroadcastTarget *pTarget;
		MC_NEW(pTarget, cBroadcastTarget());

		pTarget->m_BroadcastIPAddress = pBroadcastAddress;
		pTarget->m_Port = pPort;
		pTarget->m_bTCP = bTCP;
		pTarget->m_bUseHTTP = bUseHTTP;
		m_PresenceBroadcast.push_back(pTarget);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (...)
	{
		LOGALL("exception in cPresense::AddBroadcastAddress()");
	}
}

void cPresence::SendBroadcastToSubnet(string &BroadcastAddress, string &Port)
{
	try
	{
		string servicesList;
		GetServiceList(servicesList, true);

		string sExpression = "(";
		sExpression += m_HostMac;
		sExpression += " Active (Svs (";
		sExpression += servicesList;
		sExpression += ")))";
		m_response.Sprintf(5000,"MC.PRESENCE.BROADCAST MCP/1.0\r\n"
								"To: subnet@%s:%s\r\n"
								"From: %s@%s:%s\r\n"
								"Content-Type: text/s-expression\r\n"
								"Content-Length: %d\r\n"
								"Options: priority-low reply-none notify-no\r\n"
								"\r\n",
								BroadcastAddress.c_str(), Port.c_str(),
								m_HostName.c_str(), m_HostIP.c_str(), m_HostTCPPort.c_str(),
								sExpression.length());
		m_response.Append((char*)sExpression.c_str(), sExpression.length());

	    int Socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    	if (Socketfd == -1)
		{
			L
			return;
		}

#ifdef _PLATFORM_LINUX
		int so_broadcast = 1;
		setsockopt(Socketfd, SOL_SOCKET, SO_BROADCAST,&so_broadcast,sizeof(so_broadcast));
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
		BOOL so_broadcast = TRUE;
		setsockopt(Socketfd, SOL_SOCKET, SO_BROADCAST,(char*)&so_broadcast,sizeof(so_broadcast));
#endif //_PLATFORM_WIN32

		struct sockaddr_in server_addr;
    	memset(&server_addr, 0, sizeof(server_addr));

		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = inet_addr(BroadcastAddress.c_str());
		if (server_addr.sin_addr.s_addr == INADDR_NONE)
		{
			// invalid address
		}
		else
		{
			server_addr.sin_port = htons(atoi(Port.c_str()));
			int server_len = sizeof(server_addr);
			int rc = connect(Socketfd, (struct sockaddr*)&server_addr, server_len);
			if (rc)
				rc = send(Socketfd, (const char*)m_response.GetRawBuffer(), m_response.GetBufferCount(), 0);
			if (rc < 0)
			{
				LOG("@@@ UDP Broadcast unable to sendto");
			}
		}

#ifdef _PLATFORM_LINUX
LOG2("close(%d) called", Socketfd);
		close(Socketfd);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
		closesocket(Socketfd);
#endif //_PLATFORM_WIN32
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

void cPresence::SendBroadcastToMachine(string &BroadcastAddress, string &Port, bool bTCP, bool bHTTP)
{
	try
	{
		// (HWAddr Active (Svs (BSE LOGGER MC)))
		if (bTCP == true)
		{
			if (bHTTP == false)
			{
				string servicesList;
				GetServiceList(servicesList, true);

				string sExpression = "(";
				sExpression += m_HostMac;
				sExpression += " Active (Svs (";
				sExpression += servicesList;
				sExpression += ")))";

				m_response.Sprintf(5000,"MC.PRESENCE.BROADCAST MCP/1.0\r\n"
										"To: subnet@%s:%s\r\n"
										"From: %s@%s:%s\r\n"
										"Content-Type: text/s-expression\r\n"
										"Content-Length: %d\r\n"
										"Options: priority-low reply-none notify-no\r\n"
										"\r\n",
										BroadcastAddress.c_str(), Port.c_str(),
										m_HostName.c_str(), m_HostIP.c_str(), m_HostTCPPort.c_str(),
										sExpression.length());
				m_response.Append((char*)sExpression.c_str(), sExpression.length());
				int socketfd = OpenTCP((char*)BroadcastAddress.c_str(), atoi(Port.c_str()));
				if (socketfd != -1)
				{
					TCPSend(socketfd, (const char*)m_response.GetRawBuffer(), m_response.GetBufferCount());
				}
			}
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

int cPresence::OpenTCP(char *pServer, int port)
{
	struct in_addr	iaHost;
	struct hostent  *HostEntry=NULL;
	bool bByName=false;

	iaHost.s_addr = inet_addr(pServer);
	if (iaHost.s_addr == INADDR_NONE)
	{
		// Wasn't an IP address string, assume it is a name
		HostEntry = gethostbyname(pServer);
        if (HostEntry==NULL)
        {
			//LOG("Error, Unable to connect to %s on port %d\n", pServer, port);
            return -1;
        }
        bByName=true;
	}

	struct sockaddr_in socketaddr;
	memset(&socketaddr, 0, sizeof(socketaddr));
	socketaddr.sin_family = AF_INET;
	socketaddr.sin_port = htons(port);
    if (bByName)
	{
        memcpy(&socketaddr.sin_addr.s_addr, HostEntry->h_addr, sizeof(in_addr));
	}
    else
	{
#ifndef _PLATFORM_WIN32
		if (!inet_aton(pServer, &socketaddr.sin_addr))
		{
			HostEntry = gethostbyname(pServer);
			if (HostEntry == NULL)
				return -1;
			socketaddr.sin_addr = *(struct in_addr*)HostEntry->h_addr;
		}
#endif //!_PLATFORM_WIN32

#ifdef _PLATFORM_WIN32
		HostEntry = gethostbyname(pServer);
		if (HostEntry == NULL)
			return -1;
		socketaddr.sin_addr = *(struct in_addr*)HostEntry->h_addr;
#endif //_PLATFORM_WIN32
	}

	// create socket
	int Socketfd = socket(PF_INET, SOCK_STREAM, 0);
	if (Socketfd < 0)
	{
		//LOG("Error, Unable to create socket: %d\n", errno);
		return -1;
	}

	// connect to host
	int rc = connect(Socketfd, (struct sockaddr*)&socketaddr, sizeof(socketaddr));
	if (rc != 0)
	{
        //LOG("Error, Unable to connect to %s on port %d\n", pServer, port);
	}
	return Socketfd;
}

bool cPresence::TCPSend(int iSocketfd, const char *pBuf, int size)
{
	int bytesSent;
	int totalSent = 0;
	do
	{
		bytesSent = send(iSocketfd, pBuf + totalSent, size - totalSent, 0);

#ifdef _PLATFORM_LINUX
		if (bytesSent == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
				continue;
			LOG("TCP send error (OB): %s(%d) on socket %d", strerror(errno), errno, iSocketfd);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
		if (bytesSent == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				continue;
			LOG("TCP send error (OB): (%d) on socket %d", WSAGetLastError(), iSocketfd);
#endif //_PLATFORM_WIN32
			break;
		}
		else
		{
			totalSent += bytesSent;
		}
	} while (totalSent < size);

#ifdef _PLATFORM_LINUX
	close(iSocketfd);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
	closesocket(iSocketfd);
#endif //_PLATFORM_WIN32
	return true;
}

cNodeEntry *cPresence::GetNodeFromKey(const char *pNodeKey)
{
	cNodeEntry* pMachineEntry;
	map<string,cNodeEntry*>::iterator it;

	try
	{
		it = m_PresenceMap.find(pNodeKey);
		if (it == m_PresenceMap.end())
		{
			pMachineEntry = 0;
		}
		else
		{
			pMachineEntry = (*it).second;
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return pMachineEntry;
}

void cPresence::GetServiceList(std::string &list, bool bLocal)
{
	try
	{
		list = "";
		cService *pService = cCore::GetInstance()->GetService();
		cServiceEntry *pServiceEntry = pService->QueryServiceFirst(NULL);
		do
		{
			if (bLocal)
			{
				if (pServiceEntry != 0 && pServiceEntry->m_pNode == 0)
				{
					list.append(pServiceEntry->m_ServiceName.c_str());
					list.append(" ");
				}
			}
			else
			{
				if (pServiceEntry != 0 && pServiceEntry->m_pNode != 0)
				{
					list.append(pServiceEntry->m_ServiceName.c_str());
					list.append(" ");
				}
			}
		} while ((pServiceEntry = pService->QueryServiceNext()) != 0);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}


