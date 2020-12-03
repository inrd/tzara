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
# -> Note that the out module does not need to be declared
#    as it is always instantiated

> osc@out out@l

```

## Nodes

To print a list of the available nodes, run :

```bash
tzara --nodes
```

You can also read a dump of the above command in `nodes_list.md`.

----

Using [dr_wav](https://github.com/mackron/dr_libs) to write wave files.

