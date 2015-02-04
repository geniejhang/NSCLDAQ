INSTDIR=/usr/opt/daq/10.2
#  The line above is written by the installation procedure.
#
#  Makefile to build a readout program given the stuff in the standard
#  skeleton.
#
include $(INSTDIR)/etc/Readout_Makefile.include

#   If you require any addtional compilation flags for c++ compilations,
#   define them here:
#
USERCXXFLAGS=-DCESCAMAC -I/usr/include/tcl8.5 -I.
#
#   If you require any additional C compilation flags, define them below.
#   Note: by default this is the same as the C++ flags:
#
#  If you are using CAMAC, you will need to add:
#
#      -DCESCAMAC     (ces branch highway driver).
# or
#      -DVC32CAMAC    (wiener camac).
# To select your camac interface.

USERCCFLAGS=$(USERCXXFLAGS)

#
#   If you require any additional linker flags, define them below:
#
USERLDFLAGS=-lTclPlus -ltk8.5  -ltcl8.5 -ldl

#
#  If you want to use a VME trigger (CAENV262), uncomment the following
#  line (else trigger comes from INT2 of CBD8210)
#
VMETRIGGER=-DVME=1

#
#   Add any objects you need built to the definition below.  Note that
#  in general, the Makefile knows how to build c++ and c source files
#  into objects so you won't need to define build rules for your objects.

PROGOBJS=ReadoutMain.o skeleton.o \
CBoolConfigParam.o \
CCAENV775.o \
CCAENV775Creator.o \
CCAENV785.o \
CCAENV785Creator.o \
CConfigurationParameter.o \
CDigitizerDictionary.o \
CDigitizerModule.o \
CIntArrayParam.o \
CIntConfigParam.o \
CStringArrayparam.o \
CStringConfigParam.o \
CModuleCommand.o \
CModuleCreator.o \
CReadOrder.o \
CScalerBank.o \
CScalerModule.o \
CNullBusy.o  CCAENModule.o  \
CCAENV792.o CCAENV792Creator.o \
CCAENV830.o CCAENV830Module.o CCAENV830Creator.o


###################

Program: $(PROGOBJS)
	$(CXXLD)  -o Readout $(PROGOBJS) $(USERLDFLAGS) $(LDFLAGS)


clean: 
	rm -f $(PROGOBJS)
	rm -f Readout

depend:
	makedepend $(USERCXXFLAGS) *.cpp

help:
	echo "make          - Builds the readout program."
	echo "make clean    - removes any objects and executables from make"
	echo "make depend   - Adds header dependencies to the Makefile"
