Josh Kotler

I started working on this project by setting up the command line options parsing
mechanism. I have used getopt.h before and so I knew that it would work well
for this use case. This part went smoothly and allowed me to conveniently test
my program throughout the rest of my time working on it.

Then I broke down the Huffman Coding algorithm into its main logical steps:
1. Count character frequencies,
2. Assemble tree based on frequencies,
3. Read codes from the tree,
4. Encode the input file using the generated codes.

Since each part depends on the previous ones being functional, I had to work in 
that order. Counting the characters was simple and straightforward, however I was
struggling with a segmentation fault error for a while even after finishing this part.
After attending office hours I finally realized that there was a minor mistake in my
while-loop constraints for reading the input file, where the EOF character was not
properly handled, leading to illegal memory operations (segmentation fault). I was already
looking at other parts of my code looking for this bug because I didn't think it could
be something so simple. But eventually I got it fixed and it took care of the segmentation
faults.

After I had all of the character frequencies stored in "frequency nodes" (structs
containing the character, its frequency, and pointers to next/prev/left child/right child)
I also maintained an array of those nodes, so that I could have easy access to any node
in O(1) (because I wouldn't have to traverse the list of nodes, but simply just address it
with the relevant character as the index in the array).

Contrstructing the tree was done by popping and pushing elements out of and into the list of
nodes, which was implemented as a priority queue. This means that the list maintains its sorted
order whenever new elements are being pushed in. Once I had pop and push implemented correctly,
assembling the tree was just a matter of calling them in the right order.

I first read the codes from the tree into simple strings in order to make sure that my tree was
beign constructed correctly. This helped me find a bug where I would push nodes that did not have
a character value associated with them into the tree (characters that did not exist in the file).
Once I made sure my tree was correct, I started thinking about how to read the codes in binary. I 
ended up using a buffer (an unsigned integer) into which I wrote a bit at the position corresponding
to the depth of the current node in the tree. Whenever a leaf node was reached I simply saved whatever
was in the buffer into an array so that it could be accessed later (each binary code can be looked up
by its associated ASCII value).

Finally, writing the codes into the tree involved a unsigned char buffer because it is exactly 1 byte.
This allowed me to pick a character from the input file, look up its binary code, write the code into
the buffer, and as soon as the buffer filled up (8 bits), I sent it out to the file. Then I cleared the
buffer and continued filling it up from where I stopped.