/* addonhandlers.h
   Copyright (C) 2006 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

addonhandlers.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

addonhandlers.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with addonhandler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef ADDON_HANDLER_H
#define ADDON_HANDLER_H

#include "commandhandlerfactory.h"
#include "isysinfo.h"
#include "imsgqueue.h"
#include "icommandhandler.h"

class cAddOnHandlers
{
	public:	
		cAddOnHandlers(cIMsgQueue *pIMsgQueue, cCommandHandlerFactory *pHanderFactory, cISysInfo *pISysInfo);
		~cAddOnHandlers();
		void Register();
	private:
		cIMsgQueue *m_pIMsgQueue;
		cCommandHandlerFactory *m_pHandlerFactory;
		cISysInfo *m_pISysInfo;
};

#endif //ADDON_HANDLER_H

