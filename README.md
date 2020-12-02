# TZARA

Text based modular synthesizer.

*Work in progress...*

## Syntax

```

# Lines starting by "#" are comments

# Create a node
+ sinosc osc

# map a constant to a node input
= 440 osc@freq

# make a connection
# -> the out module does not need to be declared
#    as it is always instantiated
> osc@out out@l

```

## Nodes

To print a list of the available nodes, run :

```bash
tzara --nodes
```

----

Using [dr_wav](https://github.com/mackron/dr_libs) to write wave files.

