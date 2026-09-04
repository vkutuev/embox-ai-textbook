# Build System: Mybuild

Embox is modular and configurable. The declarative Mybuild language describes
both single modules and the whole target system. Mybuild is implemented on
top of GNU Make, keeping incremental and parallel building.

## Modules

A module is the base concept of the build system. A module description
contains:

- a list of source files
- options that can be set during configuration
- a list of dependencies

Modules are organized into hierarchical packages (namespaces). The package
name usually coincides with the path in the source tree, e.g.
`package embox.arch`. The full module name consists of the package name
appended with the module name.

Mybuild files are named `Mybuild` or have the `.my` extension.

### Module attributes

```java
package embox.cmd

@AutoCmd
@Cmd(name = "hello_world", help = "First Embox application")
module hello_world {
    source "hello_world.c"
}
```

Source files are classified by extension:

- `.c`/`.S` — source code compiled and included in the final image
- `.h` — header file; a special header with the module's public interface is
  generated during the build, allowing different implementations of an
  interface without changing the code that uses it
- `.lds.S` — linker script affecting how the module is loaded into the image

### Options

Options define numerical, boolean, and string parameters at configuration
time. They may have default values and can affect how the module is compiled,
initialized, and how it works. Inside source code, option values are obtained
with macros:

- `OPTION_STRING_GET(name)`
- `OPTION_NUMBER_GET(name)`
- `OPTION_BOOLEAN_GET(name)`

### Dependencies

The `depends` attribute declares that a module cannot work without other
modules or interfaces. The build system guarantees that dependencies are
included. Only one module implementing a given interface may be included.

The `@NoRuntime` annotation marks dependencies that are needed only during
the build and do not determine the module loading order (used for global
modules such as multiprocessing support, logging, or assertions).

### Interfaces and abstract modules

Interfaces are analogues of abstract classes in OOP: modules without
implementation (`abstract module`) or with partial implementation. This allows
choosing among several modules that implement the same interface with
different algorithms.

```java
package embox.arch

@DefaultImpl(embox.arch.generic.interrupt_stub)
abstract module interrupt {}
```

```java
module interrupt_stub extends embox.arch.interrupt {
    source "interrupt_stub.h"
}
```

### Loading order

Modules load according to dependencies (if module B depends on A, A loads
first). The `@Runlevel(level)` annotation manages the loading order
explicitly.

## Configuration

The configuration describes the whole system: the list of required modules,
their options, and build rules (cross-compiler, compiler flags, memory map).
It lives in the `conf/` directory:

- `lds.conf` — memory map used by specific hardware platforms
- `mods.conf` — names and options of modules included in the OS image
- `rootfs/` — files included into the file system
- `build.conf` — compilation and linking flags

### Loading a configuration

```console
$ make confload                 # list all available templates
$ make confload-<template>      # e.g. make confload-x86/qemu
```

The chosen configuration files are copied into `conf/`, replacing the
previous ones. Changes made in `conf/` are lost on the next `make confload-*`
or `make distclean`.

### Adding modules to a configuration

`conf/mods.conf` contains:

```java
package genconfig

configuration conf {
    include embox.cmd.help
}
```

Add `include <PACKAGE>.<MODULE>` to include a module, optionally overriding
its options:

```java
include embox.kernel.thread.core(thread_stack_size=0x4000)
```

To check that a module is in the system, use `lsmod -n <name>` in the Embox
console; a leading `*` means the module is loaded and working.

### Compilation flags

`conf/build.conf` holds flags such as:

```make
CFLAGS += -O0
LDFLAGS += -N -g
```

Change optimization level by setting `CFLAGS += -O2`. Removing `-g` reduces
the image size but disables debugging.

## Make targets

| Command | Purpose |
| ------- | ------- |
| `make` | Build the image; result is `build/base/bin/embox` |
| `make help`, `make help-mod` | List build and module management commands |
| `make mod-list` | List all modules included in the build |
| `make mod-brief-<module>` | Show module info: dependencies, options, sources |
| `make mod-include-reason-<module>` | Show dependency subtree explaining why the module is included |
| `make dot` | Generate the module graph `build/doc/embox.png` (needs `graphviz`) |
| `make disasm` | Generate a disassembly of the current image |
| `make docsgen` | Generate Doxygen documentation into `build/docs/html` (needs `doxygen`) |

### Cleaning

| Command | Effect |
| ------- | ------ |
| `make clean` | Delete `build/` (objects and binaries) |
| `make cacheclean` | Also delete `mk/.cache` with Mybuild artifacts |
| `make distclean` | Reset the project to its original state, including the working configuration |

## External repositories

External projects can contribute modules and templates:

```console
$ make ext_conf EXT_PROJECT_PATH=<your projects path>
```

For templates to be visible in `make confload`, the external repository must
have the structure `<root>/<project_name>/templates`. Modules can be placed
in arbitrary folders; the search is performed using `Mybuild` and `.my`
files.

## Sources

- [embox-docs: Modular structure of Embox](https://github.com/embox/embox-docs/blob/master/en/embox_modular_structure_en.md)
- [embox-docs: Building](https://github.com/embox/embox-docs/blob/master/en/embox_build_en.md)
- [Embox wiki: Mybuild](https://github.com/embox/embox/wiki/Mybuild)
