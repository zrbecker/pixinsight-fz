// FZ module: read-only support for tile-compressed FITS (.fits.fz) in PixInsight.
// PixInsight picks a file format from the last extension only, so ".fits.fz" reaches
// no reader; its own FITS module also cannot decode compressed images. This module
// registers ".fz" and decodes through CFITSIO.

#define MODULE_VERSION_MAJOR     01
#define MODULE_VERSION_MINOR     00
#define MODULE_VERSION_REVISION  01
#define MODULE_VERSION_BUILD     0002
#define MODULE_VERSION_LANGUAGE  eng

#define MODULE_RELEASE_YEAR      2026
#define MODULE_RELEASE_MONTH     10
#define MODULE_RELEASE_DAY       27

#include "FZModule.h"
#include "FZFormat.h"

namespace pcl
{

FZModule::FZModule()
{
}

const char* FZModule::Version() const
{
   return PCL_MODULE_VERSION( MODULE_VERSION_MAJOR,
                              MODULE_VERSION_MINOR,
                              MODULE_VERSION_REVISION,
                              MODULE_VERSION_BUILD,
                              MODULE_VERSION_LANGUAGE );
}

IsoString FZModule::Name() const
{
   return "FZ";
}

String FZModule::Description() const
{
   return "Tile-compressed FITS (.fits.fz) reader";
}

String FZModule::Author() const
{
   return "Zachary Becker";
}

String FZModule::Copyright() const
{
   return "Copyright (c) 2026 Zachary Becker";
}

String FZModule::OriginalFileName() const
{
   return "FZ-pxm.dll";
}

void FZModule::GetReleaseDate( int& year, int& month, int& day ) const
{
   year  = MODULE_RELEASE_YEAR;
   month = MODULE_RELEASE_MONTH;
   day   = MODULE_RELEASE_DAY;
}

} // pcl

PCL_MODULE_EXPORT int InstallPixInsightModule( int mode )
{
   new pcl::FZModule;
   if ( mode == pcl::InstallMode::FullInstall )
      new pcl::FZFormat;
   return 0;
}
