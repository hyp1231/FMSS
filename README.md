# FMSS

A simulation of Unix's file management system.

Developed by "Prime Luo" and his buddy "Yupeng Hou"

## Quick Start

The system uses a file named "diskdata" to simulate a disk.

So we need to generate this file firstly.

`make gen`

`./gen`

And then you have this file, so you can run the system!

`make`

`./main`

If you need help about how to use it, just input "help" to get what you need.

Have a good time~ :)

## Structure

### General

1024 blocks totally

|kind|amount|range|
|:-:|:-:|:-:|
|Super block + Inode bitmap + Data block bitmap|1|0|
|Inode table|5|1 - 5|
|File name index|80|6 - 85|
|Data block|640|86 - 725|
|None|298|726 - 1023|

### Super block + Inode bitmap + Data block bitmap (1 block)

1024B totally

|content|size|range|
|:-:|:-:|:-:|
|size of a block|2B|0 - 1|
|inode number of '/'|1B|2|
|inode bitmap|10B|3 - 12|
|data block bitmap|80B|13 - 92|
|none|931B|93 - 1023|

### Inode (1/16 of a block)

64B totally

|content|size|range|
|:-:|:-:|:-:|
|position of index block|2B|0 - 1|
|file name|8B|2 - 9|
|create time|16B|10 - 25|
|modify time|16B|26 - 41|
|data block number|8 * 2B|42 - 57|
|none|966B|58 - 1023|

> PS:
> position of index block - 0xff if it's a file inode\\
                            BLK_No. if it's a directory inode

### File name index

1024B totally

|content|size|range|
|:-:|:-:|:-:|
|file name|8B|0 - 7|
|valid|2B|8 - 9|
|inode number|2B|10 - 11|

> Repetively 80 times




