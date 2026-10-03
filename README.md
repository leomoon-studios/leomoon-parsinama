# LeoMoon ParsiNama

[پارسی](README.fa.md)

LeoMoon ParsiNama (لئومون پارسی‌نما) is a desktop and Android reader for Persian poetry. It turns the Ganjoor poetry export in this repository into a local SQLite catalog, so you can browse poets and collections, read poems, and search their text without a network connection.

The interface is in Persian and reads right to left. Poems show their verses in source order, including paired lines where the source marks a couplet. You can follow breadcrumbs through a poet's collections, move between poems in the same collection, and save bookmarks. Desktop builds can also print a poem or export it to PDF. Settings include light and dark themes, accent colors, and reading text size.

## Build and run

You need CMake 3.21 or newer, a C++17 compiler, and Qt 6.5 or newer with Core, Concurrent, Gui, Qml, Quick, Quick Controls 2, SQL with the SQLite driver, Print Support, Test, and Widgets. The repository contains the poetry export and the fonts used by the app.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
./build/leomoon-parsinama
```

The build generates `build/parsinama-catalog.sqlite` from `data/` before building the app. The source export occupies about 2.4 GB and the generated catalog about 3.5 GB, so allow additional disk space and time for the first build. The catalog is application data; bookmarks and settings are stored separately and remain available when the catalog is rebuilt.

To check the build, run `ctest --test-dir build --output-on-failure`. You can also run `./build/leomoon-parsinama --smoke-test` for an app-level check, or pass `--catalog /path/to/parsinama-catalog.sqlite` to open another catalog.

## Local Android build on Linux

Install JDK 17 and the [Android SDK command-line tools](https://developer.android.com/studio#command-line-tools-only), with `sdkmanager` at `~/Android/Sdk/cmdline-tools/latest/bin/sdkmanager`. Then install SDK platform 35, build tools 36.0.0, platform tools, and NDK 26.1.10909125. Install Qt 6.8.3 for both the Linux desktop host and Android arm64. These commands install the matching Qt packages in `~/Qt` if they are missing:

```sh
python3 -m venv /tmp/parsinama-aqt-venv
/tmp/parsinama-aqt-venv/bin/pip install aqtinstall
/tmp/parsinama-aqt-venv/bin/aqt install-qt linux desktop 6.8.3 linux_gcc_64 --outputdir "$HOME/Qt"
/tmp/parsinama-aqt-venv/bin/aqt install-qt all_os android 6.8.3 android_arm64_v8a --outputdir "$HOME/Qt"
"$HOME/Android/Sdk/cmdline-tools/latest/bin/sdkmanager" --sdk_root="$HOME/Android/Sdk" 'platforms;android-35' 'build-tools;36.0.0' 'platform-tools' 'ndk;26.1.10909125'
```

First build the desktop catalog as shown above. From the repository root, configure and compile the Android app using that catalog:

```sh
"$HOME/Qt/6.8.3/android_arm64_v8a/bin/qt-cmake" -S . -B build-android-local -G Ninja \
  -DQT_HOST_PATH="$HOME/Qt/6.8.3/gcc_64" \
  -DANDROID_SDK_ROOT="$HOME/Android/Sdk" \
  -DANDROID_NDK_ROOT="$HOME/Android/Sdk/ndk/26.1.10909125" \
  -DPARSINAMA_HOST_CATALOG="$PWD/build/parsinama-catalog.sqlite" \
  -DBUILD_TESTING=OFF
cmake --build build-android-local --target leomoon_parsinama --parallel 2
```

Package with API 35 explicitly. This avoids selecting a newer preview platform if Android Studio installed one. The larger Gradle heap is needed to compress the bundled catalog:

```sh
mkdir -p build-android-local/android-build/libs/arm64-v8a
cp build-android-local/libleomoon-parsinama_arm64-v8a.so build-android-local/android-build/libs/arm64-v8a/
GRADLE_OPTS='-Dorg.gradle.jvmargs=-Xmx8g -Dorg.gradle.workers.max=2' \
  "$HOME/Qt/6.8.3/gcc_64/bin/androiddeployqt" \
  --input build-android-local/android-leomoon_parsinama-deployment-settings.json \
  --output build-android-local/android-build \
  --apk build-android-local/android-build/leomoon_parsinama.apk \
  --android-platform android-35
adb devices -l
adb install -r build-android-local/android-build/leomoon_parsinama.apk
```

Enable USB debugging on the phone before using ADB. For subsequent code changes, repeat the build, native library copy, package, and install commands. The copy ensures that a reused Android packaging directory contains the newly compiled app. The APK contains the offline catalog, and its first launch copies roughly 3.7 GB into app-private storage. An APK signed with a different debug key cannot update an existing installation; removing that installation also removes its local settings and bookmarks.

## Reading and settings

Choose a poet in the right sidebar, then open a collection or poem. Breadcrumbs above the content return to the poet or any parent collection. On an individual poem, the Previous and Next buttons move through that collection's poems. The toolbar opens search, bookmarks, printing, and settings.

The app creates `settings.json` and `bookmarks.json` in the platform configuration directory under `leomoon-parsinama/`. On Linux this is normally `~/.config/leomoon-parsinama/`; on macOS, `~/Library/Preferences/leomoon-parsinama/`; and on Windows, `%LOCALAPPDATA%\leomoon-parsinama\`. The Linux path follows `XDG_CONFIG_HOME` when it is set.

## Package builds

The **Package test builds** GitHub Actions workflow can be started manually for Linux, Windows, macOS, or all three. Linux and Windows offer x64 and ARM64 targets, while macOS builds a universal app. A pushed version tag such as `v0.1.0` starts all package builds when it matches `metadata/VERSION`. The workflows upload test artifacts; they do not publish a GitHub Release.

## Content and attribution

The poetry export comes from [ganjoor/ganjoor-data](https://github.com/ganjoor/ganjoor-data) at commit `a64968e78425b2e8c7904fbdf5289fba8251a757`. Its manifest records 240 poets and 135,319 poems. See [SOURCES.md](SOURCES.md) for source digests and [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for font and data notices.

The export did not include an explicit data license. Confirm its redistribution terms before publishing binary packages that contain the complete catalog.
