# Android release signing

The regular `android.yml` workflow and the Android job in `release.yml` upload a test APK signed with the Android debug key. Use `.github/workflows/android-sign.yml` when a distributable APK is needed. This separate manual workflow builds the catalog and APK, then signs a release APK using repository secrets. It uploads the signed APK and `SHA256SUMS` as a separate artifact.

Set these GitHub Actions repository secrets before running the signing workflow:

| Secret | Value |
| --- | --- |
| `ANDROID_KEYSTORE_BASE64` | Base64 encoding of the release keystore file, with no line breaks |
| `ANDROID_KEYSTORE_ALIAS` | Alias of the signing key in that keystore |
| `ANDROID_KEYSTORE_STORE_PASS` | Keystore password |
| `ANDROID_KEYSTORE_KEY_PASS` | Key password |

Run `gh workflow run android-sign.yml --ref master -f version=0.1.0` after the version in `metadata/VERSION` matches the requested version. Download the `leomoon-parsinama-android-arm64-signed-*` artifact from that run and verify it with `sha256sum --check SHA256SUMS`. Keep the keystore and passwords outside the repository. Retain the same signing key for future updates so Android can install a new APK over an existing one without removing app data.
