SUBDIRS = exception configuration booter format ddasdumper readout pixieplugin qtscope

if BUILD_DDAS_DOCS
SUBDIRS += docs
endif

# 12.1 release will fully migrate to some XIA API 4.x version and many features
# that are available only for legacy API systems will be deprecated. A number
# of programs are no longer built including those to convert binary settings
# files to XML format (and back) and the suite of unittests which faked a
# version of the legacy API to test against.

# readout-extclock - external clock handling is subsumed into readout.
# traiter - legacy trace iterator class. Not installed for now.
