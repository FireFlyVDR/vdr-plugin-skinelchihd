/*
 * displayreplay.h: Replay display handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#ifndef __ELCHIHD_DISPLAYREPLAY_H
#define __ELCHIHD_DISPLAYREPLAY_H

#include <vdr/skins.h>
#include <vdr/tools.h>
//#include "scroll.h"
#include "config.h"
#include "background.h"
#include "vdrstatus.h"

class cSkinElchiHDDisplayReplay : public cSkinDisplayReplay
{
private:
   cOsd *osd;
   tColor clrBG;
   int x0, x2, x3, x4, x6, x9, xMode;
   int y0, y1, y2, y3;
   enum exSymbols { xSYM_AR, xSYM_CUTTING, xSYM_REC, xSYM_FREW, xSYM_SREW, xSYM_PLAY, xSYM_SFWD, xSYM_FFWD, xSYM_MAX};
   int xSymbols[xSYM_MAX];
   int ySymbols, ySymbolARcutRec;

   int lh, lh2, MarksWidth;
   int xCurrent, xTotal;
   int xCurrentWidth, xTotalWidth;
   const cRecording *recording;
   const cMarks *marks;
   bool isRecording, isCutting, modeonly, changed, showMessage, showVolume;
   cPixmap *pmBG, *pmTitleBG, *pmMode, *pmJump, *pmTitle, *pmMessageBG, *pmProgress, *pmVolume;
   cScrollingPixmap *spmTitle, *spmMessage;
   cTimeMs volumeTimer;
   int volumechange;
   cString oldCurrent;
   cString title;
   cString rectitle;
   int oldWidth;
   int oldHeight;
   eVideoFormat oldVideoFormat;

   int Gap;
   int SymbolGap;
   void SetScrollTitle(const char *Title);
   int GetPos(int p, int width, int total) { return int(int64_t(p) * width / total); }
   //void DrawProgressBar(cRect *Area, int Current, int Total, const cMarks *Marks, tColor ColorSeen, tColor ColorRest, 
   //                     tColor ColorSelected, tColor ColorMark, tColor ColorCurrent);
   void DrawMark(int xStart, int xEnd, int x, int yOffset, int Height, bool Start, bool Current, tColor ColorMark, tColor ColorCurrent);

/*
protected:
   const cMarks *marks;
   class cSkinElchiProgressBar { // : public cPixmap {
   private:
      cPixmap *pixmap;
   protected:
      int total, width, height;
      int Pos(int p) { return int(int64_t(p) * width / total); }
      void Mark(int x, bool Start, bool Current, tColor ColorMark, tColor ColorCurrent);
   public:
      cSkinElchiProgressBar(cPixmap *Pixmap, int Current, int Total, const cMarks *Marks, tColor ColorSeen, tColor ColorRest, tColor ColorSelected, tColor ColorMark, tColor ColorCurrent);
   };
*/
public:
   cSkinElchiHDDisplayReplay(bool ModeOnly);
   virtual ~cSkinElchiHDDisplayReplay();
   //virtual void SetRecording(const cRecording *Recording);
   virtual void SetMarks(const cMarks *Marks);
   virtual void SetTitle(const char *Title);
   virtual void SetMode(bool Play, bool Forward, int Speed);
   virtual void SetProgress(int Current, int Total);
   virtual void SetCurrent(const char *Current);
   virtual void SetTotal(const char *Total);
   virtual void SetJump(const char *Jump);
   virtual void SetMessage(eMessageType Type, const char *Text);
   virtual void Flush(void);
};

#endif //__ELCHIHD_DISPLAYREPLAY_H
