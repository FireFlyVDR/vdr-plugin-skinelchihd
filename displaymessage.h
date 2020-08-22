/*
 * displaymessage.h: Message display handling
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#ifndef __ELCHIHD_DISPLAYMESSAGE_H
#define __ELCHIHD_DISPLAYMESSAGE_H

#include <vdr/skins.h>
#include "config.h"
#include "background.h"


class cSkinElchiHDDisplayMessage : public cSkinDisplayMessage
{
private:
   cOsd *osd;
   int lh, lh2;
   bool changed;
   cPixmap *pmMsgBar;
   cScrollingPixmap *spmMsgBarText;
public:
   cSkinElchiHDDisplayMessage(void);
   virtual ~cSkinElchiHDDisplayMessage();
   virtual void SetMessage(eMessageType Type, const char *Text);
   virtual void Flush(void);
};

#endif //__ELCHIHD_DISPLAYMESSAGE_H
