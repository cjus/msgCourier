/* mcscript-incr-handler.h
   Copyright (C) 2005 Carlos Justiniano

mcscript-incr-handler.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript-incr-handler.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript-incr-handler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript-incr-handler.h
 @brief Message Courier Script Handlers
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script Handlers.
*/

#ifndef _MCSCRIPT_INCR_HANDLER_H
#define _MCSCRIPT_INCR_HANDLER_H

#include <string>
#include "mcscript.h"

/****************
 * Incr Handler *
 ****************/
class cMCIncrHandler : public cMCScriptKeywordHandler
{
public:
	int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer)
	{
		cXMLLiteElement *pElm = pXML->GetElement();
		const char *pVariable = pElm->GetAttributeValue("variable");
		if (strlen(pVariable)==0)
			return HRC_MCSCRIPT_XMLDATA_INCRCMD_MISSING_VARIABLE_ATTRIBUTE;
		const char *pBy = pElm->GetAttributeValue("by");
		if (strlen(pBy)==0)
			return HRC_MCSCRIPT_XMLDATA_INCRCMD_MISSING_BY_ATTRIBUTE;

		pOutputBuffer->append("\n");
		pOutputBuffer->append(pMCScript->GetIndentTabs());

		pOutputBuffer->append("incr ");
		if (pVariable[0] == '$')
			pOutputBuffer->append(pVariable+1);
		else
			pOutputBuffer->append(pVariable);
		pOutputBuffer->append(" ");
		pOutputBuffer->append(pBy);
		return 0;
	}
};

#endif //_MCSCRIPT_INCR_HANDLER_H
