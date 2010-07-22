/*
    ---------------------------------------------------------------------------

                   --- CAEN SpA - Front End Electronics  ---

    ---------------------------------------------------------------------------

    Name        :   V1495Upgrade.c

    Project     :   V1495Upgrade

    Description :   V1495 Upgrade Software


	  This program writes the configuration file (Altera RBF Format) into the
	  flash memory of the Module V1495. This allows to upgrade the firmware
	  for either the VME_INT and the USER fpga from the VME.
	  The program is based on the CAEN Bridge (V1718 or V2718).
	  The software can be compiled for other VME CPUs, changing the functions in
	  the custom VME functions (VME_Init(),VME_Write(), VME_Read()).
      Comment away CAENVMElib.h include in this case.

    Date        :   March 2006
    Release     :   1.0
    Author      :   C.Tintori

  ---------------------------------------------------------------------------


    ---------------------------------------------------------------------------
*/

/*

  Revision History:
  1.0    CT     First release
  1.1    LC     USER Flash page map changed. Only STD update allowed.
  1.2    RF     Support SBS PCI/VME interfaces (linux specific).

*/



/*
 *  The following VME interfaces can be supported VIA conditional compilations:
 *
 *  Symbol         Device
 *
 *  CAENVMELIB -   CAEN V1718 or V2718
 *  SBSVMELIB  -   SBS 620/618/... with NSCL modified driver.
 */

#include <stdio.h>
#include "console.h"
#ifdef CAENVMELIB
#include "CAENVMElib.h"
#endif

#ifdef SBSVMELIB
#define BT1003

#ifndef LINUX
#error "SBS Interface requires -DLINUX"
#endif

#include <btngpci.h>
#include <btapi.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

/* Some unixes use PAGESIZE instead of _SC_PAGESIZE for mapping granularity. */

#ifndef _SC_PAGESIZE
#define _SC_PAGESIZE PAGESIZE
#endif

/* Unless they've done something different, SBS max error message size is not 
  given in the headers:
*/
#ifndef BTERRORLENGTH
#define BTERRORLENGTH 100	/* Should be big enough */
#endif

#endif


// Global Variables
#ifdef CAENVMELIB
long  BHandle;                 // V1718 or V2718 handle
#endif

#ifdef SBSVMELIB
bt_desc_t BHandle;		/* SBS controller */

/*
 * The SBS interface works by mapping memory on the VME space to the
 * process virtual address space.
 * It can be inefficient to do this map once for every cycle.
 * therefore, each time we do a map, we'll map an entire
 * unix page, cache the map and then, on subsequent 
 * references, use the same map as before.
 * The cache is just a linked list with the structure shown below:
 *
 */

typedef struct _mapCacheEntry {
  struct _mapCacheEntry*  s_pNext;	/* Pointer to next cache entry or NULL  if end.    */
  uint32_t                s_VMEBase;    /* base of the map entry in VME address space      */
  uint16_t*               s_pPVABase;   /* Base of the map entry in Process VA             */
  size_t                  s_nSize;      /* Number of bytes in th ememory map               */
} MapCacheEntry, *pMapCacheEntry;

pMapCacheEntry pCache = NULL;	     /* head of the cache list.  */

#endif

unsigned long  Sel_Flash;      // VME Address of the FLASH SELECTION REGISTER
unsigned long  RW_Flash;       // VME Address of the FLASH Read/Write REGISTER


// Parameters for the access to the Flash Memory
#define VME_FIRST_PAGE_STD    768    // first page of the image STD
#define VME_FIRST_PAGE_BCK    1408   // first page of the image BCK
#define USR_FIRST_PAGE_STD    48     // first page of the image STD
#define USR_FIRST_PAGE_BCK    1048   // first page of the image BCK
#define PAGE_SIZE         264    // Number of bytes per page in the target flash

// Flash Opcodes
#define MAIN_MEM_PAGE_READ          0x00D2
#define MAIN_MEM_PAGE_PROG_TH_BUF1  0x0082




// #############################################################################
// Put here your function for the VME access....

#ifdef SBSVMELIB
/*
  Reports an error from the SBS API on stderr
*/
VME_SBSError(bt_error_t msg, const char* pDoing) 
{
  size_t messageSize = BTERRORLENGTH + strlen(pDoing) + 1;
  char*  pMessage    = (char*)malloc(messageSize);
  if (!pMessage) {
    perror("SBS API Error but could not allocate error message string");
  }
  else {
    bt_strerror(BHandle, msg, pDoing, pMessage, messageSize);
    fprintf(stderr, "%s\n", pMessage);
    free((char*)pMessage);
  }
}
#endif

// *****************************************************************************
// VME_Init
// ----------------------
// Initialize the VME; Return 0 on success or -1 in case of error.
// *****************************************************************************
int VME_Init(void)
{
#ifdef CAENVMELIB
  /* initialize the vme */
  if( CAENVME_Init(cvV2718, 0, 0, &BHandle) == cvSuccess )
      return 0;
  else if( CAENVME_Init(cvV1718, 0, 0, &BHandle) == cvSuccess )
      return 0;
  else
      return -1;
#endif

#ifdef SBSVMELIB
  /* We want to do A32 operations so get/open BT_DEV_A32 */
  /* For now only support VME crate number 0             */

  char        deviceName[BT_MAX_DEV_NAME];
  const char* pDeviceName = bt_gen_name(0, BT_DEV_A32, deviceName, sizeof(deviceName));
  bt_error_t msg  = bt_open(&BHandle, deviceName, BT_RDWR);
  if (msg != BT_SUCCESS) {
    VME_SBSError(msg, "Could not open SBS device: ");
    return -1;
  }
  return 0;
#endif


}

#ifdef SBSVMELIB

/*******************************************************************************
 ** Helper function to locate, or create the appropriate cache entry
 ** for an address.  
 **
 ** Parameters:
 **    nAddress  VME address we want a map for;
 ** Returns:
 **    Pointer to the appropriate map cache entry for success.
 **    NULL If failure.
 */
pMapCacheEntry
SBS_GetCacheEntry(uint32_t nAddress)
{
  /* If we can find a cache entry that matches return it */

  pMapCacheEntry pItem = pCache;
  while (pItem) {
    if ((nAddress >=  pItem->s_VMEBase) && 
	(nAddress <  (pItem->s_VMEBase + pItem->s_nSize))) {
      return pItem;
    }
    else {
      pItem = pItem->s_pNext;
    }
  }
  
  /* Otherwise, create a new one, do the map enter it and be done.
     We are going to create a map that is an entire Linux page
     starting at the beginning of the page that contains nAddress.
     NOTE: The page rounding algorithm assumes page sizes are some
           power of 2.
  */
  
  pItem = (pMapCacheEntry)malloc(sizeof(MapCacheEntry));
  if (!pItem) {
    perror("Could not create map cache entry :-(");
    return (pMapCacheEntry)NULL;
  }

  size_t        pagesize     = (size_t)sysconf(_SC_PAGESIZE); /* sysconf gives ints. */
  size_t        pageMask     = ~(pagesize-1); 
  bt_devaddr_t  base         = nAddress & pageMask;

  bt_error_t msg = bt_mmap(BHandle, (void**)&(pItem->s_pPVABase), 
			   base, pagesize, 
			   BT_RDWR, BT_SWAP_NONE);
  if (msg != BT_SUCCESS) {
    VME_SBSError(msg, "Failed to creating new VME Map window: ");
    return (pMapCacheEntry)NULL;
  }
  
  pItem->s_VMEBase = base;
  pItem->s_nSize = pagesize;
  pItem->s_pNext = pCache;
  pCache         = pItem;

  return pItem;
}

/*
** Given a map cache entry and a VME addresss, create the pointer to dereference
** to fetch/set the VME location.
*/
uint16_t* 
SBS_CreatePointer(pMapCacheEntry pEntry, uint32_t nAddress)
{

  uint32_t wordOffset = (nAddress - pEntry->s_VMEBase)/sizeof(uint16_t);
  return   pEntry->s_pPVABase + wordOffset;

}

#endif

// *****************************************************************************
// VME_Write_D16
// ----------------------
// VME Write Cycle: it writes Data at Address (D16)
// Return 0 on success or -1 in case of error.
// *****************************************************************************
int VME_Write_D16(unsigned long Address, unsigned short Data)
{

#ifdef CAENVMELIB
  if(CAENVME_WriteCycle(BHandle, Address, &Data, cvA32_U_DATA, cvD16) != cvSuccess)
      return -1;
  else
      return 0;
#endif

#ifdef SBSVMELIB
  pMapCacheEntry pItem = SBS_GetCacheEntry((uint32_t)Address);
  if (!pItem) return -1;

  uint16_t* p = SBS_CreatePointer(pItem, Address);
  *p          = Data;
  return 0;
#endif

}

// *****************************************************************************
// VME_Read_D16
// ----------------------
// VME Read Cycle: it reads at Address and put the value in Data (D16)
// Return 0 on success or -1 in case of error.
// *****************************************************************************
int VME_Read_D16(unsigned long Address, unsigned short *Data)
{
#ifdef CAENVMELIB
  if(CAENVME_ReadCycle(BHandle, Address, Data, cvA32_U_DATA, cvD16) != cvSuccess)
      return -1;
  else
      return 0;
#endif

#ifdef SBSVMELIB
  pMapCacheEntry pItem = SBS_GetCacheEntry((uint32_t)Address);
  if (!pItem) return -1;

  uint16_t* p = SBS_CreatePointer(pItem, Address);
  *Data       = *p;
  return 0;

#endif

}

// end of the custom VME function
// #############################################################################





// *****************************************************************************
// write_flash_page
// *****************************************************************************
int write_flash_page(unsigned char *data, int pagenum)
{
  int i, flash_addr;
  unsigned char addr0,addr1,addr2;
  int res = 0;

  flash_addr = pagenum << 9;
  addr0 = (unsigned char)flash_addr;
  addr1 = (unsigned char)(flash_addr>>8);
  addr2 = (unsigned char)(flash_addr>>16);

  // enable flash (NCS = 0)
  res |= VME_Write_D16(Sel_Flash, 0);

  // write opcode
  res |= VME_Write_D16(RW_Flash, MAIN_MEM_PAGE_PROG_TH_BUF1);
  // write address
  res |= VME_Write_D16(RW_Flash, addr2);
  res |= VME_Write_D16(RW_Flash, addr1);
  res |= VME_Write_D16(RW_Flash, addr0);
  // write flash page
  for (i=0; i<PAGE_SIZE; i++)
      res |= VME_Write_D16(RW_Flash, data[i]);

  // disable flash (NCS = 1)
  res |= VME_Write_D16(Sel_Flash, 1);

  // wait 20ms
  delay(20);
  return res;
}

// *****************************************************************************
// read_flash_page
// *****************************************************************************
int read_flash_page(unsigned char *data, int pagenum)
{
  int i, flash_addr;
  unsigned short data16;
  unsigned char addr0,addr1,addr2;
  int res = 0;

  flash_addr = pagenum << 9;
  addr0 = (unsigned char)flash_addr;
  addr1 = (unsigned char)(flash_addr>>8);
  addr2 = (unsigned char)(flash_addr>>16);

  // enable flash (NCS = 0)
  res |= VME_Write_D16(Sel_Flash, 0);

  // write opcode
  res |= VME_Write_D16(RW_Flash, MAIN_MEM_PAGE_READ);
  // write address
  res |= VME_Write_D16(RW_Flash, addr2);
  res |= VME_Write_D16(RW_Flash, addr1);
  res |= VME_Write_D16(RW_Flash, addr0);
  // additional don't care bytes
  for (i=0; i<4; i++)
      res |= VME_Write_D16(RW_Flash, 0);

  // read flash page
  for (i=0; i<PAGE_SIZE; i++) {
      res |= VME_Read_D16(RW_Flash, &data16);
      data[i] = (unsigned char)data16;
  }

  // disable flash (NCS = 1)
  res |= VME_Write_D16(Sel_Flash, 1);
  return res;
}




/*******************************************************************************/
/* MAIN
/*******************************************************************************/
int main(int argc,char *argv[])
{
  int finish,i;
  int bp, bcnt, image, pa;
  char filename [1000];
  int fw2update; // Firmware to update selector = 0 => USER, 1 => VME
  char c;
  unsigned char pdw[PAGE_SIZE], pdr[PAGE_SIZE];
  unsigned long vboard_base_address;
  FILE *cf;


  /* initialize the console and clear the screen */
  con_init();
  clrscr();

  con_printf("\n");
  con_printf("********************************************************\n");
  con_printf("* CAEN SpA - Front-End Division                        *\n");
  con_printf("* ---------------------------------------------------- *\n");
  con_printf("* Firmware Upgrade of the V1495                        *\n");
  con_printf("* Version 1.1 (27/07/06)                               *\n");
  con_printf("********************************************************\n\n");

  // Check input params
  if (argc < 3)  {
      con_printf("\n\n");
      con_printf("Syntax: V1495Upgrade FileName BaseAdd [TargetFPGA] [image]\n");
      con_printf("  where: \n");
      con_printf("  FileName is the RBF file \n");
      con_printf("  BaseAdd is the Base Address (Hex 32 bit) of the V1495\n");
      con_printf("  TargetFPGA 'user' (default) or 'vme'\n");
      con_printf("  image is '/standard' (default) or '/backup'\n");
	  con_getch();
      goto exit_prog;
  }

  strcpy(filename,argv[1]);
  sscanf(argv[2], "%x", &vboard_base_address);

  image     = 0;
  fw2update = 0;
  for (i=3; i<argc; i++) {
      if ( strcmp(argv[i],"/backup") == 0 )
          image = 1;
      else if ( strcmp(argv[i],"/standard") == 0 )
          image = 0;
      else if ( strcmp(argv[i],"vme") == 0 )
          fw2update = 1;
      else if ( strcmp(argv[i],"user") == 0 )
          fw2update = 0;
      else {
          con_printf("\n\nBad Parameter %s\n", argv[i]);
		  con_getch();
          goto exit_prog;
      }
  }


  // open the configuration file
  cf = fopen(filename,"rb");
  if(cf==NULL) {
      con_printf("\n\nCan't open file %s\n",filename);
	  con_getch();
      goto exit_prog;
  }


  if (fw2update == 0) {                                 // FPGA "User"
      Sel_Flash = vboard_base_address + 0x8012;
      RW_Flash  = vboard_base_address + 0x8014;
      if (image == 0) {
        pa = USR_FIRST_PAGE_STD;
	  }
      else if (image == 1) {
        con_printf("Backup image not supported for USER FPGA. Press a key to exit...\n");
	    con_getch();
        goto exit_prog;
	  }
      else {
        con_printf("Bad Image.\n");
	    con_getch();
        goto exit_prog;
	  }

      con_printf("Updating firmware of the FPGA USER with the file %s\n", filename);
  }
  else if (fw2update == 1) {                            // FPGA "VME_Interface"
      Sel_Flash = vboard_base_address + 0x800E;
      RW_Flash  = vboard_base_address + 0x8010;
      if (image == 0) {
        con_printf("Writing STD page of the VME FPGA\n");
        pa = VME_FIRST_PAGE_STD;
	  }
      else if (image == 1) {
        con_printf("Writing BCK page of the VME FPGA\n");
        pa = VME_FIRST_PAGE_BCK;
	  }
      else {
        con_printf("Bad Image.\n");
	    con_getch();
        goto exit_prog;
	  }

      con_printf("Updating firmware of the FPGA VME with the file %s\n", filename);
  }
  else {
      con_printf("Bad FPGA Target.\n");
	  con_getch();
      goto exit_prog;
  }



   /* initialize the vme */
  con_printf("Opening the VME controller...\n");
  if (VME_Init() < 0) {
      con_printf("Cannot open the VME controller!\n");
	  con_getch();
      goto exit_prog;
  }
  con_printf("VME controller is connected.\n");


  bcnt=0;            // byte counter
  bp=0;              // byte pointer in the page
  finish=0;

    // while loop
  while(!finish)   {
      c=(unsigned char)fgetc(cf);  // read one byte from file

	  // mirror byte (lsb becomes msb)
      pdw[bp]=0;
      for(i=0; i<8; i++)
          if(c & (1<<i))
              pdw[bp] = pdw[bp] | (0x80>>i);
      bp++;
      bcnt++;
      if(feof(cf))
          finish=1;

      // write and verify a page
      if((bp == PAGE_SIZE) || finish) {
          con_printf(".");
          // Write Page
          if (write_flash_page(pdw, pa) < 0) {
              con_printf("\n\nError Accessing the board\n");
			  con_getch();
              goto exit_prog;
          }
          // Read Page
          if (read_flash_page(pdr, pa) < 0) {
              con_printf("\n\nError Accessing the board\n");
			  con_getch();
              goto exit_prog;
          }
          // Verify Page
          for(i=0; i<PAGE_SIZE; i++)  {
              if(pdr[i] != pdw[i])  {
                  con_printf("\n\nFlash writing failure (byte %d of page %d)!",i,pa);
                  con_printf("\nFirmware not loaded!");
				  con_getch();
                  goto exit_prog;
              }
		  }
          bp=0;
          pa++;
        }
    }  // end of while loop

    fclose(cf);
    con_printf("\nFirmware loaded successfully. Written %d bytes\n", bcnt);
    con_printf("Write 1 at address 0x8016 to reload updated version of the User FPGA\n");

exit_prog:
    con_end();
    return 0;
}


