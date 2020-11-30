# TZARA

Text based modular synthesizer.

*Work in progress...*

## Syntax

```

# Lines starting by "#" are comments

# Create a node
+ phasor osc

# map a constant to a node input
= 440 osc@freq

# make a connection
> phasor@out out@l

```

----

Using [dr_wav](https://github.com/mackron/dr_libs) to write wave files.

