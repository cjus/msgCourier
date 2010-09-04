/* proccom.h
   Copyright (C) 2004 Carlos Justiniano

proccom.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

proccom.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with proccom.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file proccom.h
 @brief Process Communication
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)

 Process Communication
*/
#ifndef _PROCCOM_H
#define _PROCCOM_H

#include <string>

#include "buffer.h"

#ifdef _PLATFORM_WIN32
	#include <windows.h>
#endif //_PLATFORM_WIN32

#define HRC_PROCCOM_SUCCESS			0
#define HRC_PROCCOM_STARTUP_FAILED	1
#define HRC_PROCCOM_READ_FAILED		2
#define HRC_PROCCOM_READ_MORE		3
#define HRC_PROCCOM_READ_TIMEOUT	4
#define HRC_PROCCOM_WRITE_FAILED	5

class cProcCom
{
public:
	cProcCom();
	~cProcCom();

	int Start(const char *cmdLine, const char *dir);
	void Stop();
	void Interrupt();
	
	int Send(const char *message, int count);
	int Recv(char *buffer, int count);

	int GetBufferSize() { return m_RecvBuffer.StringLen(); } //m_RecvBuffer.GetBufferCount(); }
	char *GetBuffer() { return (m_RecvBuffer.StringLen()) ? m_RecvBuffer.cstr() : 0; }
	
private:
	bool m_start;
	cBuffer m_RecvBuffer;
	std::string m_temp;
#ifndef _PLATFORM_WIN32
	int m_pid;
	int m_fdTo;
	int m_fdFrom;
#endif //!_PLATFORM_WIN32

#ifdef _PLATFORM_WIN32
	PROCESS_INFORMATION m_ProcessInfo;
	HANDLE m_hChildRead;
	HANDLE m_hChildWrite;
	HANDLE m_hChildProcessHandle;
	DWORD  m_dwChildProcessID;
#endif //_PLATFORM_WIN32
};

#endif //_PROCCOM_H
