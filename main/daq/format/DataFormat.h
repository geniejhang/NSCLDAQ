#ifndef DATAFORMAT_H
#define DATAFORMAT_H

/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/*!
  \file DataFormat.h
  This file contains typedefs for the structures that will be put into
  ring buffers for event data.  Event data Ring buffers are filled with items.
  An item has the structure:

\verbatim
 +----------------------------------+
 |  Size of item in bytes (32 bits) |
 +----------------------------------+
 |  32 bit type code of item        |
 +----------------------------------+
 |  body (size - 8 bytes of data    |
 +----------------------------------+
\endverbatim

  Where the 32 bit type code is really a 16 bit type code stored in the lower 16 bits of the 
32 bit word in the native byte ordering of the originating system.  This allows it to serve as 
a byte order indicator, as data type 0 is not legal, and the top bits of the type code must
be zero.

  Further as of nscldaq-11.0, each body has a minimal form of:
  
\verbatim

typedef struct _DataSourceHeader {
  uint32_t   s_size;                 // sizeof(DataSourceHeader) 
  uint64_t   s_timestamp;
  uint32_t   s_sourceId;
  int32_t    s_barrier;
} DataSourceHeader, *pDataSourceHeader;

typedef union _BodyHeader {
    uint32_t         s_empty;             // Contains sizeof(uint32_t)
    DataSourceHeader s_header;          // Has full header. 
} BodyHeader;

struct Body {
    BodyHeader s_bodyhdr;
    uint8_t    s_body[]
};


\endverbatim

Due to the way in which C/C++ compute structure sizes, however the definition
cannot be expressed this way or else all items will appear to have a body header
in them even if they don't Therefore the actual union must have the actual
body in both branches of the union thus:

\verbatim

typdef union Body {
    struct {
        uint32_t s_empty;
        uint8_t  s_body[];
    } u_noHader;
    struct {
        BodyHeader s_bodyHeader;
        uint8_t    s_body[];
    } u_hasHeader;
} Body;

\endverbatim

*/

/*
   Define the type codes for the items.
   Applications can add user specific types if they use values that are at least
   FIRST_USER_TYPE
*/


#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_TIME_H
#include <time.h>
#ifndef __CRT_TIME_H
#define __CRT_TIME_H
#endif
#endif

/*
    11.0 and later define a format item that starts the run.
    so that decoders know what format the ring is in.
*/

static const uint16_t FORMAT_MAJOR  = 12;  /* nscldaq-12. */
static const uint16_t FORMAT_MINOR  =  0;  /* nscldaq-x.0 */

/* state change item type codes: */

static const uint32_t BEGIN_RUN  = 1;
static const uint32_t END_RUN    = 2;
static const uint32_t PAUSE_RUN  = 3;
static const uint32_t RESUME_RUN = 4;

/* Not quite a state change since we don't know anything about what happened. */

static const uint32_t ABNORMAL_ENDRUN = 5;

/*  Documentation item type codes: */

static const uint32_t PACKET_TYPES        = 10;
static const uint32_t MONITORED_VARIABLES = 11;
static const uint32_t RING_FORMAT         = 12; /* Has format major/minor in it. */

/* Scaler data: */

static const uint32_t PERIODIC_SCALERS = 20;


/* Note timestamped nonincremental scalers absorbed into incremental scalers. */

/* Physics events: */

static const uint32_t PHYSICS_EVENT       = 30;
static const uint32_t PHYSICS_EVENT_COUNT = 31;


/* Event builder related items: */

static const uint32_t EVB_FRAGMENT        = 40; /* Event builder fragment. */
static const uint32_t EVB_UNKNOWN_PAYLOAD = 41; /* Evb fragment whose payload isn't a ring item */
static const uint32_t EVB_GLOM_INFO       = 42; /* GLOM Parameters.                            */

/* User defined item codes */

static const uint32_t FIRST_USER_ITEM_CODE = 32768; /* 0x8000 */
                                                      
/* Glom can assign the timestamp policy as follows: */

static const uint16_t GLOM_TIMESTAMP_FIRST   =  0;
static const uint16_t GLOM_TIMESTAMP_LAST    =  1;
static const uint16_t GLOM_TIMESTAMP_AVERAGE = 2;


/* Longest allowed title: */

#ifndef TITLE_MAXSIZE
#define TITLE_MAXSIZE 80
#endif


/* Macro to make packed structs: */



#define PSTRUCT struct __attribute__((__packed__))


/*!  All ring items have common header structures: */

typedef PSTRUCT _RingItemHeader {
  uint32_t     s_size;
  uint32_t     s_type;
} RingItemHeader, *pRingItemHeader;

/*!
   Bodies either have a body item or a longword zero for the body item header
   size field (11.0 and later)
*/

typedef PSTRUCT _BodyHeader {
  uint32_t   s_size;		/* 0 or sizeof(DataSourceHeader) */
  uint64_t   s_timestamp;
  uint32_t   s_sourceId;
  uint32_t   s_barrier;   
} BodyHeader, *pBodyHeader;



/*!
  This  is the most basic item.. a generic item.  It consists only of the
  header and a generic body
*/

// The define below is for compatibility sake.. it can be turned off
// by defining DISABLE_V11_COMPATIBILITY

#ifndef DISABLE_V11_COMPATIBILITY
#define s_mbz s_empty
#endif

typedef PSTRUCT _RingItem {
  RingItemHeader s_header;
  union {
    struct {
      uint32_t s_empty;               /* sizeof(uint32_t) daqdev/NSCLDAQ#1030 */
      uint8_t  s_body[0];
    } u_noBodyHeader;
    struct {
        BodyHeader s_bodyHeader;
        uint8_t s_body[0];
    } u_hasBodyHeader;
  } s_body;
} RingItem, *pRingItem;




/*!
  Run state changes are documented by inserting state change items that have the
  structure shown below.  After 11.0, they may or mey  not have abody header
  as reflected by the fact that they contain a union as shown below:

*/
typedef PSTRUCT _StateChangeItemBody {
  uint32_t        s_runNumber;
  uint32_t        s_timeOffset;
  uint32_t        s_Timestamp;
  uint32_t        s_offsetDivisor;
  uint32_t        s_originalSid;     // 12.0 original source id.
  char            s_title[TITLE_MAXSIZE+1];

} StateChangeItemBody, *pStateChangeItemBody;

typedef PSTRUCT _StateChangeItem  {
    RingItemHeader s_header;
    union {
        struct {
            uint32_t            s_empty;       /* sizeof(uint32_t) daqdev/NSCLDAQ#1030 */
            StateChangeItemBody s_body;
        } u_noBodyHeader;
        struct {
            BodyHeader          s_bodyHeader;
            StateChangeItemBody s_body;
        } u_hasBodyHeader;
    } s_body;
    
} StateChangeItem, *pStateChangeItem;

/*
  ABNORMAL_END items are just empty ring items with the right type:
*/
typedef PSTRUCT _AbnormalEndItem {
    RingItemHeader s_header;
    uint32_t      s_empty;                 /* sizeof(uint32_t) daqdev/NSCLDAQ#1030 */
    
} AbnormalEndItem, *pAbnormalEndItem;

/*!
   Scaler items contain run time counters.  As of 11.0 they may or may  not have
   a body header too:
*/

typedef PSTRUCT _ScalerItemBody {
  uint32_t        s_intervalStartOffset;
  uint32_t        s_intervalEndOffset;
  uint32_t        s_timestamp;
  uint32_t        s_intervalDivisor;  /* 11.0 sub second time intervals */
  uint32_t        s_scalerCount;
  uint32_t        s_isIncremental;    /* 11.0 non-incremental scaler flag */
  uint32_t        s_originalSid;
  uint32_t        s_scalers[0];
} ScalerItemBody, *pScalerItemBody;

typedef PSTRUCT _ScalerItem {
    RingItemHeader s_header;
    union {
        struct {
            uint32_t       s_empty;   /* sizeof(uint32_t) daqdev/NSCLDAQ#1030 */
            ScalerItemBody s_body;
        } u_noBodyHeader;
        struct {
            BodyHeader     s_bodyHeader;
            ScalerItemBody s_body;
        } u_hasBodyHeader;
    } s_body;
} ScalerItem, *pScalerItem;


/*!
  The various documentation Events are just a bunch of null terminated strings that
  are back to back in the body of the ring buffer. item.
*/

typedef PSTRUCT _TextItemBody {
  uint32_t       s_timeOffset;
  uint32_t       s_timestamp;
  uint32_t       s_stringCount;
  uint32_t       s_offsetDivisor;
  uint32_t        s_originalSid;
  char           s_strings[0];
} TextItemBody, *pTextItemBody;


typedef PSTRUCT _TextItem {
    RingItemHeader s_header;
    union {
        struct {
            uint32_t       s_empty;    /* sizeof(uint32_t) daqdev/NSCLDAQ#1030 */
            TextItemBody   s_body;
        } u_noBodyHeader;
        struct {
            BodyHeader     s_bodyHeader;
            TextItemBody   s_body;
        } u_hasBodyHeader;
    } s_body;
} TextItem, *pTextItem;


/*!
  For now a physics event is just a header and a body of uint16_t's.
*/

typedef PSTRUCT _PhysicsEventItem {
    RingItemHeader s_header;
    union {
        struct {
            uint32_t      s_empty;    /* sizeof(uint32_t) daqdev/NSCLDAQ#1030 */
            uint16_t      s_body[0];      /* Aribrtary length body */
        } u_noBodyHeader;
        struct {
            BodyHeader    s_bodyHeader;
            uint16_t      s_body[0];
        } u_hasBodyHeader;
    } s_body;
} PhysicsEventItem, *pPhysicsEventItem;

/*!
   Clients that sample physics events will often
   need to know how many physics events have been produced
   so that they can figure out the sampling fraction.
*/
typedef PSTRUCT __PhysicsEventCountItemBody {
  uint32_t       s_timeOffset;
  uint32_t       s_offsetDivisor;
  uint32_t       s_timestamp;
  uint32_t        s_originalSid;
  uint64_t       s_eventCount;	/* Maybe 4Gevents is too small ;-) */
} PhysicsEventCountItemBody, *pPhysicsEventCountItemBody;

typedef PSTRUCT _PhysicsEventCountItem {
    RingItemHeader   s_header;
    union {
        struct {
            uint32_t             s_empty;      /* sizeof(uint32_t) daqdev/NSCLDAQ#1030 */
            PhysicsEventCountItemBody s_body;
        } u_noBodyHeader;
        struct {
            BodyHeader           s_bodyHeader;
            PhysicsEventCountItemBody s_body;
        } u_hasBodyHeader;
    } s_body;
} PhysicsEventCountItem, *pPhysicsEventCountItem;

/**
 * Event builder stages can put event fragments into the
 * ring buffer for monitoring software:
 * (EVB_FRAGMENT):
 */
typedef PSTRUCT _EventBuilderFragment {
  RingItemHeader s_header;
  BodyHeader     s_bodyHeader;
  uint8_t       s_body[0];	/* Really s_payload bytes of data.. */
} EventBuilderFragment, *pEventBuilderFragment;

/**
 * The ring item format never has an event header.  Just major and minor
 * version numbers:
 */

typedef PSTRUCT _DataFormat {
    RingItemHeader s_header;
    uint32_t       s_empty;          /* sizeof(uint32_t) daqdev/NSCLDAQ#1030 */
    uint16_t       s_majorVersion;     /* FORMAT_MAJOR */
    uint16_t       s_minorVersion;     /* FORMAT_MINOR */
} DataFormat, *pDataFormat;

/**
 *  Information about glom parameters:
 */
typedef PSTRUCT _GlomParameters  {
    RingItemHeader s_header;
    uint32_t       s_empty;              /* sizeof(uint32_t) daqdev/NSCLDAQ#1030 */
    uint64_t       s_coincidenceTicks;
    uint16_t       s_isBuilding;
    uint16_t       s_timestampPolicy;   /* See GLOM_TIMESTAMP_* above */
    
} GlomParameters, *pGlomParameters;
/**
  Below are functions that are available to format ring types.
  Note that all of these return a pointer that must be free(3)'d.
*/
#ifdef __cplusplus
extern "C" {
#endif

  pPhysicsEventItem  formatEventItem(size_t nWords, void* pPayload);    
  pPhysicsEventCountItem formatTriggerCountItem(uint32_t runTime, time_t stamp,
                                                uint64_t triggerCount); 
  pScalerItem         formatScalerItem(unsigned scalerCount, time_t timestamp, 
				      uint32_t btime, uint32_t etime,
                                      void* pCounters);                 
  pScalerItem         formatNonIncrTSScalerItem(unsigned scalerCount, time_t timestamp, 
						       uint32_t btime, uint32_t etime, 
						       uint64_t eventTimestamp, void* pCounters,
						       uint32_t timebaseDivisor);
  pTextItem          formatTextItem(unsigned nStrings, time_t stamp, uint32_t runTime,
				    const char** pStrings, int type);   
  pStateChangeItem   formatStateChange(time_t stamp, uint32_t offset, uint32_t runNumber,
				       const char* pTitle, int type);   
  
  /* Since 11.0 these functions were added: */
  
  pDataFormat           formatDataFormat();
  pGlomParameters       formatGlomParameters(uint64_t coincidenceWindow, int isBuilding,
                                             int timestampPolicy);
  
  pEventBuilderFragment formatEVBFragment(
    uint64_t timestamp, uint32_t sourceId, uint32_t barrier,
    uint32_t payloadSize, const void* pPayload
  );
  pEventBuilderFragment formatEVBFragmentUnknown (
    uint64_t timestamp, uint32_t sourceId, uint32_t barrier,
    uint32_t payloadSize, const void* pPayload
  );
  pPhysicsEventItem formatTimestampedEventItem(
    uint64_t timestamp, uint32_t sourceId, uint32_t barrier,
    uint32_t payloadSize, const void* pPayload    
  );
  pPhysicsEventCountItem formatTimestampedTriggerCountItem (
    uint64_t timestamp, uint32_t sourceId, uint32_t barrier,
    uint32_t runTime, uint32_t offsetDivisor, time_t stamp, uint64_t triggerCount  
  );
  pScalerItem formatTimestampedScalerItem(
    uint64_t timestamp, uint32_t sourceId, uint32_t barrier,
    int isIncremental, uint32_t timeIntervalDivisor, uint32_t timeofday,
    uint32_t btime, uint32_t etime, uint32_t nScalers, void* pCounters
  );
  pTextItem formatTimestampedTextItem(
    uint64_t timestamp, uint32_t sourceId, uint32_t barrier,
    unsigned nStrings, time_t stamp, uint32_t runTime,
    const char**pStrings, int type,
    uint32_t timeIntervalDivisor
  );
  pStateChangeItem formatTimestampedStateChange(
    uint64_t timestamp, uint32_t sourceId, uint32_t barrier,
    time_t stamp, uint32_t offset, uint32_t runNumber, uint32_t offsetDivisor,
    const char* pTitle, int type
  );
  pAbnormalEndItem formatAbnormalEndItem();
  
  int  hasBodyHeader(const RingItem* pItem);
  void* bodyPointer(RingItem* pItem);
  void* bodyHeader(RingItem* pItem);
  
  uint32_t itemSize(const RingItem* pItem);
  uint16_t itemType(const RingItem* pItem);
  int      mustSwap(const RingItem* pItem);

  // Make these visible so that e.g. ring item classes can use
  // proven code.
  
  void* fillRingHeader(pRingItem pItem, uint32_t size, uint32_t type);
  void* fillBodyHeader(
      pRingItem pItem, uint64_t timestamp, uint32_t sourceId,
      uint32_t barrier
  );
  void* fillEventCountBody(
    pRingItem pItem, uint32_t offset, uint32_t divisor, uint32_t unixTime,
    uint64_t count, uint32_t sid
  );
  void* fillScalerBody(
    pRingItem pItem, uint32_t start, uint32_t end, uint32_t divisor,
    uint32_t unixTime, uint32_t count, int incremental, uint32_t* pScalers,
    uint32_t sid
  );
  void* fillTextItemBody(
    pRingItem pItem, uint32_t offset, uint32_t divisor, uint32_t unixTime,
    uint32_t nStrings, const char** ppStrings, int sid
  );
  void* fillStateChangeBody(
    pRingItem pItem, uint32_t run, uint32_t offset, uint32_t divisor,
    uint32_t unixTime, const char* pTitle, int sid
  );
#ifdef __cplusplus
}
#endif

#endif
