# Tzara TODOs

- constant assignement invalid for 1st sample
    - because the constant is created while assigning so it is created after the module it is assigned to
    - need to prioritize constant creation
    - workaround for now : use vars instead of constant when critical (and create var before target module)
- avoid nasty crashes when modules parsing fails

