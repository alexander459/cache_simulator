# cache_simulator
This is a cache memory simulator. simulator.c contains the code for the simulator.
Language: C
Usage as: `./a.out <number_of_sets> <number_of_blocks_per_set> <bytes per block> <replace algorithm>`
The number of sets must be a power of 2
The number of blocks must be a power of 2
The number of bytes per block must be a power of 2 and number_of_bytes_per_block >= 4
The algorithm can be: random, lru, fifo.
The program reads a txt file with addresses as: `A 0x<address>` where A can be either S or L (Store or Load command) and
`<address>` is a hex value address. For this reason there is a file named generator.c. This file contains the code for an instruction-address
generator. The addresses are random but the generator contains a buffer which stores some of the generated addresses. The purpose is to
create a file with random addresses but some of them have to be repeted in order to test the replace algorithm of the simulator.
The user can chose the buffer's size and the total accesses to that buffer. Small buffer size and many accesses indicate a bigger frequency
between some addresses. The usage of the ganerator is of course optional. Same and better results can be achived writting the addresses in a
txt file by hand; Just to save time and to test my skills ;)

!NOTE: The program does not support S commands yet. When it reads `S 0x<address>` it just skips the line and continues to the next address
