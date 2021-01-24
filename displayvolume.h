/*
 * displayvolume.h: Volume display handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#ifndef __ELCHIHD_DISPLAYVOLUME_H
#define __ELCHIHD_DISPLAYVOLUME_H

#include <vdr/skins.h>
#include "config.h"
#include "common.h"
#include "background.h"

class cSkinElchiHDDisplayVolume : public cSkinDisplayVolume
{
private:
   cOsd *osd;
   int w, h, lh, lh2;
   int isMuted, currentVolume;
   bool changed;
   tOSDsize OSDsize;
   cPixmap *pmBG;
public:
   cSkinElchiHDDisplayVolume(void);
   virtual ~cSkinElchiHDDisplayVolume();
   virtual void SetVolume(int Current, int Total, bool Mute);
   virtual void Flush(void);
};

#endif //__ELCHIHD_DISPLAYVOLUME_H
