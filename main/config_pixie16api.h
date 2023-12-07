/*
  Extract the Pixie16 API includes and conditionalize them on 
  the API version 
*/
#if XIAAPI_VERSION >= 3

#include <pixie16/pixie16.h>

/* 
   The transition manual says these will be deprecated but the API 
   still needs them and I'm damned if I'll use magic numbers instead so:
*/
#ifndef LIST_MODE_RUN
#define LIST_MODE_RUN 0x100
#endif

#ifndef NEW_RUN
#define NEW_RUN 1
#endif

#ifndef RESUME_RUN
#define RESUME_RUN 0
#endif

/* 
   More deprecated constants used by QtScope:
*/
#ifndef MAX_HISTOGRAM_LENGTH
#define MAX_HISTOGRAM_LENGTH 32768
#endif

#ifndef MAX_ADC_TRACE_LEN
#define MAX_ADC_TRACE_LEN 8192
#endif

#ifndef MAX_NUM_BASELINES
#define MAX_NUM_BASELINES 3640
#endif

/* 
   Readout programs need this to decide when to start reading 
*/
#ifndef EXTFIFO_READ_THRESH
#define EXTFIFO_READ_THRESH   1024
#endif

#else

#ifndef PLX_LINUX
#define PLX_LINUX
#endif

#include <pixie16app_common.h>
#include <pixie16app_defs.h>
#include <pixie16app_export.h>
#include <xia_common.h>

#endif
