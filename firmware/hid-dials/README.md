# HID implementation for IBM-dials-retrofit

Works, currently hardcoded:

 - Dial 1: Volume Up/Down
 - Dial 2: nothing
 - Dial 3: KiCAD switch layer (v)
 - Dial 4: KiCAD rotate part (r|R)
 - Dial 5: KiCAD change grid settings (n|N)
 - Dial 6: nothing
 - Dial 7: KiCAD change track width (w|W)
 - Dial 8: KiCAD zoom (+|-)

 Build with `make`

 Flash with `dfu-util -a 0 -s 0x08000000:leave -D build/hid-dials.bin`

 Todo:

 - [ ] Add filesystem and csv alike config files
