#ifndef __FZFormat_h
#define __FZFormat_h

#include <pcl/MetaFileFormat.h>

namespace pcl
{

// Read-only. Writing .fz is left to the tools that already verify it byte for byte.
class FZFormat : public MetaFileFormat
{
public:

   FZFormat() = default;

   IsoString Name() const override;
   StringList FileExtensions() const override;
   uint32 Version() const override;
   String Description() const override;
   String Implementation() const override;

   bool CanRead() const override              { return true; }
   bool CanWrite() const override             { return false; }
   bool CanStore8Bit() const override         { return true; }
   bool CanStore16Bit() const override        { return true; }
   bool CanStore32Bit() const override        { return true; }
   bool CanStoreFloat() const override        { return true; }
   bool CanStoreDouble() const override       { return true; }
   bool CanStoreGrayscale() const override    { return true; }
   bool CanStoreRGBColor() const override     { return true; }
   bool CanStoreAlphaChannels() const override { return false; }
   bool CanStoreKeywords() const override     { return true; }
   bool SupportsCompression() const override  { return true; }

   FileFormatImplementation* Create() const override;
};

} // pcl

#endif   // __FZFormat_h
