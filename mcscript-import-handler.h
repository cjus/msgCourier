/* mcscript-import-handler.h
   Copyright (C) 2005 Carlos Justiniano

mcscript-import-handler.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

mcscript-import-handler.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mcscript-import-handler.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mcscript-import-handler.h
 @brief Message Courier Script Handlers
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Message Courier Script Handlers.
*/

#ifndef _MCSCRIPT_IMPORT_HANDLER_H
#define _MCSCRIPT_IMPORT_HANDLER_H

#include <string>
#include "mcscript.h"

/******************
 * Import Handler *
 ******************/
class cMCImportHandler : public cMCScriptKeywordHandler
{
public:
	int Process(cMCScript *pMCScript, cXMLLiteParser *pXML, std::string *pOutputBuffer)
	{
		cXMLLiteElement *pElm = pXML->GetElement();
		const char *pFile = pElm->GetAttributeValue("file");
		if (strlen(pFile)==0)
			return HRC_MCSCRIPT_XMLDATA_IMPORTCMD_MISSING_FILE_ATTRIBUTE;

		std::string sText;
		char buf[2048];
		int iread;
		FILE *fp = fopen(pFile,"rt");
		if (fp)
		{
			while ((iread = fread(buf,1,2048,fp)))
				sText.append(buf, iread);
			pOutputBuffer->append("\n");
			pOutputBuffer->append(sText.c_str());
		}
		else
		{
			return HRC_MCSCRIPT_XMLDATA_IMPORTCMD_FILELOAD_FAILED;
		}
		fclose(fp);
		return 0;
	}
};

#endif //_MCSCRIPT_IMPORT_HANDLER_H
