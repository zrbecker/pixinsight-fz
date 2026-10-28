FZ module 1.0.1 for PixInsight (Windows x64)
=============================================

Opens tile-compressed FITS files (.fits.fz, as written by fpack, Siril, astropy and
CFITSIO) in PixInsight 1.9.4. Without it PixInsight reports "No installed image file
format was found for the specified file extension '.fz'".

Once installed, .fits.fz files open wherever PixInsight opens image files (File > Open,
Debayer, ImageContainer), with one naming limitation (see Known limitation below).
WBPP and ImageIntegration have not been tested with it yet.
The module only reads; it cannot save .fz.


Known limitation: files named NAME.fits.fz with no other dot
------------------------------------------------------------
PixInsight picks a reader from everything after the FIRST dot in the file name, and
only falls back to the last extension (.fz) when nothing matches. Its built-in FITS
module claims ".fits.fz" but cannot decode compressed images, so:

   LIGHT_Ha_300.00s_0001.fits.fz   opens   (".00s_0001.fits.fz" matches nothing, so ".fz")
   M31.v2.fits.fz                  opens   (".v2.fits.fz" matches nothing, so ".fz")
   M31_0001.fits.fz                FAILS   (".fits.fz" goes to the FITS module:
                                            "The FITS file contains no readable image")

This module cannot also claim ".fits.fz": PixInsight refuses to install a second
format for an extension that is already registered. Workaround for affected files:
add a dot before ".fits.fz" (for example M31_0001.c.fits.fz) or rename them to .fz.
Only Pleiades can fix this properly, in the FITS module.


Why you have to sign it yourself
--------------------------------
PixInsight only loads signed modules, and a module signed by someone else is only
accepted if that person is a Certified PixInsight Developer. This one is not. Instead,
PixInsight lets you sign modules for your own use with a "local signing identity" tied
to your licence. The signature works only on machines where your licence is active.
You are vouching for this DLL on your machine, so only install it if you trust where
you got it.


Installing
----------
1. Unzip this folder somewhere permanent, for example
   C:\Users\<you>\PixInsight-modules\FZ-1.0.1-win-x64
   Keep FZ-pxm.dll and SignFZModule.js together. PixInsight loads the module from
   this folder every time it starts, so don't delete or move it afterwards.

2. In PixInsight: SCRIPT > Execute Script File..., choose SignFZModule.js.
   - First time: choose a password (8+ characters, upper and lower case letters,
     a digit or punctuation mark, no spaces). Write it down. The script creates your
     signing keys at C:\Users\<you>\PixInsight-signing\local-signing.xssk.
     Keep that file private: it holds your signing key.
   - If you already have that keys file, it asks for its password instead.
   The script writes FZ-pxm.xsgn (the signature) next to the DLL.

3. First time only: EDIT > Local Signing Identity...
   (if it is not under EDIT, look under RESOURCES)
   - Keys file: C:\Users\<you>\PixInsight-signing\local-signing.xssk
   - Enter the same password.
   - Tick "Make the local signing identity persistent", then OK.

4. PROCESS > Modules > Install Modules...
   Click Search, choose this folder, make sure FZ-pxm.dll is ticked, Install.

5. Test: File > Open a .fits.fz file whose name has another dot in it (see Known
   limitation above).


Updating to a new version
-------------------------
1. Close PixInsight.
2. Replace FZ-pxm.dll in your module folder with the new one. Delete the old
   FZ-pxm.xsgn: a signature only matches the exact DLL it was made for.
3. Start PixInsight. It will refuse the unsigned module; that is expected.
4. Run SignFZModule.js again (step 2 above, it asks for your existing password),
   then restart PixInsight.


How it reads files
------------------
- The first image in the file is read. 2D images and 3-plane colour cubes are
  supported; 8, 16 and 32-bit unsigned integers stay integers, floating-point data
  outside 0..1 and signed integers are rescaled to 0..1.
- All header keywords are passed through, so BAYERPAT and similar keywords work
  as they do with plain FITS files (Debayer "Auto" picks up the pattern).
- Rows are top-down unless the file's ROWORDER keyword says BOTTOM-UP, the same as
  PixInsight's FITS module with its default preferences. The module does not read
  the FITS module's preferences; it follows the FITS orientation hints
  (bottom-up, top-down, use-roworder-keywords, ignore-roworder-keywords) if given.
- Decoding is done one file at a time, so batch processes reading many .fz files
  are slower than with plain FITS.


Licences
--------
The module's own code is MIT licensed (LICENSE.txt). FZ-pxm.dll also contains
CFITSIO, the PixInsight Class Library and other libraries; see
THIRD-PARTY-NOTICES.txt. This product is based on software from the PixInsight
project, developed by Pleiades Astrophoto and its contributors. It is not made,
endorsed or supported by Pleiades Astrophoto.
