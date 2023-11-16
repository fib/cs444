In order to handle environment variables, I passed envp to main which contains all of the variables
from when the shell is started. Then, I iterate through envp and construct two arrays: envKeys contains
the names of the variables, and envVals contains the values at the corresponding indices. This pair of
arrays acts as a map that allows for easy lookup of variables at any point.

The env command simply iterates through the two env arrays printing out each key and its corresponding
value.

setenv iterates through the envKeys array looking for the key that was passed as an argument. If it is
found in the array, the value at the corresponding index in envVals is updated using the second argument.
If it is not found, a new key and value are appended to both arrays.

unsetenv is similar to setenv in that it first tries to find the arg in envKeys. If it is found, the key
and the corresponding values are freed, and if there are any values after the one being unset, they are
shifted down in the array to close the gap.

cd has two main execution paths: if no path is provided as an argument, the value of the HOME env 
variable is looked up and used as the path for chdir. If a path is provided, then chdir is called with
that path.

I implemented history by emulating a circular array. This means that the array is treated as a looping
structure that overwrites old values as it fills up. This is done by keeping track of the head 
and the tail. New values are inserted at the tail, and if the tail overlaps with the head, the head is
moved forward. This way we always know where the "first" (oldest) value is (the head), which is where the 
history command starts printing from.