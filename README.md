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

# Create a node
# + node_type node_name

+ sinosc osc


# map a constant to a node input
# = value node_name@input_name

= 440 osc@freq


# make a connection
# > src_node@output_name dest_node@input_name
#
# -> Note that the _out_ module does not need to be declared
#    as it is always instantiated

> osc@out _out_@l

```

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

+ add cpu

# use the declared inputs and outputs

> _in_@num1 cpu@in1
> _in_@num2 cpu@in2

> cpu@out _out_@sum

```

A module can be included in another module.

----

Using [dr_wav](https://github.com/mackron/dr_libs) to write wave files.

