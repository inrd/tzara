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
> osc@out out@l

```

## Nodes

- `add` : outputs the sum of its inputs
    - inputs : `in1` `in2`
    - outputs : `out`
- `phasor` : generates a ramp in the range [0..1]
    - inputs : `freq` (frequency in hertz)
    - outputs : `out`

----

Using [dr_wav](https://github.com/mackron/dr_libs) to write wave files.

