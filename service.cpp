/* service.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

service.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

service.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with service.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file service.cpp
 @brief Service managment
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include "core.h"
#include "exception.h"
#include "log.h"
#include "service.h"

using namespace std;

cService::cService()
{
}

cService::~cService()
{
	try
	{	
		cServiceEntry* pEntry;
		multimap<string,cServiceEntry*>::iterator it;
		for (it = m_ServiceMap.begin(); it != m_ServiceMap.end(); it++)
		{
			pEntry = (*it).second;
			delete pEntry;
		}
		m_ServiceMap.erase(m_ServiceMap.begin(), m_ServiceMap.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

bool cService::RegisterRemoteService(const char *pNodeKey, const char *pServiceName, 
									 int Version_Major, int Version_Minor, int Version_Revision,
									 bool bLocal, bool bRemote, int TCPSocketHandle)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);

	bool bFound = false;

	try
	{
		cServiceEntry *pServiceEntry = QueryServiceFirst(pServiceName);
		do
		{
			if (pServiceEntry && pServiceEntry->m_pNode != 0 &&
				pServiceEntry->m_pNode->m_Key == string(pNodeKey))
			{
				bFound = true;
				break;
			}
		} while ((pServiceEntry = QueryServiceNext()) != 0);

		if (bFound == false)
		{
			cPresence *pPresence = cCore::GetInstance()->GetPresence();
			cNodeEntry *pNode = pPresence->GetNodeFromKey(pNodeKey);
			pNode->m_TCPSocketHandle = TCPSocketHandle;

			MC_NEW(pServiceEntry, cServiceEntry());
			pServiceEntry->m_ServiceName = pServiceName;
			pServiceEntry->m_Version_Major = Version_Major;
			pServiceEntry->m_Version_Minor = Version_Minor;
			pServiceEntry->m_Version_Revision = Version_Revision;
			pServiceEntry->m_pHandler = 0;
			pServiceEntry->m_pNode = pNode;
			pServiceEntry->m_bLocal = bLocal;
			pServiceEntry->m_bRemote = bRemote;
	
			m_ServiceMap.insert(pair<string,cServiceEntry*>(pServiceName, pServiceEntry));	
			LOG("Remote service %s registered", pServiceName);
			return true;
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	catch (...)
	{
		LOGALL("exception in cService::RegisterRemoteService()");
	}

	return false;
}

void cService::UnregisterRemoteService(const char *pNodeKey, const char *pServiceName)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);

	try
	{
		cServiceEntry *pServiceEntry = QueryServiceFirst(pServiceName);
		do
		{
			if (pServiceEntry && pServiceEntry->m_pNode != 0 &&
				pServiceEntry->m_pNode->m_Key == string(pNodeKey))
			{
				delete pServiceEntry;
				m_ServiceMap.erase(m_it);
				break;
			}
		} while ((pServiceEntry = QueryServiceNext()) != 0);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

void cService::RegisterLocalService(cICommandHandler *pHandler, bool bLocal, bool bRemote)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);

	try
	{
		cServiceEntry* pServiceEntry;
		multimap<string,cServiceEntry*>::iterator it;
		string serviceName = pHandler->GetCommandName();

		//// Hardcode this for now.  Need to change plugins to hold version info
		int Version_Major = 0;
		int Version_Minor = 0;
		int Version_Revision = 0;

		it = m_ServiceMap.find(serviceName);
		if (it == m_ServiceMap.end())
		{
			// node doesn't exist, so add it.		
			MC_NEW(pServiceEntry, cServiceEntry());
			pServiceEntry->m_ServiceName = serviceName;
			pServiceEntry->m_Version_Major = Version_Major;
			pServiceEntry->m_Version_Minor = Version_Minor;
			pServiceEntry->m_Version_Revision = Version_Revision;
			pServiceEntry->m_pHandler = pHandler;
			pServiceEntry->m_pNode = 0;
			pServiceEntry->m_bLocal = bLocal;
			pServiceEntry->m_bRemote = bRemote;

			m_ServiceMap.insert(pair<string,cServiceEntry*>(serviceName, pServiceEntry));
		}
		//else
		//{
		//	pServiceEntry = (*it).second;
		//}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	catch (...)
	{
		LOGALL("exception in cService::RegisterLocalService()");
	}
}

cServiceEntry *cService::QueryServiceFirst(const char *pServiceName)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	cServiceEntry *pEntry = NULL;
	
	try
	{
		m_it = (pServiceName == NULL) ? 
			m_ServiceMap.begin() : 
			m_ServiceMap.find(pServiceName);
		pEntry = (m_it == m_ServiceMap.end()) ? 0 : (*m_it).second;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return pEntry;
}

cServiceEntry *cService::QueryServiceNext()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	cServiceEntry *pEntry = NULL;
	
	try
	{
		m_it++;
		pEntry = (m_it == m_ServiceMap.end()) ? 0 : (*m_it).second;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return pEntry;
}


