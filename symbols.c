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

#include "symbols/ar16_9_2160.xpm"
#include "symbols/ar16_9_1080.xpm"
#include "symbols/ar16_9_720.xpm"
#include "symbols/ar16_9_576.xpm"

#include "symbols/ar4_3_2160.xpm"
#include "symbols/ar4_3_1080.xpm"
#include "symbols/ar4_3_720.xpm"
#include "symbols/ar4_3_576.xpm"

#include "symbols/arHD_2160.xpm"
#include "symbols/arHD_1080.xpm"
#include "symbols/arHD_720.xpm"
#include "symbols/arHD_576.xpm"

#include "symbols/arUHD_2160.xpm"
#include "symbols/arUHD_1080.xpm"
#include "symbols/arUHD_720.xpm"
#include "symbols/arUHD_576.xpm"

#include "symbols/arrow_turn_2160.xpm"
#include "symbols/arrow_turn_1080.xpm"
#include "symbols/arrow_turn_720.xpm"
#include "symbols/arrow_turn_576.xpm"

#include "symbols/arrow_turn_remote_2160.xpm"
#include "symbols/arrow_turn_remote_1080.xpm"
#include "symbols/arrow_turn_remote_720.xpm"
#include "symbols/arrow_turn_remote_576.xpm"

#include "symbols/clock_2160.xpm"
#include "symbols/clock_1080.xpm"
#include "symbols/clock_720.xpm"
#include "symbols/clock_576.xpm"

#include "symbols/clocksml_2160.xpm"
#include "symbols/clocksml_1080.xpm"
#include "symbols/clocksml_720.xpm"
#include "symbols/clocksml_576.xpm"

#include "symbols/clockinact_2160.xpm"
#include "symbols/clockinact_1080.xpm"
#include "symbols/clockinact_720.xpm"
#include "symbols/clockinact_576.xpm"

#include "symbols/clockinactsml_2160.xpm"
#include "symbols/clockinactsml_1080.xpm"
#include "symbols/clockinactsml_720.xpm"
#include "symbols/clockinactsml_576.xpm"

#include "symbols/clockremote_2160.xpm"
#include "symbols/clockremote_1080.xpm"
#include "symbols/clockremote_720.xpm"
#include "symbols/clockremote_576.xpm"

#include "symbols/clockinactiveremote_2160.xpm"
#include "symbols/clockinactiveremote_1080.xpm"
#include "symbols/clockinactiveremote_720.xpm"
#include "symbols/clockinactiveremote_576.xpm"

#include "symbols/cutting_2160.xpm"
#include "symbols/cutting_1080.xpm"
#include "symbols/cutting_720.xpm"
#include "symbols/cutting_576.xpm"

#include "symbols/dolbydigital_2160.xpm"
#include "symbols/dolbydigital_1080.xpm"
#include "symbols/dolbydigital_720.xpm"
#include "symbols/dolbydigital_576.xpm"

#include "symbols/encrypted_2160.xpm"
#include "symbols/encrypted_1080.xpm"
#include "symbols/encrypted_720.xpm"
#include "symbols/encrypted_576.xpm"

#include "symbols/error_2160.xpm"
#include "symbols/error_1080.xpm"
#include "symbols/error_720.xpm"
#include "symbols/error_576.xpm"

#include "symbols/newsml_2160.xpm"
#include "symbols/newsml_1080.xpm"
#include "symbols/newsml_720.xpm"
#include "symbols/newsml_576.xpm"

#include "symbols/radio_2160.xpm"
#include "symbols/radio_1080.xpm"
#include "symbols/radio_720.xpm"
#include "symbols/radio_576.xpm"

#include "symbols/rec_2160.xpm"
#include "symbols/rec_1080.xpm"
#include "symbols/rec_720.xpm"
#include "symbols/rec_576.xpm"

#include "symbols/recremote_2160.xpm"
#include "symbols/recremote_1080.xpm"
#include "symbols/recremote_720.xpm"
#include "symbols/recremote_576.xpm"

#include "symbols/recsml_2160.xpm"
#include "symbols/recsml_1080.xpm"
#include "symbols/recsml_720.xpm"
#include "symbols/recsml_576.xpm"

#include "symbols/vpssml_2160.xpm"
#include "symbols/vpssml_1080.xpm"
#include "symbols/vpssml_720.xpm"
#include "symbols/vpssml_576.xpm"

#include "symbols/vps_2160.xpm"
#include "symbols/vps_1080.xpm"
#include "symbols/vps_720.xpm"
#include "symbols/vps_576.xpm"

#include "symbols/arrowdown.xpm"
#include "symbols/arrowup.xpm"
#include "symbols/audioleft.xpm"
#include "symbols/audioright.xpm"
#include "symbols/audiostereo.xpm"
#include "symbols/audio.xpm"
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
#include "symbols/pause_xpm.xpm"
#include "symbols/play.xpm"
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
#include "symbols/unmute.xpm"
#include "symbols/volume.xpm"

using namespace Magick;

static cBitmap bmAr169_2160(ar16_9_2160);
static cBitmap bmAr169_1080(ar16_9_1080);
static cBitmap bmAr169_720(ar16_9_720);
static cBitmap bmAr169_576(ar16_9_576);

static cBitmap bmAr43_2160(ar4_3_2160);
static cBitmap bmAr43_1080(ar4_3_1080);
static cBitmap bmAr43_720(ar4_3_720);
static cBitmap bmAr43_576(ar4_3_576);

static cBitmap bmArHD_2160(arHD_2160);
static cBitmap bmArHD_1080(arHD_1080);
static cBitmap bmArHD_720(arHD_720);
static cBitmap bmArHD_576(arHD_576);

static cBitmap bmArUHD_2160(arUHD_2160);
static cBitmap bmArUHD_1080(arUHD_1080);
static cBitmap bmArUHD_720(arUHD_720);
static cBitmap bmArUHD_576(arUHD_576);

static cBitmap bmArrowTurn_2160(arrow_turn_2160);
static cBitmap bmArrowTurn_1080(arrow_turn_1080);
static cBitmap bmArrowTurn_720(arrow_turn_720);
static cBitmap bmArrowTurn_576(arrow_turn_576);

static cBitmap bmArrowTurnRemote_2160(arrow_turn_remote_2160);
static cBitmap bmArrowTurnRemote_1080(arrow_turn_remote_1080);
static cBitmap bmArrowTurnRemote_720(arrow_turn_remote_720);
static cBitmap bmArrowTurnRemote_576(arrow_turn_remote_576);

static cBitmap bmClock_2160(clock_2160);
static cBitmap bmClock_1080(clock_1080);
static cBitmap bmClock_720(clock_720);
static cBitmap bmClock_576(clock_576);

static cBitmap bmClockInactive_2160(clockinact_2160);
static cBitmap bmClockInactive_1080(clockinact_1080);
static cBitmap bmClockInactive_720(clockinact_720);
static cBitmap bmClockInactive_576(clockinact_576);

static cBitmap bmClockSml_2160(clocksml_2160);
static cBitmap bmClockSml_1080(clocksml_1080);
static cBitmap bmClockSml_720(clocksml_720);
static cBitmap bmClockSml_576(clocksml_576);

static cBitmap bmClockSmlInactive_2160(clockinactsml_2160);
static cBitmap bmClockSmlInactive_1080(clockinactsml_1080);
static cBitmap bmClockSmlInactive_720(clockinactsml_720);
static cBitmap bmClockSmlInactive_576(clockinactsml_576);

static cBitmap bmClockRemote_2160(clockremote_2160);
static cBitmap bmClockRemote_1080(clockremote_1080);
static cBitmap bmClockRemote_720(clockremote_720);
static cBitmap bmClockRemote_576(clockremote_576);

static cBitmap bmClockRemoteInactive_2160(clockinactiveremote_2160);
static cBitmap bmClockRemoteInactive_1080(clockinactiveremote_1080);
static cBitmap bmClockRemoteInactive_720(clockinactiveremote_720);
static cBitmap bmClockRemoteInactive_576(clockinactiveremote_576);

static cBitmap bmCutting_2160(cutting_2160);
static cBitmap bmCutting_1080(cutting_1080);
static cBitmap bmCutting_720(cutting_720);
static cBitmap bmCutting_576(cutting_576);

static cBitmap bmDolbydigital_2160(dolbydigital_2160);
static cBitmap bmDolbydigital_1080(dolbydigital_1080);
static cBitmap bmDolbydigital_720(dolbydigital_720);
static cBitmap bmDolbydigital_576(dolbydigital_576);

static cBitmap bmEncrypted_2160(encrypted_2160);
static cBitmap bmEncrypted_1080(encrypted_1080);
static cBitmap bmEncrypted_720(encrypted_720);
static cBitmap bmEncrypted_576(encrypted_576);

static cBitmap bmError_2160(error_2160);
static cBitmap bmError_1080(error_1080);
static cBitmap bmError_720(error_720);
static cBitmap bmError_576(error_576);

static cBitmap bmNewSml_2160(newsml_2160);
static cBitmap bmNewSml_1080(newsml_1080);
static cBitmap bmNewSml_720(newsml_720);
static cBitmap bmNewSml_576(newsml_576);

static cBitmap bmRadio_2160(radio_2160);
static cBitmap bmRadio_1080(radio_1080);
static cBitmap bmRadio_720(radio_720);
static cBitmap bmRadio_576(radio_576);

static cBitmap bmRec_2160(rec_2160);
static cBitmap bmRec_1080(rec_1080);
static cBitmap bmRec_720(rec_720);
static cBitmap bmRec_576(rec_576);

static cBitmap bmRecRemote_2160(recremote_2160);
static cBitmap bmRecRemote_1080(recremote_1080);
static cBitmap bmRecRemote_720(recremote_720);
static cBitmap bmRecRemote_576(recremote_576);

static cBitmap bmRecSml_2160(recsml_2160);
static cBitmap bmRecSml_1080(recsml_1080);
static cBitmap bmRecSml_720(recsml_720);
static cBitmap bmRecSml_576(recsml_576);

static cBitmap bmVpsSml_2160(vpssml_2160);
static cBitmap bmVpsSml_1080(vpssml_1080);
static cBitmap bmVpsSml_720(vpssml_720);
static cBitmap bmVpsSml_576(vpssml_576);

static cBitmap bmVps_2160(vps_2160);
static cBitmap bmVps_1080(vps_1080);
static cBitmap bmVps_720(vps_720);
static cBitmap bmVps_576(vps_576);

static cBitmap bmArrowdown(arrowdown);
static cBitmap bmArrowup(arrowup);
static cBitmap bmAudioleft(audioleft);
static cBitmap bmAudioright(audioright);
static cBitmap bmAudiostereo(audiostereo);
static cBitmap bmAudio(audio);
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
static cBitmap bmPause(pause_xpm);
static cBitmap bmPlay(play);
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
static cBitmap bmUnmute(unmute);// unused
static cBitmap bmVolume(volume);

int center(int Gap, int Length) { return (Gap - Length)/2; }

cSymbolCache elchiSymbols;
//
// --- cSymbolCache --------------------------------------------
// 
cSymbolCache::cSymbolCache()
{
   DSYSLOG("skinelchiHD: cSymbolCache is called")

   OsdHeight = 0;
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
      if (i > SYM_VPS) {
         cBitmap *bmp = cache[i];
         DELETENULL(bmp);
      }
      else
         cache[i] = NULL;
   }
   OsdHeight = 0;
}

void cSymbolCache::Refresh(int newHeight)
{
#ifdef DEBUG
   DSYSLOG("skinelchiHD: cSymbolCache::Refresh(%d, %d) is called", OsdHeight, newHeight);
   double tp1, tp2;
   tp1 = GetTimeMS();
#endif

   if (newHeight != OsdHeight)
   {
      clearCache();
      OsdHeight = newHeight;

      if (OsdHeight >= 2160) {
         cache[SYM_AR_169]            = &bmAr169_2160;
         cache[SYM_AR_43]             = &bmAr43_2160;
         cache[SYM_AR_HD]             = &bmArHD_2160;
         cache[SYM_AR_UHD]            = &bmArUHD_2160;
         cache[SYM_ARROW_TURN]        = &bmArrowTurn_2160;
         cache[SYM_ARROW_TURN_REMOTE] = &bmArrowTurnRemote_2160;
         cache[SYM_CLOCK]             = &bmClock_2160;
         cache[SYM_CLOCK_INACTIVE]    = &bmClockInactive_2160;
         cache[SYM_CLOCKSML]          = &bmClockSml_2160;
         cache[SYM_CLOCKSML_INACTIVE] = &bmClockSmlInactive_2160;
         cache[SYM_CLOCK_REMOTE]      = &bmClockRemote_2160;
         cache[SYM_CLOCK_REMOTE_INACTIVE] = &bmClockRemoteInactive_2160;
         cache[SYM_CUTTING]           = &bmCutting_2160;
         cache[SYM_DOLBYDIGITAL]      = &bmDolbydigital_2160;
         cache[SYM_ENCRYPTED]         = &bmEncrypted_2160;
         cache[SYM_ERROR]             = &bmError_2160;
         cache[SYM_NEWSML]            = &bmNewSml_2160;
         cache[SYM_RADIO]             = &bmRadio_2160;
         cache[SYM_REC]               = &bmRec_2160;
         cache[SYM_REC_REMOTE]        = &bmRecRemote_2160;
         cache[SYM_RECSML]            = &bmRecSml_2160;
         cache[SYM_VPSSML]            = &bmVpsSml_2160;
         cache[SYM_VPS]               = &bmVps_2160;
      } else if (OsdHeight >= 1080) {
         cache[SYM_AR_43]             = &bmAr43_1080;
         cache[SYM_AR_169]            = &bmAr169_1080;
         cache[SYM_AR_HD]             = &bmArHD_1080;
         cache[SYM_AR_UHD]            = &bmArUHD_1080;
         cache[SYM_ARROW_TURN]        = &bmArrowTurn_1080;
         cache[SYM_ARROW_TURN_REMOTE] = &bmArrowTurnRemote_1080;
         cache[SYM_CLOCK]             = &bmClock_1080;
         cache[SYM_CLOCK_INACTIVE]    = &bmClockInactive_1080;
         cache[SYM_CLOCKSML]          = &bmClockSml_1080;
         cache[SYM_CLOCKSML_INACTIVE] = &bmClockSmlInactive_1080;
         cache[SYM_CLOCK_REMOTE]      = &bmClockRemote_1080;
         cache[SYM_CLOCK_REMOTE_INACTIVE] = &bmClockRemoteInactive_1080;
         cache[SYM_CUTTING]           = &bmCutting_1080;
         cache[SYM_DOLBYDIGITAL]      = &bmDolbydigital_1080;
         cache[SYM_ENCRYPTED]         = &bmEncrypted_1080;
         cache[SYM_ERROR]             = &bmError_1080;
         cache[SYM_NEWSML]            = &bmNewSml_1080;
         cache[SYM_RADIO]             = &bmRadio_1080;
         cache[SYM_REC]               = &bmRec_1080;
         cache[SYM_REC_REMOTE]        = &bmRecRemote_1080;
         cache[SYM_RECSML]            = &bmRecSml_1080;
         cache[SYM_VPSSML]            = &bmVpsSml_1080;
         cache[SYM_VPS]               = &bmVps_1080;
      } else if (OsdHeight >= 720) {
         cache[SYM_AR_43]             = &bmAr43_720;
         cache[SYM_AR_169]            = &bmAr169_720;
         cache[SYM_AR_HD]             = &bmArHD_720;
         cache[SYM_AR_UHD]            = &bmArUHD_720;
         cache[SYM_ARROW_TURN]        = &bmArrowTurn_720;
         cache[SYM_ARROW_TURN_REMOTE] = &bmArrowTurnRemote_720;
         cache[SYM_CLOCK]             = &bmClock_720;
         cache[SYM_CLOCK_INACTIVE]    = &bmClockInactive_720;
         cache[SYM_CLOCKSML]          = &bmClockSml_720;
         cache[SYM_CLOCKSML_INACTIVE] = &bmClockSmlInactive_720;
         cache[SYM_CLOCK_REMOTE]      = &bmClockRemote_720;
         cache[SYM_CLOCK_REMOTE_INACTIVE] = &bmClockRemoteInactive_720;
         cache[SYM_CUTTING]           = &bmCutting_720;
         cache[SYM_DOLBYDIGITAL]      = &bmDolbydigital_720;
         cache[SYM_ENCRYPTED]         = &bmEncrypted_720;
         cache[SYM_ERROR]             = &bmError_720;
         cache[SYM_NEWSML]            = &bmNewSml_720;
         cache[SYM_RADIO]             = &bmRadio_720;
         cache[SYM_REC]               = &bmRec_720;
         cache[SYM_REC_REMOTE]        = &bmRecRemote_720;
         cache[SYM_RECSML]            = &bmRecSml_720;
         cache[SYM_VPSSML]            = &bmVpsSml_720;
         cache[SYM_VPS]               = &bmVps_720;
      } else {  // <  720 incl. 576
         cache[SYM_AR_43]             = &bmAr43_576;
         cache[SYM_AR_169]            = &bmAr169_576;
         cache[SYM_AR_HD]             = &bmArHD_576;
         cache[SYM_AR_UHD]            = &bmArUHD_576;
         cache[SYM_ARROW_TURN]        = &bmArrowTurn_576;
         cache[SYM_ARROW_TURN_REMOTE] = &bmArrowTurnRemote_576;
         cache[SYM_CLOCK]             = &bmClock_576;
         cache[SYM_CLOCK_INACTIVE]    = &bmClockInactive_576;
         cache[SYM_CLOCKSML]          = &bmClockSml_576;
         cache[SYM_CLOCKSML_INACTIVE] = &bmClockSmlInactive_576;
         cache[SYM_CLOCK_REMOTE]      = &bmClockRemote_576;
         cache[SYM_CLOCK_REMOTE_INACTIVE] = &bmClockRemoteInactive_576;
         cache[SYM_CUTTING]           = &bmCutting_576;
         cache[SYM_DOLBYDIGITAL]      = &bmDolbydigital_576;
         cache[SYM_ENCRYPTED]         = &bmEncrypted_576;
         cache[SYM_ERROR]             = &bmError_576;
         cache[SYM_NEWSML]            = &bmNewSml_576;
         cache[SYM_RADIO]             = &bmRadio_576;
         cache[SYM_REC]               = &bmRec_576;
         cache[SYM_REC_REMOTE]        = &bmRecRemote_576;
         cache[SYM_RECSML]            = &bmRecSml_576;
         cache[SYM_VPSSML]            = &bmVpsSml_576;
         cache[SYM_VPS]               = &bmVps_576;
      }

      double factor = OsdHeight/2160.0;
      DSYSLOG("skinelchiHD: cSymbolCache::Refresh() factor=%.2f", factor);

      cache[SYM_ARROW_DOWN] = bmArrowdown.Scaled(factor, factor, false);
      cache[SYM_ARROW_UP]   = bmArrowup.Scaled(factor, factor, false);
      cache[SYM_AUDIO_LEFT] = bmAudioleft.Scaled(factor, factor, false);
      cache[SYM_AUDIO_RIGHT] = bmAudioright.Scaled(factor, factor, false);
      cache[SYM_AUDIO_STEREO] = bmAudiostereo.Scaled(factor, factor, false);
      cache[SYM_AUDIO] = bmAudio.Scaled(factor, factor, false);
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
      cache[SYM_MUTE]  = bmMute.Scaled(factor, factor, false);
      cache[SYM_PAUSE] = bmPause.Scaled(factor, factor, false);
      cache[SYM_PLAY]  = bmPlay.Scaled(factor, factor, false);
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
      cache[SYM_UNMUTE] = bmUnmute.Scaled(factor, factor, false);
      cache[SYM_VOLUME] = bmVolume.Scaled(factor, factor, false);
   }

#ifdef DEBUG
   tp2 = GetTimeMS();
   int w = cache[SYM_AR_HD]->Width();
   int h = cache[SYM_AR_HD]->Height();
   DSYSLOG("skinelchiHD: cSymbolCache::Refresh(%d) %4.3f ms %dx%d", OsdHeight, tp2-tp1, w, h);
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
{  /// return colored symbol with transparency blending between FG and BG color
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
         palSymbol.SetColor(i, Multiply(clrBackground, blend) + Multiply(clrSymbol, 255 - blend));
      }
      bmpSymbol->Replace(palSymbol);
   }
   return *bmpSymbol;
}
