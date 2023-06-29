/*
 * image.h: image handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#ifndef __ELCHIHD_IMAGE_H
#define __ELCHIHD_IMAGE_H

#include "config.h"
#include <vdr/osd.h>

class cOSDImage : public cListObject
{
public:
   cOSDImage(cString Filename, int Width, int Height);
   cOSDImage(cString Filename, int Width, int Height, int Border);
   ~cOSDImage();
   inline cImage *GetImage() { return image; }
   inline int Width() { return widthImage; }
   inline int Height() { return heightImage; }

private:
   cString imagefilename;
   int width, height, border, widthImage, heightImage;
   cImage *image;

   bool LoadImage(bool isLogo);
};

void DrawShadedRectangle(cPixmap *pm, tColor color, const cRect &area = cRect::Null);

#endif //__ELCHIHD_IMAGE_H
