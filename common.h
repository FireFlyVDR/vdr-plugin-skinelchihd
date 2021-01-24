/*
 * common.h: Common definitions
 *
 * See the main source file 'skinelchihd.c' for copyright information and
 * how to reach the author.
 *
 * $Id$
 */

#ifndef __ELCHIHD_COMMON_H
#define __ELCHIHD_COMMON_H

#include <vdr/config.h>
#include <time.h>

//#define DEBUG
//#define DEBUG2

#ifdef DEBUG
#	define ISYSLOG(x...)    isyslog(x);
#	define DSYSLOG(x...)    dsyslog(x);
#ifdef DEBUG2
#	define DSYSLOG2(x...)   dsyslog(x);
#else
#	define DSYSLOG2(x...)   ;
#endif
#else
#	define ISYSLOG(x...)    ;
#   define DSYSLOG(x...)    ;
#	define DSYSLOG2(x...)   ;
#endif

// EPGSearch Plugin (availability evaluated at runtime)
#include "services/epgsearch_services.h"
#define PLUGIN_EPGSEARCH "epgsearch"
#define EPGSEARCH_CONFLICTINFO "Epgsearch-lastconflictinfo-v1.0"

// layer definitions
#define LYR_HIDDEN  -1
#define LYR_BG       0
#define LYR_SELECTOR 1
#define LYR_TEXTBG   2
#define LYR_TEXT     3
#define LYR_SCROLL   4

struct tOSDsize {
  int left, top, width, height;
};

#define MAXCHARS 256

static inline double GetTimeMS(void)
{
#ifdef CLOCK_MONOTONIC
    struct timespec tspec;

    clock_gettime(CLOCK_MONOTONIC, &tspec);
    return (tspec.tv_sec * 1000.0 + tspec.tv_nsec / 1000000.0);
#else
    struct timeval tval;

    if (gettimeofday(&tval, NULL) < 0)
        return 0;
    return (tval.tv_sec * 1000.0 + tval.tv_usec / 1000.0);
#endif
}

union uColor {
   tColor clr;
   struct {
      uint8_t B;
      uint8_t G;
      uint8_t R;
      uint8_t A;
   } c;
};

#endif // __ELCHIHD_COMMON_H
