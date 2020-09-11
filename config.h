/*
 * config.h: Setup and configuration file handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#ifndef __ELCHIHD_CONFIG_H
#define __ELCHIHD_CONFIG_H

#include <vdr/themes.h>
#include <vdr/config.h>


#define MAX_ELCHI_THEME_COLORS 68


class cSkinElchiHDConfig
{
private:
   char logoBaseDir[MaxFileName];
   char epgimageDir[MaxFileName];

public:
   cSkinElchiHDConfig(void);
   ~cSkinElchiHDConfig();
   bool SetupParse(const char *Name, const char *Value);
   void SetLogoBaseDir(const char *dir);
   char *GetLogoBaseDir(void) { return logoBaseDir; }
   void SetEpgImageDir(const char *dir);
   char *GetEpgImageDir(void) { return epgimageDir; }

   // general
   int useScrolling;
   int showTimer;
   int showRecHD;
   int showRecDetails;
   int showEPGDetails;
   int showVideoInfo;
   int GraphicalProgressbar;
   int showIcons;
   int EpgImageSize;
   int EpgImageDisplayTime;

   int showAudioInfo;
   int showRecInfo;
   int showLogo;
   int LogoSVGFirst;
   int showSignalBars;
   int LogoMessages;

   // clr dialog
   //bool clrdlgActive;
   //tColor clrDlgFg;
   //tColor clrDlgBg;

   tColor ThemeColors[MAX_ELCHI_THEME_COLORS];
};


extern cSkinElchiHDConfig ElchiConfig;

extern cTheme Theme;

THEME_CLR(Theme, clrBackground,             0x77000066);
THEME_CLR(Theme, clrButtonRedFg,            clrWhite);
THEME_CLR(Theme, clrButtonRedBg,            0xCCCC1111);
THEME_CLR(Theme, clrButtonGreenFg,          clrBlack);
THEME_CLR(Theme, clrButtonGreenBg,          0xCC22BB22);
THEME_CLR(Theme, clrButtonYellowFg,         clrBlack);
THEME_CLR(Theme, clrButtonYellowBg,         0xCCEEEE22);
THEME_CLR(Theme, clrButtonBlueFg,           clrWhite);
THEME_CLR(Theme, clrButtonBlueBg,           0xCC2233CC);
THEME_CLR(Theme, clrMessageStatusFg,        clrBlack);
THEME_CLR(Theme, clrMessageStatusBg,        0xCC2BA7F1);
THEME_CLR(Theme, clrMessageInfoFg,          clrBlack);
THEME_CLR(Theme, clrMessageInfoBg,          0xCC22BB22);
THEME_CLR(Theme, clrMessageWarningFg,       clrBlack);
THEME_CLR(Theme, clrMessageWarningBg,       clrYellow);
THEME_CLR(Theme, clrMessageErrorFg,         clrWhite);
THEME_CLR(Theme, clrMessageErrorBg,         clrRed);
THEME_CLR(Theme, clrVolumePrompt,           clrWhite);
THEME_CLR(Theme, clrVolumeBarUpper,         0xFFCCBB22);
THEME_CLR(Theme, clrVolumeBarLower,         0x77000066);
THEME_CLR(Theme, clrVolumeSymbolMuteFg,     0xCCCC1111);
THEME_CLR(Theme, clrVolumeSymbolMuteBg,     clrWhite);
THEME_CLR(Theme, clrVolumeSymbolVolumeFg,   0xFFCCBB22);
THEME_CLR(Theme, clrVolumeSymbolVolumeBg,   0x77000066);
THEME_CLR(Theme, clrChannelNameFg,          clrWhite);
THEME_CLR(Theme, clrChannelNameBg,          0xCC2BA7F1);
THEME_CLR(Theme, clrChannelDateFg,          clrBlack);
THEME_CLR(Theme, clrChannelDateBg,          clrWhite);
THEME_CLR(Theme, clrChannelEpgTimeFg,       0xFFDDDDDD);
THEME_CLR(Theme, clrChannelEpgTimeBg,       0xAABB0000);
THEME_CLR(Theme, clrChannelEpgTitleFg,      0xFF00FCFC);
THEME_CLR(Theme, clrChannelEpgTitleBg,      0x88000000);
THEME_CLR(Theme, clrChannelEpgShortText,    0xFFCCBB22);
THEME_CLR(Theme, clrChannelSymbolOn,        clrYellow);
THEME_CLR(Theme, clrChannelSymbolOff,       0x77777777);
THEME_CLR(Theme, clrChannelTimebarSeen,     clrYellow);
THEME_CLR(Theme, clrChannelTimebarRest,     clrGray50);
THEME_CLR(Theme, clrSymbolRecFg,            clrWhite);
THEME_CLR(Theme, clrSymbolRecBg,            clrRed);
THEME_CLR(Theme, clrProgressBarUpper,       0xFFCCBB22); 
THEME_CLR(Theme, clrProgressBarLower,       0x77000066); 
THEME_CLR(Theme, clrMenuTitleFg,            clrBlack);
THEME_CLR(Theme, clrMenuTitleBg,            0xCC2BA7F1);
THEME_CLR(Theme, clrMenuDate,               clrBlack);
THEME_CLR(Theme, clrMenuItemCurrentFg,      clrBlack);
THEME_CLR(Theme, clrMenuItemCurrentBg,      0xCC2BA7F1);
THEME_CLR(Theme, clrMenuItemSelectable,     clrWhite);
THEME_CLR(Theme, clrMenuItemNonSelectable,  0xCC2BA7F1);
THEME_CLR(Theme, clrMenuEventTime,          clrWhite);
THEME_CLR(Theme, clrMenuEventVpsFg,         clrBlack);
THEME_CLR(Theme, clrMenuEventVpsBg,         clrWhite);
THEME_CLR(Theme, clrMenuEventTitle,         clrYellow);
THEME_CLR(Theme, clrMenuEventShortText,     clrWhite);
THEME_CLR(Theme, clrMenuEventDescription,   clrYellow);
THEME_CLR(Theme, clrMenuScrollbarTotal,     clrWhite);
THEME_CLR(Theme, clrMenuScrollbarShown,     clrYellow);
THEME_CLR(Theme, clrMenuText,               clrWhite);
THEME_CLR(Theme, clrReplayTitleFg,          clrBlack);
THEME_CLR(Theme, clrReplayTitleBg,          clrWhite);
THEME_CLR(Theme, clrReplayCurrent,          clrWhite);
THEME_CLR(Theme, clrReplayTotal,            clrWhite);
THEME_CLR(Theme, clrReplayModeJump,         clrWhite);
THEME_CLR(Theme, clrReplayProgressSeen,     0xCC22BB22);
THEME_CLR(Theme, clrReplayProgressRest,     clrWhite);
THEME_CLR(Theme, clrReplayProgressSelected, 0xCCCC1111);
THEME_CLR(Theme, clrReplayProgressMark,     clrBlack);
THEME_CLR(Theme, clrReplayProgressCurrent,  0xCCCC1111);
THEME_CLR(Theme, clrReplaySymbolOn,         clrYellow);
THEME_CLR(Theme, clrReplaySymbolOff,        0x77777777);

#endif //__ELCHIHD_CONFIG_H
