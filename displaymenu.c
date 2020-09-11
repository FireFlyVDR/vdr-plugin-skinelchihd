/*
 * displaymenu.c: Menu display handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

//#define DEBUG
//#define DEBUG2

#include "common.h"
#include "displaymenu.h"
#include "vdrstatus.h"
#include "image.h"
#include "symbols.h"
#include "vdr/videodir.h"
#include "vdr/cutter.h"

#include <sys/time.h>  // for timing debugging

extern cSkinElchiStatus *ElchiStatus;


/* Possible values of the stream content descriptor according to ETSI EN 300 468 */
enum stream_content
{
   sc_reserved        = 0x00,
   sc_video_MPEG2     = 0x01,
   sc_audio_MP2       = 0x02, // MPEG 1 Layer 2 audio
   sc_subtitle        = 0x03,
   sc_audio_AC3       = 0x04,
   sc_video_H264_AVC  = 0x05,
   sc_audio_HEAAC     = 0x06,
   sc_video_H265_HEVC = 0x09, // stream content 0x09, extension 0x00
   sc_audio_AC4       = 0x19, // stream content 0x09, extension 0x10 
};

static char const *mcNames[] = { "mcUndefined", "mcUnknown", "mcMain", "mcSchedule", "mcScheduleNow", "mcScheduleNext", 
                                 "mcChannel", "mcChannelEdit", "mcTimer", "mcTimerEdit", "mcRecording", "mcRecordingInfo", 
                                 "mcRecordingEdit", "mcPlugin", "mcPluginSetup", "mcSetup", "mcSetupOsd", "mcSetupEpg", 
                                 "mcSetupDvb", "mcSetupLnb", "mcSetupCam", "mcSetupRecord", "mcSetupReplay", "mcSetupMisc", 
                                 "mcSetupPlugins", "mcCommand", "mcEvent", "mcText", "mcFolder", "mcCam" };

const char* GetCategoryName(eMenuCategory category) { return mcNames[category+1]; }

//class cSkinDisplayMenu : public cSkinDisplay {
       ///< This class implements the general purpose menu display, which is
       ///< used throughout the program to display information and let the
       ///< user interact with items.
       ///< A menu consists of the following fields, each of which is explicitly
       ///< set by calls to the member functions below:
       ///< - Title: a single line of text, indicating what this menu displays
       ///< - Color buttons: the red, green, yellow and blue buttons, used for
       ///<   various functions
       ///< - Message: a one line message, indicating a Status, Info, Warning,
       ///<   or Error condition
       ///< - Central area: the main central area of the menu, used to display
       ///<   one of the following:
       ///<   - Items: a list of single line items, of which the user may be
       ///<     able to select one (non-scrollable)
       ///<   - Event: the full information about one EPG event (*cEvent passed => scrollable)
       ///<   - Text: a multi line, scrollable text (scrollable)
       ///<   - Recording: a multi line, scrollable text (*cRecording passed => scrollable)

cSkinElchiHDDisplayMenu::cSkinElchiHDDisplayMenu(void)
{
   DSYSLOG("skinelchiHD: DisplayMenu::cSkinElchiHDDisplayMenu()")
   lasttime.tv_sec = 0;
   showMessage = false;
   showVolume = false;
   menuCategory = mcUndefined;
   const cFont *font = cFont::GetFont(fontOsd);
   volumechange = ElchiStatus->GetVolumeChange(NULL);
   timercheck = ElchiConfig.showTimer;
   lastCurrentText = NULL;
   epgimageThread = NULL;

   osd = NULL;
   if ((timercheck & 2) && !cPluginManager::CallFirstService(EPGSEARCH_CONFLICTINFO))
      timercheck &= ~2;
   lastDate = NULL;
   lh = font->Height();
   int slh = cFont::GetFont(fontSml)->Height();

   OSDsize.left   = cOsd::OsdLeft();
   OSDsize.top    = cOsd::OsdTop();
   OSDsize.width  = cOsd::OsdWidth();
   OSDsize.height = cOsd::OsdHeight();

   int OSDHeight = 720;
   int OSDWidth = 1280;
   double OSDAspect = 16.0/9.0;
   cDevice::PrimaryDevice()->GetOsdSize(OSDWidth, OSDHeight, OSDAspect);
  
   DSYSLOG("skinelchiHD: OSDsize Menu %dx%d left=%d/%d top=%d/%d width=%d/%d height=%d/%d",
           OSDWidth, OSDHeight, OSDsize.left, Setup.OSDLeft, OSDsize.top, Setup.OSDTop, OSDsize.width, Setup.OSDWidth, OSDsize.height, Setup.OSDHeight);

   if (OSDHeight >= 2160) {
      symbolGap   = 12;
      border      = 30;
   } else if (OSDHeight >= 1080) {
      symbolGap   = 6;
      border      = 15;
   } else if (OSDHeight >= 720) {
      symbolGap   = 4;
      border      = 10;
   } else {  // <  720 incl. 576
      symbolGap   = 3;
      border      = 8;
   }

   elchiSymbols.Refresh(OSDHeight);

   lh2 = lh / 2;
   x0 = 0;
   x1 = x0 + lh2;
   x8 = x0 + OSDsize.width;
   x6 = x8 - lh2;
   x5 = x8 - lh;
   int wTotal = OSDsize.width;

   y0 = 0;
   y2 = y0 + lh;
   y3 = y2 + lh;
   y8 = y0 + OSDsize.height;
   y6 = y8 - lh;
   y5 = y6 - lh;
   menuHeight = y5 - y3;
   menuTop = y3;

   // height of EPG image in lines of description
   epgimagesize = ElchiConfig.EpgImageSize;
   if (epgimagesize)
   {
      epgImageLines = 0;
      switch (epgimagesize) {
         case 1: epgImageLines = 3; break;
         case 2: epgImageLines = 5; break;
         case 4: epgImageLines = 9; break;
         case 3: 
         default: epgImageLines = 7;      
      }
      x4 = x6 - epgImageLines*slh*4/3; 
      y4 = y3 + slh + epgImageLines*slh;
   }
   
   spmTitle = NULL;
   spmCurrentItem = NULL;
   pmEvent = NULL;
   osd = NULL;

   currentIndex = -1;
   lastRecording = NULL;
   previousHasEPGimages = false;

   osd = cOsdProvider::NewOsd(OSDsize.left, OSDsize.top);
   ElchiBackground->SetOSD(osd);

   tArea Area[] = { { x0, y0, x8, y8, 32} };
   if (oeOk != osd->SetAreas(Area, 1)) {
      Area[0].bpp = 8;
      if (oeOk != osd->SetAreas(Area, 1)) {
         esyslog("skinelchiHD DisplayMenu: SetAreas with TrueColor failed");
         return;
      }
   }

   pmBG = osd->CreatePixmap(LYR_BG, cRect(x0, y0, x8 - x0, y8 - y0));
   pmBG->Clear();

   tColor clrBG = Theme.Color(clrBackground);

   // menu header
   DrawShadedRectangle(pmBG, Theme.Color(clrMenuTitleBg), cRect(0, 0, x8 - x0, y2 - y0));
   pmBG->DrawEllipse(cRect(0, 0, lh2, lh2), clrTransparent, -2);
   pmBG->DrawEllipse(cRect(wTotal - lh2, 0, lh2, lh2), clrTransparent, -1);
   
   // item area
   pmBG->DrawRectangle(cRect(x0, y2, x8 - x0, y5 - y2), clrBG);

   // message line
   pmBG->DrawRectangle(cRect(x0, y5, x8 - x0, lh), clrBG);

   pmMenu = osd->CreatePixmap(LYR_TEXT, cRect(x0, y0, x8 - x0, y8 - y0));
   pmMenu->Clear();

   // EPG image
   pmEPGImage = osd->CreatePixmap(LYR_HIDDEN, cRect(x4, y3 + slh/2, epgImageLines*slh*4/3, epgImageLines*slh));

   // create selector BG
   pmCurrentItemBG = osd->CreatePixmap(LYR_HIDDEN, cRect(0, 0, x5 - x0, lh));
   DrawShadedRectangle(pmCurrentItemBG, Theme.Color(clrMenuItemCurrentBg));
   pmCurrentItemBG->DrawEllipse(cRect(0, 0, lh2, lh2), clrBG, -2);
   pmCurrentItemBG->DrawEllipse(cRect(0, lh - lh2, lh2, lh2), clrBG, -3);
   pmCurrentItemBG->DrawEllipse(cRect(x5 - lh2, 0, lh2, lh2), clrBG, -1);
   pmCurrentItemBG->DrawEllipse(cRect(x5 - lh2, lh - lh2, lh2, lh2), clrBG, -4);

   // create message pixmap
   spmMessage = new cScrollingPixmap(osd, cRect(x0 + lh2, y5, x8 - x0 - lh, lh), cFont::GetFont(fontOsd), MAXCHARS, clrTransparent, clrTransparent, true);
   spmMessage->SetLayer(LYR_HIDDEN);

   // Create buttons
   tColor lutBg[] = { clrButtonRedBg, clrButtonGreenBg, clrButtonYellowBg, clrButtonBlueBg };

   int w = x8 - x0;
   btn0 = x0;
   btn1 = btn0 + w / 4;
   btn2 = btn0 + w / 2;
   btn3 = x8 - w / 4;
   btn4 = x8;

   pmButton0BG = osd->CreatePixmap(LYR_HIDDEN, cRect(btn0, y6, btn1 - btn0, lh));
   DrawShadedRectangle(pmButton0BG, Theme.Color(lutBg[Setup.ColorKey0]));
   pmButton0BG->DrawEllipse(cRect(btn0, lh - lh2, lh2, lh2), clrTransparent, -3);
   pmButton0inactive = osd->CreatePixmap(LYR_HIDDEN, cRect(btn0, y6, btn1 - btn0, lh));
   pmButton0inactive->Fill(clrBG);
   pmButton0inactive->DrawEllipse(cRect(btn0, lh - lh2, lh2, lh2), clrTransparent, -3);

   pmButton1BG = osd->CreatePixmap(LYR_HIDDEN, cRect(btn1, y6, btn2 - btn1, lh));
   DrawShadedRectangle(pmButton1BG, Theme.Color(lutBg[Setup.ColorKey1]));
   pmButton1inactive = osd->CreatePixmap(LYR_HIDDEN, cRect(btn1, y6, btn2 - btn1, lh));
   pmButton1inactive->Fill(clrBG);

   pmButton2BG = osd->CreatePixmap(LYR_HIDDEN, cRect(btn2, y6, btn3 - btn2, lh));
   DrawShadedRectangle(pmButton2BG, Theme.Color(lutBg[Setup.ColorKey2]));
   pmButton2inactive = osd->CreatePixmap(LYR_HIDDEN, cRect(btn2, y6, btn3 - btn2, lh));
   pmButton2inactive->Fill(clrBG);

   pmButton3BG = osd->CreatePixmap(LYR_HIDDEN, cRect(btn3, y6, btn4 - btn3, lh));
   DrawShadedRectangle(pmButton3BG, Theme.Color(lutBg[Setup.ColorKey3]));
   pmButton3BG->DrawEllipse(cRect(btn4 -btn3 - lh2, lh - lh2, lh2, lh2), clrTransparent, -4);
   pmButton3inactive = osd->CreatePixmap(LYR_HIDDEN, cRect(btn3, y6, btn4 - btn3, lh));
   pmButton3inactive->Fill(clrBG);
   pmButton3inactive->DrawEllipse(cRect(btn4 -btn3 - lh2, lh - lh2, lh2, lh2), clrTransparent, -4);

   changed = true;

   int titleWidth = x6 - x1 - font->Width(*DayDateTime()) - font->Width("  ");
   spmTitle = new cScrollingPixmap(osd, cRect(x1, y0+(font->Height()-cFont::GetFont(fontSml)->Height())/2, titleWidth, cFont::GetFont(fontSml)->Height()),
                                      cFont::GetFont(fontSml), MAXCHARS, Theme.Color(clrMenuTitleFg));
}


cSkinElchiHDDisplayMenu::~cSkinElchiHDDisplayMenu()
{
   if (epgimageThread) {
      epgimageThread->Stop();
      DELETENULL(epgimageThread);
   }

   if (spmTitle) DELETENULL(spmTitle);
   if (spmCurrentItem) DELETENULL(spmCurrentItem);

   lastCurrentText = NULL;

   ElchiBackground->SetOSD(NULL);
   if (osd) {
      DELETENULL(osd);
   }
}

void cSkinElchiHDDisplayMenu::SetMenuCategory(eMenuCategory MenuCategory)
{  ///< Sets the current menu category. This allows skins to handle known
   ///< types of menus in different ways, for instance by displaying icons
   ///< or special decorations.
   ///< A derived class can reimplement this function to be informed of any
   ///< changes in the menu category. If it does, it shall call the base class
   ///< function in order to set the members accordingly for later calls to the
   ///< MenuCategory() function.

   DSYSLOG("skinelchiHD: DisplayMenu::SetMenuCategory to %d (%s)", MenuCategory, GetCategoryName(MenuCategory));
   menuCategory = MenuCategory;
}

void cSkinElchiHDDisplayMenu::DrawScrollbar(int Total, int Offset, int Shown, int Top, int Height)
{
   if (Total > 0 && Total > Shown) {
      changed = true;

      DSYSLOG("skinelchiHD: DisplayMenu::DrawScrollbar tb2=%d lh2=%d", Height * Shown / Total, lh/2);
      
      int dist = symbolGap + elchiSymbols.Width(SYM_ARROW_DOWN);
      int barlen = max((Height-2*dist) * Shown/Total, lh/2);
      int barpos = (Height -2*dist - barlen) * Offset / (Total - Shown);
      pmBG->DrawRectangle(cRect(x5 + 3*lh/8, Top+dist, lh/4, Height-2*dist), Theme.Color(clrMenuScrollbarTotal));
      pmBG->DrawRectangle(cRect(x5 + 3*lh/8, Top+dist+barpos, lh/4, barlen), Theme.Color(clrMenuScrollbarShown));
      
      if (Offset > 0)
         pmBG->DrawBitmap(cPoint(x5 + (lh - elchiSymbols.Width(SYM_ARROW_UP))/2, Top), elchiSymbols.Get(SYM_ARROW_UP, Theme.Color(clrMenuItemSelectable), Theme.Color(clrBackground)));
      else
         pmBG->DrawRectangle(cRect(x5 + (lh - elchiSymbols.Width(SYM_ARROW_UP))/2, Top, elchiSymbols.Width(SYM_ARROW_UP), elchiSymbols.Height(SYM_ARROW_UP)), Theme.Color(clrBackground));

      if (Total > (Offset + Shown))
         pmBG->DrawBitmap(cPoint(x5+ (lh - elchiSymbols.Width(SYM_ARROW_DOWN))/2, Top + Height - elchiSymbols.Height(SYM_ARROW_DOWN)), elchiSymbols.Get(SYM_ARROW_DOWN, Theme.Color(clrMenuItemSelectable), Theme.Color(clrBackground)));
      else
         pmBG->DrawRectangle(cRect(x5 + (lh - elchiSymbols.Width(SYM_ARROW_DOWN))/2, Top + Height - elchiSymbols.Height(SYM_ARROW_DOWN), elchiSymbols.Width(SYM_ARROW_DOWN), elchiSymbols.Height(SYM_ARROW_DOWN)), Theme.Color(clrBackground));
   }
}


void cSkinElchiHDDisplayMenu::SetTextScrollbar(void)
{
   if ((scrollOffsetLines > 0) || (scrollOffsetLines + scrollShownLines < scrollTotalLines)) {
      DrawScrollbar(scrollTotalLines, scrollOffsetLines, scrollShownLines, scrollbarTop, scrollbarHeight);
   }
}

void cSkinElchiHDDisplayMenu::Scroll(bool Up, bool Page)
{  ///< If this menu contains a text area that can be scrolled, this function
   ///< will be called to actually scroll the text. Up indicates whether the
   ///< text shall be scrolled up or down, and Page is true if it shall be
   ///< scrolled by a full page, rather than a single line. An object of the
   ///< cTextScroller class can be used to implement the scrolling text area.
   int slh = cFont::GetFont(fontSml)->Height();
   if (Up) {
      if (scrollOffsetLines > 0) {
         scrollOffsetLines -= Page ? scrollShownLines : 1;
         if (scrollOffsetLines < 0)
            scrollOffsetLines = 0;
         LOCK_PIXMAPS;
         pmEvent->SetDrawPortPoint(cPoint(0, -scrollOffsetLines * slh));
         DSYSLOG("skinelchiHD: DisplayMenu::Scroll Up line %d", -scrollOffsetLines)

         if (pmEPGImage) 
         {
            pmEPGImage->SetDrawPortPoint(cPoint(0, -scrollOffsetLines * slh));
            cRect vPort = pmEPGImage->ViewPort();
            vPort.SetHeight(epgImageLines * slh - scrollOffsetLines * slh);
            pmEPGImage->SetViewPort(vPort);
         }
      }
   }
   else
   {
      if (scrollOffsetLines + scrollShownLines < scrollTotalLines)
      {
         scrollOffsetLines += Page ? scrollShownLines : 1;
         if (scrollOffsetLines + scrollShownLines > scrollTotalLines)
            scrollOffsetLines = scrollTotalLines - scrollShownLines;
         LOCK_PIXMAPS;
         pmEvent->SetDrawPortPoint(cPoint(0, -scrollOffsetLines * slh));
         DSYSLOG("skinelchiHD: DisplayMenu::Scroll Up line %d", -scrollOffsetLines)
         if (pmEPGImage)
         {
            pmEPGImage->SetDrawPortPoint(cPoint(0, -scrollOffsetLines * slh));
            cRect vPort = pmEPGImage->ViewPort();
            vPort.SetHeight(epgImageLines * slh - scrollOffsetLines * slh);
            pmEPGImage->SetViewPort(vPort);
         }
      }
   }
   SetTextScrollbar();
}

int cSkinElchiHDDisplayMenu::MaxItems(void)
{  ///< Returns the maximum number of items the menu can display.
   const cFont *font = cFont::GetFont(fontOsd);
   lh = font->Height();
   return (menuHeight / lh);
}

void cSkinElchiHDDisplayMenu::Clear(void)
{  ///< Clears the entire central area of the menu.
   lastCurrentText = NULL;
   timersDisplayed = false;
   xScrollStart = -1;
   
   // clear EPG image and stop Thread
   pmEPGImage->SetLayer(LYR_HIDDEN);
   if (epgimageThread) 
      epgimageThread->PutEventID(NULL, 0);

   if(spmCurrentItem) {
      DELETENULL(spmCurrentItem);
      currentIndex = -1;
      pmCurrentItemBG->SetLayer(LYR_HIDDEN);
   }

   lastDate = NULL;
   changed = true;

   textScroller.Reset();
   tColor clrBG = Theme.Color(clrBackground);

   LOCK_PIXMAPS;
   // clear scrollbar
   pmBG->DrawRectangle(cRect(x5, y2, lh, y6 - y2), clrBG);

   // item area
   pmBG->DrawRectangle(cRect(x0, y2, x8 - x0, y5 - y2), clrBG);

   if (pmMenu)
      pmMenu->Clear();

   if (pmEvent) {
      cPixmap *tmpPixmap = pmEvent;
      pmEvent = NULL;
      osd->DestroyPixmap(tmpPixmap);
   }
}

void cSkinElchiHDDisplayMenu::DrawTitle(void)
{
   DSYSLOG("skinelchiHD: DisplayMenu::DrawTitle")

   if ((menuCategory == mcMain) || (menuCategory == mcRecording) || (menuCategory == mcTimer)) {
      
      cVideoDiskUsage::HasChanged(lastDiskUsageState);
      cString titleline = *cString::sprintf("%s - %s %d%% (%.1f GB - %d:%02d %s)", 
                    *title, trVDR("Disk"), cVideoDiskUsage::UsedPercent(), cVideoDiskUsage::FreeMB()/1024.0,
                    cVideoDiskUsage::FreeMinutes()/60, cVideoDiskUsage::FreeMinutes()%60, trVDR("free"));

      spmTitle->SetText(titleline, cFont::GetFont(fontSml));
   }
   else
      spmTitle->SetText(title, cFont::GetFont(fontSml));

   changed = true;
}

void cSkinElchiHDDisplayMenu::SetTitle(const char *Title)
{  ///< Sets the title of this menu to Title.
   DSYSLOG("skinelchiHD: DisplayMenu::SetTitle(%s)", Title)
   title = Title;
   cVideoDiskUsage::HasChanged(lastDiskUsageState);
   DrawTitle();
}

void cSkinElchiHDDisplayMenu::SetButtons(const char *Red, const char *Green, const char *Yellow, const char *Blue)
{  ///< Sets the color buttons to the given strings. If any of the values is
   ///< NULL, any previous text must be removed from the related button.
   DSYSLOG("skinelchiHD: DisplayMenu::SetButtons(\"%s\",\"%s\",\"%s\",\"%s\")", Red, Green, Yellow, Blue)

   const char *lutText[] = { Red, Green, Yellow, Blue };
   tColor lutFg[] = { clrButtonRedFg, clrButtonGreenFg, clrButtonYellowFg, clrButtonBlueFg };

   const cFont *smallfont = cFont::GetFont(fontSml);
   const cFont *font = cFont::GetFont(fontOsd);
   lh = font->Height();

   LOCK_PIXMAPS;
   pmMenu->DrawRectangle(cRect(btn0, y6, btn4, y8 - y6), clrTransparent);
   // pmButtonxBG: clrBG, wird angezeigt wenn kein Button Text vorliegt
   // pmButtonx:   clr-Button, wird als hintergrund angezeigt wenn Button Text vorliegt
   // pmMenu:      enthält Button Text
   pmButton0inactive->SetLayer(lutText[Setup.ColorKey0] ? LYR_HIDDEN : LYR_SELECTOR);
   pmButton0BG->SetLayer(lutText[Setup.ColorKey0] ? LYR_SELECTOR : LYR_HIDDEN);
   if (lutText[Setup.ColorKey0]) pmMenu->DrawText(cPoint(btn0, y6), lutText[Setup.ColorKey0], Theme.Color(lutFg[Setup.ColorKey0]), clrTransparent, smallfont, btn1 - btn0, 0, taCenter);
   
   pmButton1inactive->SetLayer(lutText[Setup.ColorKey1] ? LYR_HIDDEN : LYR_SELECTOR);
   pmButton1BG->SetLayer(lutText[Setup.ColorKey1] ? LYR_SELECTOR : LYR_HIDDEN);
   if (lutText[Setup.ColorKey1]) pmMenu->DrawText(cPoint(btn1, y6), lutText[Setup.ColorKey1], Theme.Color(lutFg[Setup.ColorKey1]), clrTransparent, smallfont, btn2 - btn1, 0, taCenter);
   
   pmButton2inactive->SetLayer(lutText[Setup.ColorKey2] ? LYR_HIDDEN : LYR_SELECTOR);
   pmButton2BG->SetLayer(lutText[Setup.ColorKey2] ? LYR_SELECTOR : LYR_HIDDEN);
   if (lutText[Setup.ColorKey2]) pmMenu->DrawText(cPoint(btn2, y6), lutText[Setup.ColorKey2], Theme.Color(lutFg[Setup.ColorKey2]), clrTransparent, smallfont, btn3 - btn2, 0, taCenter);
   
   pmButton3inactive->SetLayer(lutText[Setup.ColorKey3] ? LYR_HIDDEN : LYR_SELECTOR);
   pmButton3BG->SetLayer(lutText[Setup.ColorKey3] ? LYR_SELECTOR : LYR_HIDDEN);
   if (lutText[Setup.ColorKey3]) pmMenu->DrawText(cPoint(btn3, y6), lutText[Setup.ColorKey3], Theme.Color(lutFg[Setup.ColorKey3]), clrTransparent, smallfont, btn4 - btn3, 0, taCenter);

   changed = true;
}

void cSkinElchiHDDisplayMenu::SetMessage(eMessageType Type, const char *Text)
{  ///< Sets a one line message Text, with the given Type. Type can be used
   ///< to determine, e.g., the colors for displaying the Text.
   ///< If Text is NULL, any previously displayed message must be removed, and
   ///< any previous contents overwritten by the message must be restored.

   tColor clrBG = Theme.Color(clrBackground);
   if (Text) {
      DSYSLOG("skinelchiHD: DisplayMenu::SetMessage(%d,\"%s\")", int(Type), Text)
      changed = true;
      showMessage = true;
      const cFont *font = cFont::GetFont(fontOsd);
      lh = font->Height();

      LOCK_PIXMAPS;
      DrawShadedRectangle(pmBG, Theme.Color(clrMessageStatusBg + 2 * Type), cRect(x0, y5, x8, lh));
      pmBG->DrawEllipse(cRect(0, y5, lh2, lh2), clrBG, -2);
      pmBG->DrawEllipse(cRect(0, y6 - lh2, lh2, lh2), clrBG, -3);
      pmBG->DrawEllipse(cRect(x8 - lh2, y5, lh2, lh2), clrBG, -1);
      pmBG->DrawEllipse(cRect(x8 - lh2, y6 - lh2, lh2, lh2), clrBG, -4);
      
      spmMessage->SetColor(Theme.Color(clrMessageStatusFg + 2 * Type));
      spmMessage->SetText(Text, font);
      spmMessage->SetLayer(LYR_SCROLL);
   }
   else {
      if (showMessage) {
         DSYSLOG("skinelchiHD: DisplayMenu::SetMessage(%d,\"%s\")", int(Type), Text?Text:"NULL")
         changed = true;
         showMessage = false;
         pmBG->DrawRectangle(cRect(x0, y5, x8, lh), clrBG);
         spmMessage->SetLayer(LYR_HIDDEN);
      }
      else
         DSYSLOG("skinelchiHD: skip DisplayMenu::SetMessage(%d,\"%s\")", int(Type), Text)
   }
}

void cSkinElchiHDDisplayMenu::SetItem(const char *Text, int Index, bool Current, bool Selectable)
{  ///< Sets the item at the given Index to Text. Index is between 0 and the
   ///< value returned by MaxItems(), minus one. Text may contain tab characters ('\t'),
   ///< which shall be used to separate the text into several columns, according to the
   ///< values set by a prior call to SetTabs(). 
   ///< If Current is true, this item shall be drawn in a way indicating to the user
   ///< that it is the currently selected one.
   ///< Selectable can be used to display items differently that can't be
   ///< selected by the user.
   ///< Whenever the current status is moved from one item to another,
   ///< this function will be first called for the old current item
   ///< with Current set to false, and then for the new current item
   ///< with Current set to true.

   DSYSLOG("skinelchiHD: DisplayMenu::SetItem(\"%s\",%d,%s,%s) %s %d-%d-%d-%d-%d-%d", Text, Index, Current ? "'Current'" : "'nonCurrent'", Selectable ? "'Selectable'" : "'nonSelectable'", GetCategoryName(menuCategory), Tab(0), Tab(1), Tab(2), Tab(3), Tab(4), Tab(5) )
   tColor ColorFg = Theme.Color(Current ? clrMenuItemCurrentFg : Selectable ? clrMenuItemSelectable : clrMenuItemNonSelectable); 
   const cFont *font = cFont::GetFont(fontOsd);
   lh = font->Height();
   int y = menuTop + Index * lh;
   
   // set Background and make it scrollable after last tab 
   if (xScrollStart < 0) { // calc only if for new menu
      xScrollStart = x1;
      int i;
      for (i = 1; (i < MaxTabs) && Tab(i); i++) {}
      xScrollStart = x1 + Tab(i-1);
   }
   SetItemBackground(Index, Current, Selectable, xScrollStart);

   const char *s;
   int xOffset = x1;
   for (int i = 0; i < MaxTabs; i++) {
      bool isLastText = NULL == GetTabbedText(Text, i+1);
      s = GetTabbedText(Text, i);
      if (s) {  // symbol and progress bar detection
         char buffer[9];
         bool isTimer = false;
         bool isNewRecording = false;
         bool isProgressBar = false;
         bool hasEventTimer = false;
         bool hasPartitialEventTimer = false;
         bool isRecording = false;
         bool hasVPS = false;
         bool isRunning = false;

         if (ElchiConfig.showIcons) {
            // check for timer info symbols: " !#>"
            if (menuCategory == mcTimer && i == 0 && strlen(s) == 1 && strchr(" !#>", s[0])) {
               isTimer = true; // update status                                                 
            }
            else
            // check for new recording: "10:10*", "1:10*", "01.01.06*"
               if (menuCategory == mcRecording && (strlen(s) == 6 && s[5] == '*' && s[2] == ':' && isdigit(*s)
                      && isdigit(*(s + 1)) && isdigit(*(s + 3)) && isdigit(*(s + 4)))
                  || (strlen(s) == 5 && s[4] == '*' && s[1] == ':' && isdigit(*s)
                      && isdigit(*(s + 2)) && isdigit(*(s + 3)))
                  || (strlen(s) == 9 && s[8] == '*' && s[5] == '.' && s[2] == '.'
                      && isdigit(*s) && isdigit(*(s + 1)) && isdigit(*(s + 3))
                      && isdigit(*(s + 4)) && isdigit(*(s + 6)) && isdigit(*(s + 7)))) {
                  isNewRecording = true;  // update status
                  strncpy(buffer, s, strlen(s));   // make a copy
                  buffer[strlen(s) - 1] = '\0';  // remove the '*' character
               }
               else
                  if (menuCategory == mcSchedule || menuCategory == mcScheduleNow || menuCategory == mcScheduleNext) {
                     
                     if ((strlen(s) == 3) && ( i == 2 || i == 3 || i == 4)) {
                        if (s[0] == 'R') isRecording = true;
                        if (s[0] == 'T') hasEventTimer = true;
                        if (s[0] == 't') hasPartitialEventTimer = true;
                        if (s[1] == 'V') hasVPS = true;
                        if (s[2] == '*') isRunning = true;
                     }
                     else
                        if ((strlen(s) == 4) && ( i == 2 || i == 3 || i == 4 ) && ( s[0] == ' ' )) { //epgsearch What's on now default
                           if (s[1] == 'R') isRecording = true;
                           if (s[1] == 'T') hasEventTimer = true;
                           if (s[1] == 't') hasPartitialEventTimer = true;
                           if (s[2] == 'V') hasVPS = true;
                           if (s[3] == '*') isRunning = true;
                        }
                  }
          }

         int current = 0, total = 0;
         // check for progress bar: "[|||||||   ]"
         if (!isTimer && !isNewRecording && ElchiConfig.GraphicalProgressbar &&
            (strlen(s) > 5 && s[0] == '[' && s[strlen(s) - 1] == ']')) {
            const char *p = s + 1;
            isProgressBar = true; // update status
            for (; *p != ']'; ++p) {  // check if progressbar characters
               if (*p == ' ' || *p == '|') { // update counters
                  ++total;
                  if (*p == '|')
                     ++current;
               } else {  // wrong character detected; not a progressbar
                  isProgressBar = false;
                  break;
               }
            }
         }
         xOffset = x1 + Tab(i);

         if (isTimer) {
            // timer menu  =========================================================
            pmMenu->DrawRectangle(cRect(xOffset, y, x5, lh - 1), clrTransparent);
            switch(s[0]) {
            case '!': 
               pmMenu->DrawBitmap(cPoint(xOffset, y + (lh - elchiSymbols.Height(SYM_ARROW_TURN)) / 2), elchiSymbols.Get(SYM_ARROW_TURN, ColorFg, clrTransparent));
               break;
            case '#':
               pmMenu->DrawBitmap(cPoint(xOffset, y + (lh - elchiSymbols.Height(SYM_RECSML)) / 2), elchiSymbols.Get(SYM_RECSML, Theme.Color(clrSymbolRecFg), Theme.Color(clrSymbolRecBg)));
               break;
            case '>':
               pmMenu->DrawBitmap(cPoint(xOffset, y + (lh - elchiSymbols.Height(SYM_CLOCK)) / 2), elchiSymbols.Get(SYM_CLOCK, ColorFg, clrTransparent));
               break;
            case ' ':
            default:
               break;
            }
         } else if (isRecording || hasEventTimer || hasPartitialEventTimer || hasVPS || isRunning) {
            LOCK_PIXMAPS;
            // program schedule menu  =========================================================
            if (isRecording) {
               DSYSLOG("skinelchiHD: DisplayMenu::SetItem Recording: ERROR - this should not happen!");
               pmMenu->DrawBitmap(cPoint(xOffset, y + (lh - elchiSymbols.Height(SYM_RECSML)) / 2), elchiSymbols.Get(SYM_RECSML, Theme.Color(clrSymbolRecBg), Theme.Color(clrSymbolRecFg)));
            }
            else {
               if (hasEventTimer)
                  pmMenu->DrawBitmap(cPoint(xOffset, y + (lh - elchiSymbols.Height(SYM_CLOCK) ) / 2), elchiSymbols.Get(SYM_CLOCK, ColorFg, clrTransparent));
               if (hasPartitialEventTimer)
                  pmMenu->DrawBitmap(cPoint(xOffset + (elchiSymbols.Height(SYM_CLOCK) - elchiSymbols.Height(SYM_CLOCKSML) ) / 2, y + (lh - elchiSymbols.Height(SYM_CLOCKSML)) / 2), elchiSymbols.Get(SYM_CLOCKSML, ColorFg, clrTransparent));
            }
            xOffset += elchiSymbols.Width(SYM_CLOCK); // clock is wider than rec

            if (hasVPS)
               pmMenu->DrawBitmap(cPoint(xOffset, y + (lh - elchiSymbols.Height(SYM_VPSSML)) / 2), elchiSymbols.Get(SYM_VPSSML, ColorFg, clrTransparent));
            xOffset += elchiSymbols.Width(SYM_VPSSML);

            if (isRunning)
               pmMenu->DrawText(cPoint(xOffset, y), "*", ColorFg, clrTransparent, font, x5 - xOffset);
            
         } else if (isNewRecording) {
            // recordings menu ==================================
            // replace by: bool SetItemRecording(const cRecording *Recording, int Index, bool Current, bool Selectable, int Level, int Total, int New)
            pmMenu->DrawText(cPoint(xOffset, y), buffer, ColorFg, clrTransparent, font, x5 - xOffset);
            // draw symbol "new" centered
            int gap = std::max(0, (Tab(i+1)-Tab(i)- font->Width(buffer) - elchiSymbols.Height(SYM_NEWSML)) / 2);
            pmMenu->DrawBitmap(cPoint(xOffset + font->Width(buffer) + gap, y + (lh - elchiSymbols.Height(SYM_NEWSML)) / 2), elchiSymbols.Get(SYM_NEWSML, ColorFg, clrTransparent));
         } else if (isProgressBar) {
            // define x coordinates of progressbar
            int pxs = xOffset;
            int pxe;
            if (strlen(s) < 11)
               pxe = (Tab(i + 1)?Tab(i+1):x5);
            else
               pxe = std::min(xOffset + font->Width(s) - font->Height()/2, (int)x5);

            int px = pxs + std::max((int)((float) current * (float) (pxe - pxs) / (float) total), 1);
            //isyslog("skinelchiHD: progressbar: %d-%d (%d) %d(%d)", px0, px1, px1-px0, px, px-px0);
            // define y coordinates of progressbar
            int pys = y + lh/4;
            int pye = y + lh - lh/4;

            DrawShadedRectangle(pmMenu, Theme.Color(clrProgressBarUpper), cRect(pxs, pys, px-pxs, pye-pys));
            DrawShadedRectangle(pmMenu, Theme.Color(clrProgressBarLower), cRect(px, pys, pxe-px, pye-pys));
         }  // end isProgressBar
         else
            //DSYSLOG("skinelchiHD: DisplayMenu::SetItem(\"%s\" B i=%d, xOff=%d", s, i, xOffset)
            if (Current) {
               if (xOffset == xScrollStart) { // zeichne letzten (=scrollenden) Teil
                  spmCurrentItem->SetText(s, font);
               }
               else {
                  pmMenu->DrawText(cPoint(xOffset, y), s, ColorFg, clrTransparent, font, (isLastText?x5-lh/2:x1+Tab(i+1)) - xOffset);
               }               
            }
            else { // non-current
               pmMenu->DrawText(cPoint(xOffset, y), s, ColorFg, clrTransparent, font, (isLastText?x5-lh/2:x1+Tab(i+1)) - xOffset);
               //DSYSLOG("skinelchiHD: DisplayMenu::SetItem(\"%s\" C i=%d, xOff=%d Len=%d", s, i, xOffset, ((Tab(i+1) && NULL != GetTabbedText(Text, i+1)?x1+Tab(i+1):x5-lh/2)) - xOffset)
               
            }
      }
      if (!Tab(i + 1))
         break;
   }

   SetEditableWidth(x5 - lh/2 - xOffset);
   changed = true;
}

bool cSkinElchiHDDisplayMenu::SetItemEvent(const cEvent *Event, int Index, bool Current, bool Selectable, const cChannel *Channel, bool WithDate, eTimerMatch TimerMatch, bool TimerActive)
{
   //< Sets the item at the given Index to Event. See SetItem() for more information.
   ///< If a derived skin class implements this function, it can display an Event item
   ///< in a more elaborate way than just a simple line of text.
   ///< If Channel is not NULL, the channel's name and/or number shall be displayed.
   ///< If WithDate is true, the date of the Event shall be displayed (in addition to the time).
   ///< TimerMatch tells how much of this event will be recorded by a timer.
   ///< TimerActive tells whether the timer that will record this event is active.
   ///< If the skin displays the Event item in its own way, it shall return true.
   ///< The default implementation does nothing and returns false, which results in
   ///< a call to SetItem() with a proper text.

   return false;
}

bool cSkinElchiHDDisplayMenu::SetItemTimer(const cTimer *Timer, int Index, bool Current, bool Selectable)
{  ///< Sets the item at the given Index to Timer. See SetItem() for more information.
   ///< If a derived skin class implements this function, it can display a Timer item
   ///< in a more elaborate way than just a simple line of text.
   ///< If the skin displays the Timer item in its own way, it shall return true.
   ///< The default implementation does nothing and returns false, which results in
   ///< a call to SetItem() with a proper text.

   return false;
}

bool cSkinElchiHDDisplayMenu::SetItemChannel(const cChannel *Channel, int Index, bool Current, bool Selectable, bool WithProvider)
{  ///< Sets the item at the given Index to Channel. See SetItem() for more information.
   ///< If a derived skin class implements this function, it can display a Channel item
   ///< in a more elaborate way than just a simple line of text.
   ///< If WithProvider ist true, the provider shall be displayed in addition to the
   ///< channel's name.
   ///< If the skin displays the Channel item in its own way, it shall return true.
   ///< The default implementation does nothing and returns false, which results in
   ///< a call to SetItem() with a proper text.

   return false;
}

void cSkinElchiHDDisplayMenu::SetItemBackground(int Index, bool Current, bool Selectable, int xScrollArea)
{
   tColor ColorFg = Theme.Color(Current ? clrMenuItemCurrentFg : Selectable ? clrMenuItemSelectable : clrMenuItemNonSelectable); 
   const cFont *font = cFont::GetFont(fontOsd);
   int lh = font->Height();
   int y = menuTop + Index * lh;
   if (Current) {
      if (Index != currentIndex) {
         currentIndex = Index;
         pmMenu->DrawRectangle(cRect(x0, y, x5 - x0, lh), clrTransparent);
         pmBG->DrawRectangle(cRect(x0, y, x5 - x0, lh), clrTransparent);
         pmCurrentItemBG->SetViewPort(cRect(x0, y, x5 - x0, lh));
         pmCurrentItemBG->SetLayer(LYR_SELECTOR);
         DELETENULL(spmCurrentItem);

         spmCurrentItem = new cScrollingPixmap(osd, cRect(xScrollArea, y, x5 - lh/2 - xScrollArea, font->Height()),
                                      font, MAXCHARS, ColorFg, clrTransparent, false, taLeft);
      }
   } // end Current
   else { //non-current
      pmMenu->DrawRectangle(cRect(x0, y, x5, lh), clrTransparent);
      pmBG->DrawRectangle(cRect(x0, y, x5 - x0, lh), Theme.Color(clrBackground));

      // hide selector if we have just replaced it
      if (currentIndex == Index) { // reset previus "current index"
         DELETENULL(spmCurrentItem);
         pmCurrentItemBG->SetLayer(LYR_HIDDEN);
         currentIndex = -1;
      }
   }
}
   
bool cSkinElchiHDDisplayMenu::SetItemRecording(const cRecording *Recording, int Index, bool Current, bool Selectable, int Level, int Total, int New)
{  ///< Sets the item at the given Index to Recording. See SetItem() for more information.
   ///< If a derived skin class implements this function, it can display a Recording item
   ///< in a more elaborate way than just a simple line of text.
   ///<
   ///< Level is the currently displayed level of the video directory, where 0 is the
   ///< top level. A value of -1 means that the full path names of the recordings
   ///< shall be displayed. 
   ///< If Total is greater than 0, this is a directory with the given total number of
   ///< entries, and New contains the number of new (unwatched) recordings.
   ///< If the skin displays the Recording item in its own way, it shall return true.
   ///< The default implementation does nothing and returns false, which results in
   ///< a call to SetItem() with a proper text.

   DSYSLOG("skinelchiHD: DisplayMenu::SetItemRecording(\"%s\",%d,%s,%s) %s %d-%d-%d-%d-%d-%d", "-", Index, Current ? "'Current'" : "'nonCurrent'", Selectable ? "'Selectable'" : "'nonSelectable'", GetCategoryName(menuCategory), Tab(0), Tab(1), Tab(2), Tab(3), Tab(4), Tab(5) )
   
   const cFont *font = cFont::GetFont(fontOsd);
   tColor ColorFg;
   int lh = font->Height();
   int y = menuTop + Index * lh;
   SetItemBackground(Index, Current, Selectable, x1 + Tab(3) + 2*symbolGap);
   
   if (Total) {  // folder
      const char* tmp = Recording->Title(' ', true, Level);
      
      char* name = strdup(tmp+2);
      
      if (Current) {
         ColorFg = Theme.Color(clrMenuItemCurrentFg);
         spmCurrentItem->SetText(name, font);
      }
      else { // non-current
         ColorFg = Theme.Color(Selectable ? clrMenuItemSelectable : clrMenuItemNonSelectable);
         pmMenu->DrawText(cPoint(x1 + Tab(3) + 2*symbolGap, y), name, ColorFg, clrTransparent, font, x5 - lh/2- Tab(3) - 2*symbolGap);
      }
      // both
      pmMenu->DrawText(cPoint(x1 + Tab(0), y), cString::sprintf("%d", Total), ColorFg, clrTransparent, font, Tab(2) - Tab(0));
      pmMenu->DrawText(cPoint(x1 + Tab(2), y), cString::sprintf("%d", New), ColorFg, clrTransparent, font, Tab(3) - Tab(2));
      free (name);
   }
   else { // recording
      bool is_H264 = false, is_H265 = false;
      if (ElchiConfig.showRecHD) {
         // find H.264/H.265 videos
         if (Recording->Info() && Recording->Info()->Components()) {
            const cComponents *Components = Recording->Info()->Components();
            for (int i = 0; i < Components->NumComponents(); i++) {
               const tComponent *p = Components->Component(i);
               if (p->stream == sc_video_H264_AVC) {
                  is_H264 = true;
                  break;
               }
               if (p->stream == sc_video_H265_HEVC) {
                  is_H265 = true;
                  break;
               }
            }
         }
      }
               
      ///copied from VDR to replace Title() to add 'HD'
      bool NewIndicator = true;
      char New = NewIndicator && Recording->IsNew() ? '*' : ' ';
      //char *titleBuffer = NULL;
      if (Level < 0 || Level == Recording->HierarchyLevels()) {
         struct tm tm_r;
         time_t start = Recording->Start();
         struct tm *t = localtime_r(&start, &tm_r);
         char *s;
         if (Level > 0 && (s = (char *)strrchr(Recording->Name(), FOLDERDELIMCHAR)) != NULL)
            s++;
         else
            s = (char *)Recording->Name();
         cString Length("");
         if (NewIndicator) {
            int Minutes = std::max(0, (Recording->LengthInSeconds() + 30) / 60);
            Length = cString::sprintf("%d:%02d", Minutes / 60, Minutes % 60);
         }

         if (Current) {
            ColorFg = Theme.Color(clrMenuItemCurrentFg);
            spmCurrentItem->SetText(cString::sprintf("%s", s), font);
         }
         else { // non-current
            ColorFg = Theme.Color(Selectable ? clrMenuItemSelectable : clrMenuItemNonSelectable);
            pmMenu->DrawText(cPoint(x1 + Tab(3) + 2*symbolGap, y), cString::sprintf("%s", s), ColorFg, clrTransparent, font, x5 - lh/2- Tab(3) - 2*symbolGap);
         }
         // both
         pmMenu->DrawText(cPoint(x1 + Tab(0), y), cString::sprintf("%02d.%02d.%02d", t->tm_mday, t->tm_mon + 1, t->tm_year % 100), ColorFg, clrTransparent, font, Tab(1) - Tab(0));
         pmMenu->DrawText(cPoint(x1 + Tab(1), y), cString::sprintf("%02d:%02d", t->tm_hour, t->tm_min), ColorFg, clrTransparent, font, Tab(2) - Tab(1));
         pmMenu->DrawText(cPoint(x1 + Tab(2), y), *Length, ColorFg, clrTransparent, font, Tab(3) - Tab(2));
         
         if (Recording->IsNew())
            pmMenu->DrawBitmap(cPoint(x1 + Tab(3) + symbolGap - elchiSymbols.Width(SYM_NEWSML) - symbolGap - elchiSymbols.Width(SYM_AR_HD), y + center(lh, elchiSymbols.Height(SYM_NEWSML))), elchiSymbols.Get(SYM_NEWSML, ColorFg, clrTransparent));
         if (is_H264)
            pmMenu->DrawBitmap(cPoint(x1 + Tab(3) + symbolGap - elchiSymbols.Width(SYM_AR_HD), y + center(lh, elchiSymbols.Height(SYM_AR_HD))), elchiSymbols.Get(SYM_AR_HD, ColorFg, clrTransparent));
         if (is_H265)
            pmMenu->DrawBitmap(cPoint(x1 + Tab(3) + symbolGap - elchiSymbols.Width(SYM_AR_UHD), y + center(lh, elchiSymbols.Height(SYM_AR_UHD))), elchiSymbols.Get(SYM_AR_UHD, ColorFg, clrTransparent));
      }
   }
   changed = true;
   return true;
}

void cSkinElchiHDDisplayMenu::SetScrollbar(int TotalLines, int OffsetLines)
{  ///< Sets the Total number of items in the currently displayed list, and the
   ///< Offset of the first item that is currently displayed (the skin knows how
   ///< many items it can display at once, see MaxItems()). This can be used to
   ///< display a scrollbar.
   DrawScrollbar(TotalLines, OffsetLines, MaxItems(), menuTop, MaxItems() * lh);
}

void cSkinElchiHDDisplayMenu::SetEvent(const cEvent *Event)
{  ///< Sets the Event that shall be displayed, using the entire central area
   ///< of the menu. The Event's 'description' shall be displayed using a
   ///< cTextScroller, and the Scroll() function will be called to drive scrolling
   ///< that text if necessary.
   if (!Event) {
      DSYSLOG("skinelchiHD: skip DisplayMenu::SetEvent()")
      return;
   }

   DSYSLOG("skinelchiHD: DisplayMenu::SetEvent(%s %d)", *Event->ChannelID().ToString(), Event->EventID())

   changed = true;
   tColor clrMEvent = Theme.Color(clrMenuEventTitle);
   const cFont *font = cFont::GetFont(fontOsd);
   const cFont *smallfont = cFont::GetFont(fontSml);
   int slh = smallfont->Height();
   lh = font->Height();
   int textwidth = x5 - x1; // max. width
   bool hasEPGimages = false;
   cTextWrapper tw;  //strips trailing newlines
   
   if (epgimagesize) {  // != 0, that is: is desired and can be displayed
      textwidth = x4 - x1 - (x1 - x0);
      pmEPGImage->SetDrawPortPoint(cPoint(0, 0));
      cRect vPort = pmEPGImage->ViewPort();
      vPort.SetHeight(epgImageLines * slh);
      pmEPGImage->SetViewPort(vPort);

      if (!epgimageThread) {
         epgimageThread = new cEpgImage(pmEPGImage, pmEPGImage->DrawPort().Width(), pmEPGImage->DrawPort().Height(), border);
         DSYSLOG("skinelchiHD: DisplayMenu::SetEvent osd Height=%d %.1f lh=%d %dx%d", OSDsize.height, OSDsize.height*0.35, lh, pmEPGImage->DrawPort().Width(), pmEPGImage->DrawPort().Height());
      }

      // start conversion via PutEventID
      hasEPGimages = epgimageThread->PutEventID((const char *)Event->ChannelID().ToString(), Event->EventID());
      DSYSLOG("skinelchiHD: DisplayMenu::SetEvent %3s evID=%s_%d", hasEPGimages?"yes":"no", (const char *)Event->ChannelID().ToString(), Event->EventID());
   }

   int y = y3;

   cString t = cString::sprintf("%s  %s - %s", *Event->GetDateString(), *Event->GetTimeString(), *Event->GetEndTimeString());
   pmMenu->DrawText(cPoint(x1, y), t, Theme.Color(clrMenuEventTime), clrTransparent, font);
   y += lh;

   // VPS time
   if (Event->Vps() && Event->Vps() != Event->StartTime()) {
      cString s = cString::sprintf(" VPS: %s", *Event->GetVpsString());
      pmMenu->DrawText(cPoint(x1, y), s, Theme.Color(clrMenuEventVpsFg), Theme.Color(clrMenuEventVpsBg), smallfont);
   }

   // Parentalrating
   if (ElchiConfig.showEPGDetails == 2 && Event->ParentalRating()) {
      cString buffer = cString::sprintf(" %s ", *Event->GetParentalRatingString());
      int w = font->Width(buffer);
      pmMenu->DrawText(cPoint(x1 + textwidth - w - lh/2, y), buffer, Theme.Color(clrMenuEventVpsFg), Theme.Color(clrMenuEventVpsBg), smallfont, w);
   }
   y += lh;

   // Title
   tw.Set(Event->Title(), font, textwidth);
   for (int i = 0; i < tw.Lines(); i++) {
      pmMenu->DrawText(cPoint(x1, y), tw.GetLine(i), clrMEvent, clrTransparent, font);
      y += lh;
   }

   // ShortText
   if (!isempty(Event->ShortText())) {
      tw.Set(Event->ShortText(), smallfont, textwidth);
      for (int i = 0; i < tw.Lines(); i++) {
         pmMenu->DrawText(cPoint(x1, y), tw.GetLine(i), Theme.Color(clrMenuEventShortText), clrTransparent, smallfont);
         y += lh;
      }
   }

   // Content
   if (ElchiConfig.showEPGDetails == 2) {
      cString content = cString();
      for (int i = 0; Event->Contents(i); i++) {
         if (!isempty(Event->ContentToString(Event->Contents(i)))) { // skip empty (user defined) content
            if (!isempty(*content))
            {
               content.Append(", ");
               content.Append(Event->ContentToString(Event->Contents(i)));
            }
            else 
               content = cString(Event->ContentToString(Event->Contents(i)));
         }
      }
      if (!isempty(*content))
      {
         tw.Set(content, smallfont, textwidth);
         for (int i = 0; i < tw.Lines(); i++) {
            pmMenu->DrawText(cPoint(x1, y), tw.GetLine(i), Theme.Color(clrMenuEventDescription), clrTransparent, smallfont);
            y += lh;
         }
      }
   }
   y += slh/2;

   // Description
   cString text = cString();
   if (!isempty(Event->Description())) {
      text.Append(cString::sprintf("%s\n", Event->Description()));
   }

   if (ElchiConfig.showEPGDetails > 0) {
      text.Append(cString::sprintf("\n%s: %d:%02d", tr("Duration"), (Event->Duration()/60)/60, ((Event->Duration()/60) % 60)));
      
      const cComponents *Components = Event->Components();
      if (Components) {
         cString audio = cString();
         cString audio_type = cString();
         
         cString subtitle = cString();
         for (int i = 0; i < Components->NumComponents(); i++) {
            const tComponent *comp = Components->Component(i);
            switch (comp->stream) {
               case sc_video_MPEG2:
                  if (isempty(comp->description))
                     text.Append(cString::sprintf("\n%s: MPEG2", tr("Video")));
                  else
                     text.Append(cString::sprintf("\n%s: %s (MPEG2)", tr("Video"), comp->description));
                  break;
               case sc_video_H264_AVC:
                  if (isempty(comp->description))
                     text.Append(cString::sprintf("\n%s: H.264", tr("Video")));
                  else
                     text.Append(cString::sprintf("\n%s: %s (H.264)", tr("Video"), comp->description));
                  break;
               case sc_video_H265_HEVC:  //might be not always correct because stream_content_ext (must be 0x0) is not available in tComponent
                  if (isempty(comp->description))
                     text.Append(cString::sprintf("\n%s: H.265", tr("Video")));
                  else
                     text.Append(cString::sprintf("\n%s: %s (H.265)", tr("Video"), comp->description));
                  break;

               case sc_audio_MP2:
               case sc_audio_AC3:
               case sc_audio_HEAAC:

                  switch (comp->stream) {
                     case sc_audio_MP2:
                        // workaround for wrongfully used stream type X 02 05 for AC3
                        if (comp->type == 5)
                           audio_type = "AC3";
                        else
                           audio_type = "MP2";
                        break;
                     case sc_audio_AC3:
                        audio_type = "AC3"; break;
                     case sc_audio_HEAAC:
                        audio_type = "HEAAC"; break;
                  }

                  if (!isempty(audio))
                     audio.Append(", ");
                  
                  if (!isempty(comp->description) && !isempty(comp->language))
                  {
                     audio.Append(cString::sprintf("%s (%s, %s)", comp->description, *audio_type, comp->language));
                  }
                  else if (!isempty(comp->language))
                        audio.Append((const char *)cString::sprintf("%s (%s)", comp->language, *audio_type));
                     else
                        audio.Append((const char *)audio_type);

                  break;
               case sc_subtitle:
                  if (!isempty(subtitle))
                     subtitle.Append(", ");

                  if (!isempty(comp->description) && !isempty(comp->language))
                     subtitle.Append(cString::sprintf("%s (%s)", comp->description, comp->language));
                  else if (!isempty(comp->language))
                        subtitle.Append(comp->language);
                  break;
            }
         }
         if (!isempty(audio))
            text.Append(cString::sprintf("\n%s: %s", tr("Audio"), *audio));

         if (!isempty(subtitle))
            text.Append(cString::sprintf("\n%s: %s", tr("Subtitle"), *subtitle));
      }
   }

   if ( !isempty(text)) {
      int slh = smallfont->Height();
      cTextWrapper tw2;
      scrollShownLines = (y5 - y)/slh;
      scrollOffsetLines = 0;
      int eventLines, upperLines = 0;

      if (!hasEPGimages)
      {
         tw.Set(text, smallfont, x5 - x1);
         scrollTotalLines = tw.Lines();
         scrollbarTop = y;
         scrollbarHeight = y5 - y;
         eventLines = tw.Lines();
      }
      else {
         tw.Set(text, smallfont, x4 - x1 - lh2);
         upperLines = (y4-y + slh - 1)/slh;
         scrollTotalLines = upperLines;
         scrollbarTop = y4;
         scrollbarHeight = y5 - y4;
         eventLines = upperLines;

         if (tw.Lines() > scrollTotalLines)
         {
            tw2.Set(strstr(text,tw.GetLine(upperLines)), smallfont, x5 - x1);
            scrollTotalLines += tw2.Lines();
            eventLines += tw2.Lines();
         }
      }

      LOCK_PIXMAPS;
      if (pmEvent)
         osd->DestroyPixmap(pmEvent);
      pmEvent = osd->CreatePixmap(LYR_SCROLL, cRect(x1, y, x5 - x1, scrollShownLines*slh), cRect(0, 0, x5 - x1, eventLines * slh));
      pmEvent->Clear();

      if (!hasEPGimages)
         for (int i = 0; i < tw.Lines(); i++) {
            pmEvent->DrawText(cPoint(0, i*slh), tw.GetLine(i), Theme.Color(clrMenuEventDescription), clrTransparent, smallfont);
         }
      else
      {
         int line = 0;
         for (int i = 0; i < upperLines; i++) {
            pmEvent->DrawText(cPoint(0, line*slh), tw.GetLine(i), Theme.Color(clrMenuEventDescription), clrTransparent, smallfont);
            line++;
         }

         for (int i = 0; i < tw2.Lines(); i++) {
            pmEvent->DrawText(cPoint(0, line*slh), tw2.GetLine(i), Theme.Color(clrMenuEventDescription), clrTransparent, smallfont);
            line++;
         }
      }
      SetTextScrollbar();
   }
}

cString cSkinElchiHDDisplayMenu::stripXmlTag(cString source, const char* tag)
// returns the string between start and end tag or an empty string if tag is not found
{
   const char *start = strstr((char *)*source, *cString::sprintf("<%s>", tag));
   const char *end   = strstr((char *)*source, *cString::sprintf("</%s>", tag));
   
   if (NULL != start && NULL != end) 
      return (cString(start + strlen(tag) +2, end));

   return cString();
}

void cSkinElchiHDDisplayMenu::SetRecording(const cRecording *Recording)
{  ///< Sets the Recording that shall be displayed, using the entire central area
   ///< of the menu. The Recording's 'description' shall be displayed using a
   ///< cTextScroller, and the Scroll() function will be called to drive scrolling
   ///< that text if necessary.
   DSYSLOG("skinelchiHD: DisplayMenu::SetRecording '%s'", Recording?Recording->Name()?Recording->Name():"null":"null");
   if (!Recording)
      return;
   const cRecordingInfo *Info = Recording->Info();
   const cFont *font = cFont::GetFont(fontOsd);
   const cFont *smallfont = cFont::GetFont(fontSml);
   int slh = smallfont->Height();
   lh = font->Height();
   int y = y3;
   int textwidth = x5 - x1;
   bool hasEPGimages = false;
   cTextWrapper tw;

   if (epgimagesize) { // != 0, that is: is requested and can be displayed
      textwidth = x4 - x1 - (x1 - x0);
      pmEPGImage->SetDrawPortPoint(cPoint(0, 0));
      cRect vPort = pmEPGImage->ViewPort();
      vPort.SetHeight(epgImageLines * slh);
      pmEPGImage->SetViewPort(vPort);

      if (!epgimageThread) {
         epgimageThread = new cEpgImage(pmEPGImage, pmEPGImage->DrawPort().Width(), pmEPGImage->DrawPort().Height(), border);
         DSYSLOG("skinelchiHD: DisplayMenu::SetRecording osd Height=%d %.1f lh=%d %dx%d", OSDsize.height, OSDsize.height*0.35, lh, pmEPGImage->DrawPort().Width(), pmEPGImage->DrawPort().Height());
      }

      // start conversion via PutRecording
      hasEPGimages = epgimageThread->PutRecording((const char *)Recording->FileName());
   }
   
   cString t = cString::sprintf("%s  %s", *DateString(Recording->Start()), *TimeString(Recording->Start()));
   pmMenu->DrawText(cPoint(x1, y), t, Theme.Color(clrMenuEventTime), clrTransparent, font);
   y += lh;

   // Parentalrating
   if ((ElchiConfig.showRecDetails == 2) && Info->GetEvent()->ParentalRating()) {
      cString buffer = cString::sprintf(" %s ", *Info->GetEvent()->GetParentalRatingString());
      pmMenu->DrawText(cPoint(x1, y), buffer, Theme.Color(clrMenuEventVpsFg), Theme.Color(clrMenuEventVpsBg), smallfont);
   }
   y += lh;

   const char *Title = Info->Title();
   if (isempty(Title))
      Title = Recording->Name();
   tw.Set(Title, font, textwidth);
   for (int i = 0; i < tw.Lines(); i++) {
      pmMenu->DrawText(cPoint(x1, y), tw.GetLine(i), Theme.Color(clrMenuEventTitle), clrTransparent, font);
      y += lh;
   }

   if (!isempty(Info->ShortText())) {
      smallfont = cFont::GetFont(fontSml);
      tw.Set(Info->ShortText(), smallfont, textwidth);
      for (int i = 0; i < tw.Lines(); i++) {
         pmMenu->DrawText(cPoint(x1, y), tw.GetLine(i), Theme.Color(clrMenuEventShortText), clrTransparent, smallfont);
         y += lh;
      }
   }

   // Content
   if (ElchiConfig.showRecDetails == 2)
   {
      cString content = cString();
      for (int i = 0; Info->GetEvent()->Contents(i); i++) {
         if (!isempty(Info->GetEvent()->ContentToString(Info->GetEvent()->Contents(i)))) { // skip empty (user defined) content
            if (!isempty(content))
               content.Append(", ");
            content.Append(Info->GetEvent()->ContentToString(Info->GetEvent()->Contents(i)));
         }
      }    
      if (!isempty(content)) {
         tw.Set(content, smallfont, textwidth);
         for (int i = 0; i < tw.Lines(); i++) {
            pmMenu->DrawText(cPoint(x1, y), tw.GetLine(i), Theme.Color(clrMenuEventDescription), clrTransparent, smallfont);
            y += lh;
         }
      }
   }

   y += slh/2;
   cString text = cString();

   if (!isempty(Info->Description()))
   {
      text.Append(cString::sprintf("%s\n", Info->Description()));
   }

   if (ElchiConfig.showRecDetails > 0)
   {
      text.Append("\n");
      if (Info->ChannelName())
         text.Append(cString::sprintf("%s: %s\n", trVDR("Channel"), Info->ChannelName()));

      cMarks marks;

      bool hasMarks = marks.Load(Recording->FileName(), Recording->FramesPerSecond(), Recording->IsPesRecording()) && marks.Count();

      cIndexFile *index = new cIndexFile(Recording->FileName(), false, Recording->IsPesRecording());
      int lastIndex = 0;

      int cuttedLength = 0;
      long cutinframe = 0;
      unsigned long long recsize = 0;
      unsigned long long recsizecutted = 0;
      unsigned long long cutinoffset = 0;
      unsigned long long filesize[100000];
      filesize[0] = 0;

      int i = 0;
      int imax;
      struct stat filebuf;
      cString filename;
      int rc = 0;

      do {
         if (Recording->IsPesRecording()) {
            filename = cString::sprintf("%s/%03d.vdr", Recording->FileName(), ++i);
            imax = 999;
         }
         else {
            filename = cString::sprintf("%s/%05d.ts", Recording->FileName(), ++i);
            imax = 99999;
         }

         rc=stat(filename, &filebuf);
         if (rc == 0)
            filesize[i] = filesize[i-1] + filebuf.st_size;
         else
            if (ENOENT != errno) {
               esyslog ("skinelchiHD: error determining file size of \"%s\" %d (%s)", (const char *)filename, errno, strerror(errno));
               recsize = 0;
            }
      } while (i <= imax && !rc);
      recsize = filesize[i-1];

      if (hasMarks && index) {
         uint16_t FileNumber;
         off_t FileOffset;

         bool cutin = true;
         cMark *mark = marks.First();
         while (mark) {
            long position = mark->Position();
            index->Get(position, &FileNumber, &FileOffset);
            if (cutin) {
               cutinframe = position;
               cutin = false;
               cutinoffset = filesize[FileNumber-1] + FileOffset;
            } else {
               cuttedLength += position - cutinframe;
               cutin = true;
               recsizecutted += filesize[FileNumber-1] + FileOffset - cutinoffset;
            }
            cMark *nextmark = marks.Next(mark);
            mark = nextmark;
         }
         if (!cutin) {
            cuttedLength += index->Last() - cutinframe;
            index->Get(index->Last() - 1, &FileNumber, &FileOffset);
            recsizecutted += filesize[FileNumber-1] + FileOffset - cutinoffset;
         }
      }

      if (index) {
         lastIndex = index->Last();
         text.Append(cString::sprintf("%s: %s", tr("Length"), *IndexToHMSF(lastIndex, false, Recording->FramesPerSecond())));
         
         if (hasMarks)
            text.Append(cString::sprintf(" (%s: %s)", tr("cutted"), *IndexToHMSF(cuttedLength, false, Recording->FramesPerSecond())));
         text.Append("\n");
      }
      
      delete index;

      if (recsize > MEGABYTE(1023))
         text.Append(cString::sprintf("%s: %.2f GB", tr("Size"), (float)recsize / MEGABYTE(1024)));
      else
         text.Append(cString::sprintf("%s: %llu MB", tr("Size"), recsize / MEGABYTE(1)));
      if (hasMarks)
         if (recsize > MEGABYTE(1023))
            text.Append(cString::sprintf(" (%s: %.2f GB)", tr("cutted"), (float)recsizecutted/MEGABYTE(1024)));
         else
            text.Append(cString::sprintf(" (%s: %llu MB)", tr("cutted"), recsizecutted/MEGABYTE(1)));

      text.Append(cString::sprintf("\n%s: %d, %s: %d", trVDR("Priority"), Recording->Priority(), trVDR("Lifetime"), Recording->Lifetime()));

      if (lastIndex) {
         text.Append(cString::sprintf("\n%s: %s, %s: ~%.2f MBit/s (Video + Audio)", tr("format"), (Recording->IsPesRecording() ? "PES" : "TS"), tr("bit rate"), (float)recsize/lastIndex*Recording->FramesPerSecond()*8/MEGABYTE(1)));
      }

      const cComponents *Components = Info->Components();
      if (Components) {
         cString audio = cString();
         cString audio_type = cString();
         
         cString subtitle = cString();
         for (int i = 0; i < Components->NumComponents(); i++) {
            const tComponent *comp = Components->Component(i);
            switch (comp->stream) {
               case sc_video_MPEG2:
                  if (isempty(comp->description))
                     text.Append(cString::sprintf("\n%s: MPEG2", tr("Video")));
                  else
                     text.Append(cString::sprintf("\n%s: %s (MPEG2)", tr("Video"), comp->description));
                  break;
               case sc_video_H264_AVC:
                  if (isempty(comp->description))
                     text.Append(cString::sprintf("\n%s: H.264", tr("Video")));
                  else
                     text.Append(cString::sprintf("\n%s: %s (H.264)", tr("Video"), comp->description));
                  break;
               case sc_video_H265_HEVC:
                  if (isempty(comp->description))
                     text.Append(cString::sprintf("\n%s: H.265", tr("Video")));
                  else
                     text.Append(cString::sprintf("\n%s: %s (H.265)", tr("Video"), comp->description));
                  break;

               case sc_audio_MP2:
               case sc_audio_AC3:
               case sc_audio_HEAAC:
                  switch (comp->stream) {
                     case sc_audio_MP2:
                        // workaround for wrongfully used stream type X 02 05 for AC3
                        if (comp->type == 5)
                           audio_type = "AC3";
                        else
                           audio_type = "MP2";
                        break;
                     case sc_audio_AC3:
                        audio_type = "AC3"; break;
                     case sc_audio_HEAAC:
                        audio_type = "HEAAC"; break;
                  }
                  if (!isempty(audio))
                     audio.Append(", ");
                  
                  if (!isempty(comp->description) && !isempty(comp->language))
                     audio.Append(cString::sprintf("%s (%s, %s)", comp->description, *audio_type, comp->language));
                  else if (!isempty(comp->language))
                        audio.Append(cString::sprintf("%s (%s)", comp->language, *audio_type));
                     else
                        audio.Append(audio_type);

                  break;
               case sc_subtitle:
                  if (!isempty(subtitle))
                     subtitle.Append(", ");

                  if (!isempty(comp->description) && !isempty(comp->language))
                     subtitle.Append(cString::sprintf("%s (%s)", comp->description, comp->language));
                  else if (!isempty(comp->language))
                        subtitle.Append(comp->language);
                  break;
            }
         }
         if (!isempty(audio))
            text.Append(cString::sprintf("\n%s: %s", tr("Audio"), *audio));
         if (!isempty(subtitle))
            text.Append(cString::sprintf("\n%s: %s", tr("Subtitle"), *subtitle));
      }

      if (Info->Aux())
      {
         cString str_epgsearch = stripXmlTag(Info->Aux(), "epgsearch");
         
         cString channel, searchtimer, pattern;

         if (!isempty(str_epgsearch))
         {
            channel = stripXmlTag(str_epgsearch, "channel");
            searchtimer = stripXmlTag(str_epgsearch, "searchtimer");
            if (isempty(searchtimer))
               searchtimer = stripXmlTag(str_epgsearch, "Search timer");
         }

         cString str_vdradmin = stripXmlTag(Info->Aux(), "vdradmin-am");

         if (!isempty(str_vdradmin))
            pattern = stripXmlTag(str_vdradmin, "pattern");

         if ((!isempty(channel) && !(isempty(searchtimer))) || !isempty(pattern))
         {
            text.Append(cString::sprintf("\n\n%s:\n", tr("additional information")));
            if (*channel != NULL && *searchtimer != NULL) {
               text.Append(cString::sprintf("EPGsearch: %s: %s, %s: %s", tr("channel"), *channel, tr("search pattern"), *searchtimer));
            }
            if (*pattern != NULL) {
               text.Append(cString::sprintf("VDRadmin-AM: %s: %s", tr("search pattern"), *pattern));
            }
         }

      }
   }

   if (!isempty(text)) {
      int slh = smallfont->Height();
      cTextWrapper tw2;
      scrollShownLines = (y5 - y)/slh;
      scrollOffsetLines = 0;
      int eventLines, upperLines = 0;
      
      if (!hasEPGimages)
      {
         tw.Set(text, smallfont, x5 - x1);
         scrollTotalLines = tw.Lines();
         scrollbarTop = y;
         scrollbarHeight = y5 - y;
         eventLines = tw.Lines();
      }
      else {
         tw.Set(text, smallfont, x4 - x1 - lh2);
         upperLines = (y4-y + slh - 1)/slh;
         scrollTotalLines = upperLines;
         scrollbarTop = y4;
         scrollbarHeight = y5 - y4;
         eventLines = upperLines;

         if (tw.Lines() > scrollTotalLines)
         {
            tw2.Set(strstr(text,tw.GetLine(upperLines)), smallfont, x5 - x1);
            scrollTotalLines += tw2.Lines();
            eventLines += tw2.Lines();
         }
      }
      
      LOCK_PIXMAPS;
      if (pmEvent)
         osd->DestroyPixmap(pmEvent);
      pmEvent = osd->CreatePixmap(LYR_SCROLL, cRect(x1, y, x5 - x1, scrollShownLines * slh), cRect(0, 0, x5 - x1, eventLines * slh)); 
      pmEvent->Clear();

      if (!hasEPGimages)
         for (int i = 0; i < tw.Lines(); i++) {
            pmEvent->DrawText(cPoint(0, i*slh), tw.GetLine(i), Theme.Color(clrMenuEventDescription), clrTransparent, smallfont);
         }
      else
      {
         int line = 0;
         for (int i = 0; i < upperLines; i++) {
            pmEvent->DrawText(cPoint(0, line*slh), tw.GetLine(i), Theme.Color(clrMenuEventDescription), clrTransparent, smallfont);
            line++;
         }
         for (int i = 0; i < tw2.Lines(); i++) {
            pmEvent->DrawText(cPoint(0, line*slh), tw2.GetLine(i), Theme.Color(clrMenuEventDescription), clrTransparent, smallfont);
            line++;
         }
      }
      SetTextScrollbar();
   }

   changed = true;
}

void cSkinElchiHDDisplayMenu::SetText(const char *Text, bool FixedFont)
{  ///< Sets the Text that shall be displayed, using the entire central area
   ///< of the menu. The Text shall be displayed using a cTextScroller, and
   ///< the Scroll() function will be called to drive scrolling that text if
   ///< necessary.
   DSYSLOG("skinelchiHD: DisplayMenu::SetText(%s,%s)", Text, FixedFont ? "'FixedFont'" : "'nonFixedFont'")
   changed = true;
   //TODO replace textScroller
   textScroller.Set(osd, x1, menuTop, GetTextAreaWidth(), menuHeight, Text, GetTextAreaFont(FixedFont), Theme.Color(clrMenuText), Theme.Color(clrBackground));
   SetTextScrollbar();
}

int cSkinElchiHDDisplayMenu::GetTextAreaWidth(void) const
{  ///< Returns the width in pixel of the area which is used to display text
   ///< with SetText(). The width of the area is the width of the central area
   ///< minus the width of any possibly displayed scroll-bar or other decoration.
   ///< The default implementation returns 0. Therefore a caller of this method
   ///< must be prepared to receive 0 if the plugin doesn't implement this method.

   return x5 - x1;  // x6 - x1
}

const cFont *cSkinElchiHDDisplayMenu::GetTextAreaFont(bool FixedFont) const
{  ///< Returns a pointer to the font which is used to display text with SetText().
   ///< The parameter FixedFont has the same meaning as in SetText(). The default
   ///< implementation returns NULL. Therefore a caller of this method must be
   ///< prepared to receive NULL if the plugin doesn't implement this method.
   ///< The returned pointer is valid a long as the instance of cSkinDisplayMenu
   ///< exists.

   return cFont::GetFont(FixedFont ? fontFix : fontOsd);
}

void cSkinElchiHDDisplayMenu::Flush(void)
{  ///< Actually draws the OSD display to the output device.
#ifdef DEBUG
   DSYSLOG("skinelchiHD: DisplayMenu::Flush()");

   timeval tp1, tp2;
   gettimeofday(&tp1, NULL);
#endif
   ///if (lasttime.tv_sec) isyslog("skinelchiHD Menu: FlushDelta=%.2f msec", (((long long)tp1.tv_sec * 1000000 + tp1.tv_usec) - ((long long)lasttime.tv_sec * 1000000 + lasttime.tv_usec)) / 1000.0);
   //lasttime=tp1;

   if (cVideoDiskUsage::HasChanged(lastDiskUsageState))
      DrawTitle();
   
   const cFont *font = cFont::GetFont(fontOsd);
   lh = font->Height();
   
   cString date = DayDateTime();
   if (!*lastDate || strcmp(date, lastDate)) {
      changed = true;
      pmMenu->DrawRectangle(cRect(x6 - font->Width(lastDate), y0, font->Width(lastDate), lh), clrTransparent);
      pmMenu->DrawText(cPoint(x6 - font->Width(date), y0), date, Theme.Color(clrMenuDate), clrTransparent, font);
      lastDate = date;
   }

   if (!timersDisplayed && timercheck) {
      timersDisplayed = true;
      LOCK_TIMERS_READ
      const cTimer *nexttimer = Timers->GetNextActiveTimer();
      if (nexttimer) {
         const cFont *font = cFont::GetFont(fontOsd);
         Epgsearch_lastconflictinfo_v1_0 conflictinfo;
         conflictinfo.nextConflict = 0;
         conflictinfo.relevantConflicts = 0;
         conflictinfo.totalConflicts = 0;

         if ((timercheck & 2) && cPluginManager::CallFirstService(EPGSEARCH_CONFLICTINFO, &conflictinfo)) {
            if (conflictinfo.nextConflict) {
               DSYSLOG2("skinelchiHD: timer conflict found: %s", *DayDateTime(conflictinfo.nextConflict));
               cString conflict = cString::sprintf("%s %s", tr(" timer conflicts! "), *DayDateTime(conflictinfo.nextConflict));
               pmMenu->DrawText(cPoint(x1, y2), conflict, Theme.Color(clrMessageWarningFg), Theme.Color(clrMessageWarningBg), font, x6 - x1, y2 - y0, taRight);
            }
         }
         if (timercheck & 1 && !conflictinfo.nextConflict) {
            cString s = cString::sprintf("%s: %s %d %02d:%02d %s - %s", tr("Next recording"), *WeekDayName(nexttimer->StartTime()),
                                         nexttimer->GetMDay(nexttimer->Day()), nexttimer->Start() / 100, nexttimer->Start() % 100,
                                         nexttimer->Channel()->Name(), nexttimer->File());
            pmMenu->DrawText(cPoint(x1, y2), s, Theme.Color(clrMenuItemNonSelectable), clrTransparent, font);
         }
      }
   }

   if (!showMessage) {
      int volume, newVolumechange;
      newVolumechange = ElchiStatus->GetVolumeChange(&volume);
      if (volumechange != newVolumechange) {
         volumechange = newVolumechange;
//      if (ElchiStatus->GetVolumeChange(&newVolume) != volumechange) {
//         volumechange = ElchiStatus->GetVolumeChange();

         LOCK_PIXMAPS;
         int wVolumeBar = (x5 - lh) * volume / 255;

         DrawShadedRectangle(pmBG, Theme.Color(clrVolumeBarUpper), cRect(lh2, y5 + lh/4 , wVolumeBar, lh/2));
         DrawShadedRectangle(pmBG, Theme.Color(clrVolumeBarLower), cRect(lh2 + wVolumeBar, y5 + lh/4 , (x5 - lh) - wVolumeBar, lh/2));

         showVolume = true;
         volumeTimer.Set(1000);
         changed = true;
      }
      else {
         if (showVolume && volumeTimer.TimedOut()) {
            LOCK_PIXMAPS;
            pmBG->DrawRectangle(cRect(x0, y5, x5, lh), Theme.Color(clrBackground));
            showVolume = false;
            changed = true;
         }
      }
   }

   /*if (ElchiConfig.clrdlgActive) {
      DSYSLOG2("skinelchiHD: colorchanging - DisplayMenu::Flush()")
      lastClrDlgFg = ElchiConfig.clrDlgFg;
      lastClrDlgBg = ElchiConfig.clrDlgBg;

      pmMenu->DrawText(cPoint(x0, y5), "Test Message",
         lastClrDlgFg, lastClrDlgBg, font, x8 - x0, y8 - y5, taCenter);
   }
   else */
   {
      if (changed) {
#ifdef DEBUG
         //DSYSLOG2("skinelchiHD: DisplayMenu::Flush()")
         timeval tp1, tp2;
         gettimeofday(&tp1, NULL);
#endif
         osd->Flush();
#ifdef DEBUG         
         gettimeofday(&tp2, NULL);
         dsyslog("skinelchiHD: DisplayMenu::osd->Flush() %4.3f ms",
            (((long long)tp2.tv_sec * 1000000 + tp2.tv_usec) - ((long long)tp1.tv_sec * 1000000 + tp1.tv_usec)) / 1000.0);
#endif
         changed = false;
      }
   }

#ifdef DEBUG   
   gettimeofday(&tp2, NULL);
   double msec = (((long long)tp2.tv_sec * 1000000 + tp2.tv_usec) - ((long long)tp1.tv_sec * 1000000 + tp1.tv_usec)) / 1000.0;
   if (msec > 0.2) isyslog("skinelchiHD: DisplayMenu::Flush() all: %4.3f ms", msec);
#endif
}


