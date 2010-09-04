/* mcscript-while-handler.h
   Copyright (C) 2005 Carlos Justiniano

mcscript-while-handler.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript-while-handler.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript-while-handler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript-while-handler.h
 @brief Message Courier Script Handlers
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script Handlers.
*/

#ifndef _MCSCRIPT_WHILE_HANDLER_H
#define _MCSCRIPT_WHILE_HANDLER_H

#include <string>
#include "mcscript.h"

/*****************
 * While Handler *
 *****************/
class cMCWhileHandler : public cMCScriptKeywordHandler
{
public:
	int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer)
	{
		cXMLLiteElement *pElm = pXML->GetElement();
		const char *pExpression = pElm->GetAttributeValue("test");
		if (strlen(pExpression)==0)
			return HRC_MCSCRIPT_XMLDATA_WHILECMD_MISSING_TEST_ATTRIBUTE;
		pOutputBuffer->append("\n");
		pOutputBuffer->append(pMCScript->GetIndentTabs());

		pOutputBuffer->append("while {");
		pOutputBuffer->append(pExpression);
		pOutputBuffer->append("} {");
		pMCScript->IncreaseIndentDepth();
		return 0;
	}
};

#endif //_MCSCRIPT_WHILE_HANDLER_H
