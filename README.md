# pdp8

My simple emulator for the original [PDP8 (Straight 8)](https://en.wikipedia.org/wiki/PDP-8).
I never used an actual PDP8 so based the emulation on information from [there](http://homepage.divms.uiowa.edu/~jones/pdp8/models/#PDP8).


The following are emulated :

* CPU

![PDP8](https://www.smecc.org/pdp8.jpg)

* Teletype ASR33

![ASR33](https://upload.wikimedia.org/wikipedia/commons/3/33/Teletype-IMG_7287.jpg)

* High speed tape reader (750c)

## Building
    
There is just one C file to compile.    

    cc pdp8.c -o pdp8

## Usage

    ./pdp8 focal.bin

## Finding things to run

There are places with [Folders full of tape images](http://www.mirrorservice.org/sites/www.bitsavers.org/bits/DEC/pdp8/From_Vince_Slyngstad/misc/)

[another place](http://dustyoldcomputers.com/pdp-common/reference/papertapes/dec-08.html)

## Todo

* Implement the CPU extensions
* Emulate drives
* Run OS8





