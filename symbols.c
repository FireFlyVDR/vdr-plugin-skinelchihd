/*
 * symbols.c: symbol cache
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

//#define DEBUG
#include "symbols.h"
#include "common.h"

#define X_DISPLAY_MISSING
#include <Magick++.h>

#include "symbols/ar16_9.xpm"
#include "symbols/ar4_3.xpm"
#include "symbols/arHD.xpm"
#include "symbols/arUHD.xpm"
#include "symbols/arrowdown.xpm"
#include "symbols/arrowturn.xpm"
#include "symbols/arrowup.xpm"
#include "symbols/audioleft.xpm"
#include "symbols/audioright.xpm"
#include "symbols/audiostereo.xpm"
#include "symbols/audio.xpm"
#include "symbols/clocksml.xpm"
#include "symbols/clock.xpm"
#include "symbols/dolbydigital.xpm"
#include "symbols/encrypted.xpm"
#include "symbols/ffwd1.xpm"
#include "symbols/ffwd2.xpm"
#include "symbols/ffwd3.xpm"
#include "symbols/ffwd4.xpm"
#include "symbols/ffwd5.xpm"
#include "symbols/ffwd6.xpm"
#include "symbols/ffwd7.xpm"
#include "symbols/ffwd8.xpm"
#include "symbols/ffwd.xpm"
#include "symbols/frew1.xpm"
#include "symbols/frew2.xpm"
#include "symbols/frew3.xpm"
#include "symbols/frew4.xpm"
#include "symbols/frew5.xpm"
#include "symbols/frew6.xpm"
#include "symbols/frew7.xpm"
#include "symbols/frew8.xpm"
#include "symbols/frew.xpm"
#include "symbols/mute.xpm"
#include "symbols/newsml.xpm"
#include "symbols/pause_xpm.xpm"
#include "symbols/play.xpm"
#include "symbols/radio.xpm"
#include "symbols/rec.xpm"
#include "symbols/recsml.xpm"
#include "symbols/sfwd1.xpm"
#include "symbols/sfwd2.xpm"
#include "symbols/sfwd3.xpm"
#include "symbols/sfwd4.xpm"
#include "symbols/sfwd5.xpm"
#include "symbols/sfwd6.xpm"
#include "symbols/sfwd7.xpm"
#include "symbols/sfwd8.xpm"
#include "symbols/sfwd.xpm"
#include "symbols/signal.xpm"
#include "symbols/srew1.xpm"
#include "symbols/srew2.xpm"
#include "symbols/srew3.xpm"
#include "symbols/srew4.xpm"
#include "symbols/srew5.xpm"
#include "symbols/srew6.xpm"
#include "symbols/srew7.xpm"
#include "symbols/srew8.xpm"
#include "symbols/srew.xpm"
#include "symbols/teletext.xpm"
#include "symbols/cutting.xpm"
#include "symbols/unmute.xpm"
#include "symbols/volume.xpm"
#include "symbols/vpssml.xpm"
#include "symbols/vps.xpm"

using namespace Magick;

static cBitmap bmAr169(ar16_9);
static cBitmap bmAr43(ar4_3);
static cBitmap bmArHD(arHD);
static cBitmap bmArUHD(arUHD);
static cBitmap bmArrowdown(arrowdown);
static cBitmap bmArrowturn(arrowturn);
static cBitmap bmArrowup(arrowup);
static cBitmap bmAudioleft(audioleft);
static cBitmap bmAudioright(audioright);
static cBitmap bmAudiostereo(audiostereo);
static cBitmap bmAudio(audio);
static cBitmap bmClocksml(clocksml);
static cBitmap bmClock(clock_xpm);
static cBitmap bmDolbydigital(dolbydigital);
static cBitmap bmEncrypted(encrypted);
static cBitmap bmFfwd1(ffwd1);
static cBitmap bmFfwd2(ffwd2);
static cBitmap bmFfwd3(ffwd3);
static cBitmap bmFfwd4(ffwd4);
static cBitmap bmFfwd5(ffwd5);
static cBitmap bmFfwd6(ffwd6);
static cBitmap bmFfwd7(ffwd7);
static cBitmap bmFfwd8(ffwd8);
static cBitmap bmFfwd(ffwd);
static cBitmap bmFrew1(frew1);
static cBitmap bmFrew2(frew2);
static cBitmap bmFrew3(frew3);
static cBitmap bmFrew4(frew4);
static cBitmap bmFrew5(frew5);
static cBitmap bmFrew6(frew6);
static cBitmap bmFrew7(frew7);
static cBitmap bmFrew8(frew8);
static cBitmap bmFrew(frew);
static cBitmap bmMute(mute);
static cBitmap bmNewSml(newsml);
static cBitmap bmPause(pause_xpm);
static cBitmap bmPlay(play);
static cBitmap bmRadio(radio);
static cBitmap bmRec(rec);
static cBitmap bmRecSml(recsml);
static cBitmap bmSfwd1(sfwd1);
static cBitmap bmSfwd2(sfwd2);
static cBitmap bmSfwd3(sfwd3);
static cBitmap bmSfwd4(sfwd4);
static cBitmap bmSfwd5(sfwd5);
static cBitmap bmSfwd6(sfwd6);
static cBitmap bmSfwd7(sfwd7);
static cBitmap bmSfwd8(sfwd8);
static cBitmap bmSfwd(sfwd);
static cBitmap bmSignal(signal);
static cBitmap bmSrew1(srew1);
static cBitmap bmSrew2(srew2);
static cBitmap bmSrew3(srew3);
static cBitmap bmSrew4(srew4);
static cBitmap bmSrew5(srew5);
static cBitmap bmSrew6(srew6);
static cBitmap bmSrew7(srew7);
static cBitmap bmSrew8(srew8);
static cBitmap bmSrew(srew);
static cBitmap bmTeletext(teletext);
static cBitmap bmCutting(cutting);
static cBitmap bmUnmute(unmute);// unused
static cBitmap bmVolume(volume);
static cBitmap bmVpsSml(vpssml);
static cBitmap bmVps(vps);

int center(int Gap, int Length) { return (Gap - Length)/2; }

cSymbolCache elchiSymbols;
//
// --- cSymbolCache --------------------------------------------
// 
cSymbolCache::cSymbolCache()
{
   DSYSLOG("skinelchiHD: cSymbolCache is called")
 
   height = 0;
   for(int i =  0; i < SYM_MAX_COUNT; i++)
   {
      cache[i] = NULL;
   }

   Refresh(1080);
};

cSymbolCache::~cSymbolCache()
{
   DSYSLOG("skinelchiHD: ~cSymbolCache is called")
   clearCache();
};

void cSymbolCache::clearCache()
{
   DSYSLOG("skinelchiHD: cSymbolCache::clearCache is called");
   for(int i =  0; i < SYM_MAX_COUNT; i++)
   {
      cBitmap *bmp = cache[i];
      DELETENULL(bmp);
   }
   height = 0;
}

void cSymbolCache::Refresh(int newHeight)
{
#ifdef DEBUG   
   DSYSLOG("skinelchiHD: cSymbolCache::Refresh(%d, %d) is called", height, newHeight);
   double tp1, tp2;
   tp1 = GetTimeMS();
#endif

   if (newHeight != height)
   {
      clearCache();
      height = newHeight;
      double factor = height/2160.0;
      DSYSLOG("skinelchiHD: cSymbolCache::Refresh() factor=%.2f", factor);

      cache[SYM_AR_169] = bmAr169.Scaled(factor, factor, false);
      cache[SYM_AR_43]  = bmAr43.Scaled(factor, factor, false);
      cache[SYM_AR_HD]  = bmArHD.Scaled(factor, factor, false);
      cache[SYM_AR_UHD] = bmArUHD.Scaled(factor, factor, false);
      cache[SYM_ARROW_DOWN] = bmArrowdown.Scaled(factor, factor, false);
      cache[SYM_ARROW_TURN] = bmArrowturn.Scaled(factor, factor, false);
      cache[SYM_ARROW_UP] = bmArrowup.Scaled(factor, factor, false);
      cache[SYM_AUDIO_LEFT] = bmAudioleft.Scaled(factor, factor, false);
      cache[SYM_AUDIO_RIGHT] = bmAudioright.Scaled(factor, factor, false);
      cache[SYM_AUDIO_STEREO] = bmAudiostereo.Scaled(factor, factor, false);
      cache[SYM_AUDIO] = bmAudio.Scaled(factor, factor, false);
      cache[SYM_CLOCKSML] = bmClocksml.Scaled(factor, factor, false);
      cache[SYM_CLOCK] = bmClock.Scaled(factor, factor, false);
      cache[SYM_DOLBYDIGITAL] = bmDolbydigital.Scaled(factor, factor, false);
      cache[SYM_ENCRYPTED] = bmEncrypted.Scaled(factor, factor, false);
      cache[SYM_FFWD1] = bmFfwd1.Scaled(factor, factor, false);
      cache[SYM_FFWD2] = bmFfwd2.Scaled(factor, factor, false);
      cache[SYM_FFWD3] = bmFfwd3.Scaled(factor, factor, false);
      cache[SYM_FFWD4] = bmFfwd4.Scaled(factor, factor, false);
      cache[SYM_FFWD5] = bmFfwd5.Scaled(factor, factor, false);
      cache[SYM_FFWD6] = bmFfwd6.Scaled(factor, factor, false);
      cache[SYM_FFWD7] = bmFfwd7.Scaled(factor, factor, false);
      cache[SYM_FFWD8] = bmFfwd8.Scaled(factor, factor, false);
      cache[SYM_FFWD]  = bmFfwd.Scaled(factor, factor, false);
      cache[SYM_FREW1] = bmFrew1.Scaled(factor, factor, false);
      cache[SYM_FREW2] = bmFrew2.Scaled(factor, factor, false);
      cache[SYM_FREW3] = bmFrew3.Scaled(factor, factor, false);
      cache[SYM_FREW4] = bmFrew4.Scaled(factor, factor, false);
      cache[SYM_FREW5] = bmFrew5.Scaled(factor, factor, false);
      cache[SYM_FREW6] = bmFrew6.Scaled(factor, factor, false);
      cache[SYM_FREW7] = bmFrew7.Scaled(factor, factor, false);
      cache[SYM_FREW8] = bmFrew8.Scaled(factor, factor, false);
      cache[SYM_FREW]  = bmFrew.Scaled(factor, factor, false);
      cache[SYM_MUTE] = bmMute.Scaled(factor, factor, false);
      cache[SYM_NEWSML] = bmNewSml.Scaled(factor, factor, false);
      cache[SYM_PAUSE] = bmPause.Scaled(factor, factor, false);
      cache[SYM_PLAY] = bmPlay.Scaled(factor, factor, false);
      cache[SYM_RADIO] = bmRadio.Scaled(factor, factor, false);
      cache[SYM_REC] = bmRec.Scaled(factor, factor, false);
      cache[SYM_RECSML] = bmRecSml.Scaled(factor, factor, false);
      cache[SYM_SFWD1] = bmSfwd1.Scaled(factor, factor, false);
      cache[SYM_SFWD2] = bmSfwd2.Scaled(factor, factor, false);
      cache[SYM_SFWD3] = bmSfwd3.Scaled(factor, factor, false);
      cache[SYM_SFWD4] = bmSfwd4.Scaled(factor, factor, false);
      cache[SYM_SFWD5] = bmSfwd5.Scaled(factor, factor, false);
      cache[SYM_SFWD6] = bmSfwd6.Scaled(factor, factor, false);
      cache[SYM_SFWD7] = bmSfwd7.Scaled(factor, factor, false);
      cache[SYM_SFWD8] = bmSfwd8.Scaled(factor, factor, false);
      cache[SYM_SFWD]  = bmSfwd.Scaled(factor, factor, false);
      cache[SYM_SIGNAL] = bmSignal.Scaled(factor, factor, false);
      cache[SYM_SREW1] = bmSrew1.Scaled(factor, factor, false);
      cache[SYM_SREW2] = bmSrew2.Scaled(factor, factor, false);
      cache[SYM_SREW3] = bmSrew3.Scaled(factor, factor, false);
      cache[SYM_SREW4] = bmSrew4.Scaled(factor, factor, false);
      cache[SYM_SREW5] = bmSrew5.Scaled(factor, factor, false);
      cache[SYM_SREW6] = bmSrew6.Scaled(factor, factor, false);
      cache[SYM_SREW7] = bmSrew7.Scaled(factor, factor, false);
      cache[SYM_SREW8] = bmSrew8.Scaled(factor, factor, false);
      cache[SYM_SREW]  = bmSrew.Scaled(factor, factor, false);
      cache[SYM_TELETEXT] = bmTeletext.Scaled(factor, factor, false);
      cache[SYM_CUTTING] = bmCutting.Scaled(factor, factor, false);
      cache[SYM_UNMUTE] = bmUnmute.Scaled(factor, factor, false);
      cache[SYM_VOLUME] = bmVolume.Scaled(factor, factor, false);
      cache[SYM_VPSSML] = bmVpsSml.Scaled(factor, factor, false);
      cache[SYM_VPS] = bmVps.Scaled(factor, factor, false);
   }
   
#ifdef DEBUG   
   tp2 = GetTimeMS();
   int w = cache[SYM_AR_HD]->Width();
   int h = cache[SYM_AR_HD]->Height();
   DSYSLOG("skinelchiHD: cSymbolCache::Refresh(%d) %4.3f ms %dx%d", height, tp2-tp1, w, h);
#endif   
}


tColor cSymbolCache::Multiply(tColor Color, uint8_t Alpha)
{
   // taken from osd.c
   tColor RB = (Color & 0x00FF00FF) * Alpha;
   RB = ((RB + ((RB >> 8) & 0x00FF00FF) + 0x00800080) >> 8) & 0x00FF00FF;
   tColor AG = ((Color >> 8) & 0x00FF00FF) * Alpha;
   AG = ((AG + ((AG >> 8) & 0x00FF00FF) + 0x00800080)) & 0xFF00FF00;
   return AG | RB;
}

cBitmap &cSymbolCache::Get(eSymbols symbol, tColor clrSymbol, tColor clrBackground)
{  // return colored symbol with antialising blended between FG and BG color
   cBitmap *bmpSymbol = new cBitmap(cache[symbol]->Width(), cache[symbol]->Height(), 8);
   *bmpSymbol = *cache[symbol];

   // adapt palette if at least one clr is specified, e.g. don't touch palette for images like mute or signal
   if (clrSymbol || clrBackground)
   {
      cPalette palSymbol(8);
      int colors;
      bmpSymbol->Colors(colors);
   
      for(int i =  0; i < colors; i++)
      {
         int blend = bmpSymbol->Color(i) & 0x000000FF;
         tColor clr = Multiply(clrBackground, blend) + Multiply(clrSymbol, 255 - blend);
         palSymbol.SetColor(i, clr);
      }
      bmpSymbol->Replace(palSymbol);   
   }
   return *bmpSymbol;
}
