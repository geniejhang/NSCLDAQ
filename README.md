# NSCLDAQ
# For VMUSB controllers

| Location/Area | Board rev | Firmware | Filename | Comments |
| --- | --- | --- | --- | --- |
| S800 | 3a | 61510a09 | vmeusb_062621_atm.bin | Issue with 2sec-periodic readout, no end event signal |
| Rea3 Diag | 3a | 61510a09 | vmeusb_062621_atm.bin | Issue with 2sec-periodic readout, no end event signal |
| HIRA Lab | 2a, 3 | 61510a09 | vmeusb_062621_atm.bin | Issue with 2sec-periodic readout, no end event signal |
| Gamma Lab | 2, 3a | 3d000a00 | ... | Issue with periodic readout affecting VME scalers |
| ARIS | 2a, 3a | 61510a09 | vmeusb_062621_atm.bin | Issue with 2sec-periodic readout, no end event signal |
| DetLab | 2 | 3d000a00 | ... | Issue with periodic readout affecting VME scalers |

### Note

- Controller boards earlier than Rev4 need to use firmware files with `_atm` in the filename standing for "Atmel" (PEROM: Programmable and Erasable ROM).
- Rev4 boards have SPI IC memory manual says requiring `_spi` firmware files.
- Rev4 boards have no rotary switch to flash the firmware, instead, they have switch system on the board.

### Known Issues

- `3d000a00`: Clock for scaler stack has constant offset by 5% resulting in 5% more counts.
- `61510a09` (confirmed rev2a-3a) and `61514a09` (rev4): Saler stack with 2 second period breaks the DAQ.

### About releases:
   Releases will, in general, have several files.  The files labeled 'Source Files' are generated by github.  The files named something like nscldaq-M.m-eee.tar.gz are source tarballs generated by us using:
   
   ```make dist```

Files named nscldaq-M.m-eee-distro.tar.gz are binary tarballs that can be rolled into the /usr/opt filesystem of containerized environment using the FRIB containers for the 'distro'  environment e.g. ```nscldaq-11.3-032-bullseye.tar.gz``` is a binary distribution for the Debian bullseye container image. 
