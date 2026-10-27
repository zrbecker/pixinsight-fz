# FZ module for PixInsight

Read-only support for tile-compressed FITS (`.fits.fz`) in PixInsight 1.9.4. Current version 1.0.1.

PixInsight chooses a file format from the last extension only, so `.fits.fz` finds no reader
("No installed image file format was found for the specified file extension '.fz'"), and its
FITS module cannot decode compressed images even under a `.fits` name ("The FITS file contains
no readable image"). This module registers `.fz` and decodes with CFITSIO. All header keywords are
passed through and rows are oriented the way the FITS module orients them by default, so an image
reads the same as the FITS module reads the uncompressed file. Writing `.fz` is not supported.

Windows x64 only for now. Not made, endorsed or supported by Pleiades Astrophoto.

## Installing

Download `FZ-<version>-win-x64.zip` from [Releases](https://github.com/zrbecker/pixinsight-fz/releases)
and follow the `README.txt` inside (also in [`dist/README.txt`](dist/README.txt)). In short: PixInsight
only loads signed modules, and this one is not signed by a Certified PixInsight Developer, so you sign it
yourself with a *local signing identity* tied to your own PixInsight licence. `SignFZModule.js` does the
signing; you choose the password.

## Layout

| Path | What |
|---|---|
| `module\` | Module source (`FZModule`, `FZFormat`, `FZInstance`). Builds to `module\build\FZ-pxm.dll`. |
| `SignFZModule.js` | Signs the `FZ-pxm.dll` in its own folder with the user's local signing identity; creates the keys file on first run. |
| `dist\README.txt` | End-user instructions shipped in the release zip. |
| `test\` | `fzcheck.c` (the module's CFITSIO calls outside PixInsight) and `make_fixtures.py` (orientation and path test files). |
| `PCL\` | Not in the repo. PCL 2.10.4 and its bundled libraries, copied from your PixInsight install by `build_pcl.cmd`. |
| `cfitsio\`, `download\` | Not in the repo. CFITSIO 4.7.0, downloaded by `build_cfitsio.cmd` from github.com/HEASARC/cfitsio (tag `cfitsio-4.7.0`, sha256 `F281CA292407D8682E83A37EA68FCE6E1383D2449D923647EA62FA0570C59AEE`, checked). |

## Building

Needs PixInsight 1.9.4 installed (for PCL's sources) and Visual Studio 2022 Build Tools (MSVC v143,
Windows SDK, CMake). Run in order:

```
build_pcl.cmd       copies PCL from PixInsight, builds it with zlib, lz4, zstd, lcms, cminpack, RFC6234 (several minutes)
build_cfitsio.cmd   downloads CFITSIO, builds it static, /MD, against PCL's zlib; no curl, no threads
build_module.cmd    module\build\FZ-pxm.dll
```

Set `PIXINSIGHT_DIR` if PixInsight is not in `C:\Program Files\PixInsight`.

The module builds into `module\build`, never into a folder PixInsight has a module installed from:
Windows keeps a loaded DLL locked, so the link would fail.

After a PixInsight update, delete `PCL\`, rebuild all three, and sign again.

## Testing

```
test\build_test.cmd                                  builds test\fzcheck.exe
python test\make_fixtures.py test\fixtures           needs numpy and astropy
test\fzcheck.exe <file.fits.fz>
```

`fzcheck` reads the file into memory, opens it with `fits_open_memfile`, finds the first image HDU,
reads `ROWORDER` and flips bottom-up rows, as `FZInstance.cpp` does, then prints geometry, the header
and pixel checksums. The fixtures are one 64x48 gradient with a bright square near the top-left, stored
as top-down, bottom-up, with `[brackets] (1)` in the name, with an accented name, and as plain FITS.
All `.fz` fixtures must print the same sums and top-row signature (873600). In PixInsight all five
must look identical, with the square top-left.

Verified on a calibrated ASI2600MC frame: decoded pixels match the uncompressed file exactly (count,
sum, sum of squares, first and last values), and the header has the same 81 keywords.

## Signing, installing, releasing

PixInsight only loads signed modules. A local signing identity is tied to the signer's licence, so
each user signs their own copy (see `dist\README.txt`).

Release flow:

1. Bump `MODULE_VERSION_*` and the release date in `module\FZModule.cpp`, run `build_module.cmd`.
2. Make `release\FZ-<version>-win-x64\` with `module\build\FZ-pxm.dll`, `SignFZModule.js`,
   `dist\README.txt`, `LICENSE` (as `LICENSE.txt`) and `THIRD-PARTY-NOTICES.txt`, zip it, and attach
   the zip to a GitHub release. Never ship an `.xsgn`: it would be the packager's signature, valid only
   under their licence.
3. To use it yourself, sign inside the release folder: SCRIPT > Execute Script File... >
   `SignFZModule.js` (asks for the existing keys password), then PROCESS > Modules > Install Modules...
   on that folder.

Any rebuild invalidates the signature; sign again after every build.

## Licensing

The module's code is MIT. The DLL statically links CFITSIO (NASA), PCL (PCL License 2.0) and PCL's
bundled zlib, Little CMS, LZ4, Zstandard, MINPACK and RFC 6234 code; their notices are in
[`THIRD-PARTY-NOTICES.txt`](THIRD-PARTY-NOTICES.txt). None of their source is in this repository.
PCL License 2.0 requires an acknowledgement that the product is based on PixInsight project software,
forbids using Pleiades' names for endorsement, and forbids use of PCL source for training machine
learning systems or for API services for automated code generation. This module was written with an
AI coding assistant working from the PCL headers; whether that fits the PCL licence has not been
confirmed with Pleiades.

## Notes

- CFITSIO is built without thread support and shares buffers between open files, so the module
  serialises all CFITSIO calls behind one lock. Batch tools that read many `.fz` files at once
  decode them one at a time.
- The whole file is read into memory with PCL's file API and decoded from there, so Unicode paths
  work and CFITSIO never parses the file name (`[`, `]`, `(` would otherwise be filter syntax).
- Integer data is delivered as integers (8, 16, 32-bit unsigned). Floating-point data outside
  [0,1] and signed integers are rescaled to [0,1].
- Orientation: top-down unless `ROWORDER = 'BOTTOM-UP'`; the FITS format hints `bottom-up`,
  `top-down`/`up-bottom`, `use-roworder-keywords` and `ignore-roworder-keywords` are honoured. The
  FITS module's own preferences are not read, so a user who changed them there gets the defaults here.
- Not yet tested: WBPP and ImageIntegration runs on `.fz` lights.
