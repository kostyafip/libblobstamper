
#ifndef STAMP_H
#define STAMP_H

#include <string>
#include <list>

class StampGeneric
{
  protected:
     bool is_fixed_size;
     int  min_size;
     int  max_size;
  public:
    bool isFixedSize() {return is_fixed_size;}
    int  minSize() {return min_size;}
    int  maxSize() {return max_size;}

    virtual void *      Extract(Blob &blob);
    virtual std::string ExtractStr(Blob &blob)  {printf ("22222\n"); return "";}

};

class StampList: public StampGeneric
{
  protected:
    StampGeneric& target_stamp;
  public:
    StampList(StampGeneric &stamp) : target_stamp(stamp) {};
};

#endif  /* STAMP_H */