/*
 * vdrstatus.c: Keeping track of several VDR status settings
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

//#define DEBUG
//#define DEBUG2

#include "vdrstatus.h"
#include "setup.h"
#include "common.h"
#include <vdr/tools.h>


cRecordingEntry::cRecordingEntry(const cDevice *Device, const char *Name, const char *FileName)
{
   device = Device;
   name = Name;
   filename = FileName;
}


cRecordingEntry::~cRecordingEntry()
{
}


// --------------------- cSkinElchiStatus -------------------------
cSkinElchiStatus::cSkinElchiStatus()
{
   DSYSLOG("skinelchiHD cSkinElchiStatus()");
   volumeChange = 0;
   recordingChange = 0;
}


cSkinElchiStatus::~cSkinElchiStatus()
{
   DSYSLOG("skinelchiHD ~cSkinElchiStatus()");
}


void cSkinElchiStatus::Recording(const cDevice *Device, const char *Name, const char *FileName, bool On)
{  // The given DVB device has started (On = true) or stopped (On = false) recording Name.
   // Name is the name of the recording, without any directory path. The full file name
   // of the recording is given in FileName, which may be NULL in case there is no
   // actual file involved. If On is false, Name may be NULL.

   DSYSLOG("skinelchiHD statusrec: %d %d %s %s", Device->CardIndex(), On, Name, FileName)
   // Name may be null if On=false, but FileName is always set
   if (Name) {
      cRecordingEntry *newentry = new cRecordingEntry(Device, Name, FileName);

      cRecordingEntry *entry = recordinglist.First();
      while (entry && (Device->CardIndex() >= entry->GetDevice()->CardIndex()))
         entry = recordinglist.Next(entry);

      if (entry && (Device->CardIndex() < entry->GetDevice()->CardIndex()))
         recordinglist.Ins(newentry, entry);
      else
         recordinglist.Add(newentry, entry);
   }
   else {
      cRecordingEntry *entry = recordinglist.First();
      bool found = false;
      while (entry && !(found = !strcmp(entry->GetFilename(), FileName)))
         entry = recordinglist.Next(entry);

      if (found)
         recordinglist.Del(entry, true);
   }
   recordingChange++;
}

cString cSkinElchiStatus::GetRecordingsString(cString prefix)
{
   cString recordingstring = prefix;
   cString recs;
   cString tmp;
   cString recDevice;
   int prevCardIndex = recordinglist.First()->GetDevice()->CardIndex();

   for (cRecordingEntry *entry = recordinglist.First(); entry; entry = recordinglist.Next(entry)) {
      if (prevCardIndex != entry->GetDevice()->CardIndex()) {
         recDevice = cString::sprintf("%s DVB%d (%s)", (const char *)recordingstring, prevCardIndex, (const char *)recs);
         recordingstring = recDevice;
         recs = NULL;
      }

      tmp = cString::sprintf("%s%s%s", (const char *)recs?(const char *)recs:"", (const char *)recs?", ":"", entry->GetName());
      recs = tmp;

      prevCardIndex = entry->GetDevice()->CardIndex();
   }
   recDevice = cString::sprintf("%s DVB%d (%s)", (const char *)recordingstring, prevCardIndex, (const char *)recs);
   recordingstring = recDevice;

   return recordingstring;
}

void cSkinElchiStatus::SetVolume(int Volume, bool Absolute)
{  // The volume has been set to the given value, either
   // absolutely or relative to the current volume.

   volume = Absolute ? Volume : volume + Volume;
   volumeChange++;
}


void cSkinElchiStatus::GetVideoInfo(cVideoInfo *videoinfo)
{
   int Width, Height;
   double VideoAspect;
   cDevice::PrimaryDevice()->GetVideoSize(Width, Height, VideoAspect);
   videoinfo->height      = Height;
   videoinfo->width       = Width;
   if (Height >= 2160) videoinfo->videoFormat = videofmt_UHD;
   else if (Height >= 720) videoinfo->videoFormat = videofmt_HD;
      else if (Height == 0 && Width == 0) videoinfo->videoFormat = videofmt_unknown;
         else if (VideoAspect == 4.0/3.0) videoinfo->videoFormat = videofmt_4_3;
            else if(VideoAspect == 16.0/9.0) videoinfo->videoFormat = videofmt_16_9;
               else videoinfo->videoFormat = videofmt_unknown;
   return;
}
