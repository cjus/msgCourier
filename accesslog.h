/* accesslog.h
   Copyright (C) 2005 Carlos Justiniano

accesslog.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

accesslog.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with accesslog.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file accesslog.h 
 @brief cAccessLog handles network access logging
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 cAccessLog handles network access logging
*/

#ifndef _ACCESSLOG_H
#define _ACCESSLOG_H

#include <vector>
#include <string>

#include "thread.h"
#include "threadsync.h"
#include "timer.h"
#include "imsg.h"

#define HRC_ACCESSLOG_OK                  0x0000


class cAccessRecord
{
public:
	cAccessRecord();
	cAccessRecord(const cAccessRecord &ar);
	virtual ~cAccessRecord();
	const cAccessRecord& operator=(const cAccessRecord &ar);

	void Init(const int timestamp,
				const cIMsg::eMsgProtocolType protocol, 
				const int priority, 
				const char* command,
				const char* activity,
				const char* ipaddress);
	void toString(std::string &s);
protected:
	int m_EventTimestamp;
	cIMsg::eMsgProtocolType m_Protocol;
	int m_Priority;
	std::string m_Command;
	std::string m_Activity;
	std::string m_IPAddress;
};

/**
 @class cAccessLog 
 @brief cAccessLog handles system metrics logging
*/
class cAccessLog : public cThread
{
public:
    cAccessLog();
    virtual ~cAccessLog();

	int Start();
	int Stop();

	void AddEntry(const cIMsg::eMsgProtocolType protocol, 
					const int priority, 
					const char* command,
					const char* activity,
					const char* ipaddress);
	void GetEntries(std::vector<cAccessRecord> &records) const;

protected:
    int Run();

private:
	mutable cThreadSync m_ThreadSync;
	cTimer m_Timer;

	int m_recIndex;
	std::vector<cAccessRecord> m_records;
	bool m_recycle;
};

#endif //_ACCESSLOG_H

