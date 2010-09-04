/* service.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

service.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

service.h was developed by Carlos Justiniano for use on the
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
 @file service.h
 @brief Service managment
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef SERVICE_H
#define SERVICE_H

#include <string>
#include <map>

#include "icommandhandler.h"
#include "presence.h"
#include "threadsync.h"

class cServiceEntry
{
public:
	std::string m_ServiceName;
	int m_Version_Major;
	int m_Version_Minor;
	int m_Version_Revision;
	cICommandHandler *m_pHandler;
	cNodeEntry *m_pNode;
	bool m_bLocal;
	bool m_bRemote;
};

class cService
{
public:
	cService();
	~cService();

	bool RegisterRemoteService(const char *pNodeKey, const char *pServiceName, 
								int Version_Major, int Version_Minor, int Version_Revision,
								bool bLocal, bool bRemote, int TCPSocketHandle);
	void UnregisterRemoteService(const char *pNodeKey, const char *pServiceName);

	void RegisterLocalService(cICommandHandler *pHandler, bool bLocal, bool bRemote);

	cServiceEntry *QueryServiceFirst(const char *pServiceName);
	cServiceEntry *QueryServiceNext();

private:
	cThreadSync m_ThreadSync;
	std::multimap<std::string, cServiceEntry*> m_ServiceMap;
	std::multimap<std::string, cServiceEntry*>::iterator m_it;
};

#endif // SERVICE_H
