/* presence.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

presence.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

presence.h was developed by Carlos Justiniano for use on the
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
 @file presence.h
 @brief Presence managment
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef PRESENCE_H
#define PRESENCE_H

#include <string>
#include <map>
#include <vector>
#include "dotengine.h"
#include "thread.h"
#include "threadsync.h"
#include "timer.h"
#include "buffer.h"

#define HRC_PRESENCE_OK		0x0000

typedef enum PRESENCE_STATUS
{ 
	PS_NOTSET = -1, PS_AWAY = 0, PS_ACTIVE = 1, PS_BUSY = 2
} ePresenceStatus;

class cBroadcastTarget
{
public:
	std::string m_BroadcastIPAddress;
	std::string m_Port;
	bool m_bTCP;
	bool m_bUseHTTP;
};

class cNodeEntry
{
public:
	std::string m_IPAddress;
	std::string m_MachineName;
	std::string m_MacAddress;
	std::string m_Key;
	std::string m_Services;
	int m_LastConnect;
	int m_TCPSocketHandle;
	ePresenceStatus m_status;
};

class cPresence : public cThread
{
public:
	cPresence();
	~cPresence();

	int Start();
	int Stop();

	void SetDot(const char *pDotPath, const char *pDotFilesPath);
	void SetHostNode(const char *pIPAddress, const char *pUDPPort, const char *pTCPPort, const char *pMachineName, const char *pMacAddress);

	const char *UpdateNode(const char *pIPAddress, const char *pMachineName, const char *pMacAddress, const char *pServices, ePresenceStatus status = PS_ACTIVE);
	void AddBroadcastAddress(const char *pBroadcastAddress, const char *pPort, bool bTCP, bool bUseHTTP);
	const char *BuildKey(const char *pIPAddress, const char *pMachineName, const char *pMacAddress);
	const char *GetPresenceData();
	cNodeEntry *GetNodeFromKey(const char *pNodeKey);

protected:
    int Run();

private:
	cThreadSync m_ThreadSync;
	cTimer m_RenderTimer;
	cTimer m_BroadcastTimer;

	std::string m_HostKey;
	std::map<std::string, cNodeEntry*> m_PresenceMap;
	std::vector<cBroadcastTarget*> m_PresenceBroadcast;

	cDotEngine *m_pDotEngine;
	std::string m_DotPath;
	std::string m_DotFilesPath;

	std::string m_HostIP;
	std::string m_HostUDPPort;
	std::string m_HostTCPPort;
	std::string m_HostName;
	std::string m_HostMac;
	std::string m_Temp;
	std::string m_PresenceDataBuffer;
	cBuffer m_response;
	cBuffer m_Buf;
	bool m_bRender;

	void RenderMap();
	void DoUpdateNode();
	void DoBroadcast();
	void SendBroadcastToSubnet(std::string &BroadcastAddress, std::string &Port);
	void SendBroadcastToMachine(std::string &BroadcastAddress, std::string &Port, bool bTCP, bool bHTTP);

	int OpenTCP(char *pServer, int port);
	bool TCPSend(int iSocketfd, const char *pBuf, int size);

	void GetServiceList(std::string &list, bool bLocal);
};

#endif // PRESENCE_H
