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
   // PixInsight matches on the last extension only, so ".fz" is what selects us.
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
