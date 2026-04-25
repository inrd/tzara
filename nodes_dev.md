# Tzara nodes development

- In `nodes.h` and `nodes.c`
    - Add the new node type to the `NodeTypes` enum
    - Implement a callback function `void performMyNode (TzNode* n, TzProcessInfo* info)`
    - Implement a constructor as in the example below
    - Add the appropriate info to the `nodesDoc` array, including the constructor as the last field (the parser will dispatch on it automatically)

```c

TzNode* createMyNode () {
    TzNode* n = allocateNewNode();
    n->numInputs = 1;
    strcpy(n->inputsNames[0], "in1");
    n->numOutputs = 1;
    strcpy(n->outputsNames[0], "out1");
    /* initialize memory if needed */
    n->memory[0] = 0.f;
    n->perform = &performMyNode;
    return n;
}

```

- Register the node name as a keyword in `vim/syntax/tzara.md`

## Parametric nodes

The steps above cover nodes whose constructor takes no arguments. If the new node needs extra arguments parsed from the patch (like `module`, `matrix`, `mget`, `mset` do), then:

- Leave the `ctor` field as `NULL` in the `nodesDoc` row
- Add a `parseAndCreateMyNode(...)` helper in `parser.c` that extracts the arguments from the token list and calls the actual constructor
- Add a case for the new node type in the small `switch` inside `parseCreateNodeInstruction` that calls the helper

