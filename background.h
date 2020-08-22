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
   void Del(cBgObject*, bool);
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
   cTimeMs timer;
   int Delay;
   cString text;
   cPixmap *pixmap;
   
public:
   cScrollingPixmap(cOsd *Osd, const cRect VPort, const cFont *Font, int max_char, tColor ColorFg, tColor ColorBg = clrTransparent, bool centered = false, int Alignment = taDefault|taBorder);
   ~cScrollingPixmap();
   void SetColor(tColor ColorFg, tColor ColorBg = clrTransparent);
   void SetLayer(int Layer);
   void SeAlpha(int Alpha);
   bool Update();
   void SetText(const char *Text, const cFont *Font);
   void SetViewPort(const cRect &Rect);
};

#define MAXEPGIMAGES 6

class cEpgImage : public cThread, cBgObject
{
private:
   virtual void Action(void);
   void Clear();
   
   cPixmap *pixmap;
   int w, h, frameSize;
   cString channelID;
   tEventID eventID;
   cString recordingPath;
   cTimeMs epgimageTimer;
   int maxImage, currentImage;
   cOSDImage *imgEPG[MAXEPGIMAGES];
   cMutex mtxImages;  // protect maxImage and imgEPG[]
   cMutex mtxEventID; // protect eventID and recordingPath
   cCondWait condWait;

public:
   cEpgImage(cPixmap *Pixmap,int Width, int Height, int FrameSize);
   ~cEpgImage();
   void Stop(void);
   bool PutEventID(const char *strChannel, tEventID EventID);
   bool PutRecording(const char *recPath);
   bool Update();
};

#endif  //__ELCHIHD_BACKGROUND_H_
