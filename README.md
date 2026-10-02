# LeoMoon ParsiNama

[پارسی](README.fa.md)

LeoMoon ParsiNama (لئومون پارسی‌نما) is a desktop reader for Persian poetry. It turns the Ganjoor poetry export in this repository into a local SQLite catalog, so you can browse poets and collections, read poems, and search their text without a network connection.

The interface is in Persian and reads right to left. Poems show their verses in source order, including paired lines where the source marks a couplet. You can follow breadcrumbs through a poet's collections, move between poems in the same collection, save bookmarks, and print a poem or export it to PDF. Settings include light and dark themes, accent colors, and reading text size.

## Build and run

You need CMake 3.21 or newer, a C++17 compiler, and Qt 6.5 or newer with Core, Concurrent, Gui, Qml, Quick, Quick Controls 2, SQL with the SQLite driver, Print Support, Test, and Widgets. The repository contains the poetry export and the fonts used by the app.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
./build/leomoon-parsinama
```

The build generates `build/parsinama-catalog.sqlite` from `data/` before building the app. The source export occupies about 2.4 GB and the generated catalog about 3.5 GB, so allow additional disk space and time for the first build. The catalog is application data; bookmarks and settings are stored separately and remain available when the catalog is rebuilt.

To check the build, run `ctest --test-dir build --output-on-failure`. You can also run `./build/leomoon-parsinama --smoke-test` for an app-level check, or pass `--catalog /path/to/parsinama-catalog.sqlite` to open another catalog.

## Reading and settings

Choose a poet in the right sidebar, then open a collection or poem. Breadcrumbs above the content return to the poet or any parent collection. On an individual poem, the Previous and Next buttons move through that collection's poems. The toolbar opens search, bookmarks, printing, and settings.

The app creates `settings.json` and `bookmarks.json` in the platform configuration directory under `leomoon-parsinama/`. On Linux this is normally `~/.config/leomoon-parsinama/`; on macOS, `~/Library/Preferences/leomoon-parsinama/`; and on Windows, `%LOCALAPPDATA%\leomoon-parsinama\`. The Linux path follows `XDG_CONFIG_HOME` when it is set.

## Package builds

The **Package test builds** GitHub Actions workflow can be started manually for Linux, Windows, macOS, or all three. Linux and Windows offer x64 and ARM64 targets, while macOS builds a universal app. A pushed version tag such as `v0.1.0` starts all package builds when it matches `metadata/VERSION`. The workflows upload test artifacts; they do not publish a GitHub Release.

## Content and attribution

The poetry export comes from [ganjoor/ganjoor-data](https://github.com/ganjoor/ganjoor-data) at commit `a64968e78425b2e8c7904fbdf5289fba8251a757`. Its manifest records 240 poets and 135,319 poems. See [SOURCES.md](SOURCES.md) for source digests and [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for font and data notices.

The export did not include an explicit data license. Confirm its redistribution terms before publishing binary packages that contain the complete catalog.
