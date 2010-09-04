/* accesslog.cpp
   Copyright (C) 2005 Carlos Justiniano

accesslog.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

accesslog.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with accesslog.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file accesslog.cpp 
 @brief cAccessLog handles network access logging
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 cAccessLog handles network access logging
*/
#define ACCESSLOG_MAX_ENTRIES	256

#include <time.h>
#include <sstream>
#include "accesslog.h"
#include "exception.h"
#include "log.h"
#include <boost/format.hpp>
using namespace std;
using boost::format;

cAccessRecord::cAccessRecord()
{
	m_EventTimestamp = 0;
}

cAccessRecord::cAccessRecord(const cAccessRecord &ar)
{
	*this = ar;
}

cAccessRecord::~cAccessRecord()
{
}

const cAccessRecord& cAccessRecord::operator=(const cAccessRecord &ar)
{
	if (this != &ar)
	{
		try
		{	
			m_EventTimestamp = ar.m_EventTimestamp;
			m_Protocol = ar.m_Protocol;
			m_Priority = ar.m_Priority;
			m_Command = ar.m_Command;
			m_Activity = ar.m_Activity;
			m_IPAddress = ar.m_IPAddress;
		}
		catch (exception const &e)
		{
			LOGALL(e.what());
		}
	}
	return *this;
}

void cAccessRecord::Init(const int timestamp,
						const cIMsg::eMsgProtocolType protocol, 
						const int priority, 
						const char* command,
						const char* activity,
						const char* ipaddress)
{
	try
	{	
		m_EventTimestamp = timestamp;
		m_Protocol = protocol;
		m_Priority = priority;
		m_Command = command;
		m_Activity = activity;
		m_IPAddress = ipaddress;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

void cAccessRecord::toString(std::string &s)
{
	try
	{	
		string protocol;
		switch (m_Protocol)
		{
			case cIMsg::MSG_PROTOCOL_HTTP_MCP:
				protocol = "HTTP_MCP";
				break;
			case cIMsg::MSG_PROTOCOL_MCP:
				protocol = "MCP";
				break;
			case cIMsg::MSG_PROTOCOL_HTTP_GET:
				protocol = "HTTP_GET";
				break;
			case cIMsg::MSG_PROTOCOL_HTTP_POST:
				protocol = "HTTP_POST";
				break;
		};

		time_t t = m_EventTimestamp;
		tm *t2 = localtime(&t);
		format formater("%02d%02d%02d-%02d%02d%02d,%s,%s,%s,%s");
		formater % (t2->tm_year-100) % (t2->tm_mon+1) % t2->tm_mday % t2->tm_hour % t2->tm_min % t2->tm_sec;
		formater % m_Command %
				 protocol %
				 m_IPAddress %
			 	m_Activity;
		s = formater.str();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

cAccessLog::cAccessLog()
: m_recIndex(0)
, m_recycle(false)
{
}

cAccessLog::~cAccessLog()
{
}

int cAccessLog::Run()
{
	m_Timer.SetInterval(1);
	m_Timer.Start();

    while (ThreadRunning())
    {
		m_ThreadSync.Lock();

        if (m_Timer.IsReady())
		{
			m_Timer.Reset();
			// TODO: do something useful here.
		}

		m_ThreadSync.Unlock();
		this->Sleep(250);
    }
    return HRC_ACCESSLOG_OK;
}

int cAccessLog::Start()
{
	cThread::Create();
	cThread::Start();
	return HRC_ACCESSLOG_OK;
}

int cAccessLog::Stop()
{
	cThread::Destroy();
	return HRC_ACCESSLOG_OK;
}

void cAccessLog::AddEntry(
						const cIMsg::eMsgProtocolType protocol, 
						const int priority, 
						const char* command,
						const char* activity,
						const char* ipaddress)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync); 

	try
	{
		int i = m_records.size();
		if (!m_recycle && (m_records.size() < ACCESSLOG_MAX_ENTRIES))
		{
			cAccessRecord rec;
			rec.Init(time(0), protocol, priority, command, activity, ipaddress);
			m_records.push_back(rec);
			if (m_records.size() >= ACCESSLOG_MAX_ENTRIES)
				m_recycle = true;
		}
		else
		{
			cAccessRecord &rec = m_records[m_recIndex];
			rec.Init(time(0), protocol, priority, command, activity, ipaddress);
		}

		if (++m_recIndex == ACCESSLOG_MAX_ENTRIES)
		{
			// TODO: flush to datdabase...
			m_recIndex = 0;
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

void cAccessLog::GetEntries(std::vector<cAccessRecord> &records) const
{
	cAutoThreadSync ThreadSync(&m_ThreadSync); 

	try
	{
		records.erase(records.begin(), records.end());
		int i;
		if (m_recycle)
		{
			for (i = m_recIndex+1; i < m_recIndex; i++)
				records.push_back(m_records[i]);
			for (i = 0; i < m_recIndex; i++)
				records.push_back(m_records[i]);
		}
		else
		{
			for (i = m_recIndex-1; i != 0; i--)
				records.push_back(m_records[i]);
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

