Name:       William Reid Paape
NetID:      paape
Student ID: 2877379718


## Compilation Steps
```
make
```

## Run
```
./STDM <input file>
# prints frames to standard output
# prints log (including frame stats) to STDM-log.txt
```

## Clean
```
make clean
```

## Tasks
1.
    a. See STDM.cpp's main() for how <input file> is passed to
       StdmMux::StdmMux() and each line is passed to StdmSource::StdmSource()
       for parsing.
    b. Every frame interval (StdmMux::timeStep):
        1. Sources are polled every for incoming data blocks.
        2. These blocks are queued in an input buffer (StdmMux::backlog) and
           prioritized according to:
            1. earliest time received
            2. earliest-listed source (first line to last line)
                - sources are addressed with an integer [1,N] where N is the
                  number of lines in <input file>
                - an address of 0 indicates an empty subframe
            - see BacklogItem::operator<()
        3. A fixed number of subframes are fetched from the input buffer
           (StdmMux::frameSize).
        4. These subframes are packed into a frame.
            - If there are not enough data blocks to fill a frame, pad the
              remainder of the frame with zeros.
        5. These frames are written to standard output.

c.
    - The average transmission rate of the streams (averageTransmissionRate) is
      calculated in StdmMux::StdmMux() and logged to the "stats" section of the
      log file (STDM-log.txt).
    - The number of address bits (StdmMux::addressBits) is calculated in
      StdmMux::StdmMux() and logged to the "stats" section of the log file
      (STDM-log.txt).
    - The frame time slot (StdmMux::timeStep) is chosen to be the duration of 1
      data block (see StdmMux::readSources()).

d.
    - The input buffer is a priority queue of timestamped, addressed data
      blocks (StdmMux::backlog).
    - The output buffer is standard output.

e.
    - The output format for each frame is as follows:
        - SF
        - Address 1
        - Data Block 1
        ...
        - Address N
        - Data Block N
        - EF
      where N is the maximum number of data blocks per frame.  Frames that
      that can not fully utilize all of their available subframe due to idle
      sources are padded with zeros.
    - See (h) for example output.

f. The number of bits required to address each data block is determined by the
   number of sources (see the calculation of StdmMux::addressBits in
   StdmMux::StdmMux()).

g. See (f).
            
h. Example Output:
```
# input.txt contents...
cat input.txt
SourceA:0 1 A1,1 2 A2,2 3 A3,5 6 A4
SourceB:0 1 B1,1 2 B2
SourceC:1 2 C1,2 3 C2,3 4 C3,5 6 C4,6 7 C5
SourceD:4 5 D1,5 6 D2,8 9 D3

# run...
==================================================== start of frame 1 (time=1-2)
SF (1 bit)
------------------------------------------------- start of subframe 1 (time=1-2)
address: 1/SourceA (2 bits)
data: "A1" (14 bits)
------------------------------------------------- start of subframe 2 (time=1-2)
address: 2/SourceB (2 bits)
data: "B1" (14 bits)
EF (1 bit)
==================================================== start of frame 2 (time=2-3)
SF (1 bit)
------------------------------------------------- start of subframe 1 (time=2-3)
address: 1/SourceA (2 bits)
data: "A2" (14 bits)
------------------------------------------------- start of subframe 2 (time=2-3)
address: 2/SourceB (2 bits)
data: "B2" (14 bits)
EF (1 bit)
==================================================== start of frame 3 (time=3-4)
SF (1 bit)
------------------------------------------------- start of subframe 1 (time=3-4)
address: 3/SourceC (2 bits)
data: "C1" (14 bits)
------------------------------------------------- start of subframe 2 (time=3-4)
address: 1/SourceA (2 bits)
data: "A3" (14 bits)
EF (1 bit)
==================================================== start of frame 4 (time=4-5)
SF (1 bit)
------------------------------------------------- start of subframe 1 (time=4-5)
address: 3/SourceC (2 bits)
data: "C2" (14 bits)
------------------------------------------------- start of subframe 2 (time=4-5)
address: 3/SourceC (2 bits)
data: "C3" (14 bits)
EF (1 bit)
==================================================== start of frame 5 (time=5-6)
SF (1 bit)
------------------------------------------------- start of subframe 1 (time=5-6)
address: 4/SourceD (2 bits)
data: "D1" (14 bits)
------------------------------------------------- start of subframe 2 (time=5-6)
address: 0/<NONE> (2 bits)
data: "<ZERO-FILLED>" (14 bits)
EF (1 bit)
==================================================== start of frame 6 (time=6-7)
SF (1 bit)
------------------------------------------------- start of subframe 1 (time=6-7)
address: 1/SourceA (2 bits)
data: "A4" (14 bits)
------------------------------------------------- start of subframe 2 (time=6-7)
address: 3/SourceC (2 bits)
data: "C4" (14 bits)
EF (1 bit)
==================================================== start of frame 7 (time=7-8)
SF (1 bit)
------------------------------------------------- start of subframe 1 (time=7-8)
address: 4/SourceD (2 bits)
data: "D2" (14 bits)
------------------------------------------------- start of subframe 2 (time=7-8)
address: 3/SourceC (2 bits)
data: "C5" (14 bits)
EF (1 bit)
==================================================== start of frame 8 (time=8-9)
SF (1 bit)
------------------------------------------------- start of subframe 1 (time=8-9)
address: 0/<NONE> (2 bits)
data: "<ZERO-FILLED>" (14 bits)
------------------------------------------------- start of subframe 2 (time=8-9)
address: 0/<NONE> (2 bits)
data: "<ZERO-FILLED>" (14 bits)
EF (1 bit)
=================================================== start of frame 9 (time=9-10)
SF (1 bit)
------------------------------------------------ start of subframe 1 (time=9-10)
address: 4/SourceD (2 bits)
data: "D3" (14 bits)
------------------------------------------------ start of subframe 2 (time=9-10)
address: 0/<NONE> (2 bits)
data: "<ZERO-FILLED>" (14 bits)
EF (1 bit)

# log file contents...
cat STDM-log.txt
reading sources...
	read source "SourceA": startTime=0, endTime=6, dataBlocks=4
	read source "SourceB": startTime=0, endTime=2, dataBlocks=2
	read source "SourceC": startTime=1, endTime=7, dataBlocks=5
	read source "SourceD": startTime=4, endTime=9, dataBlocks=3
read sources!

stats:
	average transmission rate: 1.55556 data blocks per second
	data bits: 14 bits per data block (2 characters * 7 bits/character)
	address bits: 2 bits per subframe
	subframe bits: 16 bits
	frame size: 2 subframes per frame
	frame bits: 34 bits (2 subframes/frame * 16 bits/subframe + SF + EF)
	frame time duration: 1 second(s)

======================================= reading subframes for frame 1 (time=0-1)
0 data block(s) in the backlog
2 data block(s) read from sources
2/2 subframes utilized
0 data block(s) backlogged
======================================= reading subframes for frame 2 (time=1-2)
0 data block(s) in the backlog
3 data block(s) read from sources
2/2 subframes utilized
1 data block(s) backlogged
======================================= reading subframes for frame 3 (time=2-3)
1 data block(s) in the backlog
2 data block(s) read from sources
2/2 subframes utilized
0 data block(s) backlogged
======================================= reading subframes for frame 4 (time=3-4)
1 data block(s) in the backlog
1 data block(s) read from sources
2/2 subframes utilized
0 data block(s) backlogged
======================================= reading subframes for frame 5 (time=4-5)
0 data block(s) in the backlog
1 data block(s) read from sources
1/2 subframes utilized
0 data block(s) backlogged
======================================= reading subframes for frame 6 (time=5-6)
0 data block(s) in the backlog
3 data block(s) read from sources
2/2 subframes utilized
1 data block(s) backlogged
======================================= reading subframes for frame 7 (time=6-7)
1 data block(s) in the backlog
1 data block(s) read from sources
2/2 subframes utilized
0 data block(s) backlogged
======================================= reading subframes for frame 8 (time=7-8)
0 data block(s) in the backlog
0 data block(s) read from sources
0/2 subframes utilized
0 data block(s) backlogged
======================================= reading subframes for frame 9 (time=8-9)
0 data block(s) in the backlog
1 data block(s) read from sources
1/2 subframes utilized
0 data block(s) backlogged

DONE!
```
