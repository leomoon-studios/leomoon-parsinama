# لئومون پارسی‌نما

[English](README.md)

لئومون پارسی‌نما برنامه‌ای برای خواندن شعر فارسی در رایانه و اندروید است. این برنامه داده‌های شعر گنجور را که در همین مخزن قرار دارند به یک پایگاه دادهٔ محلی SQLite تبدیل می‌کند تا بتوانید بدون اتصال به اینترنت، شاعران و مجموعه‌ها را مرور کنید، شعر بخوانید و در متن شعرها جستجو کنید.

رابط برنامه فارسی و راست‌به‌چپ است. بیت‌ها و بخش‌های هر شعر به ترتیب دادهٔ اصلی نمایش داده می‌شوند. می‌توانید با مسیرهای بالای صفحه میان شاعر و مجموعه‌های زیرمجموعه جابه‌جا شوید، شعر قبلی یا بعدی همان مجموعه را باز کنید و نشانک بگذارید. در نسخهٔ رومیزی، چاپ شعر و صدور PDF نیز ممکن است. پوستهٔ روشن و تیره، رنگ تأکید و اندازهٔ متن خواندن نیز قابل تنظیم هستند.

## ساخت و اجرا

برای ساخت برنامه به CMake نسخهٔ ۳٫۲۱ یا جدیدتر، کامپایلر C++17 و Qt نسخهٔ ۶٫۵ یا جدیدتر نیاز دارید. مؤلفه‌های موردنیاز Qt عبارت‌اند از Core، Concurrent، Gui، Qml، Quick، Quick Controls 2، SQL به‌همراه راه‌انداز SQLite، Print Support، Test و Widgets. داده‌های شعر و قلم‌های مورد استفادهٔ برنامه در همین مخزن قرار دارند.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
./build/leomoon-parsinama
```

هنگام ساخت، فایل `build/parsinama-catalog.sqlite` از پوشهٔ `data/` تولید می‌شود. داده‌های اولیه حدود ۲٫۴ گیگابایت و پایگاه دادهٔ تولیدشده حدود ۳٫۵ گیگابایت فضا می‌گیرند؛ بنابراین برای نخستین ساخت، فضای دیسک و زمان کافی در نظر بگیرید. پایگاه داده بخشی از داده‌های برنامه است و تنظیمات و نشانک‌ها جداگانه ذخیره می‌شوند تا با بازسازی آن از بین نروند.

برای اجرای آزمون‌ها از `ctest --test-dir build --output-on-failure` استفاده کنید. دستور `./build/leomoon-parsinama --smoke-test` نیز یک بررسی سطح برنامه انجام می‌دهد. برای باز کردن پایگاه داده‌ای دیگر می‌توانید گزینهٔ `--catalog /path/to/parsinama-catalog.sqlite` را به برنامه بدهید.

## ساخت محلی اندروید در لینوکس

برای ساخت APK به JDK 17 و [ابزارهای خط فرمان Android SDK](https://developer.android.com/studio#command-line-tools-only) نیاز دارید؛ فایل `sdkmanager` باید در مسیر `~/Android/Sdk/cmdline-tools/latest/bin/sdkmanager` باشد. سپس پلتفرم SDK نسخهٔ 35، ابزارهای ساخت نسخهٔ 36.0.0، platform tools و NDK نسخهٔ 26.1.10909125 را نصب کنید. همچنین Qt 6.8.3 را هم برای میزبان لینوکس و هم برای Android arm64 نصب کنید. اگر بسته‌های Qt نصب نیستند، دستورهای زیر آن‌ها را در `~/Qt` نصب می‌کنند:

```sh
python3 -m venv /tmp/parsinama-aqt-venv
/tmp/parsinama-aqt-venv/bin/pip install aqtinstall
/tmp/parsinama-aqt-venv/bin/aqt install-qt linux desktop 6.8.3 linux_gcc_64 --outputdir "$HOME/Qt"
/tmp/parsinama-aqt-venv/bin/aqt install-qt all_os android 6.8.3 android_arm64_v8a --outputdir "$HOME/Qt"
"$HOME/Android/Sdk/cmdline-tools/latest/bin/sdkmanager" --sdk_root="$HOME/Android/Sdk" 'platforms;android-35' 'build-tools;36.0.0' 'platform-tools' 'ndk;26.1.10909125'
```

ابتدا طبق دستورهای بخش قبل، پایگاه داده را با ساخت رومیزی تولید کنید. سپس از ریشهٔ مخزن، برنامهٔ اندروید را با همان پایگاه داده پیکربندی و کامپایل کنید:

```sh
"$HOME/Qt/6.8.3/android_arm64_v8a/bin/qt-cmake" -S . -B build-android-local -G Ninja \
  -DQT_HOST_PATH="$HOME/Qt/6.8.3/gcc_64" \
  -DANDROID_SDK_ROOT="$HOME/Android/Sdk" \
  -DANDROID_NDK_ROOT="$HOME/Android/Sdk/ndk/26.1.10909125" \
  -DPARSINAMA_HOST_CATALOG="$PWD/build/parsinama-catalog.sqlite" \
  -DBUILD_TESTING=OFF
cmake --build build-android-local --target leomoon_parsinama --parallel 2
```

هنگام بسته‌بندی، پلتفرم API 35 را صریحاً مشخص کنید تا Qt پلتفرم آزمایشی جدیدتری را که Android Studio نصب کرده انتخاب نکند. حافظهٔ بیشتر Gradle برای فشرده‌سازی پایگاه دادهٔ همراه برنامه لازم است:

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

پیش از استفاده از ADB، USB debugging را در گوشی فعال کنید. پس از هر تغییر در کد، مراحل ساخت، کپی کتابخانهٔ بومی، بسته‌بندی و نصب را دوباره اجرا کنید. کپی کتابخانه مطمئن می‌کند که پوشهٔ بسته‌بندی قبلی از نسخهٔ جدید برنامه استفاده کند. APK پایگاه دادهٔ آفلاین را در خود دارد و در نخستین اجرا حدود ۳٫۷ گیگابایت داده را به فضای خصوصی برنامه کپی می‌کند. اگر APK نصب‌شده با کلید آزمایشی دیگری امضا شده باشد، نسخهٔ محلی نمی‌تواند آن را به‌روزرسانی کند؛ حذف نسخهٔ قبلی، تنظیمات و نشانک‌های محلی آن را نیز حذف می‌کند.

## خواندن شعر و تنظیمات

از نوار کناری سمت راست شاعری را انتخاب کنید و سپس مجموعه یا شعر دلخواه را باز کنید. مسیرهای بالای محتوا شما را به شاعر یا هر مجموعهٔ والد بازمی‌گردانند. در صفحهٔ یک شعر، دکمه‌های قبلی و بعدی میان شعرهای همان مجموعه حرکت می‌کنند. نوار ابزار نیز دسترسی به جستجو، نشانک‌ها، چاپ و تنظیمات را فراهم می‌کند.

برنامه فایل‌های `settings.json` و `bookmarks.json` را در پوشهٔ تنظیمات سیستم‌عامل، زیر پوشهٔ `leomoon-parsinama/`، می‌سازد. این مسیر در لینوکس معمولاً `~/.config/leomoon-parsinama/`، در macOS مسیر `~/Library/Preferences/leomoon-parsinama/` و در ویندوز مسیر `%LOCALAPPDATA%\leomoon-parsinama\` است. در لینوکس، اگر `XDG_CONFIG_HOME` تنظیم شده باشد، برنامه از آن پیروی می‌کند.

## ساخت بسته‌ها

گردش‌کار **Package test builds** در GitHub Actions را می‌توان به‌صورت دستی برای لینوکس، ویندوز، macOS یا همهٔ آن‌ها اجرا کرد. برای لینوکس و ویندوز، معماری‌های x64 و ARM64 قابل انتخاب‌اند؛ بستهٔ macOS به‌صورت universal ساخته می‌شود. فرستادن برچسب نسخه‌ای مانند `v0.1.0`، در صورتی که با `metadata/VERSION` یکسان باشد، ساخت همهٔ بسته‌ها را آغاز می‌کند. گردش‌کارها فایل‌های آزمایشی را بارگذاری می‌کنند و GitHub Release منتشر نمی‌کنند.

## منبع داده‌ها و سپاسگزاری

داده‌های شعر از مخزن [ganjoor/ganjoor-data](https://github.com/ganjoor/ganjoor-data) در کامیت `a64968e78425b2e8c7904fbdf5289fba8251a757` گرفته شده‌اند. فایل فهرست داده‌ها شامل ۲۴۰ شاعر و ۱۳۵٬۳۱۹ شعر است. برای شناسه‌ها و اثرانگشت‌های منابع به [SOURCES.md](SOURCES.md) و برای اطلاعات حقوقی داده‌ها و قلم‌ها به [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) مراجعه کنید.

برای این خروجی داده، مجوز صریحی ارائه نشده است. پیش از انتشار بسته‌های باینری حاوی پایگاه دادهٔ کامل، شرایط بازنشر آن باید روشن شود.
