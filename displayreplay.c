/*
 * displayreplay.c: Replay display handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#include <vdr/menu.h>
#include <vdr/device.h>

//#define DEBUG
//#define DEBUG2
#include "displayreplay.h"
#include "setup.h"
#include "common.h"
#include "symbols.h"
#include "vdrstatus.h"

#define STR_VIDEOSIZE "  0000x0000"

extern cSkinElchiStatus *ElchiStatus;

// --- cSkinElchiHDDisplayReplay ---------------------------------------------
cSkinElchiHDDisplayReplay::cSkinElchiHDDisplayReplay(bool ModeOnly)
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayReplay::cSkinElchiHDDisplayReplay(%s)", ModeOnly ? "'ModeOnly'" : "'nonModeOnly'")

   modeonly = ModeOnly;
   showVolume = false;
   showMessage = false;
   volumechange = ElchiStatus->GetVolumeChange(NULL);
   isRecording = false;
   isCutting = false;
   rectitle = NULL;
   title = NULL;
   marks = NULL;
   oldCurrent = NULL;
   oldVideoFormat = videofmt_unknown;
   oldWidth = -1;
   oldHeight = -1;
   const cFont *font = cFont::GetFont(fontOsd);
   const cFont *smallfont = cFont::GetFont(fontSml);
   lh = (font->Height() + 1) & ~0x01;   // should be even
   lh2 = lh  / 2;
   lh4 = lh2 / 2;
   lh8 = lh4 / 2;

   tOSDsize OSDsize;

   OSDsize.left   = cOsd::OsdLeft();
   OSDsize.top    = cOsd::OsdTop();
   OSDsize.width  = cOsd::OsdWidth();
   OSDsize.height = cOsd::OsdHeight();

   int OSDHeight = 720;
   int OSDWidth = 1280;
   double OSDAspect = 16.0/9.0;

   cDevice::PrimaryDevice()->GetOsdSize(OSDWidth, OSDHeight, OSDAspect);
   if (!OSDHeight)
      OSDHeight = OSDsize.height;

   DSYSLOG("skinelchiHD: OSDsize Replay %dx%d left=%d/%d top=%d/%d width=%d/%d heigth=%d/%d",
           OSDWidth, OSDHeight, OSDsize.left, Setup.OSDLeft, OSDsize.top, Setup.OSDTop, OSDsize.width, Setup.OSDWidth, OSDsize.height, Setup.OSDHeight)


   if (OSDHeight >= 2160) {
      Gap            = 12;
      SymbolGap      = 12;
      MarksWidth     = 4;
   } else if (OSDHeight >= 1080) {
      Gap            = 6;
      SymbolGap      = 6;
      MarksWidth     = 2;
   } else if (OSDHeight >= 720) {
      Gap            = 4;
      SymbolGap      = 4;
      MarksWidth     = 2;
   } else {  // <  720 incl. 576
      Gap            = 3;
      SymbolGap      = 3;
      MarksWidth     = 1;
   }

   elchiSymbols.Refresh(OSDHeight);
   x0 = 0;
   x9 = OSDsize.width;
   x6 = x9 - lh2 - smallfont->Width("00:00:00.00 / 00:00:00");

   xSymbols[xSYM_AR]   = 2*lh + SymbolGap; 
   xSymbols[xSYM_CUTTING]  = (ElchiConfig.showVideoInfo ? xSymbols[xSYM_AR] + elchiSymbols.Width(SYM_AR_HD) + SymbolGap : 2*lh + SymbolGap);
   xSymbols[xSYM_REC]  = xSymbols[xSYM_CUTTING] + elchiSymbols.Width(SYM_CUTTING) + SymbolGap;
   xSymbols[xSYM_FREW] = xSymbols[xSYM_REC] + elchiSymbols.Width(SYM_REC) + SymbolGap;
   xSymbols[xSYM_SREW] = xSymbols[xSYM_FREW] + elchiSymbols.Width(SYM_FREW) + SymbolGap;
   xSymbols[xSYM_PLAY] = xSymbols[xSYM_SREW] + elchiSymbols.Width(SYM_SREW) + SymbolGap;
   xSymbols[xSYM_SFWD] = xSymbols[xSYM_PLAY] + elchiSymbols.Width(SYM_PLAY) + SymbolGap;
   xSymbols[xSYM_FFWD] = xSymbols[xSYM_SFWD] + elchiSymbols.Width(SYM_SFWD) + SymbolGap;

   xMode = x9 - lh + lh2 - xSymbols[xSYM_FFWD] - elchiSymbols.Width(SYM_FFWD) - SymbolGap;
   int wMode = x9 - xMode;

   x4 = x6 - 2 * lh;
   x2 = xMode;

   y0 = 0;
   y1 = lh;
   y2 = y1 + lh;
   y3 = y2 + lh;
   ySymbols   = (lh - elchiSymbols.Height(SYM_PLAY))/2;
   ySymbolARcutRec = (lh - elchiSymbols.Height(SYM_AR_UHD))/2;

   osd = cOsdProvider::NewOsd(OSDsize.left, OSDsize.top + OSDsize.height - y3);
   ElchiBackground->SetOSD(osd);
   tArea Area[] = { { x0, y0, x9 - 1, y3 - 1, 32 } };
   osd->SetAreas(Area, 1);
   clrBG = Theme.Color(clrBackground);

   pmBG = osd->CreatePixmap(LYR_BG, cRect(x0, y0, x9, y2));
   pmBG->Clear();
   pmTitleBG = osd->CreatePixmap(LYR_BG, cRect(x0, y0, x4 - x0, lh));
   pmTitleBG->Clear();
   pmMode = osd->CreatePixmap(LYR_BG, cRect(xMode, y2, x9 - xMode, lh));
   pmMode->Clear();
   pmJump = osd->CreatePixmap(LYR_HIDDEN, cRect(0, y2, xMode, lh));
   pmJump->Clear();
   pmProgress = osd->CreatePixmap(LYR_BG, cRect(lh2, y1, x9 - lh2, lh));
   pmProgress->Clear();
   pmProgressBar = osd->CreatePixmap(LYR_SELECTOR, cRect(lh, y1 + lh8, x9 - 3*lh2, lh - 2*lh8));
   pmProgressBar->Clear();
   pmVolume = osd->CreatePixmap(LYR_HIDDEN, cRect(lh2, y1, x9 - lh2, lh));
   pmVolume->Fill(clrBG);

   pmMessageBG = osd->CreatePixmap(LYR_HIDDEN, cRect(lh2, y1, x9 - lh2, lh));
   spmMessage = new cScrollingPixmap(osd, cRect(lh2, y1, x9 - lh2, lh),
                                     cFont::GetFont(fontOsd), 256, clrBlack, clrTransparent, true);

   if (ModeOnly) {
      pmMode->DrawRectangle(cRect(2*lh - lh2, 0, wMode - 2*lh + lh2, lh), clrBG);
      pmMode->DrawEllipse(cRect(2*lh - lh2, 0, lh2, lh2), clrTransparent, -2);
      pmMode->DrawEllipse(cRect(2*lh - lh2, lh - lh2, lh2, lh2), clrTransparent, -3);
      pmMode->DrawEllipse(cRect(wMode - lh2, 0, lh2, lh2), clrTransparent, -1);
      pmMode->DrawEllipse(cRect(wMode - lh2, lh - lh2, lh2, lh2), clrTransparent, -4);

      pmVolume->DrawEllipse(cRect(0, 0, lh - lh2, lh2), clrTransparent, -2);
      pmVolume->DrawEllipse(cRect(0, lh - lh2, lh - lh2, lh2), clrTransparent, -3);
      pmVolume->DrawEllipse(cRect(x9 - lh2 - lh2, 0, lh - lh2, lh2), clrTransparent, -1);
      pmVolume->DrawEllipse(cRect(x9 - lh2 - lh2, lh - lh2, lh - lh2, lh2), clrTransparent, -4);
   }
   else {
      pmBG->DrawSlope(cRect(x4, y0, 2*lh, lh), clrBG, 0);
      pmBG->DrawRectangle(cRect(x6, y0, x9 -x6, lh), clrBG);

      pmProgress->DrawRectangle(cRect(0, 0, x9 - lh2, lh), clrBG);
      pmProgress->DrawEllipse(cRect(0, 0, lh - lh2, lh2), clrTransparent, -2);
      pmProgress->DrawEllipse(cRect(0, lh - lh2, lh - lh2, lh2), clrTransparent, -3);
      pmVolume->DrawEllipse(cRect(0, 0, lh - lh2, lh2), clrTransparent, -2);
      pmVolume->DrawEllipse(cRect(0, lh - lh2, lh - lh2, lh2), clrTransparent, -3);
      pmBG->DrawRectangle(cRect(x6, y0, x9 -lh2 - lh2, lh), clrBG);
      pmBG->DrawEllipse(cRect(x9 - lh2, y0, lh2, lh2), clrTransparent, -1);

      if (Setup.ShowReplayMode) {
         pmMode->DrawSlope(cRect(0, 0, 2*lh, lh), clrBG, 3);
         pmMode->DrawRectangle(cRect(2*lh, 0, x9 - xMode -2*lh, lh), clrBG);
         pmMode->DrawEllipse(cRect(wMode - lh2, lh - lh2, lh2, lh2), clrTransparent, -4);
      }
      else {
         pmBG->DrawEllipse(cRect(x9 - lh2, y2 - lh2, lh2, lh2), clrTransparent, -4);
      }

      const char *separator = " / ";
      xTotalWidth = smallfont->Width("00:00:00");
      xTotal = x9 - xTotalWidth - y1 / 2;
      pmBG->DrawText(cPoint(xTotal - smallfont->Width(separator), y0), separator, Theme.Color(clrReplayCurrent), Theme.Color(clrBackground), smallfont);
      xCurrentWidth = smallfont->Width("00:00:00.00");
      xCurrent = xTotal - smallfont->Width(separator) - xCurrentWidth;

   }
   spmTitle = NULL;

   changed = true;
}

cSkinElchiHDDisplayReplay::~cSkinElchiHDDisplayReplay()
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayReplay::~cSkinElchiHDDisplayReplay()")
   DELETENULL(spmTitle);
   DELETENULL(spmMessage);

   ElchiBackground->SetOSD(NULL);
   DELETENULL(osd);
}

/*
void cSkinElchiHDDisplayReplay::SetRecording(const cRecording *Recording)
{
   recording = Recording;
   SetTitle( recording->Info()->Title() );
}
*/

void cSkinElchiHDDisplayReplay::SetMarks(const cMarks *Marks)
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayReplay::SetMarks")
   marks = Marks;
}

void cSkinElchiHDDisplayReplay::SetTitle(const char *Title)
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayReplay::SetTitle(%s)", Title)

   rectitle = Title;
   SetScrollTitle(rectitle);
}


void cSkinElchiHDDisplayReplay::SetScrollTitle(const char *Title)
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayReplay::SetScrollTitle(%s)", Title)

   const cFont *font = cFont::GetFont(fontOsd);
   const cFont *smallFont = cFont::GetFont(fontSml);

   int w = smallFont->Width(Title);
   if (showVolume)
      w += 2*smallFont->Height()/TEXT_ALIGN_BORDER;

   if(ElchiConfig.showVideoInfo > 1 && !showVolume)
      w += smallFont->Width(STR_VIDEOSIZE);

   int h = font->Height() - smallFont->Height();
   w = min(w, x4 - x0 - h);

   DELETENULL(spmTitle);

   spmTitle = new cScrollingPixmap(osd, cRect(x0, y0, w, smallFont->Height()),
                                   smallFont, 256, Theme.Color(clrMenuTitleFg));
   spmTitle->SetText(Title, smallFont);

   LOCK_PIXMAPS;
   pmTitleBG->Clear();
   pmTitleBG->SetLayer(LYR_BG);

   pmTitleBG->DrawRectangle(cRect(x0, y0, w, lh - h), Theme.Color(clrReplayTitleBg));
   pmTitleBG->DrawRectangle(cRect(w, y0 + h, h, lh - h), clrBG);
   pmTitleBG->DrawRectangle(cRect(x0 + h, y1 - h, w - x0 - h, h), clrBG);
   changed = true;
}

void cSkinElchiHDDisplayReplay::SetMode(bool Play, bool Forward, int Speed)
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayReplay::SetMode(%s,%s,Speed=%d)", Play ? "'Play'" : "'Pause'", Forward ? "'Forward'" : "'Backward'", Speed)

   if (Setup.ShowReplayMode) {
      tColor clrOn  = Theme.Color(clrReplaySymbolOn);
      tColor clrOff = Theme.Color(clrReplaySymbolOff);

      if (Speed < -1)
         Speed = -1;
      if (Speed > 9)
         Speed = 9;

      isRecording = cRecordControls::Active();
      pmMode->DrawBitmap(cPoint(xSymbols[xSYM_REC], ySymbolARcutRec), elchiSymbols.Get(SYM_REC, isRecording ? Theme.Color(clrSymbolRecFg) : clrOff, isRecording ? Theme.Color(clrSymbolRecBg) : clrBG));

      isCutting = RecordingsHandler.Active();
      pmMode->DrawBitmap(cPoint(xSymbols[xSYM_CUTTING], ySymbolARcutRec), elchiSymbols.Get(SYM_CUTTING, isCutting ? clrOn : clrOff, clrBG));

      cBitmap *bmp = NULL;
      if ((Speed > -1) && Play && !Forward) {
         switch (Speed) {
            case 0:
            case 1: bmp = &elchiSymbols.Get(SYM_FREW,  clrOn, clrBG); break;
            case 2: bmp = &elchiSymbols.Get(SYM_FREW1, clrOn, clrBG); break;
            case 3: bmp = &elchiSymbols.Get(SYM_FREW2, clrOn, clrBG); break;
            case 4: bmp = &elchiSymbols.Get(SYM_FREW3, clrOn, clrBG); break;
            case 5: bmp = &elchiSymbols.Get(SYM_FREW4, clrOn, clrBG); break;
            case 6: bmp = &elchiSymbols.Get(SYM_FREW5, clrOn, clrBG); break;
            case 7: bmp = &elchiSymbols.Get(SYM_FREW6, clrOn, clrBG); break;
            case 8: bmp = &elchiSymbols.Get(SYM_FREW7, clrOn, clrBG); break;
            case 9: bmp = &elchiSymbols.Get(SYM_FREW8, clrOn, clrBG); break;
         }
         pmMode->DrawBitmap(cPoint(xSymbols[xSYM_FREW], ySymbols), *bmp);
      }
      else
         pmMode->DrawBitmap(cPoint(xSymbols[xSYM_FREW], ySymbols), elchiSymbols.Get(SYM_FREW, clrOff, clrBG));

      if ((Speed > -1) && !Play && !Forward) {
         switch (Speed) {
            case 0:
            case 1: bmp = &elchiSymbols.Get(SYM_SREW,  clrOn, clrBG); break;
            case 2: bmp = &elchiSymbols.Get(SYM_SREW1, clrOn, clrBG); break;
            case 3: bmp = &elchiSymbols.Get(SYM_SREW2, clrOn, clrBG); break;
            case 4: bmp = &elchiSymbols.Get(SYM_SREW3, clrOn, clrBG); break;
            case 5: bmp = &elchiSymbols.Get(SYM_SREW4, clrOn, clrBG); break;
            case 6: bmp = &elchiSymbols.Get(SYM_SREW5, clrOn, clrBG); break;
            case 7: bmp = &elchiSymbols.Get(SYM_SREW6, clrOn, clrBG); break;
            case 8: bmp = &elchiSymbols.Get(SYM_SREW7, clrOn, clrBG); break;
            case 9: bmp = &elchiSymbols.Get(SYM_SREW8, clrOn, clrBG); break;
         }
         pmMode->DrawBitmap(cPoint(xSymbols[xSYM_SREW], ySymbols), *bmp);
      }
      else
         pmMode->DrawBitmap(cPoint(xSymbols[xSYM_SREW], ySymbols), elchiSymbols.Get(SYM_SREW, clrOff, clrBG));

      if (Speed == -1)
         pmMode->DrawBitmap(cPoint(xSymbols[xSYM_PLAY], ySymbols), Play ? elchiSymbols.Get(SYM_PLAY, clrOn, clrBG) : elchiSymbols.Get(SYM_PAUSE, clrOn, clrBG));
      else
         pmMode->DrawBitmap(cPoint(xSymbols[xSYM_PLAY], ySymbols), elchiSymbols.Get(SYM_PLAY, clrOff, clrBG));

      if ((Speed > -1) && !Play && Forward) {
         switch (Speed) {
            case 0: 
            case 1: bmp = &elchiSymbols.Get(SYM_SFWD,  clrOn, clrBG); break;
            case 2: bmp = &elchiSymbols.Get(SYM_SFWD1, clrOn, clrBG); break;
            case 3: bmp = &elchiSymbols.Get(SYM_SFWD1, clrOn, clrBG); break;
            case 4: bmp = &elchiSymbols.Get(SYM_SFWD1, clrOn, clrBG); break;
            case 5: bmp = &elchiSymbols.Get(SYM_SFWD1, clrOn, clrBG); break;
            case 6: bmp = &elchiSymbols.Get(SYM_SFWD1, clrOn, clrBG); break;
            case 7: bmp = &elchiSymbols.Get(SYM_SFWD1, clrOn, clrBG); break;
            case 8: bmp = &elchiSymbols.Get(SYM_SFWD1, clrOn, clrBG); break;
            case 9: bmp = &elchiSymbols.Get(SYM_SFWD1, clrOn, clrBG); break;
         }
         pmMode->DrawBitmap(cPoint(xSymbols[xSYM_SFWD], ySymbols), *bmp);
      }
      else
         pmMode->DrawBitmap(cPoint(xSymbols[xSYM_SFWD], ySymbols), elchiSymbols.Get(SYM_SFWD, clrOff, clrBG));

      if ((Speed > -1) && Play && Forward) {
         switch (Speed) {
            case 0: 
            case 1: bmp = &elchiSymbols.Get(SYM_FFWD,  clrOn, clrBG); break;
            case 2: bmp = &elchiSymbols.Get(SYM_FFWD1, clrOn, clrBG); break;
            case 3: bmp = &elchiSymbols.Get(SYM_FFWD2, clrOn, clrBG); break;
            case 4: bmp = &elchiSymbols.Get(SYM_FFWD3, clrOn, clrBG); break;
            case 5: bmp = &elchiSymbols.Get(SYM_FFWD4, clrOn, clrBG); break;
            case 6: bmp = &elchiSymbols.Get(SYM_FFWD5, clrOn, clrBG); break;
            case 7: bmp = &elchiSymbols.Get(SYM_FFWD6, clrOn, clrBG); break;
            case 8: bmp = &elchiSymbols.Get(SYM_FFWD7, clrOn, clrBG); break;
            case 9: bmp = &elchiSymbols.Get(SYM_FFWD8, clrOn, clrBG); break;
         }
         pmMode->DrawBitmap(cPoint(xSymbols[xSYM_FFWD], ySymbols), *bmp);
      }
      else
         pmMode->DrawBitmap(cPoint(xSymbols[xSYM_FFWD], ySymbols), elchiSymbols.Get(SYM_FFWD, clrOff, clrBG));

      changed = true;
   }
}

void cSkinElchiHDDisplayReplay::SetProgress(int Current, int Total)
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayReplay::SetProgress(%d,%d)", Current, Total)
   if (!showVolume) 
   {
      int width = x9 - 3*lh2;
      int height = lh - 2*lh8;
      if (Total > 0)
      {
         int p = GetPos(Current, width, Total);
         LOCK_PIXMAPS;
         DrawShadedRectangle(pmProgressBar, Theme.Color(clrReplayProgressSeen), cRect(0, 0, p, height));
         pmProgressBar->DrawRectangle(cRect(p, 0, width - p, height), Theme.Color(clrReplayProgressRest));
         changed = true;

         if (marks)
         {
            bool Start = true;
            for (const cMark *m = marks->First(); m; m = marks->Next(m))
            {
               int p1 = GetPos(m->Position(), width, Total);
               if (Start)
               {
                  const cMark *m2 = marks->Next(m);
                  int p2 = m2 ? GetPos(m2->Position(), width, Total) : width;
                  int h4 = height / 4;
                  DrawShadedRectangle(pmProgressBar, Theme.Color(clrReplayProgressSelected), cRect(p1, h4, p2 - p1, height - 2*h4));
               }
               DrawMark(width, p1, height, Start, m->Position() == Current, Theme.Color(clrReplayProgressMark), Theme.Color(clrReplayProgressCurrent));
               Start = !Start;
            }
         }

#if APIVERSNUM >= 30004
         if (errors) {
            tColor colorError = Theme.Color(clrReplayProgressMark);
            int LastPos = -1;
            for (int i = 0; i < errors->Size(); i++) {
               int p = errors->At(i);
               if (p != LastPos) {
                  DrawError(width, GetPos(p, width, Total), height, Theme.Color(clrReplayProgressError));
                  LastPos = p;
               }
            }
         }
#endif
      }
   }
}

void cSkinElchiHDDisplayReplay::DrawMark(int Width, int Pos, int Height, bool Start, bool Current, tColor ColorMark, tColor ColorCurrent)
{
   int x = std::min(Pos, Width - MarksWidth);
   pmProgressBar->DrawRectangle(cRect(x, 0, MarksWidth, Height), Theme.Color(clrReplayProgressMark));
   int d = Height / (Current ? 3 : 4);
   for (int i = 0; i <= d; i++)
   {
      int y = Start ? d - i : Height - d + i - 1;
      int l = MarksWidth + 2*i;
      pmProgressBar->DrawRectangle(cRect(Pos - i, y, l, 1), Current ? Theme.Color(clrReplayProgressCurrent) : Theme.Color(clrReplayProgressMark));
   }
}

#if APIVERSNUM >= 30004
void cSkinElchiHDDisplayReplay::DrawError(int Width, int Pos, int Height, tColor ColorError)
{
   const int d = (Height / 9) & ~0x01; // must be even
   const int h = Height / 2;    // Mitte des waagrechten Strichs
   const int e = Height / 4;    // Abstand oben & unten

   int x = std::min(Pos, Width - MarksWidth);
   pmProgressBar->DrawRectangle(cRect(x, e - MarksWidth, MarksWidth, h + 2* MarksWidth), ColorError);   // senkrechter Strich
   pmProgressBar->DrawRectangle(cRect(x - d, h, MarksWidth + d + d, MarksWidth), ColorError);  // waagrechter Strich in der Mitte
   for (int i = 1; i <= d; i++) {
      pmProgressBar->DrawRectangle(cRect(x - d + i, h - i, d + d - i - i + MarksWidth, 1), ColorError); // Dreieck oben
      pmProgressBar->DrawRectangle(cRect(x - d + i, h + i + 1, d + d - i - i + MarksWidth, 1), ColorError); // Dreieck unten
   }
}
#endif

void cSkinElchiHDDisplayReplay::SetCurrent(const char *Current)
{
   if (!*oldCurrent || strcmp(*oldCurrent, Current)) {
      DSYSLOG2("skinelchiHD: cSkinElchiHDDisplayReplay::SetCurrent(%s)", Current)
      int oldLength = *oldCurrent ? strlen(*oldCurrent) : 0;
      int Length = strlen(Current);
      oldCurrent = Current;

      if (oldLength > Length)
         pmBG->DrawRectangle(cRect(xCurrent, y0, xCurrentWidth - xTotalWidth - 1, lh), Theme.Color(clrBackground));

      pmBG->DrawText(cPoint(Length > 8 ? xCurrent : xCurrent + xCurrentWidth - xTotalWidth, y0), Current, Theme.Color(clrReplayCurrent), Theme.Color(clrBackground), cFont::GetFont(fontSml), Length > 8 ? xCurrentWidth : xTotalWidth, 0, taLeft);

      changed = true;
   }
   else
      DSYSLOG2("skinelchiHD: cSkinElchiHDDisplayReplay::SetCurrent(%s) skipped", Current)
}

void cSkinElchiHDDisplayReplay::SetTotal(const char *Total)
{
   DSYSLOG2("skinelchiHD: cSkinElchiHDDisplayReplay::SetTotal(%s)", Total)

   pmBG->DrawText(cPoint(xTotal, y0), Total, Theme.Color(clrReplayTotal), Theme.Color(clrBackground), cFont::GetFont(fontSml), xTotalWidth, 0 ,taLeft);
   changed = true;
}

void cSkinElchiHDDisplayReplay::SetJump(const char *Jump)
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayReplay::SetJump(%s)", Jump)
   LOCK_PIXMAPS;
   if (Jump) {
      pmJump->SetLayer(LYR_BG);
      pmJump->DrawRectangle(cRect(0, 0, xMode, lh), clrTransparent);
      const cFont *font = cFont::GetFont(fontOsd);
      int w = font->Width(Jump);
      pmJump->DrawText(cPoint(xMode - w, 0), Jump, Theme.Color(clrReplayModeJump), Theme.Color(clrBackground), font, w);
   }
   else {
      pmJump->SetLayer(LYR_HIDDEN);
   }
   changed = true;
}

void cSkinElchiHDDisplayReplay::SetMessage(eMessageType Type, const char *Message)
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayReplay::SetMessage(%d,%s)", (int)Type, Message)
   if (Message) {
      if (showVolume) {  // destroy Volume Display first
         showVolume = false;
         pmVolume->SetLayer(LYR_HIDDEN);

         if (modeonly) {
            DELETENULL(spmTitle);
            pmTitleBG->SetLayer(LYR_HIDDEN);
         }
         else {
            SetScrollTitle(rectitle);
            oldWidth = -1;
         }
      } // end if show Volume

      showMessage = true;

      DrawShadedRectangle(pmMessageBG, Theme.Color(clrMessageStatusBg + 2 * Type));
      pmMessageBG->DrawEllipse(cRect(x0, y0, lh2, lh2), clrTransparent, -2);
      pmMessageBG->DrawEllipse(cRect(x0, lh - lh2, lh2, lh - lh2), clrTransparent, -3);
      pmMessageBG->DrawEllipse(cRect(x9 - lh2 - lh2, 0, lh2, lh2), modeonly ? clrTransparent:Theme.Color(clrBackground), -1);
      pmMessageBG->DrawEllipse(cRect(x9 - lh2 - lh2, lh - lh2, lh2, lh - lh2), modeonly ? clrTransparent:Theme.Color(clrBackground), -4);

      spmMessage->SetColor(Theme.Color(clrMessageStatusFg + 2 * Type), clrTransparent);
      spmMessage->SetText(Message, cFont::GetFont(fontOsd));

      pmProgress->SetLayer(LYR_HIDDEN);
      pmProgressBar->SetLayer(LYR_HIDDEN);
      pmMessageBG->SetLayer(LYR_TEXTBG);
      spmMessage->SetLayer(LYR_SCROLL);
      changed = true;
   }
   else {  //no text, delete message and restore previous layout
      if (showMessage) {
         pmMessageBG->SetLayer(LYR_HIDDEN);
         spmMessage->SetLayer(LYR_HIDDEN);
         pmProgress->SetLayer(LYR_BG);
         pmProgressBar->SetLayer(LYR_SELECTOR);
         showMessage = false;
         changed = true;
      }
   }
}

void cSkinElchiHDDisplayReplay::Flush(void)
{
   DSYSLOG2("skinelchiHD: cSkinElchiHDDisplayReplay::Flush()")

   // ----------------------- volume display --------------------------------
   int volume, newVolumechange;
   newVolumechange = ElchiStatus->GetVolumeChange(&volume);
   if (volumechange != newVolumechange) {
      volumechange = newVolumechange;
      int w = x9 - 3*lh2;
      int y = lh4;

      if (!showVolume) {
         showVolume = true;
         pmVolume->SetLayer(LYR_BG);
         if (!modeonly) {
            pmProgress->SetLayer(LYR_HIDDEN);
            pmProgressBar->SetLayer(LYR_HIDDEN);
         }
      }

      SetScrollTitle(cString::sprintf("%s %d%%", trVDR("Volume "), 100*volume / 255));
      int p = w * volume / 255;
      DrawShadedRectangle(pmVolume, Theme.Color(clrVolumeBarUpper), cRect(lh2, y, p, lh - 2*lh4));
      DrawShadedRectangle(pmVolume, Theme.Color(clrVolumeBarLower), cRect(lh2+p, y, w - p, lh - 2*lh4));

      changed = true;
      volumeTimer.Set(1500);
   }
   else {
      if (showVolume && volumeTimer.TimedOut()) {
         showVolume = false;
         pmVolume->SetLayer(LYR_HIDDEN);
         if (modeonly) { // Delete Rectitle
            pmTitleBG->Clear();
            DELETENULL(spmTitle);
         }
         else {
            pmProgress->SetLayer(LYR_BG);
            pmProgressBar->SetLayer(LYR_SELECTOR);
            SetScrollTitle(rectitle);
            oldWidth = -1;
         }
         changed = true;
      }
   }

   // ----------------------- video format --------------------------------
   if (ElchiConfig.showVideoInfo) {
      cVideoInfo videoinfo;

      ElchiStatus->GetVideoInfo(&videoinfo);
      if (Setup.ShowReplayMode && !showMessage && oldVideoFormat != videoinfo.videoFormat) {
         cBitmap *bmp = NULL;
         switch (videoinfo.videoFormat) {
            case videofmt_HD:     bmp = &elchiSymbols.Get(SYM_AR_HD,  Theme.Color(clrReplaySymbolOn), Theme.Color(clrBackground)); break;
            case videofmt_UHD:    bmp = &elchiSymbols.Get(SYM_AR_UHD, Theme.Color(clrReplaySymbolOn), Theme.Color(clrBackground)); break;
            case videofmt_4_3:    bmp = &elchiSymbols.Get(SYM_AR_43,  Theme.Color(clrReplaySymbolOn), Theme.Color(clrBackground)); break;
            case videofmt_16_9:   bmp = &elchiSymbols.Get(SYM_AR_169, Theme.Color(clrReplaySymbolOn), Theme.Color(clrBackground)); break;
            default:              break;
         }

         if (bmp)
            pmMode->DrawBitmap(cPoint(xSymbols[xSYM_AR], ySymbolARcutRec), *bmp);
         else
            pmMode->DrawRectangle(cRect(xSymbols[xSYM_AR], ySymbolARcutRec, elchiSymbols.Width(SYM_AR_HD), elchiSymbols.Height(SYM_AR_HD)), Theme.Color(clrBackground));

         changed = true;
         oldVideoFormat = videoinfo.videoFormat;
      }

      if (ElchiConfig.showVideoInfo == 2 && !modeonly && !showVolume) {
         if ((oldWidth != videoinfo.width) || (oldHeight != videoinfo.height)) {

            if (videoinfo.width && videoinfo.height) {
               title = cString::sprintf("%s %dx%d", (const char *)rectitle, videoinfo.width, videoinfo.height);
            }
            else {
               title = rectitle;
            }
            spmTitle->SetText(title, cFont::GetFont(fontSml));
            oldWidth = videoinfo.width;
            oldHeight = videoinfo.height;
            changed = true;
         }
      }
   }  //end videoinfo

   // ----------------------- cutting symbol --------------------------------
   bool cuttingtemp = RecordingsHandler.Active();
   if (isCutting != cuttingtemp) {
      isCutting = cuttingtemp;
      pmMode->DrawBitmap(cPoint(xSymbols[xSYM_CUTTING], ySymbolARcutRec), elchiSymbols.Get(SYM_CUTTING, Theme.Color(isCutting ? clrChannelSymbolOn : clrChannelSymbolOff), clrBG));
      changed = true;
   }

   // ----------------------- recording symbol --------------------------------
   if (Setup.ShowReplayMode && !showMessage) {
      bool rectemp = cRecordControls::Active();
      if (isRecording != rectemp) {
         isRecording = rectemp;
         pmMode->DrawBitmap(cPoint(xSymbols[xSYM_REC], ySymbolARcutRec), elchiSymbols.Get(SYM_REC, isRecording ? Theme.Color(clrSymbolRecFg) : Theme.Color(clrReplaySymbolOff), isRecording ? Theme.Color(clrSymbolRecBg) : clrBG));
         changed = true;
      }
   }

   if (changed) {
      DSYSLOG2("skinelchiHD: cSkinElchiHDDisplayReplay::Flush()ing!")
      ElchiBackground->Flush();
      changed = false;
   }
}

