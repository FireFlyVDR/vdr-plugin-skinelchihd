/*
 * setup.c: Setup and configuration file handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#include <vdr/themes.h>
#include <vdr/device.h>
#include <vdr/menuitems.h>
#include "common.h"
#include "config.h"
#include "setup.h"

extern const char *OSDSKIN;

extern cSkinElchiHDConfig ElchiConfig;

// --- cSkinElchiHDSetupGeneral ------------------------------------------------------
class cSkinElchiHDSetupGeneral : public cOsdMenu
{
private:
   cSkinElchiHDConfig *tmpConfig;
   int oldepgimagesize;
   int oldshowRecErrors;
   void Setup(void);
   const char * EpgImageSizeItems[6];
   const char * ResizeItems[3];

   const char * RecInfoItems[3];
   const char * VideoFormatItems[3];
   const char * TimerCheckItems[4];
   const char * ErrorWarningItems[3];
   const char * MailIconItems[3];
   const char * EpgDetails[3];
   const char * EpgImageSearch[2];

protected:
   virtual eOSState ProcessKey(eKeys Key);

public:
   cSkinElchiHDSetupGeneral(cSkinElchiHDConfig *tmpElchiConfig);
   virtual ~cSkinElchiHDSetupGeneral();
};

cSkinElchiHDSetupGeneral::cSkinElchiHDSetupGeneral(cSkinElchiHDConfig *TmpConfig)
:cOsdMenu("", 33)
{
   SetTitle(cString::sprintf("%s - '%s' %s", trVDR("Setup"), "skinElchiHD", tr("General")));

   tmpConfig = TmpConfig;
   oldepgimagesize = tmpConfig->EpgImageSize;
   oldshowRecErrors = tmpConfig->showRecErrors;

   EpgImageSizeItems[0] = tr("don't show");
   EpgImageSizeItems[1] = tr("small");
   EpgImageSizeItems[2] = tr("medium");
   EpgImageSizeItems[3] = tr("large");
   EpgImageSizeItems[4] = tr("extra large");
   EpgImageSizeItems[5] = tr("huge");

   VideoFormatItems[0]  = trVDR("no");
   VideoFormatItems[1]  = tr("format");
   VideoFormatItems[2]  = tr("size and format");

   TimerCheckItems[0] = trVDR("no");
   TimerCheckItems[1] = tr("Next recording");
   TimerCheckItems[2] = tr("Timer-Conflict");
   TimerCheckItems[3] = tr("all");

   ErrorWarningItems[0] = trVDR("no");
   ErrorWarningItems[1] = tr("only uncut recordings");
   ErrorWarningItems[2] = tr("all recordings");

   EpgDetails[0]   = trVDR("EPG");
   EpgDetails[1]   = tr("EPG + Details");
   EpgDetails[2]   = tr("EPG + Details + Genre");

   EpgImageSearch[0] = tr("channel ID + event ID");
   EpgImageSearch[1] = tr("event ID only");

   Setup();
}

cSkinElchiHDSetupGeneral::~cSkinElchiHDSetupGeneral()
{
}


void cSkinElchiHDSetupGeneral::Setup(void)
{
   Add(new cMenuEditBoolItem(tr("scroll text"), &tmpConfig->useScrolling));
   Add(new cMenuEditStraItem(tr("show timer and conflict in menu"), &tmpConfig->showTimer, 4, TimerCheckItems));
   Add(new cMenuEditBoolItem(tr("show logo if recording is HD/UHD"), &tmpConfig->showRecHD));
#if defined(APIVERSNUM) && APIVERSNUM >= 20506
   Add(new cMenuEditStraItem(tr("show warning if recording has errors"), &tmpConfig->showRecErrors, 3, ErrorWarningItems));
   if (tmpConfig->showRecErrors > 0) {
      // TRANSLATORS: note the two leading spaces
      Add(new cMenuEditIntItem(tr("  show warning only if recording errors are more than"), &tmpConfig->showRecNumErrors, 0, 10));
   }
#endif
   Add(new cMenuEditStraItem(tr("show recording details"), &tmpConfig->showRecDetails, 3, EpgDetails));
   Add(new cMenuEditStraItem(tr("show EPG details"), &tmpConfig->showEPGDetails, 3, EpgDetails));
   Add(new cMenuEditStraItem(tr("show video format info (if available)"), &tmpConfig->showVideoInfo, 3, VideoFormatItems));
   Add(new cMenuEditBoolItem(tr("use graphical progressbar"), &tmpConfig->GraphicalProgressbar));
   Add(new cMenuEditBoolItem(tr("show symbols"), &tmpConfig->showIcons));
   Add(new cMenuEditStraItem(tr("EPG picture size"), &tmpConfig->EpgImageSize, 6, EpgImageSizeItems));
   if (tmpConfig->EpgImageSize) {
      // TRANSLATORS: note the two leading spaces
      Add(new cMenuEditIntItem(tr("  duration of each EPG image [s]"), &tmpConfig->EpgImageDisplayTime, 2, 15));
      // TRANSLATORS: note the two leading spaces
      Add(new cMenuEditStraItem(tr("  search for EPG images with"), &tmpConfig->EpgImageEventIdOnly, 2, EpgImageSearch));
   }
   Add(new cMenuEditBoolItem(tr("show remote timers in EPG"), &tmpConfig->EpgShowRemoteTimers));
}

eOSState cSkinElchiHDSetupGeneral::ProcessKey(eKeys Key)
{
   eOSState state = cOsdMenu::ProcessKey(Key);

   if (state == osUnknown) {
      switch (Key) {
         case kOk:
         case kBack:
               state = osBack;
               break;
         default: break;
      }
   }

   if (oldepgimagesize != tmpConfig->EpgImageSize || oldshowRecErrors != tmpConfig->showRecErrors) {
      oldepgimagesize = tmpConfig->EpgImageSize;
      oldshowRecErrors = tmpConfig->showRecErrors;
      int oldcurrent = Current();
      Clear();
      Setup();
      SetCurrent(Get(oldcurrent));
      Display();
   }

   return state;
}

// --- cSkinElchiHDSetupChannelDisplay ------------------------------------------------------
class cSkinElchiHDSetupChannelDisplay : public cOsdMenu
{
private:
   cSkinElchiHDConfig *tmpConfig;
   void Setup(void);
   const char *RecInfoItems[3];
   const char *ShowLogoItems[3];
   const char *LogoTypeItems[2];

protected:
   virtual eOSState ProcessKey(eKeys Key);

public:
   cSkinElchiHDSetupChannelDisplay(cSkinElchiHDConfig *TmpConfig);
   virtual ~cSkinElchiHDSetupChannelDisplay();
};

cSkinElchiHDSetupChannelDisplay::cSkinElchiHDSetupChannelDisplay(cSkinElchiHDConfig *TmpConfig)
:cOsdMenu("", 33)
{
   SetTitle(cString::sprintf("%s - '%s' %s", trVDR("Setup"), "skinElchiHD", tr("Channel Display")));

   tmpConfig = TmpConfig;

   RecInfoItems[0] = trVDR("no");
   RecInfoItems[1] = tr("only when recording");
   RecInfoItems[2] = tr("always");

   ShowLogoItems[0] = trVDR("no");
   ShowLogoItems[1] = tr("normal size");
   ShowLogoItems[2] = tr("large size");

   LogoTypeItems[0] = "SVG";
   LogoTypeItems[1] = "PNG";

   Setup();
}

cSkinElchiHDSetupChannelDisplay::~cSkinElchiHDSetupChannelDisplay()
{
}

void cSkinElchiHDSetupChannelDisplay::Setup(void)
{
   Add(new cMenuEditBoolItem(tr("show audio Info"), &tmpConfig->showAudioInfo));
   Add(new cMenuEditStraItem(tr("show recording Info"), &tmpConfig->showRecInfo, 3, RecInfoItems));
   Add(new cMenuEditStraItem(tr("show channel logos"), &tmpConfig->showLogo, 3, ShowLogoItems));
#ifndef GRAPHICSMAGICK
   Add(new cMenuEditStraItem(tr("search logos first as"), &tmpConfig->LogoSVGFirst, 2, LogoTypeItems));
#endif
   Add(new cMenuEditBoolItem(tr("show signal bars"), &tmpConfig->showSignalBars));
   Add(new cMenuEditBoolItem(tr("show remote timers"), &tmpConfig->ShowRemoteTimers));
   Add(new cMenuEditBoolItem(tr("write logo messages to syslog"), &tmpConfig->LogoMessages));
}

eOSState cSkinElchiHDSetupChannelDisplay::ProcessKey(eKeys Key)
{
   eOSState state = cOsdMenu::ProcessKey(Key);

   if (state == osUnknown) {
      switch (Key) {
         case kOk:
         case kBack:
               state = osBack;
               break;
         default: break;
      }
   }
   return state;
}


// --- cSkinElchiHDSetup ------------------------------------------------------
cSkinElchiHDSetup::cSkinElchiHDSetup(void)
{
   tmpElchiConfig = ElchiConfig;

   Setup();
}

cSkinElchiHDSetup::~cSkinElchiHDSetup()
{
}

void cSkinElchiHDSetup::Setup(void)
{
   Add(new cOsdItem(tr("General"), osUser1));
   Add(new cOsdItem(tr("Channel Display"), osUser2));

#ifdef GRAPHICSMAGICK
   Add(new cOsdItem("image lib: GraphicsMagick", osUnknown, false));
#else
   Add(new cOsdItem("image lib: ImageMagick", osUnknown, false));
#endif
}

eOSState cSkinElchiHDSetup::ProcessKey(eKeys Key)
{
   bool hadSubMenu = HasSubMenu();
   eOSState state = cOsdMenu::ProcessKey(Key);

   if (hadSubMenu && !HasSubMenu() && Key == kOk)
      Store();

   switch (state) {
      case osUser1:
         AddSubMenu(new cSkinElchiHDSetupGeneral(&tmpElchiConfig));
         state=osContinue;
         break;
      case osUser2:
         AddSubMenu(new cSkinElchiHDSetupChannelDisplay(&tmpElchiConfig));
         state=osContinue;
         break;
      default:
         break;
   }
   return state;
}

void cSkinElchiHDSetup::Store(void)
{
   ElchiConfig = tmpElchiConfig;

   // general values
   SetupStore("useScrolling", ElchiConfig.useScrolling);
   SetupStore("showTimer", ElchiConfig.showTimer);
   SetupStore("showRecHD", ElchiConfig.showRecHD);
#if defined(APIVERSNUM) && APIVERSNUM >= 20506
   SetupStore("showRecErrors", ElchiConfig.showRecErrors);
   SetupStore("showRecNumErrors", ElchiConfig.showRecNumErrors);
#endif
   SetupStore("showRecDetails", ElchiConfig.showRecDetails);
   SetupStore("showEPGDetails", ElchiConfig.showEPGDetails);
   SetupStore("showVideoInfo", ElchiConfig.showVideoInfo);
   SetupStore("GraphicalProgressbar", ElchiConfig.GraphicalProgressbar);
   SetupStore("showIcons", ElchiConfig.showIcons);
   SetupStore("EpgImageSize", ElchiConfig.EpgImageSize);
   SetupStore("EpgImageDisplayTime", ElchiConfig.EpgImageDisplayTime);
   SetupStore("EpgImageEventIdOnly", ElchiConfig.EpgImageEventIdOnly);
   SetupStore("EpgShowRemoteTimers", ElchiConfig.EpgShowRemoteTimers);

   // channel Display values
   SetupStore("showAudioInfo", ElchiConfig.showAudioInfo);
   SetupStore("showRecInfo", ElchiConfig.showRecInfo);
   SetupStore("showLogo", ElchiConfig.showLogo);
   SetupStore("LogoSVGFirst", ElchiConfig.LogoSVGFirst);
   SetupStore("showSignalBars", ElchiConfig.showSignalBars);
   SetupStore("LogoMessages", ElchiConfig.LogoMessages);
   SetupStore("LogoSearchNameFirst"); // remove setting
   SetupStore("ShowRemoteTimers", ElchiConfig.ShowRemoteTimers);
}
