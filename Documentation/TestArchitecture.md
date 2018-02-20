# Test Architecture

Host-side KLL support was design around testing some of the major general-purpose features of KLL.
Most notably state scheduling and pixel (LED) support.
Both of these features have near limitless configuration permutations, so an automated method of testing was devised to speed up testing and reduce regressions.

Build-wise, the Test Architecture behaves in the same way as the [firmware generation](BuildArchitecture.md); however, instead of generating a firmware binary a shared library is created instead.
This shared library, `kiibohd.so`, has 3 main interfaces.

```
                    +------------+
     Commands +---> |            |
                    | kiibohd.so +-----> Callbacks
Memory Access +---> |            |
                    +------------+
```


## Commands

Commands are useful commands are called from the test script.
These may (or may not) interface with `kiibohd.so` and use the form `cmd('<command name>')(<arguments>)`.

As `kiibohd.so` has no access to hardware when running in host mode, this is the only way to spur it into action.

A command can be registered by adding a function to a `Command` class in either [Scan/TestIn/host.py](../Scan/TestIn/host.py) or [Output/TestOut/host.py](../Output/TestOut/host.py).
The internal lookup combines all `Command` class functions.


## Memory Access

Another method of querying information from `kiibohd.so` is by directly accessing memory using ctypes.
This is useful for both querying and modifying data inside `kiibohd.so`.

While very powerful, it's also quite error-prone as it's easy for a struct to be off or a variable type to be mis-configured.
To help with this as much as possible, it is recommended to generate new Python ctype structures for each of the accessed/modified `kiibohd.so` structs.
This is complicated by build-time computation of variable widths.

Build-time variable widths are used for firmware space optimizations.
Many of the internal KLL firmware datastructures uses indices that may not need a 32-bit variable, but may sometimes need a 16-bit variable with some keyboard configurations.


## Callbacks

Callbacks are very useful for implementing hardware specific functionality directly in Python.
Instead of registering callbacks for every single function, a signal 2 argument callback is implemented instead.
This callback uses a command + argument string lookup in Python to determine what is supposed to occur.

A callback can be registered by adding a single argument function to a `Callback` class in either [Scan/TestIn/host.py](../Scan/TestIn/host.py) or [Output/TestOut/host.py](../Output/TestOut/host.py).
The internal lookup combines all `Callback` class functions.

Due Python annoyance, after each usage, the callback memory address must be refreshed (likely due to the garbage collector).
Don't worry, this is taken care of for you in [Lib/host.py](../Lib/host.py).

Callbacks shouldn't be used to transfer complicated data structures as the ctypes interface must be decoded for each case.
Instead, signal that an action occured, then use a memory access to retrieve/edit the necessary information.

