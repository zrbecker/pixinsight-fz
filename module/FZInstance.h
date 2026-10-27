#ifndef __FZInstance_h
#define __FZInstance_h

#include <pcl/ByteArray.h>
#include <pcl/FileFormatImplementation.h>
#include <pcl/Image.h>

#include <fitsio.h>  // fitsfile is a typedef there, so it cannot be forward-declared

namespace pcl
{

class FZFormat;

class FZInstance : public FileFormatImplementation
{
public:

   FZInstance( const FZFormat* f );
   ~FZInstance() override;

   ImageDescriptionArray Open( const String& filePath, const IsoString& hints ) override;
   bool IsOpen() const override;
   String FilePath() const override;
   void Close() override;
   String ImageFormatInfo() const override;

   FITSKeywordArray ReadFITSKeywords() override;

   void ReadImage( Image& image ) override;
   void ReadImage( DImage& image ) override;
   void ReadImage( UInt8Image& image ) override;
   void ReadImage( UInt16Image& image ) override;
   void ReadImage( UInt32Image& image ) override;

private:

   // How the stored samples become a PixInsight image.
   enum class Sample { UInt8, UInt16, UInt32, Float, Double, SignedRescaled, Int64Rescaled };

   fitsfile* m_file = nullptr;
   String    m_path;
   ByteArray m_buffer;            // the whole file; CFITSIO decodes from memory
   void*     m_memPtr = nullptr;  // CFITSIO keeps pointers to these two
   size_t    m_memSize = 0;
   bool      m_bottomUp = false;  // rows are flipped to top-down while reading
   int       m_width = 0, m_height = 0, m_channels = 0;
   int       m_naxis = 0;
   int       m_equivType = 0;   // CFITSIO image type after BZERO/BSCALE (e.g. USHORT_IMG)
   Sample    m_sample = Sample::UInt16;

   template <class I> void ReadInto( I& image );
   template <class P> void ReadPlanes( GenericImage<P>& image, int dataType );
};

} // pcl

#endif   // __FZInstance_h
