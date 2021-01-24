/*
 * displaychannel.h: Channel display handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#ifndef __ELCHIHD_DISPLAYCHANNEL_H
#define __ELCHIHD_DISPLAYCHANNEL_H

#include <vdr/skins.h>

#include <vdr/device.h>

#include "config.h"
#include "vdrstatus.h"
#include "background.h"

class cSkinElchiHDDisplayChannel : public cSkinDisplayChannel
{
private:
   cOsd *osd;
   int wLogo, hLogo, bLogo;
   int xLogo, xLeft, xChName, xTimeBar, xEvTime, xEvText, xDateTime, xSymbolStart, xRight;
   int yLogo, yChDateTime, yChName, yRecordings, yEvText, yBottom;
   enum exSymbols { xSYM_AR, xSYM_VPS, xSYM_Teletext, xSYM_Audio, xSYM_DolbyDigital, xSYM_encrypted, xSYM_cutting, xSYM_REC, xSYM_MAX};
   int xSymbols[xSYM_MAX];
   int ySymbols, ySymbolARRec;
   int wEvTime, wEvText, wChNumber, wChName, wVsize, wDateTime, wChDateTime, wTimeBar;
   int hEvents;
   bool withInfo, isRecording, isCutting, showMessage, showVolume, sleeptimermessage;
   int Gap, SymbolGap;
   int lh, slh, lh2, lineOffset;
   tColor bg;
   char Channelnumber[6];
   const cEvent *PresentEvent, *FollowingEvent;
   cTimeMs volumeTimer;
   int volumechange, recordingchange;
   cString lastdate;
   bool changed, hasVideo;
   int old_width, old_height, OSDHeight;
   eAspectRatio old_ar;
   int LastSignalStrength, LastSignalQuality; 
   int presentLastOffset, followingLastOffset;
   cPixmap *pmBG, *pmLogo, *pmChannelNameBg, *pmChDateTime, *pmVideoSize, *pmSymbols, *pmMessageBG;
   cScrollingPixmap *spmChannelName, *spmPresentTitle, *spmPresentShort, *spmFollowingTitle, *spmFollowingShort, *spmAudio, *spmRecording, *spmMessage;

   void DrawBackground();
   cString CheckLogoFile(const cChannel *Channel, const char *path);
   cPixmap *CreateTextPixmap(cOsd *osd, int layer, const cRect vPort, const cFont *font, const char *text, tColor fg, tColor bg, int Alignment = taDefault);

public:
   cSkinElchiHDDisplayChannel(bool WithInfo);
   virtual ~cSkinElchiHDDisplayChannel();
   virtual void SetChannel(const cChannel *Channel, int Number);
   virtual void SetEvents(const cEvent *Present, const cEvent *Following);
   virtual void SetMessage(eMessageType Type, const char *Text);
   virtual void Flush(void);
};

#endif //__ELCHIHD_DISPLAYCHANNEL_H
