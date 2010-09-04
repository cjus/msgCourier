/* sexprwrapper.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

sexprwrapper.h is a wrapper for SFSEXP: A Small, Fast S-Expression 
Library written by Matthew Sottile (matt@lanl.gov)

sexprwrapper.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

sexprwrapper.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with sexprwrapper.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file sexprwrapper.h
 @brief  S-Expression Wrapper
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/
#ifndef SEXPR_H
#define SEXPR_H

#include "sexp.h"
#include <string>

typedef void* SEXPR_NODE;

#define HRC_SEXPR_OK		0x0000
#define HRC_PARSE_FAILED	0x0001

class cSExpr
{
public:
	cSExpr();
	~cSExpr();

	enum { MAX_BUFFER_SIZE = 32000 };

	int Parse(std::string &expression);
	const char *GetSExpr();

	SEXPR_NODE BFSFind(std::string &atom, SEXPR_NODE node = 0);
	SEXPR_NODE DFSFind(std::string &atom, SEXPR_NODE node = 0);

	const char* GetAtomValue(SEXPR_NODE node = 0);
	SEXPR_NODE GetNextAtom(SEXPR_NODE node);
	SEXPR_NODE GetNextList(SEXPR_NODE node);
	SEXPR_NODE GetBaseNode();
	SEXPR_NODE GetNextNode(SEXPR_NODE node);

	SEXPR_NODE AddList(SEXPR_NODE node);
	SEXPR_NODE AddAtom(SEXPR_NODE node, std::string &atom);

	bool HasAtom(SEXPR_NODE node);
	bool HasList(SEXPR_NODE node);

	void Reset();
private:
	sexp_t *m_sx;
	char m_buffer[MAX_BUFFER_SIZE];
};

#endif //SEXPR_H
