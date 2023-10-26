1) Josh Kotler

2) My project 2 implementation consists of raid.c and diar.c.
raid.c reads an input file and encodes it using Hamming(7, 4) codes, which means
that for each 4 bits of data there are 3 parity bits that can help recover data
in the event of corruption. The output of raid.c is 7 files (emulating 7 drives)
in a RAID 2 configuration: this means that each bit of the 7-bit hamming codes is
stored in a separate file. This is done so that if one drive fails (flips a bit erroneously),
the data can still be recovered.

diar.c is the decoder: it reads in the 7 RAID files and re-assembles the hamming codes by taking 1 bit
from each of the RAID files.
Each hamming code is then verified by checking whether the parity bits are correctly set.
If they are not, the incorrect bit (determined by the combination of the incorrect parity bits)
gets flipped to recover the correct data. Hamming(7, 4) can only reliably correct 1 error per 
code, so we assume that an error can only occur in one of the files.

3) Makefile:
# the first two lines define variables: CC for the compiler to use, and CFLAGS for the compiler flags
# to be passed in for compilation
CC=cc
CFLAGS=-Wall -O2

# since this is the first rule, it gets executed when make is called without any arguments (default).
# since raid and diar are declared as dependencies for this target, make proceeds to look for other
# targets that can satisfy this prerequisite. Since the next target is a wildcard (%) it gets matched,
# so make executes the %: %.c target for both raid and diar.
all: raid diar

# % is the wildcard operator. This means that whatever is passed to this target
# will be compiled as a .c file
# $@ is a variable provided by make that contains the matched pattern.
# $< is the name of the first pre-requisite, i.e. the matched pattern but with the extension .c
%: %.c
	$(CC) $(CFLAGS) -o $@ $<

# the clean target simply cleans up the directory by removing generated output files
clean:
	rm -f a.out *.part? *.2

4) getopt.h provides a convenient way to parse command-line arguments by predefining the accepted
arguments of the program.
getopt(argc, argv, opts) gets called in a loop. Each time it is called, it returns the next option 
that it detects in argv. Once there are no more options in argv it returns -1, which is when the loop 
exits.
Inside of the loop is a switch statement that defines how to handle each option. The argument passed to
each option is stored in a global variable called `optarg`, which is how this value can be accessed. It is
a string, so for cases when an option accepts a number, the string has to be converted to an integer or another
appropriate type (for example using the atoi() function). Otherwise, strcpy can be used to save the string value.
The third argument of getopt, `opts` defines which options the program expects to receive. This enables getopt to
throw an error when an invalid option is passed and also notify the users if an option that is required has been
omitted.