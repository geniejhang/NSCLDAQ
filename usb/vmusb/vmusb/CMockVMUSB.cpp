
#include "CMockVMUSB.h"
#include <sstream>
#include <iomanip>

using namespace std;

CMockVMUSB::CMockVMUSB()
  : CVMUSB(), 
    m_opRecord(), 
    m_registers(),
    m_registerNames()
{
  setUpRegisterMap();
  setUpRegisterNameMap();
}

int CMockVMUSB::executeList(CVMUSBReadoutList& list, void* pReadBuffer, size_t readBufferSize, size_t* bytesRead)
{
  vector<uint32_t> stack = list.get();
  uint32_t stackLength = stack.size();

  // Simply copy the stack entirely into the operation list
  ostringstream command;
  m_opRecord.push_back("executeList::begin"); // bookend

  for (int stackIndex=0;  stackIndex<stackLength; ++stackIndex) {
    command.str(""); command.clear(); // clear string and stream status bits
    command << dec << stackIndex << ":" << hex << stack.at(stackIndex); 
    m_opRecord.push_back(command.str());
  }

  m_opRecord.push_back("executeList::end"); //bookend

  // We don't actually read anything so tell the caller that no data will be
  // returned. At the same time, there was no error so return a status of 0.
  bytesRead=0;
  return 0;
}

int CMockVMUSB::loadList(uint8_t listNumber, CVMUSBReadoutList& list, off_t listOffset)
{
  vector<uint32_t> stack = list.get();
  uint32_t stackLength = stack.size();

  m_opRecord.push_back("loadList::begin"); // bookend

  ostringstream command;
  command << "listnumber:" << listNumber;
  command.str(""); command.clear(); // clear string and stream status bits
  command << "offset:" << listOffset;
  m_opRecord.push_back(command.str()); // bookend

  for (int stackIndex=0;  stackIndex<stackLength; ++stackIndex) {
    command.str(""); command.clear(); // clear string and stream status bits
    command << dec << stackIndex << ":" << hex << stack.at(stackLength); 
    m_opRecord.push_back(command.str());
  }

  m_opRecord.push_back("loadList::end"); //bookend

  return 0;
}

int CMockVMUSB::usbRead(void* data, size_t bufferSize, size_t* transferCount, int timeout)
{
  m_opRecord.push_back("usbRead:begin"); 

  ostringstream command;
  command << "buffersize:" << bufferSize;
  m_opRecord.push_back(command.str()); 

  command.str(""); command.clear();
  command << "timeout:" << timeout;
  m_opRecord.push_back(command.str()); 

  transferCount = 0;

  if (bufferSize!=0) {
    uint32_t* buffer = reinterpret_cast<uint32_t*>(data);
    for (int i=0; i<bufferSize/sizeof(uint32_t); ++i) {
      buffer[i] = i;
      transferCount += 4;
    }
  }

  m_opRecord.push_back("usbRead:end");
  return 0;
}

uint32_t CMockVMUSB::readRegister(uint32_t reg)
{
  uint32_t value = m_registers[reg];
  // add entry to log that we did this operation

  string actionString = "read" + m_registerNames[reg];
  recordOperation(actionString, value);

  return value;
}

void CMockVMUSB::writeRegister(uint32_t reg, uint32_t value)
{
  m_registers[reg] = value;  // add entry to log that we did this operation

  string actionString = "write" + m_registerNames[reg];
  recordOperation(actionString, value);
}

void CMockVMUSB::setUpRegisterMap()
{
  m_registers[0x0] = 0xffffffff; // firmwareID
  m_registers[0x1] = 0; // action register 
  m_registers[0x4] = 0; // global mode
  m_registers[0x8] = 0; // daq settings
  m_registers[0xC] = 0; // user led
  m_registers[0x10] = 0; // user devices source selecor
  m_registers[0x14] = 0; // dgg_A settings
  m_registers[0x18] = 0; // dgg_B settings
  m_registers[0x1C] = 0; // scaler_A data
  m_registers[0x20] = 0; // scaler_B data
  m_registers[0x24] = 0; // events per buffer
  m_registers[0x28] = 0; // IRQ vectors 1&2
  m_registers[0x2C] = 0; // IRQ vectors 3&4
  m_registers[0x30] = 0; // IRQ vectors 5&6
  m_registers[0x34] = 0; // IRQ vectors 7&8
  m_registers[0x38] = 0; // extended dgg_A/B settings
  m_registers[0x3C] = 0; // USB bulk transfer

}

void CMockVMUSB::setUpRegisterNameMap()
{
  m_registerNames[0x0] = "FirmwareID";
  m_registerNames[0x1] = "ActionRegister"; // action register 
  m_registerNames[0x4] = "GlobalMode"; // global mode
  m_registerNames[0x8] = "DAQSettings"; // daq settings
  m_registerNames[0xC] = "LEDSource"; // user led
  m_registerNames[0x10] = "DeviceSource"; // user devices source selecor
  m_registerNames[0x14] = "DGG_A"; // dgg_A settings
  m_registerNames[0x18] = "DGG_B"; // dgg_B settings
  m_registerNames[0x1C] = "ScalerA"; // scaler_A data
  m_registerNames[0x20] = "ScalerB"; // scaler_B data
  m_registerNames[0x24] = "EventsPerBuffer"; // events per buffer
  m_registerNames[0x28] = "IRQ12"; // IRQ vectors 1&2
  m_registerNames[0x2C] = "IRQ34"; // IRQ vectors 3&4
  m_registerNames[0x30] = "IRQ56"; // IRQ vectors 5&6
  m_registerNames[0x34] = "IRQ78"; // IRQ vectors 7&8
  m_registerNames[0x38] = "DGG_Extended"; // extended dgg_A/B settings
  m_registerNames[0x3C] = "BulkXferSetup"; // USB bulk transfer

}

template<class T>
void CMockVMUSB::recordVMEOperation(std::string opname, uint32_t address, uint8_t addrMod, T data)
{

    std::stringstream logentry;
    logentry << opname << "(0x" 
             << std::hex << std::setfill('0') << std::setw(8) 
             << address;
    logentry << "," << std::setw(2) << uint16_t(addrMod);
    logentry << "," << std::setw(8) << data;
    logentry << ")";

    m_opRecord.push_back(logentry.str());
}

template<class T>
void CMockVMUSB::recordOperation(std::string opname, T data)
{

    std::stringstream logentry;
    logentry << opname << "(0x" 
             << std::hex << std::setfill('0') << std::setw(8) 
             << data << ")";

    m_opRecord.push_back(logentry.str());
}
