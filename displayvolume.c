/*
 * displayvolume.c: Volume display handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#include <vdr/device.h>

#include "displayvolume.h"
#include "setup.h"
#include "common.h"
#include "image.h"
#include "symbols.h"

// --- cSkinElchiHDDisplayVolume ---------------------------------------------

cSkinElchiHDDisplayVolume::cSkinElchiHDDisplayVolume(void)
{
   changed = false;
   const cFont *font = cFont::GetFont(fontOsd);
   lh = font->Height();
   lh2 = lh / 2;

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

   DSYSLOG("skinelchiHD: OSDsize Volume %dx%d left=%d/%d top=%d/%d width=%d/%d heigth=%d/%d",
           OSDWidth, OSDHeight, OSDsize.left, Setup.OSDLeft, OSDsize.top, Setup.OSDTop, OSDsize.width, Setup.OSDWidth, OSDsize.height, Setup.OSDHeight);

   elchiSymbols.Refresh(OSDHeight);

   osd = cOsdProvider::NewOsd(OSDsize.left, OSDsize.top + OSDsize.height - 2 * lh);
   ElchiBackground->SetOSD(osd);
   tArea Area[] = { { 0, 0, OSDsize.width - 1, 2 * lh - 1, 32 } };
   osd->SetAreas(Area, 1);

   w = osd->Width();
   h = 2 * lh;
   pmBG = osd->CreatePixmap(LYR_BG, cRect(0, 0, w, h));

   isMuted = -1;
   currentVolume = -1;
}

cSkinElchiHDDisplayVolume::~cSkinElchiHDDisplayVolume()
{
   changed = true;
   ElchiBackground->SetOSD(NULL);
   DELETENULL(osd);
}

void cSkinElchiHDDisplayVolume::SetVolume(int CurrentVolume, int TotalVolume, bool Mute)
{
   const cFont *font = cFont::GetFont(fontOsd);
   int xVolume = font->Width(trVDR("Volume ")) + lh2;

   if (Mute) {
      if (isMuted != 1){
         currentVolume = 0;
         changed = true;
         cString Prompt = cString::sprintf("%3d", CurrentVolume * 100 / TotalVolume);
         int l = font->Width(Prompt) + lh2;

         pmBG->Clear();
         pmBG->DrawText(cPoint(lh2 + 3, lh + 1), Prompt, Theme.Color(clrBackground), clrTransparent, font);
         pmBG->DrawText(cPoint(lh2, lh - 2), Prompt, Theme.Color(clrVolumePrompt), clrTransparent, font);
         pmBG->DrawBitmap(cPoint(l + 3, lh + (lh - elchiSymbols.Height(SYM_MUTE)) / 2), elchiSymbols.Get(SYM_MUTE, 0, 0));
         isMuted = 1;
      }
   }
   else {
      if (isMuted != 0) {
         changed = true;
         tColor clrBG = Theme.Color(clrBackground);

         pmBG->Clear();
         pmBG->DrawRectangle(cRect(lh2, lh, w - lh2 - lh2, lh), clrBG);
         pmBG->DrawEllipse(cRect(0, lh, lh2, lh), clrBG, 7);
         pmBG->DrawEllipse(cRect(w - lh2, lh, lh2, lh), clrBG, 5);

         const cString Prompt = trVDR("Volume "); 
         pmBG->DrawText(cPoint(lh2 + 3, 1), Prompt, Theme.Color(clrBackground), clrTransparent, font);
         pmBG->DrawText(cPoint(lh2, -2), Prompt, Theme.Color(clrVolumePrompt), clrTransparent, font);
         pmBG->DrawBitmap(cPoint(xVolume + 3, (lh - elchiSymbols.Height(SYM_VOLUME)) / 2), elchiSymbols.Get(SYM_VOLUME, Theme.Color(clrVolumeSymbolVolumeFg)));

         isMuted = 0;
      }
      if (currentVolume != CurrentVolume){
         const cString Level = cString::sprintf("%3d", CurrentVolume * 100 / TotalVolume);
         pmBG->DrawRectangle(cRect(xVolume + elchiSymbols.Width(SYM_VOLUME) + lh2, 0, 3 + font->Width("888"), lh), clrTransparent);
         pmBG->DrawText(cPoint(xVolume + elchiSymbols.Width(SYM_VOLUME) + lh2 + 3, 1), Level, Theme.Color(clrBackground), clrTransparent, font);
         pmBG->DrawText(cPoint(xVolume + elchiSymbols.Width(SYM_VOLUME) + lh2, -2), Level, Theme.Color(clrVolumePrompt), clrTransparent, font);

         DrawShadedRectangle(pmBG, Theme.Color(clrVolumeBarUpper), cRect(lh2, lh + lh/4 , (w - lh) * CurrentVolume / TotalVolume, lh/2));
         DrawShadedRectangle(pmBG, Theme.Color(clrVolumeBarLower), cRect(lh2 + (w - lh) * CurrentVolume / TotalVolume, lh + lh/4 , w - lh - (w - lh) * CurrentVolume / TotalVolume, lh/2));

         currentVolume = CurrentVolume;
         changed = true;
      }
   }
}

void cSkinElchiHDDisplayVolume::Flush(void)
{
   if (changed) {
      DSYSLOG("skinelchiHD: cSkinElchiHDDisplayVolume::Flush()")
      ElchiBackground->Flush();
      changed = false;
   }
}
