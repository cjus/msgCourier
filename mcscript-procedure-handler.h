/* mcscript-procedure-handler.h
   Copyright (C) 2005 Carlos Justiniano

mcscript-procedure-handler.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript-procedure-handler.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript-procedure-handler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript-procedure-handler.h
 @brief Message Courier Script Handlers
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script Handlers.
*/

#ifndef _MCSCRIPT_PROCEDURE_HANDLER_H
#define _MCSCRIPT_PROCEDURE_HANDLER_H

#include <string>
#include "mcscript.h"

/*********************
 * Procedure Handler *
 *********************/
class cMCProcedureHandler : public cMCScriptKeywordHandler
{
public:
	int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer)
	{
		cXMLLiteElement *pElm = pXML->GetElement();
		const char *pProcName = pElm->GetAttributeValue("name");
		if (strlen(pProcName)==0)
			return HRC_MCSCRIPT_XMLDATA_PROCEDURECMD_MISSING_PROCNAME_ATTRIBUTE;
		const char *pParamName = pElm->GetAttributeValue("param");
		if (strlen(pParamName)==0)
			return HRC_MCSCRIPT_XMLDATA_PROCEDURECMD_MISSING_PARAMNAME_ATTRIBUTE;

		const char *pRetVar = pElm->GetAttributeValue("retvar");
		bool bHasRetVar = false;
		if (strlen(pRetVar)!=0)
			bHasRetVar = true;

		pOutputBuffer->append("\n");
		pOutputBuffer->append(pMCScript->GetIndentTabs());

		pOutputBuffer->append("proc ");
		pOutputBuffer->append(pProcName);
		pOutputBuffer->append(" {");
		pOutputBuffer->append(pParamName);
		pOutputBuffer->append("} {");
		pMCScript->IncreaseIndentDepth();

		std::string sProcName = pProcName;
		pMCScript->AddProcedure(sProcName);
		return 0;
	}
};


#endif //_MCSCRIPT_PROCEDURE_HANDLER_H
