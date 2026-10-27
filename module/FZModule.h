#ifndef __FZModule_h
#define __FZModule_h

#include <pcl/MetaModule.h>

namespace pcl
{

class FZModule : public MetaModule
{
public:

   FZModule();

   const char* Version() const override;
   IsoString Name() const override;
   String Description() const override;
   String Author() const override;
   String Copyright() const override;
   String OriginalFileName() const override;
   void GetReleaseDate( int& year, int& month, int& day ) const override;
};

} // pcl

#endif   // __FZModule_h
