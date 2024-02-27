SUBDIRS = exception configuration booter format ddasdumper readout sorter readoutdriver pixieplugin simple_setups qtscope

if BUILD_DDAS_DOCS
SUBDIRS += docs
endif

# 12.1 release will fully migrate to some XIA API 4.x version and will no
# longer support lecacy API. A number of programs are no longer built
# including those to convert binary settings files to XML format (and back)
# and the suite of unittests which faked a version of the legacy API to test
# against. Note that this simplifies the DDAS setup in configure.ac.

# readout-extclock - external clock handling is subsumed into readout.
# traiter - legacy trace iterator class. Not installed for now.
