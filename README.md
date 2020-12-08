# TZARA

Text based modular synthesizer.

*Work in progress...*

## Install

```bash

$ make

# make install

```

## Syntax

```

# Lines starting by "#" are comments
# All patch instructions start by an operator

# The instruction below sets a metadata value
# Here the value is the duration (in seconds) of the output wav file

! duration 30


# Create a node
# + [node_type] [node_name]

+ sinosc osc


# Map a constant to a node input
# = [value] [node_name@input_name]

= 440 osc@freq

# Create another node

+ mult amp

# Make a connection
# > [src_node@output_name] [dest_node@input_name]

> osc@out amp@in1

# Map a constant to scale down the level of the oscillator

= 0.5 amp@in2

# Connect the amp node to the main output
# -> Note that the _out_ module does not need to be declared
#    as it is always instantiated

> amp@out _out_@l
> amp@out _out_@r

# The signal flow of the patch is :
#
# [osc] ---> [amp] ---> (l)[_out_]
#              |
#              |------> (r)[_out_]
#

```
To run the patch, if you saved the file as `synth.tzara`, run the following from the patch directory (it would work with relative paths from another directory but would break if the patch uses modules) :

```
tzara synth.tzara synth.wav
```

Tzara will output a log of the build process to the standard output and if the patch was successfully built it will output an audio file named `synth.wav`.

The audio output is normalized to 0dB.

If you do not pass a name for the wav file, one will be generated automatically.

The wav file duration can be specified in the patch metadata (see example above). If not set, Tzara will use a default duration of 60 seconds.

## Nodes

To print a list of the available nodes, run :

```bash
tzara --nodes
```

You can also read a dump of the above command in `nodes_list.md`.

You can easily extend the existing set of nodes using modules (see below).

**A node input can only be connected to 1 output/constant.** To route multiple signals to a single input, use nodes like `add`, `or`, `merge` and `pmerge`.

The nodes are processed sequentially in the order they appear in the patch, one sample at a time. This means that connecting the output of a node to the input of another node which was instantiated before it will introduce a 1 sample delay (so the value received at the input will be the one from the previous sample/frame). 

This behavior allows to easily create feedback loops but can also have unwanted side effects.

Try to instantiate the nodes in the same order they are supposed to appear in the signal flow to avoid erratic results. 


## Modules

A module is a separate tzara patch that can be called from the main patch.

A module can have up to 16 user defined inputs and 16 user defined outputs.

You instantiate a module like this :

```

# the patch synth.tzara must be located in the directory where the tzara process is called.

+ module mysynth <synth.tzara>


# the module can now be used as any other node

= 440 mysynth@freq
> mysynth@out _out_@l

```

Here is how to declare inputs and outputs in module files :

```

# create 2 inputs

@ in num1
@ in num2

# create 1 output

@ out sum

# optionally map a default value to an input

+ defaultval n2
> _in_@num2 n2@in
= 10 n2@val

# create the needed nodes

+ add cpu

# use the declared inputs and outputs

> _in_@num1 cpu@in1

# use the output of the defaultval node instead of the raw input

> n2@out cpu@in2

> cpu@out _out_@sum

```

A module can be included in another module.


## Vim syntax

A very basic syntax file is provided for Vim. You can install it with :

```
make vimsyntax
```

----

Using [dr_wav](https://github.com/mackron/dr_libs) to write wave files.

