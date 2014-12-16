
#ifndef WienerMDGG16Register_H
#define WienerMDGG16Register_H

#include <stdint.h>

namespace WienerMDGG16 
{
  namespace Regs {
    const uint32_t FirmwareID    = 0x0000;
    const uint32_t Global        = 0x0004;
    const uint32_t Auxiliary     = 0x0008;
    const uint32_t ECL_Output    = 0x000c;
    const uint32_t Logical_OR_AB     = 0x00b8;
    const uint32_t Logical_OR_CD     = 0x00bc;
  }

  namespace ECL_Output {
    const uint32_t ECL9_Offset  = 0;
    const uint32_t ECL10_Offset = 4;
    const uint32_t ECL11_Offset = 8;
    const uint32_t ECL12_Offset = 12;
    const uint32_t ECL13_Offset = 16;
    const uint32_t ECL14_Offset = 20;
    const uint32_t ECL15_Offset = 24;
    const uint32_t ECL16_Offset = 28;

    const uint32_t Disable    = 0;
    const uint32_t DGG        = 1;
    const uint32_t ECL_Input  = 2;
    const uint32_t Logical_OR = 3;
  }

  namespace Logical_OR {
    const uint32_t A_Offset = 0;
    const uint32_t B_Offset = 16;
    const uint32_t C_Offset = 0;
    const uint32_t D_Offset = 16;
  }

}

#endif

