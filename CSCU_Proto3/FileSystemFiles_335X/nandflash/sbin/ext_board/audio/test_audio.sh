#!/bin/sh

echo "Testing Audio"
echo

playtime=11
song=/home/root/media/stereo_test.wav

for vol in 100 0 75
do
    echo "Set volume to $vol%"
    amixer cset name='PCM Playback Volume' $vol%,$vol% > /dev/null 
    aplay -d $playtime $song #>/dev/null
    sleep 1
    echo 
done

echo "Done!"
exit 0

