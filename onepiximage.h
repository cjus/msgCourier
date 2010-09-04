/* onepiximage.cpp
   Copyright (C) 2006 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

onepiximage.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

onepiximage.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with onepiximage.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _ONEPIXIMAGE_H
#define _ONEPIXIMAGE_H

class cOnePixImage
{
    public:
        cOnePixImage();
        int GetImageSize();
        const char *GetImageData();
    private:
        enum { IMAGESIZE = 71 };
        static const char PNGImage[IMAGESIZE];
};

#endif //_ONEPIXIMAGE_H
