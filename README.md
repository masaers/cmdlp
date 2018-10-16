# cmdlp

A template-based command line parser for c++.

## Philosophy

This is a fairly idea driven library, so you might find yourself struggling against it if you don't understand and/or agree with the guiding philosophy.

A computer program can be viewed as a function: it takes some inputs and produce some outputs. Most would agree that the typical terminal command takes some input file(s) and produces some output file(s), but for it to be a viewable as a function it must also do so *deterministically*. Most terminal commands also have some *parameters*, that, when taken as part of the input makes the command completely deterministic. This library takes the view that your program has a set of *knobs* and *switches* that completely describes it behavior, and the job of a command line parser is to provide a way to turn and flip these knobs and switches.

Obviously, the POSIX standard should be followed.

### But what about randomization?

Hate to break it to you, but your computer is incapable of randomness. What you are doing is implicitly setting the random seed of a pseudo random number generator typically based on the time of execution. *If you are a scientist, for the love of all that is reproducible, provide a knob to deterministically set the random seed!* If your result tables cannot be recreated reproducibility is out the window. Don't make wall clock at runtime an input to your program. (For funsies you may add a switch to explicitly disable reproducibility, if that's your thing.)

### In practice

In practice, this philosophy means that you already have a set of variables in your main function that needs to be given values; this is where the command line parser comes in. To separate the knobs-and-switches variables from other variables, you are encouraged to collect them in a class, and provide a method for the command-line parser to be initialized. The library makes it easy to initialize an object of such a class and keep it `const` after.

## Tutorial

Coming soon.