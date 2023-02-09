/*
 * background.h: thread for background tasks
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#ifndef __ELCHIHD_BACKGROUND_H_
#define __ELCHIHD_BACKGROUND_H_

#include <vdr/thread.h>
#include <vdr/plugin.h>
#include <vdr/osd.h>
#include "image.h"

class cBgObject : public cListObject
{
public:
   virtual bool Update() = 0;
};


class cElchiBackground : public cThread
{
private:
   cMutex mtxBg;
   cCondVar cvBlock;

   cCondWait cwDelay;
   cOsd *osd;
   virtual void Action(void);

   class cBgObjectList : public cList<cBgObject> { };
   cBgObjectList bgObjectList;

public:
   cElchiBackground(void);
   ~cElchiBackground();
   void Start();
   void Stop();
   void SetOSD(cOsd *Osd) { osd = Osd; }
   void Add(cBgObject*);
   void Del(cBgObject*);
};

extern cElchiBackground *ElchiBackground;


class cScrollingPixmap : public cBgObject
{
private:
   cOsd *osd;
   cRect vPort;
   tColor colorFg;
   tColor colorBg;
   bool centered;
   int alignment;
   int maxwidth;
   int textWidth;
   int direction;
   int xoffset, maxXoffset;
   bool active;
   cTimeMs spmTimer;
   int Delay;
   int fh;
   cString text;
   cPixmap *pixmap;

public:
   cScrollingPixmap(cOsd *Osd, const cRect VPort, const cFont *Font, int max_char, tColor ColorFg, tColor ColorBg = clrTransparent, bool centered = false, int Alignment = taDefault|taBorder);
   ~cScrollingPixmap();
   void SetColor(tColor ColorFg, tColor ColorBg = clrTransparent) { colorFg = ColorFg; colorBg = ColorBg; }
   void SetLayer(int Layer) { pixmap->SetLayer(Layer); }
   void SeAlpha(int Alpha) { pixmap->SetAlpha(Alpha); }
   void SetText(const char *Text, const cFont *Font);  // caller must not lock pixmaps
   void SetOffset(int Offset);
   bool Update();
};

#define MAXEPGIMAGES 6

class cEpgImage : public cThread, cBgObject
{
private:
   virtual void Action(void);
   void Stop(void);
   void Clear();

   cPixmap *pixmap;
   int w, h, frameSize;
   bool active;
   cString channelID;
   tEventID eventID;
   cString recordingPath;
   cTimeMs epgimageTimer;
   int maxImage, currentImage;
   cOSDImage *imgEPG[MAXEPGIMAGES];
   cMutex mtxImages;
   cMutex mtxEventID;
   cCondWait condWait;

public:
   cEpgImage(cPixmap *Pixmap,int Width, int Height, int FrameSize);
   ~cEpgImage();
   bool PutEventID(const char *strChannel, tEventID EventID);
   bool PutRecording(const char *recPath);
   bool Update();
};

#endif  //__ELCHIHD_BACKGROUND_H_
