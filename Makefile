# Makefile for Makefile.pdlibbuilder. 
# 

lib.name = tof

argument.class.sources       = src/argument.c
arguments.class.sources      = src/arguments.c
breakpoints~.class.sources   = src/breakpoints~.c
breakpoints.class.sources    = src/breakpoints.c
common~.class.sources        = src/common~.c
crossfade~.class.sources     = src/crossfade~.c
folderpanel.class.sources    = src/folderpanel.c
from_ascii_code.class.sources = src/from_ascii_code.c
getdollarzero.class.sources  = src/getdollarzero.c
imagebang.class.sources      = src/imagebang.c
increment.class.sources      = src/increment.c
iterate.class.sources        = src/iterate.c
list_accum.class.sources     = src/list_accum.c
list_unfold.class.sources    = src/list_unfold.c
listUnfold.class.sources     = src/listUnfold.c
menubutton.class.sources     = src/menubutton.c
onlyone.class.sources        = src/onlyone.c
open_help.class.sources      = src/open_help.c
openHelp.class.sources       = src/openHelp.c
param.class.sources          = src/param.c
path.class.sources           = src/path.c
phasorshot~.class.sources    = src/phasorshot~.c
pmenu.class.sources          = src/pmenu.c
streamMinMax.class.sources   = src/streamMinMax.c
to_ascii_code.class.sources  = src/to_ascii_code.c

datafiles = \
LICENSE.txt \
README.txt \
$(wildcard tof/*-help.pd) \
$(wildcard tof/*.gif) \
tof/animate.pd \
tof/gemwin+.pd \
tof/OSCToParam.pd \
tof/pix_film+.pd \
tof/sample_granule~.pd \
tof/sample_packel.pd \
tof/sample.pd \
tof/sample_play~.pd \
tof/sample_record~.pd \
tof/sample_shifft~.pd \
tof/sample_unpack.pd \
tof/test-argument.pd \
tof/test-arguments-all.pd \
tof/test-arguments-comma.pd \
tof/test-arguments-token.pd \
tof/test-param-2.pd \
tof/test-param.pd \


externalsdir = ..
# include Makefile.pdlibbuilder from parent or current directory 
-include $(externalsdir)/Makefile.pdlibbuilder 

ifndef Makefile.pdlibbuilder 
include Makefile.pdlibbuilder 
endif
 
