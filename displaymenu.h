/*
 * displaymenu.h: Menu display handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#ifndef __ELCHIHD_DISPLAYMENU_H
#define __ELCHIHD_DISPLAYMENU_H

#include <vdr/skins.h>

#include "config.h"
#include "background.h"

class cSkinElchiHDDisplayMenu : public cSkinDisplayMenu
{
private:
   cOsd *osd;
   eMenuCategory menuCategory;
   int epgimagesize;
   cEpgImage *epgimageThread;
  
   void DrawScrollbar(int Total, int Offset, int Shown, int Top, int Height);
   void SetTextScrollbar(void);
   void DrawTitle(void);
   void SetItemBackground(int Index, bool Current, bool Selectable, int xScrollArea);
   int scrollTotalLines, scrollShownLines, scrollOffsetLines, scrollbarTop, scrollbarHeight;
   
   bool showMessage, showVolume, timersDisplayed;
   cTimeMs volumeTimer;
   int volumechange;
   int symbolGap, border;
   int x0, x1, x2, /*x3,*/ x4, x5, x6, x7, x8;
   int y0, y11, y2, y3, y4, y5, y6, y17, y8, y9;
   int btn0, btn1, btn2, btn3, btn4;

   int lh, lh2, menuTop, menuHeight, currentIndex, area;
   cString lastDate;
   bool changed;
   const cRecording *lastRecording;
   bool previousHasEPGimages;
   
   cPixmap *pmBG, *pmMenu, *pmEvent, *pmCurrentItemBG, *pmEPGImage;
   cScrollingPixmap *spmTitle, *spmCurrentItem, *spmMessage;
   cPixmap *pmButton0BG, *pmButton1BG, *pmButton2BG, *pmButton3BG;
   cPixmap *pmButton0inactive, *pmButton1inactive, *pmButton2inactive, *pmButton3inactive;
   
   tOSDsize OSDsize;
   cString lastCurrentText;
   int xScrollStart;
   tColor lastClrDlgFg;
   tColor lastClrDlgBg;
   cString title;
   int lastDiskUsageState;

   u_int32_t eventID;
   int timercheck;
   //int EventImages;
   int epgImageLines;
   timeval lasttime;
   cString stripXmlTag(cString source, const char* tag);

public:
   cSkinElchiHDDisplayMenu(void);
   virtual ~cSkinElchiHDDisplayMenu();
   virtual void SetMenuCategory(eMenuCategory MenuCategory);
   //virtual void SetTabs(int Tab1, int Tab2 = 0, int Tab3 = 0, int Tab4 = 0, int Tab5 = 0);
   //virtual void SetMenuSortMode(eMenuSortMode MenuSortMode);
   virtual void Scroll(bool Up, bool Page);
   virtual int MaxItems(void);
   virtual void Clear(void);
   virtual void SetTitle(const char *Title);
   virtual void SetButtons(const char *Red, const char *Green = NULL, const char *Yellow = NULL, const char *Blue = NULL);
   virtual void SetMessage(eMessageType Type, const char *Text);
   virtual void SetItem(const char *Text, int Index, bool Current, bool Selectable);

#ifdef DEPRECATED_SKIN_SETITEMEVENT
   using cSkinDisplayMenu::SetItemEvent;
#endif
   virtual bool SetItemEvent(const cEvent *Event, int Index, bool Current, bool Selectable, const cChannel *Channel, bool WithDate, eTimerMatch TimerMatch, bool TimerActive);
   virtual bool SetItemTimer(const cTimer *Timer, int Index, bool Current, bool Selectable);
   virtual bool SetItemChannel(const cChannel *Channel, int Index, bool Current, bool Selectable, bool WithProvider);
   virtual bool SetItemRecording(const cRecording *Recording, int Index, bool Current, bool Selectable, int Level, int Total, int New);
   virtual void SetScrollbar(int Total, int Offset);
   virtual void SetEvent(const cEvent *Event);
   virtual void SetRecording(const cRecording *Recording);
   virtual void SetText(const char *Text, bool FixedFont);
   virtual int GetTextAreaWidth(void) const;
   virtual const cFont *GetTextAreaFont(bool FixedFont) const;
   virtual void Flush(void);
};

#endif //__ELCHIHD_DISPLAYMENU_H
