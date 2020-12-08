# Tzara TODOs

- crashes when writing wav files with some render duration values
    - should probably  adjust the duration to be a multiple of the buffer size
    - safe values for now : 30, 60 etc..
- warn when making a backward connection
    - -> when dest node index < source node index
- avoid nasty crashes when modules parsing fails

