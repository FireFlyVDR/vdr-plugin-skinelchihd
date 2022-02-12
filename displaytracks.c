/*
 * displaytracks.c: Audio track display handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#include <vdr/device.h>

#include "displaytracks.h"
#include "setup.h"
#include "common.h"
#include "image.h"
#include "symbols.h"


// --- cSkinElchiHDDisplayTracks ---------------------------------------------

cSkinElchiHDDisplayTracks::cSkinElchiHDDisplayTracks(const char *Title, int NumTracks, const char * const *Tracks)
{
   const cFont *font = cFont::GetFont(fontOsd);
   const cFont *smallfont = cFont::GetFont(fontSml);
   lh = font->Height();
   lh2 = lh / 2;
   currentIndex = -1;
   index = -1;
   offset = 0;
   numTracks = NumTracks;
   int ItemsWidth = max(font->Width(Title), 20*smallfont->Width());
   for (int i = 0; i < NumTracks; i++)
      ItemsWidth = max(ItemsWidth, font->Width(Tracks[i]));

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

   DSYSLOG("skinelchiHD: OSDsize Tracks %dx%d left=%d/%d top=%d/%d width=%d/%d heigth=%d/%d",
           OSDWidth, OSDHeight, OSDsize.left, Setup.OSDLeft, OSDsize.top, Setup.OSDTop, OSDsize.width, Setup.OSDWidth, OSDsize.height, Setup.OSDHeight);

   elchiSymbols.Refresh(OSDHeight);

   x0 = 0;
   x1 = x0 + lh2;
   x4 = min(OSDsize.width, ItemsWidth + 2*lh);

   x2 = x4 - lh;
   x3 = x4 - lh2;

   y0 = 0;
   y1 = y0 + lh;
   y2 = y1 + lh;

   maxTracks = numTracks;
   if ( numTracks * lh > OSDsize.height/2)
      maxTracks = OSDsize.height / lh / 2;

   y3 = y2 + maxTracks * lh;
   y4 = y3 + lh;
   y5 = y4 + lh;

   osd = cOsdProvider::NewOsd(OSDsize.left, OSDsize.top + (Setup.ChannelInfoPos ? 0 : OSDsize.height - y5));
   ElchiBackground->SetOSD(osd);
   tArea Area[] = { { x0, y0, x4 - 1, y5 - 1, 32 } };
   osd->SetAreas(Area, 1);

   tColor clrTitleBG = Theme.Color(clrMenuTitleBg);
   tColor clrTitleFG = Theme.Color(clrMenuTitleFg);

   pmBG = osd->CreatePixmap(LYR_BG, cRect(x0, y0, x4 - x0, y5 - y0));

   int w = x4 - x0;
   // header
   DrawShadedRectangle(pmBG, clrTitleBG, cRect(x0, y0, w, lh));
   pmBG->DrawEllipse(cRect(x0, y0, lh2, lh2), clrTransparent, -2);
   pmBG->DrawEllipse(cRect(w - lh2, y0, lh2, lh2), clrTransparent, -1);
   pmBG->DrawText(cPoint(x1,y0), Title, clrTitleFG, clrTransparent, font, w - lh, lh, taCenter);

   // footer
   DrawShadedRectangle(pmBG, clrTitleBG, cRect(x0, y4, w, lh));
   pmBG->DrawEllipse(cRect(x0, y5 - lh2, lh2, lh2), clrTransparent, -3);
   pmBG->DrawEllipse(cRect(w - lh2, y5 - lh2, lh2, lh2), clrTransparent, -4);

   pmSymbols = osd->CreatePixmap(LYR_TEXT, cRect(x1, y4, x3 - x1, lh));
   pmSymbols->Clear();

   pmSelectorBG = osd->CreatePixmap(LYR_SELECTOR, cRect(x0, y0, x2 - x0, lh));
   DrawShadedRectangle(pmSelectorBG, Theme.Color(clrMenuItemCurrentBg));
   pmSelectorBG->DrawEllipse(cRect(x0, y0, lh2, lh2), Theme.Color(clrBackground), -2);
   pmSelectorBG->DrawEllipse(cRect(x0, lh - lh2, lh2, lh2), Theme.Color(clrBackground), -3);
   pmSelectorBG->DrawEllipse(cRect(x2 - lh2, y0, lh2, lh2), Theme.Color(clrBackground), -1);
   pmSelectorBG->DrawEllipse(cRect(x2 - lh2, lh - lh2, lh2, lh2), Theme.Color(clrBackground), -4);

   pmSelector = osd->CreatePixmap(LYR_TEXT, cRect(x0, y0, x2 - x0, lh));

   int MaxTracks = maxTracks;
   Clear();
   if (MaxTracks > (numTracks - offset))
      MaxTracks = numTracks - offset;

   for (int i = offset; i < offset + MaxTracks; i++)
      SetItem(Tracks[i], i, false);
}

cSkinElchiHDDisplayTracks::~cSkinElchiHDDisplayTracks()
{
   ElchiBackground->SetOSD(NULL);
   DELETENULL(osd);
}

void cSkinElchiHDDisplayTracks::Clear(void)
{
   tColor clrBG = Theme.Color(clrBackground);
   tColor clrSelectable = Theme.Color(clrMenuItemSelectable);
   pmBG->DrawRectangle(cRect(x0, y1, x4, y4 - y1), Theme.Color(clrBackground));
   pmBG->DrawBitmap(cPoint(x2 + (lh - elchiSymbols.Width(SYM_ARROW_UP))/2, y1 + (lh - elchiSymbols.Height(SYM_ARROW_UP)) / 2), elchiSymbols.Get(SYM_ARROW_UP, (offset > 0) ? clrSelectable : clrBG, clrBG));
   pmBG->DrawBitmap(cPoint(x2 + (lh - elchiSymbols.Width(SYM_ARROW_DOWN))/2, y3 + (lh - elchiSymbols.Height(SYM_ARROW_DOWN)) / 2), elchiSymbols.Get(SYM_ARROW_DOWN, (numTracks > (offset + maxTracks)) ? clrSelectable : clrBG, clrBG));

   if (numTracks > maxTracks) {
      int tt = (y3 - y2) * offset / numTracks;
      int tb = (y3 - y2) * (offset + maxTracks) / numTracks;

      pmBG->DrawRectangle(cRect(x2 + 3*lh/8, y2, lh/4, y3 - y2), Theme.Color(clrMenuScrollbarTotal));
      pmBG->DrawRectangle(cRect(x2 + 3*lh/8, y2+tt, lh/4, tb-tt), Theme.Color(clrMenuScrollbarShown));
   }
}

void cSkinElchiHDDisplayTracks::SetItem(const char *Text, int Index, bool Current)
{
   int y = y2 + Index * lh - offset * lh;
   tColor clrFG, clrBG;
   if (Current) {
      clrFG = Theme.Color(clrMenuItemCurrentFg);
      clrBG = Theme.Color(clrMenuItemCurrentBg);
      currentIndex = Index;

      pmSelector->Clear();
      pmSelector->DrawText(cPoint(x1,0), Text, clrFG, clrTransparent, cFont::GetFont(fontSml), x2 - x1);
      pmSelector->SetViewPort(cRect(x0, y, x2 - x1, lh)); 
      pmSelectorBG->SetViewPort(cRect(x0, y, x4 - x0, lh)); 

      pmBG->DrawRectangle(cRect(x0,y, x2 - x0, lh), clrTransparent); // delete entry behind selector

   }
   else {
      clrFG = Theme.Color(clrMenuItemSelectable);
      clrBG = Theme.Color(clrBackground);
      pmBG->DrawText(cPoint(x1,y), Text, clrFG, clrBG, cFont::GetFont(fontSml), x2 - x1, lh); // re-draw entry behind selector
      pmBG->DrawRectangle(cRect(x0, y, lh2, lh), clrBG);
   }
}

void cSkinElchiHDDisplayTracks::SetTrack(int Index, const char * const *Tracks)
///< Sets the current track to the one given by Index, which
///< points into the Tracks array of strings.
{
   if (index != Index) {
      if (currentIndex >= 0) {
         int MaxTracks = maxTracks;
         if ((Index - offset) >= MaxTracks) {
            offset += MaxTracks - 1;
            Clear();
            if (MaxTracks > (numTracks - offset))
               MaxTracks = numTracks - offset;
            for (int i = offset; i < offset + MaxTracks; i++)
               SetItem(Tracks[i], i, false);
         }
         else {
            if (Index < offset) {
               offset -= MaxTracks - 1;
               Clear();
               if (MaxTracks > (numTracks - offset))
                  MaxTracks = numTracks - offset;
               for (int i = offset; i < offset + MaxTracks; i++)
                  SetItem(Tracks[i], i, false);
            }
            else
               SetItem(Tracks[currentIndex], currentIndex, false);
         }
      }
      else {
         int MaxTracks = maxTracks;
         if ((Index + 1) > MaxTracks) {
            int temp = Index + 1;
            while (temp > MaxTracks) {
               offset += MaxTracks - 1;
               temp   -= MaxTracks - 1;
            }
            Clear();
            if (MaxTracks > (numTracks - offset))
               MaxTracks = numTracks - offset;
            for (int i = offset; i < offset + MaxTracks; i++)
               SetItem(Tracks[i], i, false);
         }
      }
      SetItem(Tracks[Index], Index, true);
      index = Index;
   }
}

void cSkinElchiHDDisplayTracks::SetAudioChannel(int AudioChannel)
///< Sets the audio channel indicator.
///< 0=stereo, 1=left, 2=right, -1=don't display the audio channel indicator.
///< there is no DD indicator
{
   cBitmap *bmp = NULL;
   switch (AudioChannel) {
     case 0: bmp = &elchiSymbols.Get(SYM_AUDIO_STEREO, Theme.Color(clrMenuTitleFg), clrTransparent); break;
     case 1: bmp = &elchiSymbols.Get(SYM_AUDIO_LEFT,   Theme.Color(clrMenuTitleFg), clrTransparent); break;
     case 2: bmp = &elchiSymbols.Get(SYM_AUDIO_RIGHT,  Theme.Color(clrMenuTitleFg), clrTransparent); break;
     default: break;
   }
   if (bmp)
      pmSymbols->DrawBitmap(cPoint(x0, 0 + (lh - bmp->Height()) / 2), *bmp);
   else
      pmSymbols->Clear();
}

void cSkinElchiHDDisplayTracks::Flush(void)
{
   osd->Flush();
}
