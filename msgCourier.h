/* msgCourier.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

msgCourier.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

msgCourier.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with msgCourier.h; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file msgCourier.h
 @brief Built in handler for MsgCourier messages
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
 Built in handler for MsgCourier messages
*/

#ifndef MSGCOURIER_H
#define MSGCOURIER_H

#include "msgCourierPlugin.h"

class cMsgCourier : cICommandHandler
{
public:
	char *GetCommandName();

	void OnStartup(cIMsgQueue *pMsgQueue, cISysInfo *pSysInfo, cILogger *pILogger);
	int  OnProcess(cIMsg *pMessage);
	const char *OnProcessInternal(const char *pMessage);
	void OnShutdown();
private:
	cIMsgQueue *m_pMsgQueue;
	cISysInfo *m_pSysInfo;
	void HandleXoolleMessage(cIMsg* pMessage);
};

#endif //MSGCOURIER_H

