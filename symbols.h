/*
 * symbols.h: symbol cache
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#ifndef __ELCHIHD_SYMBOLS_H
#define __ELCHIHD_SYMBOLS_H

#include <vdr/osd.h>

enum eSymbols
{
   SYM_AR_169,
   SYM_AR_43,
   SYM_AR_HD,
   SYM_AR_UHD,
   SYM_ARROW_DOWN,
   SYM_ARROW_TURN,
   SYM_ARROW_UP,
   SYM_AUDIO_LEFT,
   SYM_AUDIO_RIGHT,
   SYM_AUDIO_STEREO,
   SYM_AUDIO,
   SYM_CLOCKSML,
   SYM_CLOCK,
   SYM_DOLBYDIGITAL,
   SYM_ENCRYPTED,
   SYM_FFWD1,
   SYM_FFWD2,
   SYM_FFWD3,
   SYM_FFWD4,
   SYM_FFWD5,
   SYM_FFWD6,
   SYM_FFWD7,
   SYM_FFWD8,
   SYM_FFWD,
   SYM_FREW1,
   SYM_FREW2,
   SYM_FREW3,
   SYM_FREW4,
   SYM_FREW5,
   SYM_FREW6,
   SYM_FREW7,
   SYM_FREW8,
   SYM_FREW,
   SYM_MUTE,
   SYM_NEWSML,
   SYM_PAUSE,
   SYM_PLAY,
   SYM_RADIO,
   SYM_REC,
   SYM_RECSML,
   SYM_SFWD1,
   SYM_SFWD2,
   SYM_SFWD3,
   SYM_SFWD4,
   SYM_SFWD5,
   SYM_SFWD6,
   SYM_SFWD7,
   SYM_SFWD8,
   SYM_SFWD,
   SYM_SIGNAL,
   SYM_SREW1,
   SYM_SREW2,
   SYM_SREW3,
   SYM_SREW4,
   SYM_SREW5,
   SYM_SREW6,
   SYM_SREW7,
   SYM_SREW8,
   SYM_SREW,
   SYM_TELETEXT,
   SYM_CUTTING,
   SYM_UNMUTE,
   SYM_VOLUME,
   SYM_VPSSML,
   SYM_VPS,
   SYM_MAX_COUNT
};

class cSymbolCache
{
public:
   cSymbolCache();
   ~cSymbolCache();
   void Refresh(int Height);
   int Width(eSymbols symbol) { return height ? cache[symbol]->Width() : 0; }
   int Height(eSymbols symbol) { return height ? cache[symbol]->Height() : 0; }
   cBitmap& Get(eSymbols Symbol, tColor clrSymbol = clrWhite, tColor clrBackground = clrTransparent);

private:
   int height;
   cBitmap *cache[SYM_MAX_COUNT];
   void clearCache();
   tColor Multiply(tColor Color, uint8_t Alpha);
};

extern cSymbolCache elchiSymbols;

int center(int Gap, int Length);

#endif  //__ELCHIHD_SYMBOLS_H
