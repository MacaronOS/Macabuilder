# Beelder
Simple and yet efficient build system primarily oriented on c++ with good attention on low level details and configurability.

<img src="https://github.com/Plunkerusr/Beelder/blob/main/assets/wisteria-build-test.gif" width="500" alt="WisteriaOS build demo">

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
### Building [WisteriaOS](https://github.com/Plunkerusr/WisteriaOS) - Beelder vs Make (with -j flag)

Attempt | #1 | #2 | #3 | #4 | #5
--- | --- | --- | --- |--- |--- |
Make (seconds) | 1,326 | 1,334 | 1,321 | 1,312 | 1,317
Beelder (seconds) | **0,985** | **1,016** | **1,049** | **1,027** | **0,955**

### Building [this project itself](https://github.com/Plunkerusr/Beelder) - Beelder vs CMake

Attempt | #1 | #2 | #3 | #4 | #5
--- | --- | --- | --- |--- |--- |
CMake (seconds) | 4,271 | 4,557 | 4,324 | 4,297 | 4,144
Beelder (seconds) | **3,552** | **3,907** | **3,586** | **3,624** | **3,642**

## If you want to try and build something
Check out my other project [WisteriaOS](https://github.com/Plunkerusr/WisteriaOS).
Since I'm trying to be consistent with all the new Beelder features
that project pretty much contains the examples for all you need.

## Further plans:
- [ ] Incremental build
- [ ] Define variables
- [ ] If statements
- [ ] Include .bee files into each other (f.e. for shared variables)
    