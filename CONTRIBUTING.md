# Description for contribution

This library is part of my set of modular C/CPP libraries which can be use and reuse in any project. 
The central idea is to keep the folders and files hierachical order to ensure the generic CMakeLists.txt file included in each library can build recursively the libraries used in the project.
Here is the schematic view of each project structure of my set.

```
┌─────────┐                        ┌────────────────────┐
│ project │                        │ CMakeList.txt      │
└─┬───────┘                     ┌──┤ CMakeConfig.txt    │
  │                             │  │ CmakeFunctions.txt │
  ├───────────┬───────┬──────┬──┘  └────────────────────┘
  │           │       │      │
  │      ┌────┴────┐  │   ┌──┴──┐     ┌─────────────┐
  │      │ include │  │   │ lib ├──┬──┤ subproject1 │
  │      └────┬────┘  │   └─────┘  │  └──────┬──────┘
  │           │       │            │         │
  │      ┌────┴────┐  │    modules │     ┌───┴──────────────┐
  │      │ lib.hpp │  │    for the │     │                  │
  │      └─────────┘  │    library │  ┌──┴──┐          ┌────┴────┐
  │                   │    target  │  │ src │          │ include │
 ┌┴────┐              │            │  └─┬───┘          └────┬────┘
 │ src │              │            │    │                   │
 └─┬───┘              │            │    │ ┌──────────┐ ┌────┴────┐
   │                  │            │    ├─┤ main.cpp │ │ lib.hpp │
   │ ┌──────────┐     │            │    │ └──────────┘ └─────────┘
   ├─┤ main.cpp │     │            │    │
   │ └──────────┘     │            │    │ ┌─────────┐
   │                  │            │    └─┤ lib.cpp │
   │ ┌─────────┐      │            │      └─────────┘
   └─┤ lib.cpp │      │            │
     └─────────┘      │            │  ┌───────────────┐
                      │            └──┤ subproject ...│
                      │               └───────────────┘
                      │
                     ┌┴─────────┐     ┌─────────────┐
                     │ tool_lib ├──┬──┤ subproject1 │
                     └──────────┘  │  └──────┬──────┘
                                   │         │
                         modules   │     ┌───┴──────────────┐
                         for the   │     │                  │
                         exe file  │  ┌──┴──┐          ┌────┴────┐
                         target    │  │ src │          │ include │
                                   │  └─┬───┘          └────┬────┘
                                   │    │                   │
                                   │    │ ┌──────────┐ ┌────┴────┐
                                   │    ├─┤ main.cpp │ │ lib.hpp │
                                   │    │ └──────────┘ └─────────┘
                                   │    │
                                   │    │ ┌─────────┐
                                   │    └─┤ lib.cpp │
                                   │      └─────────┘
                                   │
                                   │  ┌───────────────┐
                                   └──┤ subproject ...│
                                      └───────────────┘
```
     

This structure enable you to quickly include previous library modules in your curent project.

# How to?
To include a previous library module to your current prject, add a git submodule of the desired library into a folder named lib in your current project. 
This is it, you can now use all the functions and ojects defined if the added library.
The generic CMakeLists.txt file knows how to deal with the rest.
