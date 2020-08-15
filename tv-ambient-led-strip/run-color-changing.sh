#!/bin/bash

# Run Youtube Color Changing Mood Light on fullscreen. If you use Vimium add
# exception rule for key "f".

# Go to directory of this script
cd $(dirname $(readlink -f $0))

# Launch video on fullscreen and LEDs
google-chrome --app="https://www.youtube.com/watch?v=amRDYrLHwpI" && sleep 2 && xdotool key f && ./run.sh
