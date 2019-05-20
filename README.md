# ![logo](https://i.imgur.com/mRklRsg.png) MidiPeakVis

This is a simple peak visualiser that outputs to my Novation LaunchKey 49 midi keyboard pad lights.  It listens to the default audio device so all windows sounds will light up the midi keyboard.

![GIF Demo](https://media.giphy.com/media/JsDf0JVW5bi6dA3zWx/giphy.gif)

Video demo here: https://youtu.be/7T36cBQyrSI

* Download the exe from the releases section on github: https://github.com/spacem/midi-peak-vis/releases
* I don't have a code signing certificate so windows will "protect" your PC. Simply click "More info" then "run anyway". All the code is here so feel free to build yourself if you prefer.
* After running it turn on your bottom in-control button to see the lights

This project needs a little polish. If you want to use it and it doesnt work feel free to add an issue to github.

Limitations:
* Only listens to default device (ie. no asio support)
* Although both rows of pads light up you only view a single combined channel
* Only tested on one midi keyboard (my launchkey 49)
* Only works with in-control on

This software sends standard 'note on' midi signals to the keyboard using the mmsystem windows api library. This process is [documented by novation](
https://support.novationmusic.com/hc/en-gb/articles/206857849-How-do-I-create-LED-feedback-on-the-Launchkey-MkII-) so should be safe however I take no responisbility for any damage caused by the software.

I am not affiliated with novation in any way.
