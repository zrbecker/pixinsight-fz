#include "FZInstance.h"
#include "FZFormat.h"

#include <pcl/ErrorHandler.h>
#include <pcl/File.h>
#include <pcl/Mutex.h>

namespace pcl
{

// CFITSIO is built without thread support here and shares I/O buffers between all
// open files, so every call into it goes through this one lock. PixInsight may read
// several images at once (ImageIntegration, batch processes); they decode in turn.
static Mutex s_cfitsio;

static String CFITSIOError( int status, const char* what )
{
   char text[ FLEN_STATUS ] = {};
   fits_get_errstatus( status, text );
   char detail[ FLEN_ERRMSG ] = {};
   IsoString stack;
   while ( fits_read_errmsg( detail ) )
      stack << '\n' << detail;
   return String( what ) + ": " + String( text ) + String( stack );
}

#define CHECK( call, what ) \
   if ( (call) != 0 ) throw Error( CFITSIOError( status, what ) )

// ----------------------------------------------------------------------------

FZInstance::FZInstance( const FZFormat* f )
   : FileFormatImplementation( f )
{
}

FZInstance::~FZInstance()
{
   Close();
}

bool FZInstance::IsOpen() const
{
   return m_file != nullptr;
}

String FZInstance::FilePath() const
{
   return m_path;
}

void FZInstance::Close()
{
   if ( m_file != nullptr )
   {
      volatile AutoLock lock( s_cfitsio );
      int status = 0;
      fits_close_file( m_file, &status );
      m_file = nullptr;
   }
   m_buffer.Clear();
   m_memPtr = nullptr;
   m_memSize = 0;
}

// Same vocabulary and defaults as PixInsight's FITS module: top-down unless the file
// says otherwise in ROWORDER, and format hints can override either.
static bool BottomUp( fitsfile* f, const IsoString& hints )
{
   bool bottomUp = false, useRowOrder = true;
   IsoStringList tokens;
   hints.Break( tokens, ' ', true );
   for ( const IsoString& h : tokens )
      if ( h == "bottom-up" )
         bottomUp = true;
      else if ( h == "up-bottom" || h == "top-down" )
         bottomUp = false;
      else if ( h == "use-roworder-keywords" )
         useRowOrder = true;
      else if ( h == "ignore-roworder-keywords" )
         useRowOrder = false;
   if ( useRowOrder )
   {
      char value[ FLEN_VALUE ] = {};
      int status = 0;
      if ( fits_read_key( f, TSTRING, "ROWORDER", value, nullptr, &status ) == 0 )
      {
         IsoString v = IsoString( value ).Trimmed().Uppercase();
         if ( v == "BOTTOM-UP" )
            bottomUp = true;
         else if ( v == "TOP-DOWN" )
            bottomUp = false;
      }
   }
   return bottomUp;
}

ImageDescriptionArray FZInstance::Open( const String& filePath, const IsoString& hints )
{
   Close();
   // The whole file is read into memory and CFITSIO decodes from there. PCL's file I/O
   // handles any Unicode path, and CFITSIO never sees the file name, so characters it
   // would otherwise parse as filter syntax ([, ], () cannot break the open.
   m_buffer = File::ReadFile( filePath );
   m_memPtr = m_buffer.Begin();
   m_memSize = m_buffer.Size();

   volatile AutoLock lock( s_cfitsio );
   int status = 0;
   CHECK( fits_open_memfile( &m_file, "image.fits.fz", READONLY, &m_memPtr, &m_memSize, 0, nullptr, &status ),
          "Unable to open file" );
   m_path = filePath;

   // Move to the first HDU holding an image. In a .fz file that is the compressed-image
   // extension after the empty primary placeholder.
   int hdus = 0;
   CHECK( fits_get_num_hdus( m_file, &hdus, &status ), "Cannot read the file structure" );
   for ( int hdu = 1; ; ++hdu )
   {
      if ( hdu > hdus )
         throw Error( "The file contains no image" );
      int type = 0, dims = 0;
      CHECK( fits_movabs_hdu( m_file, hdu, &type, &status ), "Cannot read the file structure" );
      if ( fits_get_img_dim( m_file, &dims, &status ) == 0 && dims > 0 )
         break;
      status = 0;                               // not an image (an ordinary table): keep looking
   }
   m_bottomUp = BottomUp( m_file, hints );

   long naxes[ 3 ] = { 0, 0, 0 };
   int bitpix = 0;
   CHECK( fits_get_img_param( m_file, 3, &bitpix, &m_naxis, naxes, &status ), "Cannot read image dimensions" );
   CHECK( fits_get_img_equivtype( m_file, &m_equivType, &status ), "Cannot read image data type" );

   if ( m_naxis < 2 || m_naxis > 3 )
      throw Error( String().Format( "Unsupported image: %d axes (only 2D images and 3D cubes are read)", m_naxis ) );
   m_width = int( naxes[0] );
   m_height = int( naxes[1] );
   m_channels = (m_naxis == 3) ? int( naxes[2] ) : 1;
   if ( m_channels != 1 && m_channels != 3 )
      throw Error( String().Format( "Unsupported image: %d planes (only 1 or 3 are read)", m_channels ) );

   ImageInfo info;
   info.width = m_width;
   info.height = m_height;
   info.numberOfChannels = m_channels;
   info.colorSpace = (m_channels == 3) ? ColorSpace::RGB : ColorSpace::Gray;
   info.supported = true;

   ImageOptions options;
   options.signedIntegers = false;
   switch ( m_equivType )
   {
   case BYTE_IMG:     m_sample = Sample::UInt8;  options.bitsPerSample = 8;  options.ieeefpSampleFormat = false; break;
   case USHORT_IMG:   m_sample = Sample::UInt16; options.bitsPerSample = 16; options.ieeefpSampleFormat = false; break;
   case ULONG_IMG:    m_sample = Sample::UInt32; options.bitsPerSample = 32; options.ieeefpSampleFormat = false; break;
   case FLOAT_IMG:    m_sample = Sample::Float;  options.bitsPerSample = 32; options.ieeefpSampleFormat = true;  break;
   case DOUBLE_IMG:   m_sample = Sample::Double; options.bitsPerSample = 64; options.ieeefpSampleFormat = true;  break;
   case SBYTE_IMG:
   case SHORT_IMG:
   case LONG_IMG:     m_sample = Sample::SignedRescaled; options.bitsPerSample = 64; options.ieeefpSampleFormat = true; break;
   case LONGLONG_IMG:
   case ULONGLONG_IMG: m_sample = Sample::Int64Rescaled; options.bitsPerSample = 64; options.ieeefpSampleFormat = true; break;
   default:
      throw Error( String().Format( "Unsupported FITS data type %d", m_equivType ) );
   }

   return ImageDescriptionArray() << ImageDescription( info, options );
}

String FZInstance::ImageFormatInfo() const
{
   const char* type = "unknown";
   switch ( m_equivType )
   {
   case BYTE_IMG:      type = "8-bit unsigned integer"; break;
   case SBYTE_IMG:     type = "8-bit signed integer"; break;
   case USHORT_IMG:    type = "16-bit unsigned integer"; break;
   case SHORT_IMG:     type = "16-bit signed integer"; break;
   case ULONG_IMG:     type = "32-bit unsigned integer"; break;
   case LONG_IMG:      type = "32-bit signed integer"; break;
   case ULONGLONG_IMG: type = "64-bit unsigned integer"; break;
   case LONGLONG_IMG:  type = "64-bit signed integer"; break;
   case FLOAT_IMG:     type = "32-bit floating point"; break;
   case DOUBLE_IMG:    type = "64-bit floating point"; break;
   }
   return String( "Tile-compressed FITS, " ) + type;
}

// ----------------------------------------------------------------------------

// Splits one 80-character header card into name, value and comment the way PCL keeps
// them: string values keep their quotes, COMMENT and HISTORY text goes in the comment.
static FITSHeaderKeyword ParseCard( const char* card )
{
   IsoString c( card, 0, 80 );
   IsoString name = c.Left( 8 ).Trimmed();
   if ( c.Length() < 10 || c.Substring( 8, 2 ) != "= " )
      return FITSHeaderKeyword( name, IsoString(), c.Substring( 8 ).Trimmed() );

   IsoString rest = c.Substring( 10 );
   IsoString value, comment;
   size_type i = 0;
   while ( i < rest.Length() && rest[i] == ' ' )
      ++i;
   if ( i < rest.Length() && rest[i] == '\'' )
   {
      size_type j = i + 1;
      for ( ; j < rest.Length(); ++j )
         if ( rest[j] == '\'' )
         {
            if ( j+1 < rest.Length() && rest[j+1] == '\'' )
               ++j;                              // '' is an escaped quote
            else
               break;
         }
      value = rest.Substring( i, j - i + 1 );
      size_type slash = rest.Find( '/', j + 1 );
      if ( slash != String::notFound )
         comment = rest.Substring( slash + 1 ).Trimmed();
   }
   else
   {
      size_type slash = rest.Find( '/' );
      value = rest.Left( slash ).Trimmed();
      if ( slash != String::notFound )
         comment = rest.Substring( slash + 1 ).Trimmed();
   }
   return FITSHeaderKeyword( name, value, comment );
}

FITSKeywordArray FZInstance::ReadFITSKeywords()
{
   if ( m_file == nullptr )
      throw Error( "ReadFITSKeywords(): no file is open" );
   volatile AutoLock lock( s_cfitsio );
   int status = 0;
   char* header = nullptr;
   int count = 0;
   // For a compressed image this yields the header of the image it decompresses to,
   // not the binary table that stores it.
   CHECK( fits_convert_hdr2str( m_file, 0, nullptr, 0, &header, &count, &status ), "Cannot read the FITS header" );

   FITSKeywordArray keywords;
   for ( int k = 0; k < count; ++k )
   {
      FITSHeaderKeyword kw = ParseCard( header + 80*k );
      if ( kw.name == "END" )
         break;
      // The decompressed image would sit in an extension; present it as the primary
      // image it was before compression, as the FITS module shows a plain file.
      if ( kw.name == "XTENSION" )
         kw = FITSHeaderKeyword( "SIMPLE", "T", "file does conform to FITS standard" );
      else if ( kw.name == "PCOUNT" || kw.name == "GCOUNT" )
         continue;
      keywords << kw;
   }
   fits_free_memory( header, &status );
   return keywords;
}

// ----------------------------------------------------------------------------

template <class P>
void FZInstance::ReadPlanes( GenericImage<P>& image, int dataType )
{
   image.AllocateData( m_width, m_height, m_channels, (m_channels == 3) ? ColorSpace::RGB : ColorSpace::Gray );
   int status = 0;
   for ( int c = 0; c < m_channels; ++c )
   {
      long first[ 3 ] = { 1, 1, c + 1 };
      CHECK( fits_read_pix( m_file, dataType, first, LONGLONG( m_width )*m_height,
                            nullptr, image.PixelData( c ), nullptr, &status ), "Cannot read image data" );
      if ( m_bottomUp )                          // FITS row 1 is the bottom row: flip to top-down
         for ( typename P::sample* top = image.PixelData( c ), * bottom = top + size_type( m_height - 1 )*m_width;
               top < bottom; top += m_width, bottom -= m_width )
            for ( int x = 0; x < m_width; ++x )
               pcl::Swap( top[x], bottom[x] );
   }
}

template <class I>
void FZInstance::ReadInto( I& image )
{
   if ( m_file == nullptr )
      throw Error( "ReadImage(): no file is open" );
   volatile AutoLock lock( s_cfitsio );

   switch ( m_sample )
   {
   case Sample::UInt8:  { UInt8Image  t; ReadPlanes( t, TBYTE );   image.Assign( t ); break; }
   case Sample::UInt16: { UInt16Image t; ReadPlanes( t, TUSHORT ); image.Assign( t ); break; }
   case Sample::UInt32: { UInt32Image t; ReadPlanes( t, TUINT );   image.Assign( t ); break; }
   case Sample::Float:
   case Sample::Double:
   case Sample::SignedRescaled:
   case Sample::Int64Rescaled:
      {
         DImage t;
         ReadPlanes( t, TDOUBLE );
         double lo, hi;
         switch ( m_equivType )
         {
         case SBYTE_IMG: lo = -128;          hi = 127;        break;
         case SHORT_IMG: lo = -32768;        hi = 32767;      break;
         case LONG_IMG:  lo = -2147483648.0; hi = 2147483647; break;
         default:        lo = t.MinimumSampleValue(); hi = t.MaximumSampleValue(); break;
         }
         // Floating-point data already in [0,1] is used as is; anything else is
         // rescaled to [0,1], like the FITS module does by default.
         bool rescale = (m_sample != Sample::Float && m_sample != Sample::Double) || lo < 0 || hi > 1;
         if ( rescale && hi > lo )
            for ( int c = 0; c < t.NumberOfChannels(); ++c )
               for ( double* p = t.PixelData( c ), * e = p + t.NumberOfPixels(); p < e; ++p )
                  *p = (*p - lo)/(hi - lo);
         image.Assign( t );
         break;
      }
   }
}

void FZInstance::ReadImage( Image& image )       { ReadInto( image ); }
void FZInstance::ReadImage( DImage& image )      { ReadInto( image ); }
void FZInstance::ReadImage( UInt8Image& image )  { ReadInto( image ); }
void FZInstance::ReadImage( UInt16Image& image ) { ReadInto( image ); }
void FZInstance::ReadImage( UInt32Image& image ) { ReadInto( image ); }

} // pcl
