Semestral assignment (Subject: Programming in C)

WARNING! This code is working only on Linux (Ubuntu 18.04 guaranteed).

MAIN TASK:  
Creating asynchronous program communicating with virtual module through pipes.
Program displays computated fractal in window. Computation can be done by PC or by virtual module.
When computation on module is set, main program starts sending messages to module. Each message specificates
part of computation - chunk ID.

REQUIERED LIBRARIES:
linux-vdso.so.1
libpthread.so.0
libdl.so.2
librt.so.1
libc.so.6
/lib64/ld-linux-x86-64.so.2

HOW TO RUN PROGRAM: 
- open cmd window
- create pipes running cmd: ./create_pipes.sh
- open second cmd window
- run module in dir bin: ./prgsem-comp_module
- back in first window run main program: ./prgsem

CONTROLS: 
s - set compute
c - compute on pc
1 - compute on module
r - reset chunk id
l - clear buffer (and screen)
p - print current buffer
+ - increase win size
- - decrease win size
u - start gif
i - stop gif

 