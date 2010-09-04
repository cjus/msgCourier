/* peerroutingmap.h
   Copyright (C) 2006 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

peerroutingmap.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

peerroutingmap.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mimemap.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file peerroutingmap.h
 @brief Peer Routing Map
 @author Carlos Justiniano
 @attention Copyright (C) 2006 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef PEER_ROUTING_MAP_H
#define PEER_ROUTING_MAP_H

#include <string>
#include <map>
#include "threadsync.h"

class cPeerRoutingRecord
{
public:
	std::string key;
	std::string ipAddress;
    int iPort;
    int iSocketfd;
};

/**
 @class cPeerRoutingMap
 @brief Peer Routing Map
 */
class cPeerRoutingMap
{
public:
	cPeerRoutingMap();
	~cPeerRoutingMap();

	int AddRecord(const char *pKey, const char *pIPAddress, int iPort, int iSocketfd);
	bool HasKey(const char *pKey);
	int GetSocket(const char *pKey);
	cPeerRoutingRecord *GetRecord(const char *pKey);
private:
	cThreadSync m_ThreadSync;
	std::map<std::string,cPeerRoutingRecord*> m_PeerMap;
};

#endif // PEER_ROUTING_MAP_H


