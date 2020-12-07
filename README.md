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

Tzara will output a log of the build process to the standard output and if the patch was successfully built it will output a 1 minute audio file called `synth.wav` (more options regarding the file rendering are going to be implemented).

The audio output is normalized to 0dB.

If you do not pass a name for the wav file, one will be generated automatically.

## Nodes

To print a list of the available nodes, run :

```bash
tzara --nodes
```

You can also read a dump of the above command in `nodes_list.md`.

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

----

Using [dr_wav](https://github.com/mackron/dr_libs) to write wave files.

