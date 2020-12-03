# Tzara nodes development

- In `nodes.h` and `nodes.c`
    - Add the new node type to the `NodeTypes` enum
    - Add the appropriate info to the `nodesDoc` array
    - Implement a callback function `void performMyNode (TzNode* n, TzProcessInfo* info)`
    - Implement a constructor as in the example below

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

- In `parser.c`
    - complete the `parseCreateNodeInstruction` function

- Register the node name as a keyword in `vim/syntax/tzara.md`

