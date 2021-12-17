#!/bin/bash

# param 1 pause flag          (true/false)
# param 2 platform            (win/rvl/twl)
# param 3 database file       (edge/edge_twl)
# param 4 debug out           (true/false)
# param 5 check time          (true/false)
# param 6 errors allowed      (true/false)
# param 7 output path postfix (rvl/twl/win-rvl/win-twl)
# param 8 only convert changed files (true/false)

config/convert_with_param.sh true osx rive_steam false false false osx-mac false
