# Source data archive

The files `data.z01` onward and `data.zip` form one split ZIP containing the complete `data/` tree. `manifest.json` records the size and SHA-256 hash of each part. Keep all parts together.

To regenerate the archive, prepare the updated export as a directory named `data` and install Info-ZIP `zip` and Python 3.11 or newer. Run `bash scripts/create-data-archive.sh /path/to/parent` from the repository root, replacing the argument with the parent of that `data` directory. On Windows, run the command in Git Bash with both tools on `PATH`. Review and commit all archive parts and `manifest.json` together.

To reconstruct and extract it, use Info-ZIP `zip` and `unzip` in a terminal on Linux, macOS, or Git Bash on Windows:

```sh
zip -s 0 data-archive/data.zip --out /path/to/recombined.zip
unzip /path/to/recombined.zip -d /path/to/extraction-directory
```

The extracted directory contains `data/manifest.json` and the rest of the source tree. Choose an output directory outside the existing `data/` tree. Recombining needs space for an additional full ZIP, and extraction needs space for all source files.
