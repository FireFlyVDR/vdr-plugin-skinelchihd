/*
 * config.c: Setup and configuration file handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#include "common.h"
#include "config.h"

cSkinElchiHDConfig ElchiConfig;

cTheme Theme;

// --- cSkinElchiHDConfig ----------------------------------------------------------
cSkinElchiHDConfig::cSkinElchiHDConfig(void)
{
   // general
   useScrolling = 1;
   showTimer = 0;
   showRecHD = 1;
   showRecErrors = 1;
   showRecDetails = 1;
   showEPGDetails = 0;
   showVideoInfo = 0;
   GraphicalProgressbar = 1;
   showIcons = 1;
   EpgImageSize = 3;
   EpgImageDisplayTime = 3;
   EpgImageEventIdOnly = 0;
   EpgShowRemoteTimers = 1;

   // channel display
   strcpy(logoBaseDir, "");
   strcpy(epgimageDir, "");
   showAudioInfo = 1;
   showRecInfo = 0;
   showLogo = 1;
   LogoSVGFirst = 0;
   showSignalBars = 1;
   LogoMessages = 0;
   ShowRemoteTimers = 1;
}

cSkinElchiHDConfig::~cSkinElchiHDConfig()
{
}

void cSkinElchiHDConfig::SetLogoBaseDir(const char *dir)
{
   if (dir) {
      strncpy(logoBaseDir, dir, sizeof(logoBaseDir));
      DSYSLOG("skinElchiHD: setting logoBaseDir to '%s'", logoBaseDir)
   }
}


void cSkinElchiHDConfig::SetEpgImageDir(const char *dir)
{
   if (dir) {
      strncpy(epgimageDir, dir, sizeof(epgimageDir));
      DSYSLOG("skinelchiHD: setting epgimageDir to '%s'", epgimageDir)
   }
}

bool cSkinElchiHDConfig::SetupParse(const char *Name, const char *Value)
{
   if      (strcmp(Name, "useScrolling") == 0)                 useScrolling = atoi(Value);
   else if (strcmp(Name, "showTimer") == 0)                    showTimer = atoi(Value);
   else if (strcmp(Name, "showRecHD") == 0)                    showRecHD = atoi(Value);
   else if (strcmp(Name, "showRecErrors") == 0)                showRecErrors = atoi(Value);
   else if (strcmp(Name, "showRecDetails") == 0)               showRecDetails = atoi(Value);
   else if (strcmp(Name, "showEPGDetails") == 0)               showEPGDetails = atoi(Value);
   else if (strcmp(Name, "showVideoInfo") == 0)                showVideoInfo = atoi(Value);
   else if (strcmp(Name, "GraphicalProgressbar") == 0)         GraphicalProgressbar = atoi(Value);
   else if (strcmp(Name, "showIcons") == 0)                    showIcons = atoi(Value);
   else if (strcmp(Name, "EpgImageSize") == 0)                 EpgImageSize = atoi(Value);
   else if (strcmp(Name, "EpgImageDisplayTime") == 0)          EpgImageDisplayTime = atoi(Value);
   else if (strcmp(Name, "EpgImageEventIdOnly") == 0)          EpgImageEventIdOnly = atoi(Value);
   else if (strcmp(Name, "EpgShowRemoteTimers") == 0)          EpgShowRemoteTimers = atoi(Value);

   else if (strcmp(Name, "showAudioInfo") == 0)                showAudioInfo = atoi(Value);
   else if (strcmp(Name, "showRecInfo") == 0)                  showRecInfo = atoi(Value);
   else if (strcmp(Name, "showLogo") == 0)                     showLogo = atoi(Value);
   else if (strcmp(Name, "LogoSVGFirst") == 0)                 LogoSVGFirst = atoi(Value);
   else if (strcmp(Name, "showSignalBars") == 0)               showSignalBars = atoi(Value);
   else if (strcmp(Name, "LogoMessages") == 0)                 LogoMessages = atoi(Value);
   else if (strcmp(Name, "ShowRemoteTimers") == 0)             ShowRemoteTimers = atoi(Value);

   else return false;
   return true;
}
