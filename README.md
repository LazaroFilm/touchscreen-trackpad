# touchscreen-trackpad
A trackpad using WaveShare RP2040-Touch-LCD-1.28 module

![Alt text](https://raw.githubusercontent.com/LazaroFilm/touchscreen-trackpad/main/Touchscreen_Picture.jpg)

still a work in progress but heres what it does: 
- scrollwheel: start your touch on the esge of the screen (wuthin a 10 pixel border whenre the white line is) then spin the dial with your finger to scroll. Scrill is still jerky, im searching for an option to amooth it out. 
- coasting: flick yoir finger at the end of a mouse move to make it coast a bit. this helps crossing the screen faster on this tiny screen. 
- clicking: tap your finger once to click, twice to double click.

TODO:
- tap and drag to hold
- right click with long press 
- optimize display library. (maybe move to TFT_eSPI) the current library makes the mouse hang when updating the graphics. 
- clean up unused files (remove QMI8658 IMU library and so on) 
