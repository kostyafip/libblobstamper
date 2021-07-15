
#include "blob.h"
#include "stamp.h"
#include "stamp_arithm.h"
#include "stamp_dict.h"

StampFixed&
StampDict::GuessStamp(DictBase & dict)
{
    if (dict.size()<= UCHAR_MAX+1)
    {
        stamp_max_value = UCHAR_MAX;
        return stamp8;
    }
    if (dict.size()<= USHRT_MAX+1)
    {
        stamp_max_value = USHRT_MAX;
        return stamp16;
    }
    if (dict.size()<= UINT_MAX+1)
    {
        stamp_max_value = UINT_MAX;
        return stamp32;
    }
    stamp_max_value = ULONG_MAX;
    return stamp64;
}

std::string
StampDict::ExtractStr(Blob &blob)
{
    unsigned long long index_oracle;

    /* Shifting index oracle according to size of dictionary index variable*/
    switch (stamp.minSize())
    {
       case 1:
       {
          std::vector<char> v = blob.ShiftSingleStampBin(stamp);
          unsigned char * i =  (unsigned char *) &v[0];
          index_oracle = * i;
          break;
       }
       case 2:
       {
          std::vector<char> v = blob.ShiftSingleStampBin(stamp);
          unsigned short int * i = (unsigned short int *) &v[0];
          index_oracle = * i;
          break;
       }
       case 4:
       {
          std::vector<char> v = blob.ShiftSingleStampBin(stamp);
          unsigned int * i = ( unsigned int *) &v[0];
          index_oracle = * i;
          break;
       }
       case 8:
       {
          std::vector<char> v = blob.ShiftSingleStampBin(stamp);
          unsigned long long * i = ( unsigned long long *) &v[0];
          index_oracle = * i;
          break;
       }
       default:
         printf("StampDict::ExtractStr: Something is really wrong\n");
         exit(1);
    }
    long long actual_index = ((double) index_oracle) / stamp_max_value * dict.size();
    if ( actual_index == dict.size())  actual_index--; /* If we hit the boundary step inside a bit*/
    return dict.get(actual_index);
}
