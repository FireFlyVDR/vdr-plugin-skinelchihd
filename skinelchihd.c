/*
 * skinelchihd.c: 'ElchiHD' skin for the Video Disk Recorder
 *
 * Copyright (C) 2002?- 2004? Andy Grobb, Rolf Ahrenberg, Andreas Kool
 * Copyright (C) 2004?- 2005? sezz @ vdr-portal.de, Christoph Haubrich
 * Copyright (C) 2005?- 2006 _Frank_ @ vdrportal.de
 * Copyright (C) 2007 - 2021 Christoph Haubrich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 * Or, point your browser to http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 *
 * See the README file how to reach the author.
 *
 * $Id$
 */
#include <getopt.h>

#include "setup.h"
#include "config.h"
#include "common.h"
#include "image.h"
#include "background.h"
#include "displaychannel.h"
#include "displaymenu.h"
#include "displayreplay.h"
#include "displayvolume.h"
#include "displaytracks.h"
#include "displaymessage.h"
#include "vdrstatus.h"
#include "skinelchihd.h"

#include <vdr/font.h>
#include <vdr/osd.h>
#include <vdr/menu.h>
#include <vdr/themes.h>
#include <vdr/plugin.h>
#include <vdr/thread.h>


#if defined(APIVERSNUM) && APIVERSNUM < 20403
#error THIS VERSION OF THE SKINELCHIHD-PLUGIN REQUIRES AT LEAST VDR 2.4.3
#endif

static const char *VERSION      = "0.5.3" ;
static const char *DESCRIPTION  = trNOOP("ElchiHD skin");
const char *OSDSKIN             = "ElchiHD";

cSkinElchiStatus *ElchiStatus;
cElchiBackground *ElchiBackground;


// --- cSkinElchiHD ----------------------------------------------------------
cSkinElchiHD::cSkinElchiHD(void)
:cSkin(OSDSKIN, &::Theme)
{
}


const char *cSkinElchiHD::Description(void)
{
   return tr("ElchiHD");
}


cSkinDisplayChannel *cSkinElchiHD::DisplayChannel(bool WithInfo)
{
   return new cSkinElchiHDDisplayChannel(WithInfo);
}


cSkinDisplayMenu *cSkinElchiHD::DisplayMenu(void)
{
   return new cSkinElchiHDDisplayMenu;
}


cSkinDisplayReplay *cSkinElchiHD::DisplayReplay(bool ModeOnly)
{
   return new cSkinElchiHDDisplayReplay(ModeOnly);
}


cSkinDisplayVolume *cSkinElchiHD::DisplayVolume(void)
{
   return new cSkinElchiHDDisplayVolume;
}


cSkinDisplayTracks *cSkinElchiHD::DisplayTracks(const char *Title, int NumTracks, const char * const *Tracks)
{
   return new cSkinElchiHDDisplayTracks(Title, NumTracks, Tracks);
}


cSkinDisplayMessage *cSkinElchiHD::DisplayMessage(void)
{
   return new cSkinElchiHDDisplayMessage;
}

// --- cPluginSkinElchiHD -----------------------------------------------------

class cPluginSkinElchiHD : public cPlugin {
private:

public:
   cPluginSkinElchiHD(void);
   virtual ~cPluginSkinElchiHD();
   virtual const char *Version(void) { return VERSION; }
   virtual const char *Description(void) { return tr(DESCRIPTION); }
   virtual const char *CommandLineHelp(void);
   virtual bool ProcessArgs(int argc, char *argv[]);
   virtual bool Initialize(void);
   virtual bool Start(void);
   virtual void Stop(void);
   virtual void Housekeeping(void);
   //virtual void MainThreadHook(void);
   //virtual cString Active(void);
   //virtual time_t WakeupTime(void);
   //virtual const char *MainMenuEntry(void) { return ((!ElchiConfig.hidemenu || strcasecmp(OSDSKIN, Setup.OSDSkin)) ? NULL : tr(DESCRIPTION)); }
   virtual const char *MainMenuEntry(void) { return NULL; }
   //virtual cOsdObject *MainMenuAction(void);
   virtual cMenuSetupPage *SetupMenu(void);
   virtual bool SetupParse(const char *Name, const char *Value);
   //bool Service(const char *Id, void *Data);
   //virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
};


cPluginSkinElchiHD::cPluginSkinElchiHD(void)
{
   // Initialize any member variables here.
   // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
   // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
   ElchiStatus = NULL;
   ElchiBackground = NULL;
}


cPluginSkinElchiHD::~cPluginSkinElchiHD()
{
   // Clean up after yourself!
   if(ElchiStatus) {
      DELETENULL(ElchiStatus);
   }

   if (ElchiBackground) {
      DELETENULL(ElchiBackground);
   }
}


const char *cPluginSkinElchiHD::CommandLineHelp(void)
{
   // Return a string that describes all known command line options.
   return
          "  -e <EPGImageDir>, --epgimages=<ImageDir>  path to EPG images\n"
          "                                            (default: '<CacheDir>/plugins/skinelchi/epgimages')\n"
#ifdef GRAPHICSMAGICK
          "  -l <LogoPath>,  --logopath=<LogoPath>     path to channel logos (PNG format)\n"
#else
          "  -l <LogoPath>,  --logopath=<LogoPath>     path to channel logos (SVG or PNG format)\n"
#endif
          "                                            (default: '<ResourceDir>/plugins/skinelchi/logos')\n";
}


bool cPluginSkinElchiHD::ProcessArgs(int argc, char *argv[])
{
   // Implement command line argument processing here if applicable.
   static struct option long_options[] = {
       { "epgimages", required_argument, NULL, 'e' },
       { "logopath", required_argument, NULL, 'l' },
       { NULL }
   };
   int c, option_index = 0;
   while ((c = getopt_long(argc, argv,
    "e:l:",
       long_options, &option_index)) != -1) {
      switch (c) {
        case 'e': ElchiConfig.SetEpgImageDir(optarg);
                  break;
        case 'l': ElchiConfig.SetLogoBaseDir(optarg);
                  break;
        default:
                  isyslog("skinelchiHD: unknown command-line argument: '%s'", optarg);
                  break;
      }
   }
   return true;
}


bool cPluginSkinElchiHD::Initialize(void)
{
   // Initialize any background activities the plugin shall perform.
   return true;
}


bool cPluginSkinElchiHD::Start(void)
{
   // Start any background activities the plugin shall perform.

   if (!cOsdProvider::SupportsTrueColor()) {
      esyslog("skinElchiHD: TrueColor OSD not found! Exiting ....");
      return false;
   }

   // set default logo and epg image dir if not supplied by commandline arguments
   if (isempty(ElchiConfig.GetLogoBaseDir()))
      ElchiConfig.SetLogoBaseDir(cString::sprintf("%s/logos",cPlugin::ResourceDirectory(PLUGIN_NAME_I18N)));
   isyslog("skinElchiHD: using channel logo path '%s'", ElchiConfig.GetLogoBaseDir());
   
   if (isempty(ElchiConfig.GetEpgImageDir()))
      ElchiConfig.SetEpgImageDir(cString::sprintf("%s/epgimages", cPlugin::CacheDirectory(PLUGIN_NAME_I18N)));
   isyslog("skinElchiHD: using EPG image path '%s'", ElchiConfig.GetEpgImageDir());

   new cSkinElchiHD;

   ElchiStatus = new cSkinElchiStatus;

   ElchiBackground = new cElchiBackground;
   ElchiBackground->Start();

   return true;
}


void cPluginSkinElchiHD::Stop(void)
{
   // Stop any background activities the plugin is performing.
   if (ElchiBackground) {
      ElchiBackground->Stop();
   }
}


void cPluginSkinElchiHD::Housekeeping(void)
{
   // Perform any cleanup or other regular tasks.
}


cMenuSetupPage *cPluginSkinElchiHD::SetupMenu(void)
{
   // Return a setup menu in case the plugin supports one.
   return new cSkinElchiHDSetup();
}


bool cPluginSkinElchiHD::SetupParse(const char *Name, const char *Value)
{
   // Parse your own setup parameters and store their values.
   return ElchiConfig.SetupParse(Name, Value);
}


VDRPLUGINCREATOR(cPluginSkinElchiHD); // Don't touch this!
