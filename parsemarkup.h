/* parsemarkup.h
   Copyright (C) 2006 Carlos Justiniano

parsemarkup.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

parsemarkup.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with parsemarkup.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file parsemarkup.h
 @brief Parses markup to extract text and other useful information
 @author Carlos Justiniano
 @attention Copyright (C) 2006 Carlos Justiniano, GNU GPL Licence (see source file header)
 Parses markup to extract text and other useful information
*/

#ifndef PARSEMARKUP_H
#define PARSEMARKUP_H

#include "buffer.h"
#include <string>
#include <map>

class cParseMarkup
{
public:
	cParseMarkup(void);
	~cParseMarkup(void);

	void SetBaseURL(const char *pBaseURL) { m_sBaseURL = pBaseURL; }
	void ParseFile(const char *pFileName);
	void Parse(const char *pData, bool bLightParse = false);

	const char *GetText() { return m_Text.c_str(); }
	const char *GetTitle() { return m_Title.c_str(); }

	const char *GetFirstURL();
	const char *GetNextURL();

	int GetWordCount() { return m_WordCount; }
	
private:
	std::map<std::string, std::string> m_URIMAP;
	std::map<std::string, std::string>::iterator m_URIMAPIT;

	std::string m_sBaseURL;
	std::string m_MetaContentKeywords;
	std::string m_Text;
	std::string m_Title;
	cBuffer m_Buffer;
	int m_WordCount;
	
};

#endif //PARSEMARKUP_H
