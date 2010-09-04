/* peerroutingmap.cpp
   Copyright (C) 2006 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

peerroutingmap.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

peerroutingmap.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with peerroutingmap.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file peerroutingmap.cpp
 @brief  Peer Routing Map
 @author Carlos Justiniano
 @attention Copyright (C) 2006 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include "exception.h"
#include "log.h"
#include "peerroutingmap.h"

using namespace std;

cPeerRoutingMap::cPeerRoutingMap()
{
}

cPeerRoutingMap::~cPeerRoutingMap()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	try
	{	
		cPeerRoutingRecord *pRec = NULL;
		map<std::string,cPeerRoutingRecord*>::iterator it;
		for (it = m_PeerMap.begin();
			 it != m_PeerMap.end();
			 it++)
		{
			pRec = it->second;
			delete pRec;
		}
		m_PeerMap.erase(m_PeerMap.begin(), m_PeerMap.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

int cPeerRoutingMap::AddRecord(const char *pKey, const char *pIPAddress, int iPort, int iSocketfd)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	try
	{
		cPeerRoutingRecord *pRec = NULL;
		map<std::string,cPeerRoutingRecord*>::iterator it;

		// first check if record exists
		it = m_PeerMap.find(pKey);
		if (it == m_PeerMap.end())
		{
			// record does not exists so create it.
			MC_NEW(pRec, cPeerRoutingRecord());
			m_PeerMap.insert(pair<string,cPeerRoutingRecord*>(pKey, pRec));
		}
		else
		{
			// record does exists so update it
			pRec = (*it).second;
		}
		pRec->key = pKey;
		pRec->ipAddress = pIPAddress;
		pRec->iPort = iPort;
		pRec->iSocketfd = iSocketfd;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return 0;
}

bool cPeerRoutingMap::HasKey(const char *pKey)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	bool bRet = false;
	try
	{
		map<std::string,cPeerRoutingRecord*>::iterator it;
		it = m_PeerMap.find(pKey);
		bRet = (it == m_PeerMap.end()) ? false : true;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return bRet;
}

int cPeerRoutingMap::GetSocket(const char *pKey)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);

	int socketfd = -1;
	try
	{
		map<std::string,cPeerRoutingRecord*>::iterator it;
		it = m_PeerMap.find(pKey);
		socketfd = (it == m_PeerMap.end()) ? -1 : ((*it).second)->iSocketfd;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return socketfd;
}

cPeerRoutingRecord *cPeerRoutingMap::GetRecord(const char *pKey)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);

	cPeerRoutingRecord *pRec = NULL;
	try
	{
		map<std::string,cPeerRoutingRecord*>::iterator it;
		it = m_PeerMap.find(pKey);
		if (it == m_PeerMap.end())
			pRec = NULL;
		else
			pRec = (*it).second;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return pRec;
}
