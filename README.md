Simple demonstration using the main facilities of FreeRTOS, with FreeRTOS coding conventions.

This is a small application that does some things in a hard way for no reason other than to demonstrate that I understand FreeRTOS. It creates two periodic sender tasks and one blocking receiver task. The senders are higher priority than the receiver, but the senders block in `xTaskDelayUntil`. This allows the receiver to run eventually, and, provided that `printf` is fast enough, in time to prevent the queue from "overflowing". Of course, there are no guarantees running this under my x86_64 Linux system.

It uses the heap_1.c allocator which is a trivial implementation of an allocator that has no free functionality but is enough for a small hard real time program as long as the worst case code path does not use up the full heap.

Forked from FreeRTOSv202212.01/FreeRTOS/Demo/Posix_GCC from FreeRTOSv202212.01.zip

My actual code is mostly in main.c

Build and run
=============

In this folder, create Src/FreeRTOS and download and extract FreeRTOS: https://github.com/FreeRTOS/FreeRTOS/releases/download/202212.01/FreeRTOSv202212.01.zip, copying the FreeRTOSv202212.01/FreeRTOS/Source/ to Src/FreeRTOS.

Run `make` and then `build/posix_demo`.

In other words:

```
git clone https://github.com/f1x3dp01nt/freertos_hello
cd freertos_hello
wget --quiet https://github.com/FreeRTOS/FreeRTOS/releases/download/202212.01/FreeRTOSv202212.01.zip -O /tmp/FreeRTOSv202212.01.zip
unzip /tmp/FreeRTOSv202212.01.zip -d /tmp/
mkdir -p Src/FreeRTOS/
cp -ia /tmp/FreeRTOSv202212.01/FreeRTOS/Source/ Src/FreeRTOS/
make
build/posix_demo
```

Tested on Debian 12, Ubuntu 24, Fedora 40.
