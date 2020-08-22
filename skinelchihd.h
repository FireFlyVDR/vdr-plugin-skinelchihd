/*
 * skinelchi.h: 'ElchiHD' skin for the Video Disk Recorder
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#ifndef __SKINELCHIHD_H
#define __SKINELCHIHD_H

#include <vdr/skins.h>
#include "setup.h"

extern const char *OSDSKIN;
extern int SKINVERSION;

class cSkinElchiHD : public cSkin {
public:
   cSkinElchiHD(void);
   virtual const char *Description(void);
   virtual cSkinDisplayChannel *DisplayChannel(bool WithInfo);
   virtual cSkinDisplayMenu *DisplayMenu(void);
   virtual cSkinDisplayReplay *DisplayReplay(bool ModeOnly);
   virtual cSkinDisplayVolume *DisplayVolume(void);
   virtual cSkinDisplayTracks *DisplayTracks(const char *Title, int NumTracks, const char * const *Tracks);
   virtual cSkinDisplayMessage *DisplayMessage(void);
};

#endif //__SKINELCHIHD_H
