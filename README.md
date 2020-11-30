# TZARA

Text based modular synthesizer.

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

