# Macabuilder
Simple and yet efficient build system primarily oriented on C and C++ mixed with Assembly with good attention on low level details and configurability.

<img src="https://github.com/MacaronOS/Macabuilder/blob/main/Assets/wisteria-build-test.gif" width="500" alt="WisteriaOS build demo">

## How to build this project?
```bash
mkdir build
cd build
cmake ..
make
```
Now you have a Macabuilder executable inside your build folder. You can create an alias
or add it in your PATH or just reference it by an absolute path from other projects. 


## What's the point?
If you want to build a low level project (Operating system is a perfect example)
usually you have a lot of configurability to deal with. For example:

1. Sometimes you're required to use a custom cross-compiler for the system you are targeting
2. Or you need to be aware of compiler / linker flags
3. You need to generate multiple binaries and process them further
4. You need a tool to create userspace / runtime libraries and link them into your binaries
5. Everything should stay readable and expandable as your project grows
6. ...

There really aren't tools for that especially for beginners so I decided to build my own "dream" build system 
and use it in my projects.

## Tests
### Building [MacaronOS](https://github.com/MacaronOS/Macaron) - Macabuilder vs Make (with -j flag)

Attempt | #1 | #2 | #3 | #4 | #5
--- | --- | --- | --- |--- |--- |
Make (seconds) | 1,326 | 1,334 | 1,321 | 1,312 | 1,317
Macabuilder (seconds) | **0,985** | **1,016** | **1,049** | **1,027** | **0,955**

### Building [this project itself](https://github.com/MacaronOS/Macabuilder) - Macabuilder vs CMake

Attempt | #1 | #2 | #3 | #4 | #5
--- | --- | --- | --- |--- |--- |
CMake (seconds) | 4,271 | 4,557 | 4,324 | 4,297 | 4,144
Macabuilder (seconds) | **3,552** | **3,907** | **3,586** | **3,624** | **3,642**

## Features / usage guide
- Use "Build" field to specify either an executable or static library mode
    - Use "Src" subfield to select all sources for your project
    - Use "Extensions" subfield to filter sources by extension and setup and then specify compiler and flags for those extension
    - If you are building an executable use "Link" subfield to specify linker and linker flags
    - If you are building a static library use "Archive" subfield to specify an archiver
    - Use "Depends" subfield to list all dependencies for the current build target
        - If a static library is listed, it will be linked into the target
        - If an executable is listed, it will be built before the current target
    

- Use "Commands" field to specify shell commands
  - There's at least one command "Build" that's declared implicitly. It's used to launch build field.
  - You can run them by passing them as arguments when launching Macabuilder binary
    
- Use "Default" field to specify a default sequence of commands
  - the default commands sequence is launched when there are no arguments passed to the Macabuilder binary

## If you want to try and build something
Check out my other project [MacaronOS](https://github.com/MacaronOS/Macabuilder).
Since I'm trying to be consistent with all the new Macabuilder features
that project pretty much contains the examples for all you need.

## Further plans:
- [ ] Incremental build
- [ ] Define variables
- [ ] If statements
- [ ] Include .maca files into each other (f.e. for shared variables)
    