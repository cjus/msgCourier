/* mcscript-else-handler.h
   Copyright (C) 2005 Carlos Justiniano

mcscript-else-handler.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript-else-handler.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript-else-handler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript-else-handler.h
 @brief Message Courier Script Handlers
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script Handlers.
*/

#ifndef _MCSCRIPT_ELSE_HANDLER_H
#define _MCSCRIPT_ELSE_HANDLER_H

#include <string>
#include "mcscript.h"


/****************
 * else Handler *
 ***************/
class cMCElseHandler : public cMCScriptKeywordHandler
{
public:
	int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer)
	{
		cXMLLiteElement *pElm = pXML->GetElement();

		if (strcmp(pMCScript->GetLastCommand(), "if")!=0 && 
			strcmp(pMCScript->GetLastCommand(), "elseif")!=0 && 
			strcmp(pMCScript->GetLastCommand(), "else")!=0)
			return HRC_MCSCRIPT_XMLDATA_ELSECMD_MUST_FOLLOW_IFORELSEIF;
		pOutputBuffer->append(" else {");
		pMCScript->IncreaseIndentDepth();
		return 0;
	}
};

#endif //_MCSCRIPT_ELSE_HANDLER_H
