/* Exercises the same CFITSIO calls as FZInstance.cpp outside PixInsight: the file is read
   into memory (by a Unicode path) and opened with fits_open_memfile, the first image HDU is
   found, ROWORDER is read, and rows are flipped when it says BOTTOM-UP. Prints the geometry,
   the start and end of the converted header, and checksums of the 16-bit pixels.
   Usage: fzcheck FILE.fits.fz */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <fitsio.h>

int wmain( int argc, wchar_t** argv )
{
   fitsfile* f; int status = 0, bitpix, naxis, equiv, n, k, hdus = 0, hdu, type, dims, bottomUp = 0;
   long naxes[3] = { 0, 0, 0 }, first[3] = { 1, 1, 1 };
   char *hdr, roworder[ FLEN_VALUE ] = "";
   unsigned short* px; unsigned long long sum = 0, sq = 0, row0 = 0; long long npix, y, x;
   FILE* in; void* buf; size_t size;

   if ( argc < 2 ) return 2;
   in = _wfopen( argv[1], L"rb" );
   if ( in == NULL ) { fwprintf( stderr, L"cannot open %ls\n", argv[1] ); return 1; }
   fseek( in, 0, SEEK_END ); size = (size_t)ftell( in ); fseek( in, 0, SEEK_SET );
   buf = malloc( size ); fread( buf, 1, size, in ); fclose( in );

   if ( fits_open_memfile( &f, "image.fits.fz", READONLY, &buf, &size, 0, NULL, &status ) ) { fits_report_error( stderr, status ); return 1; }
   fits_get_num_hdus( f, &hdus, &status );
   for ( hdu = 1; hdu <= hdus; ++hdu )
   {
      fits_movabs_hdu( f, hdu, &type, &status );
      if ( fits_get_img_dim( f, &dims, &status ) == 0 && dims > 0 ) break;
      status = 0;
   }
   if ( hdu > hdus ) { printf( "no image\n" ); return 1; }
   if ( fits_read_key( f, TSTRING, "ROWORDER", roworder, NULL, &status ) == 0 && strcmp( roworder, "BOTTOM-UP" ) == 0 ) bottomUp = 1;
   status = 0;
   fits_get_img_param( f, 3, &bitpix, &naxis, naxes, &status );
   fits_get_img_equivtype( f, &equiv, &status );
   printf( "image in HDU %d of %d  naxis %d  %ld x %ld  bitpix %d  equivtype %d  ROWORDER '%s'\n", hdu, hdus, naxis, naxes[0], naxes[1], bitpix, equiv, roworder );
   fits_convert_hdr2str( f, 0, NULL, 0, &hdr, &n, &status );
   printf( "header cards: %d (first: %.30s)\n", n, hdr );
   fits_free_memory( hdr, &status );

   npix = (long long)naxes[0]*naxes[1];
   px = (unsigned short*)malloc( npix*sizeof( unsigned short ) );
   fits_read_pix( f, TUSHORT, first, npix, NULL, px, NULL, &status );
   if ( bottomUp )
      for ( y = 0; y < naxes[1]/2; ++y )
         for ( x = 0; x < naxes[0]; ++x )
         {
            unsigned short t = px[y*naxes[0] + x];
            px[y*naxes[0] + x] = px[(naxes[1] - 1 - y)*naxes[0] + x];
            px[(naxes[1] - 1 - y)*naxes[0] + x] = t;
         }
   for ( k = 0; k < npix; ++k ) { sum += px[k]; sq += (unsigned long long)px[k]*px[k]; }
   for ( x = 0; x < naxes[0]; ++x ) row0 += px[x]*(unsigned long long)(x + 1);
   printf( "pixels %lld  sum %llu  sumsq %llu  top-row signature %llu  first %u %u %u\n", npix, sum, sq, row0, px[0], px[1], px[2] );
   fits_close_file( f, &status );
   if ( status ) { fits_report_error( stderr, status ); return 1; }
   return 0;
}
