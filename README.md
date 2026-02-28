# ReachVariantTool

A tool for editing Halo: Reach game variants, tested on files obtained from Halo: The Master Chief Collection's PC release.

This fork is focused on a Linux-native build using CMake and Qt while keeping the full editor feature set (multiplayer, firefight, forge, and scripting variant workflows).

## Build (Linux)

Prerequisites:
- CMake 3.21+
- A C++23 compiler (GCC or Clang)
- Qt 6 development packages with `Core`, `Gui`, and `Widgets`
- zlib development package (`zlib1g-dev` on Debian/Ubuntu, `zlib-devel` on Fedora, `zlib` on Arch)

Build commands:

```bash
cmake -S . -B build
cmake --build build -j
```

Binary output:
- `build/ReachVariantTool`

Install (optional, includes desktop entry + launcher icon on Linux):

```bash
cmake --install build --prefix ~/.local
```

Settings file:
- Default location: same folder as the executable
- Format: JSON (`ReachVariantTool.json`)
- Older `.ini` settings files are migrated automatically on first run when found

## Contributing

For detailed contribution guidelines, see `HOW TO CONTRIBUTE.md`.

## License

Qt and its components are used under the terms of LGPLv3. zlib is used under the zlib license (<https://zlib.net/zlib_license.html>). Some game assets are used per Microsoft's Game Content Usage Rules (see `native/src/ReachVariantTool/README.txt` for further attribution).

Prior to September 2021, ReachVariantTool was licensed under [Creative Commons CC-BY-NC 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/) unless otherwise stated (some files were licensed under CC0).

As of September 2021, ReachVariantTool's program code is provided under the GNU General Public License version 3, while ReachVariantTool's documentation (including source XML files, rendered HTML files, and embedded images) is provided under CC0. Some individual source code files from ReachVariantTool are still provided under CC0, with an appropriate notice at the top of each such file. As with any license change, content which has yet to be modified since the change may legally be used under either the previous license(s) or the current one(s), at the user's discretion.
