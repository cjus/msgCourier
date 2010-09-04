/* scriptenginebase.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

scriptenginebase.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

scriptenginebase.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with scriptenginebase.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file scriptenginebase.h
 @brief Base class for TCL Scripting Engine wrapper
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
 Base class for TCL Scripting Engine wrapper
*/

#ifndef SCRIPTENGINEBASE_H
#define SCRIPTENGINEBASE_H

#include <string>

class cScriptEngineBase
{
public:
	virtual bool Execute(std::string &sNamespace, std::string &sScriptInput, std::string &sScriptOutput, bool silent) = 0;
};

#endif // SCRIPTENGINEBASE_H

