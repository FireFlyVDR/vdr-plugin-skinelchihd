/*
 * displaytracks.h: Audio track display handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#ifndef __ELCHIHD_DISPLAYTRACKS_H
#define __ELCHIHD_DISPLAYTRACKS_H

#include <vdr/skins.h>
#include "background.h"
#include "config.h"

class cSkinElchiHDDisplayTracks : public cSkinDisplayTracks
{
private:
   cOsd *osd;
   int x0, x1, x2, x3, x4;
   int y0, y1, y2, y3, y4, y5;
   int lh, lh2;
   int currentIndex;
   int index;
   int offset;
   int numTracks, maxTracks;
   void SetItem(const char *Text, int Index, bool Current);
   cPixmap *pmBG, *pmSelector, *pmSelectorBG, *pmSymbols;

public:
   cSkinElchiHDDisplayTracks(const char *Title, int NumTracks, const char * const *Tracks);
   virtual ~cSkinElchiHDDisplayTracks();
   virtual void SetTrack(int Index, const char * const *Tracks);
   virtual void SetAudioChannel(int AudioChannel);
   virtual void Flush(void);
   virtual void Clear(void);
};

#endif //__ELCHIHD_DISPLAYTRACKS_H
