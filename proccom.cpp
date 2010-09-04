/* proccom.cpp
   Copyright (C) 2004 Carlos Justiniano

proccom.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

proccom.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with proccom.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file proccom.cpp
 @brief Process Communication
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)

 Process Communication
*/
#include <stdio.h>
#include <stdlib.h>

#include "proccom.h"

#ifdef _PLATFORM_WIN32
	#include <direct.h>
#endif //_PLATFORM_WIN32

#ifndef _PLATFORM_WIN32
	#include <signal.h>
	#include <unistd.h>
	#include <sys/wait.h>
	#include <errno.h>
	#include <fcntl.h>
#endif //!_PLATFORM_WIN32

#include <string.h>
#include <time.h>
#include <assert.h>
#include "log.h"

cProcCom::cProcCom()
: m_start(false)
{
}

cProcCom::~cProcCom()
{
}

int cProcCom::Start(const char *cmdLine, const char *dir)
{
#ifndef _PLATFORM_WIN32
	char buf[256];
	int to_prog[2], from_prog[2];
	char *argv[64], *p;
	int i, pid;

	// build blank-seperated argument list
	//
	i = 0;
	strcpy(buf, cmdLine);
	p = buf;
	for (;;)
	{
		argv[i++] = p;
		p = strchr(p, ' ');
		if (p == NULL)
			break;
		*p++ = '\0';
	}
	argv[i] = NULL;

	// Setup child IO
	//
	signal(SIGPIPE, SIG_IGN);
	pipe(to_prog);
	pipe(from_prog);

	if ((pid = fork()) == 0)
	{
		// Child process
		dup2(to_prog[0], 0);
		dup2(from_prog[1], 1);
		close(to_prog[0]);
		close(to_prog[1]);
		close(from_prog[0]);
		close(from_prog[1]);
		dup2(1, fileno(stderr)); // force stderr to the pipe
		if (dir[0] != '\0' && chdir(dir) != 0)
			return HRC_PROCCOM_STARTUP_FAILED;

		//printf("CP: %s, %s\n", argv[0], dir);
		execvp(argv[0], argv);

		// If we get here, exec failed 
		return HRC_PROCCOM_STARTUP_FAILED;
	}

	// Parent process
	close(to_prog[0]);
	close(from_prog[1]);

	m_pid = pid;
	m_fdFrom = from_prog[0];
	m_fdTo = to_prog[1];

	// modify file descriptors to be nonblocking
	if (fcntl(m_fdFrom, F_SETFL, O_NONBLOCK) != 0)
		return HRC_PROCCOM_STARTUP_FAILED;
	if (fcntl(m_fdTo, F_SETFL, O_NONBLOCK) != 0)
		return HRC_PROCCOM_STARTUP_FAILED;
#else
	char curdir[_MAX_PATH];

	/* Get the current working directory: */
	_getcwd(curdir, _MAX_PATH);
#ifdef _BORLAND
	chdir(dir);
#else
	_chdir(dir);
#endif //_BORLAND

    HANDLE	hChildStdinRd, hChildStdinWr,
			hChildStdoutRd, hChildStdoutWr;
    HANDLE	hChildStdinWrDup, hChildStdoutRdDup;
    SECURITY_ATTRIBUTES saAttr;
    BOOL fSuccess;

    STARTUPINFO siStartInfo;

    // Set the bInheritHandle flag so pipe handles are inherited.
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    
    // The steps for redirecting child's STDOUT:
    //     1. Create anonymous pipe to be STDOUT for child.
    //     2. Create a noninheritable duplicate of read handle,
    //         and close the inheritable read handle.
    //

    // Create a pipe for the child's STDOUT.
    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0)) 
	{
		// return GetLastError();
		return HRC_PROCCOM_STARTUP_FAILED;
    }

    // Duplicate the read handle to the pipe, so it is not inherited.
    fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdoutRd,
							   GetCurrentProcess(), &hChildStdoutRdDup, 0,
							   FALSE,	// not inherited
							   DUPLICATE_SAME_ACCESS);
    if (!fSuccess) 
	{
		// return GetLastError();
   		return HRC_PROCCOM_STARTUP_FAILED;
	}
    CloseHandle(hChildStdoutRd);


    // The steps for redirecting child's STDIN:
    //     1. Create anonymous pipe to be STDIN for child.
    //     2. Create a noninheritable duplicate of write handle,
    //         and close the inheritable write handle.

    // Create a pipe for the child's STDIN.
    if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0)) 
	{
		//return GetLastError();
 		return HRC_PROCCOM_STARTUP_FAILED;
    }

    // Duplicate the write handle to the pipe, so it is not inherited.
    fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdinWr,
			       GetCurrentProcess(), &hChildStdinWrDup, 0,
			       FALSE,	// not inherited
			       DUPLICATE_SAME_ACCESS);
    if (!fSuccess) 
	{
		// return GetLastError();
 		return HRC_PROCCOM_STARTUP_FAILED;
   }
    CloseHandle(hChildStdinWr);

    // Create the child process.
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.lpReserved = NULL;
    siStartInfo.lpDesktop = NULL;
    siStartInfo.lpTitle = NULL;
    siStartInfo.dwFlags = STARTF_USESTDHANDLES;
    siStartInfo.cbReserved2 = 0;
    siStartInfo.lpReserved2 = NULL;
    siStartInfo.hStdInput = hChildStdinRd;
    siStartInfo.hStdOutput = hChildStdoutWr;
    siStartInfo.hStdError = hChildStdoutWr;

    fSuccess = CreateProcess(NULL,
							 (LPSTR)(char*)cmdLine,		// command line
							 NULL,			// process security attributes
							 NULL,			// primary thread security attrs
							 TRUE,			// handles are inherited 
							 DETACHED_PROCESS | CREATE_NEW_PROCESS_GROUP,
							 NULL,			// use parent's environment
							 NULL,			// use parent's current directory
							 &siStartInfo,	// STARTUPINFO pointer
							 &m_ProcessInfo); // receives PROCESS_INFORMATION

    if (!fSuccess) 
	{
		//return GetLastError();
		return HRC_PROCCOM_STARTUP_FAILED;
    }

    // Close the handles we don't need in the parent
    CloseHandle(hChildStdinRd);
    CloseHandle(hChildStdoutWr);

//    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
//			  ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT |
//			  ENABLE_ECHO_INPUT);


	// save handles for use with read, write of pipe and shutdown of external app
	m_hChildRead = hChildStdoutRdDup;
	m_hChildWrite = hChildStdinWrDup;
	m_dwChildProcessID = m_ProcessInfo.dwProcessId;
	m_hChildProcessHandle  = m_ProcessInfo.hProcess;

	// restore current directory
#ifdef _BORLAND
	chdir(curdir);
#else
	_chdir(curdir);
#endif //_BORLAND

#endif //_PLATFORM_WIN32

	m_start = true;
	return HRC_PROCCOM_SUCCESS;
}

void cProcCom::Stop()
{
#ifndef _PLATFORM_WIN32
	kill(m_pid, SIGTERM);
	wait((int *)0);
	close(m_fdFrom);
	close(m_fdTo);
#else
	CloseHandle(m_hChildRead);
	CloseHandle(m_hChildWrite);
	// TerminateProcess is considered harmful, so... send ctrl-c !
	GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, m_dwChildProcessID);
	CloseHandle(m_hChildProcessHandle);
#endif //_PLATFORM_WIN32
}

void cProcCom::Interrupt()
{
#ifndef _PLATFORM_WIN32
	kill(m_pid, SIGINT);
#else
	GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, m_dwChildProcessID);
#endif //_PLATFORM_WIN32
}

int cProcCom::Send(const char *message, int count)
{
	m_temp = message;
	//m_temp.Chomp();

	if (m_temp.length() > 3)
	{
		if (!m_start)
		{
			LOG("Attempt to send message to process): [%s]", m_temp.c_str());
			LOG("...but connection to process is not established!");
		}
	}
	//LTS();
	const char *pts = message;
	int outCount = 0;
#ifndef _PLATFORM_WIN32
	int n;
	while (outCount != count)
	{
		n = write(m_fdTo, pts+outCount, count-outCount);
		if (n < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			{
				continue;
			}
			else
			{
				LOG("cProcCom: Unable to write: %d", errno);
				return HRC_PROCCOM_WRITE_FAILED;
			}
		}
		outCount += n;
	}
#else
	DWORD dwCount;
	BOOL bSuccess;
	while (outCount != count)
	{
		bSuccess = WriteFile(m_hChildWrite, (LPSTR)pts+outCount, count-outCount, &dwCount, 0);
		if (!bSuccess)
		{
			if (GetLastError() != ERROR_BROKEN_PIPE)
			{
				continue;
			}
			else
			{
				LOG("cProcCom: Unable to write: %d", errno);
				return HRC_PROCCOM_WRITE_FAILED;
			}
		}
		outCount += dwCount;
	}
#endif //_PLATFORM_WIN32

	return HRC_PROCCOM_SUCCESS;
}

int cProcCom::Recv(char *buffer, int count)
{
	int grab;

    /*
	int delay = 1; // in seconds
	time_t start, end;
	time(&start);
	end = start;
    */

	grab = count;
	m_RecvBuffer.Reset();

#ifndef _PLATFORM_WIN32
//	while (1)
//	{
//		if ((start + delay) <= end)
// 		{
// 			return HRC_PROCCOM_READ_TIMEOUT;
// 		}

		int recvcount = read(m_fdFrom, buffer, grab);
		if (recvcount > 0)
		{
			m_RecvBuffer.Append(buffer, recvcount);
			if (recvcount == grab)
			{
				return HRC_PROCCOM_SUCCESS;
			}
			else
			{
				grab -= recvcount;
			}
			assert(grab > 0);
		}
		else if (recvcount < 0)
		{
			//printf("errno = %d (%s)\n", errno, strerror(errno));
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			{
				return HRC_PROCCOM_SUCCESS;
			}
			return HRC_PROCCOM_READ_FAILED;
		}
//		else if (recvcount == 0)
//		{
//			break;
//		}
//		usleep(10*1000);
//		time(&end);
//	}
#else
	DWORD dwTotalBytesAvail;
	DWORD dwCount;
	BOOL bSuccess;
//	while (1)
//	{
//		if ((start + delay) <= end)
//		{
//			return HRC_PROCCOM_READ_TIMEOUT;
//		}

		if (::PeekNamedPipe(m_hChildRead, NULL, 0, NULL, &dwTotalBytesAvail, NULL))
		{
			if (dwTotalBytesAvail != 0)
			{
				bSuccess = ReadFile(m_hChildRead, buffer, grab, &dwCount, NULL);
				if (!bSuccess && (GetLastError() == ERROR_BROKEN_PIPE))
				{
					return HRC_PROCCOM_READ_FAILED;
				}
				if (dwCount > 0)
				{
					m_RecvBuffer.Append(buffer, dwCount);
					if (dwCount == (DWORD)grab)
					{
						return HRC_PROCCOM_SUCCESS;
					}
					else
					{
						grab -= dwCount;
					}
					assert(grab > 0);
				}
			}
		}

//		::Sleep(1);
//		time(&end);
//	}
#endif //_PLATFORM_WIN32
	return HRC_PROCCOM_SUCCESS;
}

