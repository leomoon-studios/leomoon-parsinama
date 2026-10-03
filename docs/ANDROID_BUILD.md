# Android catalog build

Build the poetry catalog with native host tools before configuring the Android application. The Android cross-build accepts the finished SQLite file as an input and does not run an Android executable on the build host.

Use desktop Qt 6.8.3, CMake, Ninja, and the repository's `data/` export for the host build:

```sh
cmake -S . -B build-host -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build-host --target parsinama_catalog --parallel 2
```

This builds `src/tools/build_catalog.cpp` as a host executable and generates `build-host/parsinama-catalog.sqlite` from `data/`. The full export and catalog require several gigabytes of free space.

With Qt for Android 6.8.3, the matching host Qt installation, Android SDK, and NDK 26.1.10909125 installed, configure the arm64 app using the Qt for Android `qt-cmake` wrapper:

```sh
"$QT_ANDROID_ROOT/bin/qt-cmake" -S . -B build-android -G Ninja \
  -DQT_HOST_PATH="$QT_HOST_ROOT" \
  -DANDROID_SDK_ROOT="$ANDROID_SDK_ROOT" \
  -DANDROID_NDK_ROOT="$ANDROID_NDK_ROOT" \
  -DPARSINAMA_HOST_CATALOG="$PWD/build-host/parsinama-catalog.sqlite" \
  -DBUILD_TESTING=OFF
cmake --build build-android --target leomoon_parsinama --parallel 2
cmake --build build-android --target apk --parallel 2
```

`PARSINAMA_HOST_CATALOG` is required for Android configuration. CMake checks that the path exists and has a SQLite file header, computes its SHA-256 checksum, and stages a link to the catalog under the generated Android package source directory. Qt's APK target copies the catalog into `assets/parsinama-catalog.sqlite` and bundles its checksum in `assets/parsinama-catalog.sha256`. The desktop `parsinama_catalog` target and desktop install paths are unchanged.

The catalog stays inside the APK at install time. On first launch, the app copies it to app-private storage so SQLite can open it. The screen shows copy progress and reports when more free space is needed. The copy uses a temporary file and verifies SHA-256 before replacing the installed catalog. Later launches skip copying when the stored checksum and file size match; an app update with a different catalog checksum replaces the old copy. The device needs free space for the extracted catalog in addition to the installed APK.
