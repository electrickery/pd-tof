tof is written by Thomas Ouellet Fredericks around 2009 and probably GPL licensed. 

These are the objects in the tof library:
animate       - abstraction around iemguts/canvasobjectposition
argument      - outputs arguments on bang
arguments     - outputs arguments on bang
breakpoints~  - signal driven graphical envelope table editor / generator (based on ggee/envgen)
breakpoints   - message driven graphical envelope table editor / generator (based on ggee/envgen)
common~       - signal bus object
crossfade~    - multi-channel stereo crossfade
folderpanel   - opens Tk directory selector
from_ascii_code - converts ASCII to messages
gemwin+       - Gem window manager 
getdollarzero - get window id of current or parent window (recursive)
increment     - increments float on bang
iterate       - iterates over range of floats
list_accum    - collects input into a list
list_unfold   - decomposes list into element values and types
listUnfold    - depricated version of list_unfold
menubutton    - drop down menu tool
onlyone       - named uniqueness switch
open_help     - arbitrary help patch finder/launcher
openHelp      - depricated version of open_help
OSCToParam    - message forwarder, abstraction based on oscx/dompOSC and maxlib/remote
param         - multi-function object
path          - path utility
phasorshot~   - phasor~ with features
pix_film      - Gem based movie player
pmenu         - pop-up menu tool
sample_granule~ - pitchshifting/timestretching sample player
sample        - reads wav file into array
sample_packel - edits array start and end
sample_play   - array sample player
sample_record - writes signal streal to array
sample_shift~ - fft based pitchshifting/timestretching sample player
sample_unpack - outputs array meta information and original wav file name
streamMinMax  - outputs minimum and maximum of a stream of floats
to_ascii_code - convert message characters to list of floats


breakpoints and breakpoints~ are derived from ggee/envgen. Common code in w_breakpoints.h, struct in breakpoints~.h.
imagebang is derived from [image] from ggee (the original) and moonlib. 
