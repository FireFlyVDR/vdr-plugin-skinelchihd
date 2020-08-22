/*
 * displaychannel.c: Channel display handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#include <time.h>

#include <vdr/device.h>
#include <vdr/menu.h>
#include <vdr/osd.h>

//#define DEBUG
#include "common.h"
#include "displaychannel.h"
#include "vdrstatus.h"

#include "image.h"
#include "symbols.h"

extern cSkinElchiStatus *ElchiStatus;

#define STR_VIDEOSIZE " 0000x0000"

// --- cSkinElchiHDDisplayChannel --------------------------------------------

cSkinElchiHDDisplayChannel::cSkinElchiHDDisplayChannel(bool WithInfo)
{
   hasVideo = true;
   old_ar = ar_unknown;
   old_width = -1;
   old_height = -1;
   LastSignalStrength = -1;
   LastSignalQuality = -1; 
   showMessage = false;
   changed = false;
   showVolume = false;
   volumechange = ElchiStatus->GetVolumeChange(NULL);
   recordingchange = -1;
   lastdate = NULL;
   Channelnumber[0] = 0;
   recording = false;
   cutting = false;
   withInfo = WithInfo;
   const cFont *smallfont = cFont::GetFont(fontSml);
   const cFont *font = cFont::GetFont(fontOsd);
   lh = std::max(font->Height(), elchiSymbols.Height(SYM_REC)); //not smaller than REC symbol
   slh = smallfont->Height();
   lh2 = lh / 2;
   lineOffset = (lh - slh) / 2;
   bg = Theme.Color(clrBackground);
   tOSDsize OSDsize;

   OSDsize.left   = cOsd::OsdLeft();
   OSDsize.top    = cOsd::OsdTop();
   OSDsize.width  = cOsd::OsdWidth();
   OSDsize.height = cOsd::OsdHeight();
   
   OSDHeight = 720;
   int OSDWidth = 1280;
   double OSDAspect = 16.0/9.0;

   cDevice::PrimaryDevice()->GetOsdSize(OSDWidth, OSDHeight, OSDAspect);

   if (OSDHeight >= 2160) {
      wLogo          = 320;
      hLogo          = 240;
      bLogo          = 12;
      Gap            = 18;
      SymbolGap      = 12;
   } else if (OSDHeight >= 1080) {
      wLogo          = 160;
      hLogo          = 120;
      bLogo          = 6;
      Gap            = 9;
      SymbolGap      = 6;
   } else if (OSDHeight >= 720) {
      wLogo          = 104;
      hLogo          = 78;
      bLogo          = 4;
      Gap            = 6;
      SymbolGap      = 4;
   } else {  // <  720 incl. 576
      wLogo          = 64;
      hLogo          = 48;
      bLogo          = 3;
      Gap            = 5;
      SymbolGap      = 3;
   }

   elchiSymbols.Refresh(OSDHeight);
      
   if (!ElchiConfig.showLogo)
      wLogo = hLogo = 0;

   xLogo = 0;
   xLeft = ElchiConfig.showLogo ? (wLogo/4) : 0;
   xRight = OSDsize.width;

   xEvTime = xLeft;
   wEvTime = font->Width("00:00") + 2*Gap;

   xTimeBar = xEvTime + wEvTime;
   wTimeBar = lh/4;
             
   xEvText = xTimeBar + Gap + wTimeBar + Gap;  // Text window (not text itself?)
   wEvText = xRight - xEvText - lh2;
   xChName = ElchiConfig.showLogo ? (wLogo + Gap/2) : xLeft;
   wChNumber = font->Width("00000-");
   
   xDateTime = xChName + wChNumber;
   wDateTime = smallfont->Width("_MMM 00.00 00:00_");

   wChDateTime = wChNumber + wDateTime + lh2;
   
   xSymbols[xSYM_AR]           = SymbolGap;
   xSymbols[xSYM_VPS]          = (ElchiConfig.showVideoInfo ? xSymbols[xSYM_AR] + elchiSymbols.Width(SYM_AR_HD) + SymbolGap : SymbolGap);
   xSymbols[xSYM_Teletext]     = xSymbols[xSYM_VPS] + elchiSymbols.Width(SYM_VPS) + SymbolGap;
   xSymbols[xSYM_Audio]        = xSymbols[xSYM_Teletext] + elchiSymbols.Width(SYM_TELETEXT) + SymbolGap;
   xSymbols[xSYM_DolbyDigital] = xSymbols[xSYM_Audio] + elchiSymbols.Width(SYM_AUDIO) + SymbolGap;
   xSymbols[xSYM_encrypted]    = xSymbols[xSYM_DolbyDigital] + elchiSymbols.Width(SYM_DOLBYDIGITAL) + SymbolGap;
   xSymbols[xSYM_cutting]      = xSymbols[xSYM_encrypted] + elchiSymbols.Width(SYM_ENCRYPTED) + SymbolGap;
   xSymbols[xSYM_REC]          = xSymbols[xSYM_cutting] + elchiSymbols.Width(SYM_CUTTING) + SymbolGap;
   
   xSymbolStart = xRight - lh2 - lh - xSymbols[xSYM_REC];
   wChName = xSymbolStart - xChName -lh;
   
   yLogo = 0;                      // Logo Top
   yChDateTime = (ElchiConfig.showLogo &&  hLogo >= lh) ?  (hLogo - lh)/2 : 0;  // Date Time Top
   yChName = yChDateTime + lh + SymbolGap;    //Channel Name Top
   ySymbols = (lh - elchiSymbols.Height(SYM_VPS)) / 2;  // Symbols Top (centered in Channel Name Bar)
   ySymbolARRec = (lh - elchiSymbols.Height(SYM_REC)) / 2;  // Symbols Top (centered in Channel Name Bar)
   yRecordings = yChName + lh;              // RecLine Top
   int numRecordings;
   ElchiStatus->GetRecordingChange(&numRecordings);
   yEvText = yRecordings + (withInfo && ((ElchiConfig.showRecInfo == 2) ||        // Event Box Top
                            (ElchiConfig.showRecInfo == 1 && numRecordings)) ? lh : 0);
   hEvents = withInfo ? lh * 4 : 0;
   yBottom = yEvText + hEvents;

   osd = cOsdProvider::NewOsd(OSDsize.left, OSDsize.top + (Setup.ChannelInfoPos ? 0 : OSDsize.height - yBottom));
   ElchiBackground->SetOSD(osd);

   tArea Area = { 0, 0, xRight - 1, withInfo ? yBottom - 1 : yRecordings - 1, 32 };
   if (oeOk != osd->SetAreas(&Area, 1)) {
      esyslog("skinelchiHD DisplayChannel: SetAreas failed");
      return;
   }

   pmBG = osd->CreatePixmap(LYR_BG, cRect(0, 0, xRight, withInfo ? yBottom : yRecordings));
   pmChannelNameBg = osd->CreatePixmap(LYR_TEXTBG, cRect(xLeft, yChName, xSymbolStart - xLeft, lh));
   
   DrawBackground();

   spmPresentTitle = new cScrollingPixmap(osd, cRect(xEvText, yEvText, wEvText, lh),
                                          font, MAXCHARS, Theme.Color(clrChannelEpgTitleFg));
   presentLastOffset = 0;
   spmPresentShort = new cScrollingPixmap(osd, cRect(xEvText, yEvText + lh, wEvText, slh),
                                          smallfont, MAXCHARS, Theme.Color(clrChannelEpgShortText));

   spmFollowingTitle = new cScrollingPixmap(osd, cRect(xEvText, yEvText + 2*lh, wEvText, lh),
                                            font, MAXCHARS, Theme.Color(clrChannelEpgTitleFg));
   followingLastOffset = 0;
   spmFollowingShort = new cScrollingPixmap(osd, cRect(xEvText, yEvText + 3*lh, wEvText, slh),
                                            smallfont, MAXCHARS, Theme.Color(clrChannelEpgShortText));
   PresentEvent = NULL;
   FollowingEvent = NULL;


   spmAudio = NULL;
   if (ElchiConfig.showAudioInfo)
   {
      spmAudio = new cScrollingPixmap(osd, cRect(xChName + wChDateTime + lh + lh2, yChDateTime + lineOffset, 
                                                 xRight - lh2 - elchiSymbols.Width(SYM_SIGNAL) -lh2 - (xChName + wChDateTime + lh + lh2) , lh),
                                                 smallfont, MAXCHARS, Theme.Color(clrChannelSymbolOn), clrTransparent, false, taRight);
   }
   
   spmRecording = NULL;
   if (yRecordings != yEvText)
      spmRecording = new cScrollingPixmap(osd, cRect(xLeft + lh2, yRecordings, xRight - xLeft -lh , lh),
                                         smallfont, MAXCHARS, Theme.Color(clrChannelEpgShortText));

   pmChDateTime = CreateTextPixmap(osd, LYR_TEXT, cRect(xChName+lh2, yChDateTime + lineOffset, wChDateTime, lh),
                                             smallfont, "", Theme.Color(clrChannelDateFg), clrTransparent);
   pmChDateTime->Clear();

   if (ElchiConfig.showVideoInfo  == 0) {
      pmVideoSize = NULL;
      wVsize = 0;
   }
   else {
      wVsize = smallfont->Width(STR_VIDEOSIZE);
      wChName -= wVsize;
      int xVsize = xSymbolStart - lh2 - wVsize;
      pmVideoSize = CreateTextPixmap(osd, LYR_TEXT, cRect(xVsize, yChName + lineOffset, wVsize, slh),
                                             smallfont, "", Theme.Color(clrChannelNameFg), clrTransparent, taRight);
   }

   // symbol area
   pmSymbols = osd->CreatePixmap(LYR_BG, cRect(xSymbolStart, yChName, xRight - xSymbolStart, lh));
   pmSymbols->Fill(Theme.Color(clrBackground));
   if (!withInfo) pmSymbols->DrawEllipse(cRect(xRight - xSymbolStart - lh2, lh - lh2, lh2, lh2), clrTransparent, -4);

   spmChannelName = new cScrollingPixmap(osd, cRect(xChName + lh2, yChName + lineOffset, wChName -lh2, slh),
                                            cFont::GetFont(fontSml), MAXCHARS, Theme.Color(clrChannelNameFg));

   pmMessageBG = osd->CreatePixmap(LYR_HIDDEN, cRect(xLeft, yChName, xRight - xLeft, lh));
   
   spmMessage = new cScrollingPixmap(osd, cRect(xChName + lh2, yChName, xRight -xLeft - lh, lh),
                                         cFont::GetFont(fontOsd), MAXCHARS, Theme.Color(clrChannelNameFg), clrTransparent, true);
   spmMessage->SetLayer(LYR_HIDDEN);
}


cSkinElchiHDDisplayChannel::~cSkinElchiHDDisplayChannel()
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayChannel::~cSkinElchiHDDisplayChannel()")
   showMessage = changed = false;

   DELETENULL(spmPresentTitle);
   DELETENULL(spmPresentShort);
   DELETENULL(spmFollowingTitle);
   DELETENULL(spmFollowingShort);
   DELETENULL(spmChannelName);
   DELETENULL(spmMessage);
   DELETENULL(spmRecording);

   ElchiBackground->SetOSD(NULL);

   DELETENULL(osd);
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayChannel::~cSkinElchiHDDisplayChannel() end")
}

void cSkinElchiHDDisplayChannel::DrawBackground()
{
   LOCK_PIXMAPS;
   //clear background
   pmBG->Clear();

   /// Header BG
   pmBG->DrawRectangle(cRect(xLeft, yChDateTime, xRight - xLeft, yEvText - yChDateTime - lh), Theme.Color(clrBackground));
   pmBG->DrawEllipse  (cRect(xRight - lh2, yChDateTime, lh2, lh2), clrTransparent, -1);

   /// Date Time BG
   DrawShadedRectangle(pmBG, Theme.Color(clrChannelDateBg), cRect(xLeft, yChDateTime, wChDateTime + wLogo + lh2, lh));
   pmBG->DrawEllipse(cRect(xLeft+wChDateTime + wLogo, yChDateTime, lh2, lh2), Theme.Color(clrBackground), -1);
   pmBG->DrawEllipse(cRect(xLeft+wChDateTime + wLogo, yChDateTime + lh -lh2, lh2, lh2), Theme.Color(clrBackground), -4);
   
   /// Channel Name BG
   DrawShadedRectangle(pmChannelNameBg, Theme.Color(clrChannelNameBg));
   pmChannelNameBg->DrawEllipse(cRect(xSymbolStart - xLeft - lh2, 0, lh2, lh2), Theme.Color(clrBackground), -1);
   pmChannelNameBg->DrawEllipse(cRect(xSymbolStart - xLeft - lh2, lh - lh2, lh2, lh2), Theme.Color(clrBackground), -4);

   if(withInfo) {
      // draw backgrounds with info

      // recording info BG
      if (yRecordings != yEvText)
         pmBG->DrawRectangle(cRect(xLeft, yRecordings, xRight - xLeft,  lh), Theme.Color(clrChannelEpgTitleBg));      
      
      // EPG Time BG
      pmBG->DrawRectangle(cRect(xEvTime, yEvText, wEvTime, hEvents), Theme.Color(clrChannelEpgTimeBg));
      pmBG->DrawEllipse  (cRect(xEvTime, yBottom - lh2, lh2, lh2), clrTransparent, -3);
      //isyslog("skinelchiHD Channel 5");

      // EPG Event BG
      pmBG->DrawRectangle(cRect(xTimeBar, yEvText, xRight - xTimeBar,  hEvents), Theme.Color(clrChannelEpgTitleBg));
      pmBG->DrawEllipse  (cRect(xRight - lh2, yBottom - lh2, lh2, lh2), clrTransparent, -4);
      
      if (ElchiConfig.showLogo) {
         // empty Logo area
         pmBG->DrawRectangle(cRect(xLogo, yLogo, wLogo + Gap/2, hLogo + Gap/2), clrTransparent);
         pmChannelNameBg->DrawRectangle(cRect(xLogo, yLogo, wLogo - xLeft + Gap/2, hLogo + Gap/2 - yChName), clrTransparent);
      }
      else {
         // with Info, without Logo
         pmBG->DrawEllipse  (cRect(xLeft, yChDateTime, lh2, lh2), clrTransparent, -2);
      }
   }
   else {  //withoutinfo
      pmBG->DrawEllipse  (cRect(xRight - lh2, yRecordings - lh2, lh2, lh2), clrTransparent, -4);

      // draw backgrounds without info
      if (ElchiConfig.showLogo) {
         // without Info, with Logo
         // empty Logo area
         pmBG->DrawRectangle(cRect(xLogo, yLogo, wLogo + Gap/2, hLogo + Gap/2), clrTransparent);
         pmChannelNameBg->DrawRectangle(cRect(xLogo, yLogo, wLogo - xLeft + Gap/2, hLogo + Gap/2 - yChName), clrTransparent);
      }
      else {
         // without Info, without Logo
         pmBG->DrawEllipse  (cRect(xLeft, yChDateTime, lh2, lh2), clrTransparent, -2);
         pmChannelNameBg->DrawEllipse(cRect(0, lh - lh2, lh2, lh2), clrTransparent, -3);
      }
   }

   recordingchange = -1; // force redraw of recording pixmap
   lastdate = NULL; // force date redraw
}

void toLowerCase(char *str) {
    const int length = strlen(str);
    for(int i=0; i < length; ++i) {
        str[i] = std::tolower(str[i]);
    }
    return;
}

cString cSkinElchiHDDisplayChannel::CheckLogoFile(const cChannel *Channel, const char *path)
{
   if (!Channel || !Channel->Name())
      return NULL;

   cString filename;
   char *lowerFilename = strdup(Channel->Name());
   toLowerCase(lowerFilename);
   cString ChannelName  = cString(strreplace(lowerFilename, '/', '~'), true);
   if (const char *s = strrchr(ChannelName, '('))
      ChannelName = cString(ChannelName, --s);
   cString ChannelName_ = cString(strreplace(strdup(*ChannelName), ' ', '_'), true);

   filename = cString::sprintf("%s/%s.svg", path, *ChannelName);
   if (access(filename, F_OK) == 0) // the file exists
      return filename;
   else {
      filename = cString::sprintf("%s/%s.svg", path, *ChannelName_);
      if (access(filename, F_OK) == 0) // the file exists
         return filename;
      else
      {
         filename = cString::sprintf("%s/%s.svg", path, *Channel->GetChannelID().ToString());
         if (access(filename, F_OK) == 0) // the file exists
            return filename;
      }
   }

   return NULL;
}


void cSkinElchiHDDisplayChannel::SetChannel(const cChannel *Channel, int ChannelNumber)
///< Sets the current channel to Channel. If Number is not 0, the
///< user is in the process of entering a channel number, which must
///< be displayed accordingly.
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayChannel::SetChannel(%d,%d)", Channel ? Channel->Number():0, ChannelNumber)

   changed = true;
   LastSignalStrength = -1;  // force display refresh of signal indicators
   LastSignalQuality = -1;
   
   if (!Channel) {
      // if no channel object is available (e.g., while entering new channel number), update at least Channelnumber
      if (ChannelNumber)
         snprintf(Channelnumber, sizeof(Channelnumber), "%d-", ChannelNumber);
      else
         Channelnumber[0] = 0;

      LOCK_PIXMAPS;
      pmChDateTime->DrawRectangle(cRect(0, 0, wChNumber, lh), clrTransparent);
      pmChDateTime->DrawText(cPoint(0, 0), Channelnumber, Theme.Color(clrChannelDateFg), clrTransparent, cFont::GetFont(fontOsd));
   }
   else
   {
      if (Channel->GroupSep()) {
         snprintf(Channelnumber, sizeof(Channelnumber), " ");
         if (AudioPixmap) {
            osd->DestroyPixmap(AudioPixmap);
            AudioPixmap = NULL;
         }
         hasVideo = false;
      }
      else  // Channel is not a group separator
      {
         audiostring = NULL;

         if (AudioPixmap) {
            LOCK_PIXMAPS;
            osd->DestroyPixmap(AudioPixmap);
            AudioPixmap = NULL;
         }

         // clear and redraw complete display
         DrawBackground();

         old_ar = ar_unknown;
         old_width = -1;
         old_height = -1;

         //  draw symbols: rec, encrypted, DD, Audio, Teletext
         LOCK_PIXMAPS;
         pmSymbols->DrawBitmap(cPoint(xSymbols[xSYM_REC], ySymbolARRec), elchiSymbols.Get(SYM_REC, Theme.Color(recording ? clrSymbolRecFg : clrChannelSymbolOff), recording ? Theme.Color(clrSymbolRecBg) : bg));
         pmSymbols->DrawBitmap(cPoint(xSymbols[xSYM_cutting], ySymbolARRec), elchiSymbols.Get(SYM_CUTTING, Theme.Color(cutting ? clrChannelSymbolOn : clrChannelSymbolOff), bg));
         pmSymbols->DrawBitmap(cPoint(xSymbols[xSYM_encrypted], ySymbols), elchiSymbols.Get(SYM_ENCRYPTED, Theme.Color(Channel->Ca() ? clrChannelSymbolOn : clrChannelSymbolOff), bg));
         pmSymbols->DrawBitmap(cPoint(xSymbols[xSYM_DolbyDigital], ySymbols), elchiSymbols.Get(SYM_DOLBYDIGITAL, Theme.Color(Channel->Dpid(0) ? clrChannelSymbolOn : clrChannelSymbolOff), bg));
         pmSymbols->DrawBitmap(cPoint(xSymbols[xSYM_Audio], ySymbols), elchiSymbols.Get(SYM_AUDIO, Theme.Color(Channel->Apid(1) ? clrChannelSymbolOn : clrChannelSymbolOff), bg));

         if (Channel->Vpid()) { // Channel has Video
            pmSymbols->DrawBitmap(cPoint(xSymbols[xSYM_Teletext], ySymbols), elchiSymbols.Get(SYM_TELETEXT, Theme.Color(Channel->Tpid() ? clrChannelSymbolOn : clrChannelSymbolOff), bg));
            hasVideo = true;
         }
         else // Channel has no Video
         {
            hasVideo = false;
            if (Channel->Apid(0)) {
               if (Channel->Tpid()) {
                  pmSymbols->DrawBitmap(cPoint(xSymbols[xSYM_Teletext], ySymbols), elchiSymbols.Get(SYM_TELETEXT, Theme.Color(clrChannelSymbolOn), bg));
               }
               if (ElchiConfig.showVideoInfo) {
                  pmSymbols->DrawBitmap(cPoint(xSymbols[xSYM_AR], ySymbolARRec), elchiSymbols.Get(SYM_RADIO, Theme.Color(clrChannelSymbolOn), bg));
               
               }
            }
         }
         snprintf(Channelnumber, sizeof(Channelnumber), "%d%s", ChannelNumber ? ChannelNumber : Channel->Number(), ChannelNumber ? "-" : "");
      } // end not GroupSep()

      spmChannelName->SetText(Channel->Name(), cFont::GetFont(fontSml));
       
      pmChDateTime->DrawRectangle(cRect(0, 0, wChNumber, lh), clrTransparent);
      pmChDateTime->DrawText(cPoint(0, 0), Channelnumber, Theme.Color(clrChannelDateFg), clrTransparent, cFont::GetFont(fontOsd));

      // load Logo
      if (ElchiConfig.showLogo)
      {
         if (!Channel || !Channel->Name()) {
            // no channel or channel without name: draw empty rectangle
            pmBG->DrawRectangle(cRect(xLogo, yLogo, wLogo, hLogo), Theme.Color(clrChannelDateBg));
         }
         else
         {
            cString filename;
            filename = CheckLogoFile(Channel, ElchiConfig.GetLogoBaseDir());
            
            if (isempty(filename)) {
               if (ElchiConfig.LogoMessages) {
                  esyslog("skinElchiHD: no logo found for \"%s\" (%s) in %s", *cString(strreplace(strdup(Channel->Name()), '/', '~'), true), *Channel->GetChannelID().ToString(), ElchiConfig.GetLogoBaseDir());
               }
               pmBG->DrawRectangle(cRect(xLogo, yLogo, wLogo, hLogo), Theme.Color(clrChannelDateBg));
            }
            else {
               cOSDImage *imgLogo = new cOSDImage(filename, wLogo, hLogo, Theme.Color(clrChannelDateBg), bLogo);
               if (imgLogo && imgLogo->GetImage()) {
                  pmBG->DrawImage(cPoint(0, 0), *imgLogo->GetImage());
                  if (ElchiConfig.LogoMessages)
                     isyslog("skinElchiHD: logo loaded %s", *filename);
               }
               else {
                  pmBG->DrawRectangle(cRect(xLogo, yLogo, wLogo, hLogo), Theme.Color(clrChannelDateBg));
               }
            }
         }
      }
   }
}


cPixmap * cSkinElchiHDDisplayChannel::CreateTextPixmap(cOsd *osd, int layer, const cRect vPort, const cFont *font, const char *text, tColor ColorFg, tColor ColorBg, int Alignment)
{
   int width = font->Width(text);
   LOCK_PIXMAPS;
   cPixmap *pixmap = osd->CreatePixmap(layer, vPort, vPort.Width() > width ? vPort : cRect(0, 0, width, lh));
   pixmap->Clear();
   pixmap->DrawText(cPoint(0, 0), text, ColorFg, ColorBg, font, width, font->Height(), Alignment);
   pixmap->SetDrawPortPoint(cPoint(0, 0));

   return pixmap;
}


void cSkinElchiHDDisplayChannel::SetEvents(const cEvent *Present, const cEvent *Following)
///< Sets the Present and Following EPG events. If either of these
///< is not available, NULL will be given.
{
   DSYSLOG2("skinelchiHD: SetEvents(%d,%d)", Present ? Present->EventID():0, Following ? Following->EventID():0)
   if (withInfo) {
      const cFont *smallfont = cFont::GetFont(fontSml);
      const cFont *font = cFont::GetFont(fontOsd);
      LOCK_PIXMAPS;
      
      if (!Present || !Present->StartTime()) {
         pmBG->DrawRectangle(cRect(xTimeBar + Gap, yEvText, wTimeBar, hEvents), Theme.Color(clrChannelEpgTitleBg));
         changed = true;
      }
      else {  //TimeBar
         time_t t = time(NULL);
         int seen = std::max(0, std::min(hEvents, int((hEvents) * double(t - Present->StartTime()) / Present->Duration())));
         pmBG->DrawRectangle(cRect(xTimeBar + Gap, yEvText + seen, wTimeBar, hEvents - seen), Theme.Color(clrChannelTimebarRest));
         pmBG->DrawRectangle(cRect(xTimeBar + Gap, yEvText, wTimeBar, seen), Theme.Color(clrChannelTimebarSeen));
         changed = true;
      }
      PresentEvent = Present;
      FollowingEvent = Following;

      pmBG->DrawText(cPoint(xEvTime, yEvText), Present ? Present->GetTimeString() : (cString)NULL, Theme.Color(clrChannelEpgTimeFg), Theme.Color(clrChannelEpgTimeBg), cFont::GetFont(fontOsd), wEvTime, 0, taCenter);
      pmBG->DrawText(cPoint(xEvTime, yEvText + 2 * lh), Following ? Following->GetTimeString() : (cString)NULL, Theme.Color(clrChannelEpgTimeFg), Theme.Color(clrChannelEpgTimeBg), cFont::GetFont(fontOsd), wEvTime, 0, taCenter);
      
      spmPresentTitle->SetText(Present ? Present->Title() : NULL, font);
      spmPresentShort->SetText(Present ? Present->ShortText() : NULL, smallfont);

      spmFollowingTitle->SetText(Following ? Following->Title() : NULL, font);
      spmFollowingShort->SetText(Following ? Following->ShortText() : NULL, smallfont);
   }

   if (Present && Present->Vps()) {
      pmSymbols->DrawBitmap(cPoint(xSymbols[xSYM_VPS], ySymbols), elchiSymbols.Get(SYM_VPS, Theme.Color(Present->Vps() - Present->StartTime() ? clrChannelSymbolOn : clrChannelSymbolOff), bg));
      changed = true;
   }
   DSYSLOG2("skinelchiHD: SetEvents end")
}

void cSkinElchiHDDisplayChannel::SetMessage(eMessageType Type, const char *Text)
///< Sets a one line message Text, with the given Type. Type can be used
///< to determine, e.g., the colors for displaying the Text.
///< If Text is NULL, any previously displayed message must be removed, and
///< any previous contents overwritten by the message must be restored.
{
   DSYSLOG("skinelchiHD: cSkinElchiHDDisplayChannel::SetMessage(%d,%s)", (int)Type, Text)

   if (Text) { // draw msg
      if (showVolume) {
         showVolume = false;
      }
      
      tColor clrMsgFG = Theme.Color(clrMessageStatusFg + 2 * Type);
      tColor clrMsgBG = Theme.Color(clrMessageStatusBg + 2 * Type);
      
      pmChannelNameBg->SetLayer(LYR_HIDDEN);
      spmChannelName->SetLayer(LYR_HIDDEN);
      pmSymbols->SetLayer(LYR_HIDDEN);
      if (pmVideoSize)
         pmVideoSize->SetLayer(LYR_HIDDEN);
      
      DrawShadedRectangle(pmMessageBG, clrMsgBG);
      pmMessageBG->SetLayer(LYR_TEXTBG);
      spmMessage->SetLayer(LYR_SCROLL);
      spmMessage->SetColor(clrMsgFG, clrTransparent);
      showMessage = true;
      
      if (withInfo) {
         if (ElchiConfig.showLogo) {
            // clear Logo Area
            pmMessageBG->DrawRectangle(cRect(xLogo, yLogo, wLogo - xLeft + Gap/2, hLogo + Gap/2 - yChName), clrTransparent);
         }
      }
      else { //withoutinfo
         pmMessageBG->DrawEllipse(cRect(xRight - xLeft - lh2, lh - lh2, lh2, lh2), clrTransparent, -4);
         if (ElchiConfig.showLogo) {
            pmMessageBG->DrawRectangle(cRect(xLogo, yLogo, wLogo - xLeft + Gap/2, hLogo + Gap/2 - yChName), clrTransparent);
         }
         else {
            pmMessageBG->DrawEllipse(cRect(0, lh - lh2, lh2, lh2), clrTransparent, -3);
         }
      }
      
      spmMessage->SetText(Text, cFont::GetFont(fontOsd));
      changed = true;
   }
   else { // remove msg and restore previous content
      if (showMessage) {
         pmChannelNameBg->SetLayer(LYR_TEXTBG);
         spmChannelName->SetLayer(LYR_SCROLL);
         pmSymbols->SetLayer(LYR_BG);
         if (pmVideoSize)
            pmVideoSize->SetLayer(LYR_TEXT);
         
         pmMessageBG->SetLayer(LYR_HIDDEN);
         spmMessage->SetLayer(LYR_HIDDEN);

         changed = true;
         showMessage = false;
      }
   }
}


void cSkinElchiHDDisplayChannel::Flush(void)
///< Actually draws the OSD display to the output device.
{
   const cFont *smallfont = cFont::GetFont(fontSml);

   // ----------------------- REC symbols --------------------------------
   int presentOffset = 0;
   int followingOffset = 0;
   eTimerMatch TimerMatch = tmNone;
   const cTimer *Timer;
   if (PresentEvent) {
      LOCK_TIMERS_READ
      Timer = Timers->GetMatch(PresentEvent, &TimerMatch);
      if (Timer && Timer->Recording() && Timer->Local()) {
         changed = true;
         presentOffset = elchiSymbols.Width(SYM_REC) + Gap;
         if (PresentEvent->IsRunning())
            pmBG->DrawBitmap(cPoint(xEvText, yEvText + (lh - elchiSymbols.Height(SYM_REC))/2), elchiSymbols.Get(SYM_REC,
                           Theme.Color(clrSymbolRecFg), Theme.Color(clrSymbolRecBg)));
         else
            pmBG->DrawBitmap(cPoint(xEvText, yEvText + (lh - elchiSymbols.Height(SYM_REC))/2), elchiSymbols.Get(SYM_REC,
                           Theme.Color(clrChannelEpgShortText), Theme.Color(clrChannelEpgTitleBg)));
      }
   }
   if (presentOffset != presentLastOffset) {
      if (presentOffset < presentLastOffset)
         pmBG->DrawRectangle(cRect(xEvText, yEvText + (lh - elchiSymbols.Height(SYM_REC))/2, elchiSymbols.Width(SYM_REC), elchiSymbols.Height(SYM_REC)), Theme.Color(clrBackground));
      spmPresentTitle->SetViewPort(cRect(xEvText + presentOffset, yEvText, wEvText - presentOffset, lh));
      presentLastOffset = presentOffset;
   }

   if (FollowingEvent) {
      LOCK_TIMERS_READ
      Timer = Timers->GetMatch(FollowingEvent, &TimerMatch);
      if (Timer && TimerMatch == tmFull && Timer->HasFlags(tfActive) && Timer->Local()) {
         changed = true;
         followingOffset = elchiSymbols.Width(SYM_REC) + Gap;
         pmBG->DrawBitmap(cPoint(xEvText, yEvText + 2*lh + (lh - elchiSymbols.Height(SYM_REC))/2), elchiSymbols.Get(SYM_REC,
                         Theme.Color(clrChannelEpgTitleBg), Theme.Color(clrChannelEpgShortText)));
      }
   }
   if (followingOffset != followingLastOffset) {
      if (followingOffset < followingLastOffset)
         pmBG->DrawRectangle(cRect(xEvText, yEvText + 2*lh + (lh - elchiSymbols.Height(SYM_REC))/2, elchiSymbols.Width(SYM_REC), elchiSymbols.Height(SYM_REC)), Theme.Color(clrBackground));
      spmFollowingTitle->SetViewPort(cRect(xEvText + followingOffset, yEvText + 2*lh, wEvText - followingOffset, lh));
      followingLastOffset = followingOffset;
   }

   if (!showMessage && !showVolume) {

      // ----------------------- video format --------------------------------
      if (ElchiConfig.showVideoInfo) {
         if (hasVideo) { // Channel has video and is allowed to display
            cVideoInfo videoinfo;
            ElchiStatus->GetVideoInfo(&videoinfo);
            
            //  draw AR symbol
            if (old_ar != videoinfo.aspectratio ) {
               cBitmap *bmp = NULL;
               switch (videoinfo.aspectratio) {
                  case arHD:     bmp = &elchiSymbols.Get(SYM_AR_HD, Theme.Color(clrChannelSymbolOn), bg); break;
                  case arUHD:    bmp = &elchiSymbols.Get(SYM_AR_UHD, Theme.Color(clrChannelSymbolOn), bg); break;
                  case ar4_3:    bmp = &elchiSymbols.Get(SYM_AR_43, Theme.Color(clrChannelSymbolOn), bg); break;
                  case ar16_9:   bmp = &elchiSymbols.Get(SYM_AR_169, Theme.Color(clrChannelSymbolOn), bg); break;
                  default:       break;
               }

               if (bmp) 
                  pmSymbols->DrawBitmap(cPoint(xSymbols[xSYM_AR], ySymbolARRec), *bmp);
               else
                  pmSymbols->DrawRectangle(cRect(xSymbols[xSYM_AR], ySymbolARRec, elchiSymbols.Width(SYM_AR_HD), elchiSymbols.Height(SYM_AR_HD)), bg);

               changed = true;
               old_ar = videoinfo.aspectratio;
            }

            //  display width x height
            if (2 == ElchiConfig.showVideoInfo) {
               if ((old_width != videoinfo.width) || (old_height != videoinfo.height)) {
                  cString videoformatstring;
                  if ((videoinfo.width != 0) && (videoinfo.height != 0))
                     videoformatstring = cString::sprintf(" %dx%d", videoinfo.width, videoinfo.height);
                  else
                     videoformatstring = cString();

                  pmVideoSize->DrawRectangle(cRect(0, 0, wVsize, lh), clrTransparent);
                  pmVideoSize->DrawText(cPoint(0, 0), *videoformatstring, Theme.Color(clrChannelNameFg), clrTransparent, smallfont, wVsize, smallfont->Height(), taRight);

                  changed = true;

                  old_width = videoinfo.width;
                  old_height = videoinfo.height;
               }
            }
         }
      }

      // ----------------- signal info --------------------------------
      if (ElchiConfig.showSignalBars) {
         int SignalStrength = cDevice::ActualDevice()->SignalStrength();
         if (SignalStrength >= 0 && (SignalStrength != LastSignalStrength)) {
            int s = SignalStrength * elchiSymbols.Width(SYM_SIGNAL) / 100;
            pmBG->DrawBitmap(cPoint(xRight - lh2 - elchiSymbols.Width(SYM_SIGNAL), yChDateTime + SymbolGap), elchiSymbols.Get(SYM_SIGNAL, 0, 0));
            pmBG->DrawRectangle(cRect(xRight - lh2 - elchiSymbols.Width(SYM_SIGNAL) + s, yChDateTime + SymbolGap, elchiSymbols.Width(SYM_SIGNAL) - s, elchiSymbols.Height(SYM_SIGNAL)), bg);
            LastSignalStrength = SignalStrength;
            changed = true;
         }

         int SignalQuality = cDevice::ActualDevice()->SignalQuality();
         if (SignalQuality >= 0 && (SignalQuality != LastSignalQuality)) {
            int q = SignalQuality * elchiSymbols.Width(SYM_SIGNAL) / 100;
            pmBG->DrawBitmap(cPoint(xRight - lh2 - elchiSymbols.Width(SYM_SIGNAL), yChDateTime + lh/2 + SymbolGap), elchiSymbols.Get(SYM_SIGNAL, 0, 0));
            pmBG->DrawRectangle(cRect(xRight - lh2 - elchiSymbols.Width(SYM_SIGNAL) + q, yChDateTime + lh/2 + SymbolGap, elchiSymbols.Width(SYM_SIGNAL) - q, elchiSymbols.Height(SYM_SIGNAL)), bg);
            LastSignalQuality = SignalQuality;
            changed = true;
         }
      }

      // ----------------------- audio --------------------------------
      // display audio string
      if (spmAudio)
      {
         cDevice *Device = cDevice::PrimaryDevice();
         cString newaudiostring;
         
         int numaudiotracks = Device->NumAudioTracks();
         if (!numaudiotracks)
            newaudiostring = trVDR("No audio available!");
         else {
            const tTrackId *Track = Device->GetTrack(Device->GetCurrentAudioTrack());
            if (!Track)
            {  // kein AudioTrack, dann auch keine Anzeige
               newaudiostring = NULL;
            }
            else
            {  // AudioTrack vorh., numaudiotracks => 1
               // Get AudioChannels, Track->description and Track->Language
               int audiochannel = Device->GetAudioChannel();
               if (isempty(Track->description))
                  if (audiochannel > 0)
                     if (numaudiotracks > 1)
                        newaudiostring = cString::sprintf("%s /%d (%s)", Track->language, numaudiotracks, audiochannel == 1?tr("left channel"): tr("right channel"));
                     else
                        newaudiostring = cString::sprintf("%s (%s)", Track->language, audiochannel == 1?tr("left channel"): tr("right channel"));
                     else
                        if (numaudiotracks > 1)
                           newaudiostring = cString::sprintf("%s /%d", Track->language, numaudiotracks);
                        else
                           newaudiostring = cString::sprintf("%s", Track->language);
                        else
                           if (audiochannel > 0)
                              if (numaudiotracks > 1)
                                 newaudiostring = cString::sprintf("%s /%d (%s, %s)", Track?Track->description:"?", numaudiotracks, Track?Track->language:"?", audiochannel == 1?tr("left channel"): tr("right channel"));
                              else
                                 newaudiostring = cString::sprintf("%s (%s, %s)", Track?Track->description:"?", Track?Track->language:"?", audiochannel == 1?tr("left channel"): tr("right channel"));
                              else
                                 if (numaudiotracks > 1)
                                    newaudiostring = cString::sprintf("%s /%d (%s)", Track?Track->description:"?", numaudiotracks, Track?Track->language:"?");
                                 else
                                    newaudiostring = cString::sprintf("%s (%s)", Track?Track->description:"?", Track?Track->language:"?");
            }
         }
         
         audiostring = newaudiostring;
         spmAudio->SetText(audiostring, cFont::GetFont(fontSml));
      }
      
      
      // ----------------------- recording --------------------------------
      if (yRecordings != yEvText)
      {
         int numRecordings = 0;
         int newrecordingChange = ElchiStatus->GetRecordingChange(&numRecordings);
         if (recordingchange != newrecordingChange) {
         
            recordingchange = newrecordingChange;
            if (0 == numRecordings) {
               spmRecording->SetLayer(LYR_HIDDEN);
            }
            else {
               cString prefix = cString::sprintf("%s:", numRecordings == 1 ? trVDR("Recording") : trVDR("Recordings"));
               spmRecording->SetText(ElchiStatus->GetRecordingsString(prefix), smallfont);
               spmRecording->SetLayer(LYR_SCROLL);
            }
         }
      } //end recording
   } // end !showMessage && !showVolume

   // ----------------------- cutting symbol --------------------------------
   bool cuttingtemp = RecordingsHandler.Active();
   if (!showVolume && !showMessage && cutting != cuttingtemp) {
      cutting = cuttingtemp;
      pmSymbols->DrawBitmap(cPoint(xSymbols[xSYM_cutting], ySymbolARRec), elchiSymbols.Get(SYM_CUTTING, Theme.Color(cutting ? clrChannelSymbolOn : clrChannelSymbolOff), bg));
      changed = true;
   }
   
   // ----------------------- recording symbol --------------------------------
   bool rectemp = cRecordControls::Active();
   if (!showVolume && !showMessage && recording != rectemp) {
      recording = rectemp;
      pmSymbols->DrawBitmap(cPoint(xSymbols[xSYM_REC], ySymbolARRec), elchiSymbols.Get(SYM_REC, Theme.Color(recording ? clrSymbolRecFg : clrChannelSymbolOff), recording ? Theme.Color(clrSymbolRecBg) : bg));
      changed = true;
   }

   // ----------------------- volume display --------------------------------
   int volume, newVolumechange;
   newVolumechange = ElchiStatus->GetVolumeChange(&volume);
   if (volumechange != newVolumechange) {
      volumechange = newVolumechange;
      if (!showVolume) {
         // draw it if not already shown
         spmChannelName->SetLayer(LYR_HIDDEN);
         pmChannelNameBg->SetLayer(LYR_HIDDEN);
         pmSymbols->SetLayer(LYR_HIDDEN);
         if (pmVideoSize)     pmVideoSize->SetLayer(LYR_HIDDEN);
         pmMessageBG->SetLayer(LYR_TEXTBG);

         showVolume = true;
      }

      pmChDateTime->Clear();
      pmChDateTime->DrawText(cPoint(0, 0), cString::sprintf("%s %d%%", trVDR("Volume "), volume * 100 / 255), Theme.Color(clrChannelDateFg), clrTransparent, smallfont, wChNumber+wDateTime);
      
      // draw volume bar and set timeout
      pmMessageBG->Fill(Theme.Color(clrBackground));
      int wVolumeBar = (pmMessageBG->DrawPort().Width() - xChName + xLeft - lh) * volume / 255;
      DrawShadedRectangle(pmMessageBG, Theme.Color(clrVolumeBarUpper), cRect(xChName + lh2 - xLeft, lh/4, wVolumeBar, lh2));
      DrawShadedRectangle(pmMessageBG, Theme.Color(clrVolumeBarLower), cRect(xChName + lh2 - xLeft + wVolumeBar, lh/4, (pmMessageBG->DrawPort().Width() - xChName + xLeft - lh) - wVolumeBar, lh2));

      if (withInfo) {
         if (ElchiConfig.showLogo) {
            // clear Logo Area
            pmMessageBG->DrawRectangle(cRect(xLogo, yLogo, wLogo - xLeft + Gap/2, hLogo + Gap/2 - yChName), clrTransparent);
         }
      }
      else { //withoutinfo
         pmMessageBG->DrawEllipse(cRect(xRight - xLeft - lh2, lh - lh2, lh2, lh2), clrTransparent, -4);
         if (ElchiConfig.showLogo) {
            pmMessageBG->DrawRectangle(cRect(xLogo, yLogo, wLogo - xLeft + Gap/2, hLogo + Gap/2 - yChName), clrTransparent);
         }
         else {
            pmMessageBG->DrawEllipse(cRect(0, lh - lh2, lh2, lh2), clrTransparent, -3);
         }
      }
      
      changed = true;
      volumeTimer.Set(1000);
   }
   else { // volume has not changed
      if (showVolume) {
         if(volumeTimer.TimedOut()) {
            // redraw Channelnumber/name and update date/time
            lastdate = DayDateTime();
            pmChDateTime->Clear();
            pmChDateTime->DrawText(cPoint(wChNumber, 0), lastdate, Theme.Color(clrChannelDateFg), clrTransparent, smallfont, wDateTime, smallfont->Height(), taRight);
            pmChDateTime->DrawText(cPoint(0, 0), Channelnumber, Theme.Color(clrChannelDateFg), clrTransparent, cFont::GetFont(fontOsd));

            pmMessageBG->SetLayer(LYR_HIDDEN);
            spmChannelName->SetLayer(LYR_SCROLL);
            pmChannelNameBg->SetLayer(LYR_TEXTBG);
            pmSymbols->SetLayer(LYR_BG);
            if (pmVideoSize)     pmVideoSize->SetLayer(LYR_TEXT);
            showVolume = false;
            changed = true;
         }
      }
      else {  // !showVolume
         // update date and time
         cString date = DayDateTime();
         if (!*lastdate || strcmp(lastdate, date)) {
            lastdate = date;
            pmChDateTime->Clear();
            pmChDateTime->DrawText(cPoint(wChNumber, 0), date, Theme.Color(clrChannelDateFg), clrTransparent, smallfont, wDateTime, smallfont->Height(), taRight);

            pmChDateTime->DrawText(cPoint(0, 0), Channelnumber, Theme.Color(clrChannelDateFg), clrTransparent, cFont::GetFont(fontOsd));
            changed = true;
         }
      }
   } // volume stuff end

   if (changed) {
      DSYSLOG2("skinelchiHD: DisplayChannel::Flush()ing, changed")
      osd->Flush();
      changed = false;
   }
}
