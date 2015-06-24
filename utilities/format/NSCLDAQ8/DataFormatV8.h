#ifndef DATAFORMATV8_H
#define DATAFORMATV8_H

namespace DAQ
{
  namespace V8
  {
    enum BufferVersion { StandardVsn=5, JumboVsn=6 };

    enum BufferTypes {
      VOID         = 0,
      DATABF       = 1,
      SCALERBF     = 2,
      SNAPSCBF     = 3,
      STATEVARDB   = 4,
      RUNVARBF     = 5,
      PKTDOCBF     = 6,
      BEGRUNBF     = 11,
      ENDRUNBF     = 12,
      PAUSEBF      = 13,
      RESUMEBF     = 14,
      PARAMDESCRIP = 30,
      GENERIC      = 40
    };

    enum ByteOrderMark {
      BOM16 = 0x0102,
      BOM32 = 0x01020304
    };
  } // end of V8
} // end of DAQ

#endif // DATAFORMATV8_H
