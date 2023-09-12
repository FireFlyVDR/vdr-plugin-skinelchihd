/*
 * displaymessage.c: Message display handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#include <vdr/device.h>

#include "displaymessage.h"
#include "setup.h"
#include "common.h"
#include "image.h"

// --- cSkinElchiHDDisplayMessage --------------------------------------------
cSkinElchiHDDisplayMessage::cSkinElchiHDDisplayMessage(void)
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayMessage::cSkinElchiHDDisplayMessage()")

   const cFont *font = cFont::GetFont(fontOsd);
   lh = font->Height();
   lh2 = lh / 2;

   osd = cOsdProvider::NewOsd(cOsd::OsdLeft(), cOsd::OsdTop() + cOsd::OsdHeight() - lh);
   ElchiBackground->SetOSD(osd);
   tArea Area[] = { { 0, 0, cOsd::OsdWidth() - 1, lh - 1, 32 } };
   osd->SetAreas(Area, 1);

   pmMsgBar = osd->CreatePixmap(LYR_BG, cRect(0, 0, cOsd::OsdWidth(), lh));
   spmMsgBarText = new cScrollingPixmap(osd, cRect(lh2, 0, cOsd::OsdWidth() - lh, lh),
                                        cFont::GetFont(fontOsd), 256, clrBlack, clrTransparent, true);
   changed = true;
}

cSkinElchiHDDisplayMessage::~cSkinElchiHDDisplayMessage()
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayMessage::~cSkinElchiHDDisplayMessage()")
   changed = true;
   DELETENULL(spmMsgBarText);

   ElchiBackground->SetOSD(NULL);
   DELETENULL(osd);
}

void cSkinElchiHDDisplayMessage::SetMessage(eMessageType Type, const char *Text)
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayMessage::SetMessage(%d,%s)", (int)Type, Text)
   changed = true;
   tColor MsgBarColor = Theme.Color(clrMessageStatusBg + 2 * Type);
   int width = pmMsgBar->DrawPort().Width();
   const cFont *font = cFont::GetFont(fontOsd);

   DrawShadedRectangle(pmMsgBar, MsgBarColor);
   pmMsgBar->DrawEllipse(cRect(0, 0, lh2, lh2), clrTransparent, -2);
   pmMsgBar->DrawEllipse(cRect(0, lh - lh2, lh2, lh - lh2), clrTransparent, -3);
   pmMsgBar->DrawEllipse(cRect(width - lh2, 0, lh2, lh2), clrTransparent, -1);
   pmMsgBar->DrawEllipse(cRect(width - lh2, lh - lh2, lh2, lh - lh2), clrTransparent, -4);

   spmMsgBarText->SetColor(Theme.Color(clrMessageStatusFg + 2 * Type), clrTransparent);
   spmMsgBarText->SetText(Text, font);
}

void cSkinElchiHDDisplayMessage::Flush(void)
{
   if (changed) {
      ElchiBackground->Flush();
      changed = false;
   }
}
