/* dotengine.h
   Copyright (C) 2005 Carlos Justiniano

dotengine.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

dotengine.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with dotengine.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file dotengine.h 
 @brief cDotEngine Invokes and serializes access to ATT's GraphViz
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 cDotEngine Invokes and serializes access to ATT's GraphViz
*/

#ifndef _DOTENGINE_H
#define _DOTENGINE_H

#include <string>
#include "thread.h"
#include "threadsync.h"
#include "timer.h"
#include "proccom.h"

#define HRC_DOTENGINE_OK					0x0000
#define HRC_DOTENGINE_INVALID_OUTPUT_FORMAT	0x0001
#define HRC_DOTENGINE_UNABLE_TO_LAUNCH_DOT	0x0002
#define HRC_DOTENGINE_UNABLE_TO_COMMUNICATE_WITH_DOT	0x0003

/**
 @class cDotEngine 
 @brief cDotEngine Invokes and serializes access to ATT's GraphViz
*/
class cDotEngine : public cThread
{
public:
	cDotEngine(std::string &dotpath, std::string &WebDotFolder);
    ~cDotEngine();

	int Start();
	int Stop();

	typedef enum IMAGE_FORMATS
	{
		DOT_GIF,	// Graphics Interchange Format
		DOT_CMAPX,	// Client-size Image Map
		DOT_JPG,	// Joint Photograph Exports Group
		DOT_PNG,	// Portable Network Graphics Format
		DOT_WRL,	// VRML WRL 3D Format
		DOT_SVG		// Scalable Vector Graphics
	}  eDotImageFormat;

	int GenerateGraph(std::string &dotScript, cDotEngine::eDotImageFormat format, std::string &resultImageFile);
	const char *GetMapFilePath() { return m_MapFilePath.c_str(); }
protected:
    int Run();

private:
	static cDotEngine *m_pInstance;
	cThreadSync m_ThreadSync;
	cTimer m_Timer;
	cProcCom m_ProcComm;
	std::string m_dotpath;
	std::string m_WebDotFolder;
	std::string m_MapFilePath;
};

/* USAGE:
		string dotScript, resultImageFile;
		dotScript = 
			"digraph G {\n"
			"size=\"7.5,10\";\n"
			"\"4.3.203.128\";\n"
			"\"4.3.203.128\" -> \"64.71.165.204\";\n"
			"\"4.3.203.128\" -> \"192.168.1.100\";\n"
			"\"4.3.203.128\" -> \"192.168.1.105\";\n"
			"\"4.3.203.128\" -> \"192.168.1.34\";\n"

			"\"192.168.1.34\" -> \"10.0.0.1\";\n"
			"\"192.168.1.34\" -> \"10.0.0.2\";\n"
			"\"192.168.1.34\" -> \"10.0.0.3\";\n"
			"\"192.168.1.34\" -> \"10.0.0.4\";\n"
			"\"192.168.1.34\" -> \"10.0.0.5\";\n"

			"\"192.168.1.34\" -> \"4.3.203.128\";\n"
			"\"10.0.0.5\" -> \"64.71.165.204\";\n"

			"\"4.55.23.190\" -> \"4.3.203.128\";\n"

			"\"4.3.203.128\" [shape=polygon,sides=8,color=deepskyblue4,style=filled];\n"
			"\"4.55.23.190\" [color=firebrick,style=filled];\n"
			"}\n";
		cDotEngine dot;
		dot.GenerateGraph(dotScript, cDotEngine::DOT_GIF, resultImageFile);
*/
#endif //_DOTENGINE_H

