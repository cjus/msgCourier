/* parsemarkup.cpp
   Copyright (C) 2006 Carlos Justiniano

parsemarkup.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

parsemarkup.cpp was developed by Carlos Justiniano for use on the
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
 @file parsemarkup.cpp
 @brief Parses markup to extract text and other useful information
 @author Carlos Justiniano
 @attention Copyright (C) 2006 Carlos Justiniano, GNU GPL Licence (see source file header)
 Parses markup to extract text and other useful information
*/

#include "parsemarkup.h"

using namespace std;

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find.hpp>

using namespace boost;

cParseMarkup::cParseMarkup(void)
{
}

cParseMarkup::~cParseMarkup(void)
{	
}

void cParseMarkup::ParseFile(const char *pFileName)
{
	char buf[2048];
	int iread;
	cBuffer buffer;

	FILE *fp = fopen(pFileName, "rt");
	if (fp)
	{
		while ((iread = fread(buf,1,2048,fp)))
		{
			buffer.Append(buf, iread);
		}
		fclose(fp);
	}
	Parse(buffer.c_str());
}

void cParseMarkup::Parse(const char *pData, bool bLightParse)
{
	if (pData == 0)
		return;
	cBuffer temp;
	cBuffer token;

	m_Buffer.ReplaceWith((char*)pData);
	if (!bLightParse)
		m_Buffer.ToLower();

	char *p = m_Buffer.cstr();
	int length = m_Buffer.StringLen();
	int loc1, loc2 = 0;
	int mark = 0;

	m_Buffer.ReplaceAll("&lt;", "<");
	m_Buffer.ReplaceAll("&gt;", ">");

	m_Buffer.ReplaceAll(">", "> ");

	// Parse <title> and <meta> tags
	while ((loc1 = m_Buffer.FindChar('<', mark)) > -1)
	{
		mark = loc1;
		loc2 = m_Buffer.FindChar('>', mark);
		if (loc2 != -1)
		{
			token.ReplaceWith(m_Buffer.cstr() + loc1, loc2 - mark + 1);

			if (strnicmp(token.c_str(), "<style", 6)==0 || strnicmp(token.c_str(), "</style", 7)==0 || 
				strnicmp(token.c_str(), "<script", 7)==0 || strnicmp(token.c_str(), "</script", 8)==0 ||
				strnicmp(token.c_str(), "<a ", 3)==0)
			{
				mark += 1;
				continue; 
			}

			if (strcmp(token.c_str(), "<title>")==0)
			{
				temp.ReplaceWith(m_Buffer.cstr() + loc1 + token.StringLen(), 
								m_Buffer.FindChar('<', loc1 + token.StringLen()) - (loc1 + token.StringLen()));
				if (temp.GetBufferCount() < 1)
					continue;
				m_Title = temp;
			}
			if (strnicmp(token.c_str(), "<meta",5)==0)
			{
				if (strstr(token.c_str(), "\"keywords\"") != NULL)
				{
					char *pContent = strstr(token.c_str(), "content=\"");
					if (pContent != NULL)
					{
						int len = strstr(pContent + 9, "\"") - (pContent + 9);
						m_MetaContentKeywords.append(pContent + 9, len);
					}
				}
			}
			mark = 0;

			m_Buffer.ReplaceAll(token.cstr(), "");
		}
		else
		{
			break;
		}
	}

	// append keywords
	if (!bLightParse && m_MetaContentKeywords.length() > 0)
	{
		m_Buffer.Append(" ", 1);
		m_Buffer.Append((char*)m_MetaContentKeywords.c_str(), m_MetaContentKeywords.length());
	}

	mark = 0;

	// Parse rest of text
	while ((loc1 = m_Buffer.FindChar('<', mark)) > -1)
	{
		mark = loc1;
		loc2 = m_Buffer.FindChar('>', mark);
		if (loc2 != -1)
		{
			token.ReplaceWith(m_Buffer.cstr() + loc1, loc2 - mark + 1);			

			if (strnicmp(token.c_str(), "<style", 6)==0)
			{
				int endtag = m_Buffer.Find("</style>", loc1);
				if (endtag == -1)
					endtag = loc2;
				if (endtag != -1)
					m_Buffer.Remove(loc1, (endtag - loc1) + 8);
				continue;
			}

			if (strnicmp(token.c_str(), "<script", 7)==0)
			{
				int endtag = m_Buffer.Find("</script>", loc2);
				if (endtag == -1)
					endtag = loc2;
				if (endtag != -1)
					m_Buffer.Remove(loc1, (endtag - loc1) + 8);
				continue;
			}
			
			if (strnicmp(token.c_str(), "<a ", 3)==0)
			{
				char *p = strstr(token.c_str(), "href=\"");
				if (p != NULL)
				{
					if (strnicmp(p+6, "http://", 7) != 0)
					{
						temp.ReplaceWith((char *)m_sBaseURL.c_str());
						temp.Append(p + 6);
					}
					else
					{
						temp.ReplaceWith(p + 6);
					}
					if (temp.FindChar('"') > 0)
						temp.Truncate(temp.StringLen() - temp.FindChar('"'));
					temp.ReplaceAll("//","/");
					temp.Replace("http:/","http://");
					temp.Replace("http:"," ");
					temp.Replace("www.www.","www.");

					temp.Cleanup();

					m_URIMAP.insert(pair<string,string>(string(temp.c_str()), string(temp.c_str())));
				}
				m_Buffer.ReplaceAll(token.cstr(), "");
			}
			else
			{
				m_Buffer.ReplaceAll(token.cstr(), "");
			}
		}
		else
		{
			break;
		}
	}

	if (!bLightParse)
	{
		// append title 
		m_Buffer.Append(" ", 1);
		m_Buffer.Append((char*)m_Title.c_str(), m_Title.length());

		// append website url as name
		m_Buffer.Append(" ", 1);
		m_Buffer.Append((char*)m_sBaseURL.c_str(), m_sBaseURL.length());
	}
	
	if (!bLightParse)
	{
		// remove commons
		m_Buffer.ReplaceAll("http", "");
		m_Buffer.ReplaceAll("www", "");
		m_Buffer.ReplaceAll(".com", "");
		m_Buffer.ReplaceAll(".net", "");
		m_Buffer.ReplaceAll(".org", "");
	}

	if (!bLightParse)
		m_Buffer.ReplaceAll(".", " ");

	// remove other characters
	if (!bLightParse)
	{
		m_Buffer.ReplaceAll("\t", " ");
		m_Buffer.ReplaceAll("\r\n", " ");
		m_Buffer.ReplaceAll("\n", " ");
		m_Buffer.ReplaceAll("&apos;", "'");
		m_Buffer.ReplaceAll("&nbsp;", " ");
		m_Buffer.ReplaceAll("&amp;", "&");
		m_Buffer.ReplaceAll("&raquo;", " ");
		m_Buffer.ReplaceAll("&quot;","\"");
	}
	else 
	{
		m_Buffer.ReplaceAll("\t", " ");
		m_Buffer.ReplaceAll("\r\n", " ");
		m_Buffer.ReplaceAll("\n", " ");
		m_Buffer.ReplaceAll("&apos;", "'");
		m_Buffer.ReplaceAll("&nbsp;", " ");
		m_Buffer.ReplaceAll("&amp;", "&");
		m_Buffer.ReplaceAll("&raquo;", " ");
		m_Buffer.ReplaceAll("&quot;","\"");
	}


	m_WordCount = 0;
	
	string word;
	string s = m_Buffer.c_str();
	m_Buffer.Reset();

	typedef tokenizer<char_separator<char> > tokenizer;
	if (!bLightParse)
	{
		char_separator<char> sep(" |<>[]{}?:&+-#$`~()@%^=;,!\\/'\"");
		tokenizer tokens(s, sep);
		for (tokenizer::iterator tok_iter = tokens.begin();
			tok_iter != tokens.end(); ++tok_iter)
		{
			word = *tok_iter;
			m_Buffer.Append((char*)word.c_str());
			m_Buffer.Append((char*)" ");
			m_WordCount++;
		}
	}
	else
	{
		char_separator<char> sep(" |<>[]{}$()^");
		tokenizer tokens(s, sep);
		for (tokenizer::iterator tok_iter = tokens.begin();
			tok_iter != tokens.end(); ++tok_iter)
		{
			word = *tok_iter;
			m_Buffer.Append((char*)word.c_str());
			m_Buffer.Append((char*)" ");
			m_WordCount++;
		}
	}

	m_Text = m_Buffer.c_str();
}

const char *cParseMarkup::GetFirstURL()
{
	const char *p = NULL;
	m_URIMAPIT = m_URIMAP.begin();
	if (m_URIMAPIT != m_URIMAP.end())
	{
		p = (*m_URIMAPIT).first.c_str();
		m_URIMAPIT++;
	}
	return p;
}

const char *cParseMarkup::GetNextURL()
{
	const char *p = NULL;
	if (m_URIMAPIT != m_URIMAP.end())
	{
		p = (*m_URIMAPIT).first.c_str();
		m_URIMAPIT++;
	}
	return p;
}
