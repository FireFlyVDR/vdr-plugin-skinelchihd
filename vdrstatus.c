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
   audioTrack = NULL;
   audioChannel = NULL;
   tracks = NULL;
   volumeChange = 0;
   recordingChange = 0;
}


cSkinElchiStatus::~cSkinElchiStatus()
{
   DSYSLOG("skinelchiHD ~cSkinElchiStatus()");
   if (audioTrack) {
      free(audioTrack);
   }
}


void cSkinElchiStatus::ChannelSwitch(const cDevice * device, int channelNumber, bool Liveview)
{  // Indicates a channel switch on the given DVB device.
   // If ChannelNumber is 0, this is before the channel is being switched,
   // otherwise ChannelNumber is the number of the channel that has been switched to.
   int i = device->CardIndex();
   ChannelDevice[i] = device;
   ChannelNumber[i] = channelNumber;
   if (!channelNumber && cDevice::ActualDevice()->CardIndex() == i) {
      if (audioTrack) {
         free(audioTrack);
         audioTrack = NULL;
      }
      if (audioChannel) {
         audioChannel = NULL;
      }
   }
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

/*
void cSkinElchiStatus::SetAudioTrack(int Index, const char * const *Tracks)
{  // The audio track has been set to the one given by Index, which
   // points into the Tracks array of strings. Tracks is NULL terminated.
   //isyslog("skinelchiHD-Status: SetAudioTrack (%d, %s)", Index, tracks?(const char *)tracks[Index]:"NULL");
   if (tracks) {
      audioTrackIndex = Index + 1;
      tracks = Tracks;
      audioTrack = strdup(Tracks[Index]);
   }
   else
      if (audioTrack) {
         audioTrackIndex = 0;
         tracks = NULL;
         free(audioTrack);
         audioTrack = NULL;
      }
}


void cSkinElchiStatus::SetAudioChannel(int AudioChannel)
{  // The audio channel has been set to the given value.
   // 0=stereo, 1=left, 2=right, -1=no information available.
   switch (AudioChannel) {
     case -1: audioChannel = tr("Digital"); break;
     case 0:  audioChannel = tr("Stereo"); break;
     case 1:  audioChannel = tr("Left channel"); break;
     case 2:  audioChannel = tr("Right channel"); break;
     default: audioChannel = NULL;
   }
}
*/
void cSkinElchiStatus::GetVideoInfo(cVideoInfo *videoinfo)
{
   int Width, Height;
   double VideoAspect;
   cDevice::PrimaryDevice()->GetVideoSize(Width, Height, VideoAspect);
   videoinfo->height      = Height;
   videoinfo->width       = Width;
   if (Height >= 2160) videoinfo->aspectratio = arUHD;
   else if (Height >= 720) videoinfo->aspectratio = arHD;
      else if (Height == 0 && Width == 0) videoinfo->aspectratio = ar_unknown;
         else if (VideoAspect == 4.0/3.0) videoinfo->aspectratio = ar4_3;
            else if(VideoAspect == 16.0/9.0) videoinfo->aspectratio = ar16_9;
               else videoinfo->aspectratio = ar_unknown;
   return;
}
