#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>

#include "libblobstamper.h"


Blob::Blob (char * data_in, int size_in)
{
    data = data_in;
    size = size_in;
    begin = 0;
    end = size -1; /* i.e. size=1 means begin=0 && end=0 */
}

bool
Blob::isEmpty ()
{
    if (! data) return true;
    return false;
}

/* Borrowed from http://www.stahlworks.com/dev/index.php?tool=csc01 */
/* Code is not nice for support, better rewrite it */

void hexdump(void *pAddressIn, long  lSize)
{
 char szBuf[100];
 long lIndent = 1;
 long lOutLen, lIndex, lIndex2, lOutLen2;
 long lRelPos;
 struct { char *pData; unsigned long lSize; } buf;
 unsigned char *pTmp,ucTmp;
 unsigned char *pAddress = (unsigned char *)pAddressIn;

   buf.pData   = (char *)pAddress;
   buf.lSize   = lSize;

   while (buf.lSize > 0)
   {
      pTmp     = (unsigned char *)buf.pData;
      lOutLen  = (int)buf.lSize;
      if (lOutLen > 16)
          lOutLen = 16;

      // create a 64-character formatted output line:
      sprintf(szBuf, " >                            "
                     "                      "
                     "    %08lX", pTmp-pAddress);
      lOutLen2 = lOutLen;

      for(lIndex = 1+lIndent, lIndex2 = 53-15+lIndent, lRelPos = 0;
          lOutLen2;
          lOutLen2--, lIndex += 2, lIndex2++
         )
      {
         ucTmp = *pTmp++;

         sprintf(szBuf + lIndex, "%02X ", (unsigned short)ucTmp);
         if(!isprint(ucTmp))  ucTmp = '.'; // nonprintable char
         szBuf[lIndex2] = ucTmp;

         if (!(++lRelPos & 3))     // extra blank after 4 bytes
         {  lIndex++; szBuf[lIndex+2] = ' '; }
      }

      if (!(lRelPos & 3)) lIndex--;

      szBuf[lIndex  ]   = '\t';
      szBuf[lIndex+1]   = '\t';

      printf("%s\n", szBuf);

      buf.pData   += lOutLen;
      buf.lSize   -= lOutLen;
   }
}


void
Blob::Dump()
{
    int length = end - begin +1 ;
    hexdump(data + begin, length);
}

Blob
Blob::ShiftBytes(size_t n)
{
    if (begin + n > end)
    {
        Blob empty(NULL, -1);
        return empty; /* not enough data */
    }

    Blob new_blob(data, size);

    new_blob.begin = begin;   /* FIXME this should go private once */
    new_blob.end = begin + n - 1;

    begin += n;

    return new_blob;
}

std::string
wflShiftDouble(Blob &blob)
{
    int ret;
    double * d;
    char * resc;
    std::string res;


    Blob b2 = blob.ShiftBytes(sizeof(double));
    if (b2.isEmpty()) return "";

    d = (double *)( (char*)b2.data + b2.begin);

    int size_s = snprintf( nullptr, 0, "%.999g", *d) + 1;
    if (size_s <= 0)
    {
        printf("ai-ai-ai\n");
        return "";
    }

    resc =(char *) malloc(size_s);
    if (! resc)
    {
        printf("oh-oh-oh\n");
        return "";
    }

    ret = snprintf(resc,size_s,"%.999g", *d);
    if (ret <= 0)
    {
        printf("oi-oi-oi\n");
        free(resc);
        return "";
    }
    res = resc;
    free(resc);
    return res;
}

std::string
wflShiftPgPoint(Blob &blob)
{
    std::string res = "";
    std::string x, y;

    x = wflShiftDouble(blob);
    if (x.empty()) return "";

    y = wflShiftDouble(blob);
    if (y.empty()) return "";

    res = (std::string) "(" + x +", " + y + ")";
    return res;
}


std::string
wflShiftPgPath(Blob &blob)
{
    std::string res = "";
    std::string point;

    while (1)
    {
        point = wflShiftPgPoint(blob);
        if (point.empty())
            break;
        if (!res.empty()) res = res + ", ";
        res = res + point;
    }
    if (res.empty())
        return res;
    res = "(" + res + ")";
    return res;
}
