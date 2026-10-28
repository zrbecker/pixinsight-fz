#include "FZFormat.h"
#include "FZInstance.h"

namespace pcl
{

IsoString FZFormat::Name() const
{
   return "FZ";
}

StringList FZFormat::FileExtensions() const
{
   // PixInsight first tries everything after the first dot of the name, then the last
   // extension. "image_180.00s.fits.fz" falls through to ".fz" and reaches us, but
   // "image.fits.fz" asks for ".fits.fz", which the FITS module claims (without being able
   // to decode it). Claiming ".fits.fz" here too is not possible: PixInsight refuses to
   // install a second format for an extension that is already registered.
   return StringList() << ".fz";
}

uint32 FZFormat::Version() const
{
   return 0x100;
}

String FZFormat::Description() const
{
   return "Tile-compressed FITS (read-only)";
}

String FZFormat::Implementation() const
{
   return "<html>"
          "<p>Reads tile-compressed FITS images (.fits.fz, as written by fpack, Siril and "
          "CFITSIO) through the CFITSIO library. The first image in the file is read. "
          "Every header keyword is passed through. Rows are top-down unless the file's "
          "ROWORDER keyword says BOTTOM-UP, and the FITS module's orientation hints "
          "(bottom-up, top-down, use-roworder-keywords, ignore-roworder-keywords) are "
          "honoured, so a decompressed image matches what the FITS module reads from the "
          "same data stored uncompressed with its default preferences.</p>"
          "<p>Read-only: this format cannot write files.</p>"
          "</html>";
}

FileFormatImplementation* FZFormat::Create() const
{
   return new FZInstance( this );
}

} // pcl
