PDFxTMD cross-platform Python wheel fixes
=========================================

This archive contains COMPLETE replacement versions of the three files changed
by the wheel fix. It is intentionally not a git patch, so it does not depend on
your local commit history or exact previous file contents.

Files:
  .github/workflows/python-wheels.yml
  setup.py
  wrappers/python/CMakeLists.txt

From the root of your local PDFxTMD checkout, after extracting this archive next
to the repository, you can copy the files with:

  cp -a PDFxTMD-wheel-fix-full-replacements/.github/workflows/python-wheels.yml .github/workflows/python-wheels.yml
  cp -a PDFxTMD-wheel-fix-full-replacements/setup.py setup.py
  cp -a PDFxTMD-wheel-fix-full-replacements/wrappers/python/CMakeLists.txt wrappers/python/CMakeLists.txt

Then inspect:

  git diff -- .github/workflows/python-wheels.yml setup.py wrappers/python/CMakeLists.txt
  git diff --check

The workflow fixes:
  - invalid job-level runner.temp usage
  - retired macOS 13 runner
  - macOS deployment target for std::filesystem (10.15+)
  - vcpkg bootstrap and target triplets on Linux/macOS/Windows
  - Windows static native vcpkg libraries with dynamic MSVC runtime
  - Windows CMake/Python extension output placement
  - explicit verification that pdfxtmd*.pyd/.so exists before wheel packaging
  - post-install wheel import and DPD capability test
