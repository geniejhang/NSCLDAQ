#
#   Makefile to build the scalerserver
#   scalerserver is a tcl server with extra commands to support
#   the scaler configuration stuff in the hardware.tcl file.
#
Objects = Server.o serverinstance.o tkAppInit.o TclAuthorizer.o tclAppInit.o \
	CDigitizerDictionary.o CModuleCommand.o CDigitizerModule.o \
	CConfigurationParameter.o CBoolConfigParam.o \
	CIntArrayParam.o CIntConfigParam.o CStringArrayparam.o \
	CStringConfigParam.o CModuleCreator.o \
	CCAENModule.o CCAENV775.o CCAENV775Creator.o \
	CCAENV785.o CCAENV785Creator.o \
	CCAENV792.o CCAENV792Creator.o \
	CCAENV830Module.o CCAENV830Creator.o \
	CConfigurator.o


LINK=g++ -ggdb 
LIBRARIES= -L/usr/opt/daq/current/lib \
	  -L/usr/X11R6/lib \
	-ltclPlus -lException -lLicense -ltk8.5 -ltcl8.5 -lXt -lX11 -lm \
	-Wl,"-rpath=/usr/opt/daq/current/lib"

INCLUDEDIRS=-I. -I/usr/opt/daq/current/include -I/usr/include/tcl8.5 

COMPILE=g++ -c -ggdb $(INCLUDEDIRS)


.cpp.o:
	$(COMPILE) $<

scalerserver: $(Objects)
	$(LINK) -o scalerserver $(Objects) $(LIBRARIES)


clean:
	rm -f $(Objects)
	rm -f scalerserver

depend:
	makedepend -Y. *.cpp

# DO NOT DELETE

CBoolConfigParam.o: CBoolConfigParam.h CConfigurationParameter.h
CCAENModule.o: CCAENModule.h CDigitizerModule.h CConfigurationParameter.h
CCAENModule.o: CIntConfigParam.h CBoolConfigParam.h CIntArrayParam.h
CCAENV775.o: CCAENV775.h CDigitizerModule.h CConfigurationParameter.h
CCAENV775.o: CCAENModule.h CIntConfigParam.h CIntArrayParam.h
CCAENV775.o: CBoolConfigParam.h
CCAENV775Creator.o: CCAENV775Creator.h CModuleCreator.h CCAENV775.h
CCAENV775Creator.o: CDigitizerModule.h CConfigurationParameter.h
CCAENV775Creator.o: CCAENModule.h
CCAENV785.o: CCAENV785.h CDigitizerModule.h CConfigurationParameter.h
CCAENV785.o: CCAENModule.h CIntConfigParam.h CBoolConfigParam.h
CCAENV785.o: CIntArrayParam.h
CCAENV785Creator.o: CCAENV785Creator.h CModuleCreator.h CDigitizerModule.h
CCAENV785Creator.o: CConfigurationParameter.h CCAENV785.h CCAENModule.h
CCAENV792.o: CCAENV792.h CDigitizerModule.h CConfigurationParameter.h
CCAENV792.o: CCAENModule.h CIntConfigParam.h CBoolConfigParam.h
CCAENV792.o: CIntArrayParam.h
CCAENV792Creator.o: CCAENV792Creator.h CModuleCreator.h CCAENV792.h
CCAENV792Creator.o: CDigitizerModule.h CConfigurationParameter.h
CCAENV792Creator.o: CCAENModule.h
CCAENV830Creator.o: CCAENV830Creator.h CModuleCreator.h CCAENV830Module.h
CCAENV830Creator.o: CDigitizerModule.h CConfigurationParameter.h
CCAENV830Module.o: CCAENV830Module.h CDigitizerModule.h
CCAENV830Module.o: CConfigurationParameter.h CBoolConfigParam.h
CCAENV830Module.o: CIntConfigParam.h CIntArrayParam.h
CConfigurationParameter.o: CConfigurationParameter.h
CConfigurator.o: CDigitizerDictionary.h CModuleCommand.h CCAENV775Creator.h
CConfigurator.o: CModuleCreator.h CCAENV785Creator.h CCAENV792Creator.h
CConfigurator.o: CCAENV830Creator.h
CDigitizerDictionary.o: CDigitizerDictionary.h CDigitizerModule.h
CDigitizerDictionary.o: CConfigurationParameter.h
CDigitizerModule.o: CDigitizerModule.h CConfigurationParameter.h
CDigitizerModule.o: CIntConfigParam.h CIntArrayParam.h CBoolConfigParam.h
CDigitizerModule.o: CStringConfigParam.h CStringArrayparam.h
CIntArrayParam.o: CIntArrayParam.h
CIntConfigParam.o: CIntConfigParam.h
CModuleCommand.o: CModuleCommand.h CDigitizerDictionary.h CModuleCreator.h
CModuleCommand.o: CDigitizerModule.h CConfigurationParameter.h
CModuleCreator.o: CModuleCreator.h CDigitizerModule.h
CModuleCreator.o: CConfigurationParameter.h
CStringArrayparam.o: CStringArrayparam.h
CStringConfigParam.o: CStringConfigParam.h
Server.o: server.h serverinstance.h TclAuthorizer.h
serverinstance.o: serverinstance.h server.h
tclAppInit.o: server.h CConfigurator.h
TclAuthorizer.o: TclAuthorizer.h
