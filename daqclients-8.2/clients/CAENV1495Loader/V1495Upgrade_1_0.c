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

*/



#include <stdio.h>
#include "console.h"
#include "CAENVMElib.h"

// Global Variables
long  BHandle;                 // V1718 or V2718 handle
unsigned long  Sel_Flash;      // VME Address of the FLASH SELECTION REGISTER
unsigned long  RW_Flash;       // VME Address of the FLASH Read/Write REGISTER


// Parameters for the access to the Flash Memory
#define FIRST_PAGE_STD    768    // first page of the image STD
#define FIRST_PAGE_BCK    1408   // first page of the image BCK
#define PAGE_SIZE         264    // Number of bytes per page in the target flash

// Flash Opcodes
#define MAIN_MEM_PAGE_READ          0x00D2
#define MAIN_MEM_PAGE_PROG_TH_BUF1  0x0082




// #############################################################################
// Put here your function for the VME access....

// *****************************************************************************
// VME_Init
// ----------------------
// Initialize the VME; Return 0 on success or -1 in case of error.
// *****************************************************************************
int VME_Init(void)
{
  /* initialize the vme */
  if( CAENVME_Init(cvV2718, 0, 0, &BHandle) == cvSuccess )
      return 0;
  else if( CAENVME_Init(cvV1718, 0, 0, &BHandle) == cvSuccess )
      return 0;
  else
      return -1;
}

// *****************************************************************************
// VME_Write_D16
// ----------------------
// VME Write Cycle: it writes Data at Address (D16)
// Return 0 on success or -1 in case of error.
// *****************************************************************************
int VME_Write_D16(unsigned long Address, unsigned short Data)
{
  if(CAENVME_WriteCycle(BHandle, Address, &Data, cvA32_U_DATA, cvD16) != cvSuccess)
      return -1;
  else
      return 0;
}

// *****************************************************************************
// VME_Read_D16
// ----------------------
// VME Read Cycle: it reads at Address and put the value in Data (D16)
// Return 0 on success or -1 in case of error.
// *****************************************************************************
int VME_Read_D16(unsigned long Address, unsigned short *Data)
{
  if(CAENVME_ReadCycle(BHandle, Address, Data, cvA32_U_DATA, cvD16) != cvSuccess)
      return -1;
  else
      return 0;
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
void main(int argc,char *argv[])
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
  con_printf("* Version 1.0 (07/02/06)                               *\n");
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
      con_printf("Updating firmware of the FPGA USER with the file %s\n", filename);
  }
  else if (fw2update == 1) {                            // FPGA "VME_Interface"
      Sel_Flash = vboard_base_address + 0x800E;
      RW_Flash  = vboard_base_address + 0x8010;
      con_printf("Updating firmware of the FPGA VME with the file %s\n", filename);
  }
  else {
      con_printf("Bad FPGA Target.\n");
	  con_getch();
      goto exit_prog;
  }

  if (image == 0) {
      con_printf("Writing copy STD of the firmware\n");
      pa = FIRST_PAGE_STD;
  }
  else if (image == 1) {
      con_printf("Writing copy BCK of the firmware\n");
      pa = FIRST_PAGE_BCK;
  }
  else {
      con_printf("Bad Image.\n");
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
    con_printf("Write 1 or 0 at address 0x8016 to load standard or backup version of the User FPGA\n");

exit_prog:
    con_end();
}


