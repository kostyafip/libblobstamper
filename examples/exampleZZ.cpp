#include<stdio.h>
#include<string.h>

#include<string>
#include<iostream>
#include<cstdlib>

#include<blobstamper/blobstamper.h>

namespace std
{
  template<class T> using ref_vector = vector<reference_wrapper<T>>;
}

template<class StampT> class StampLottery: public StampT
{
  protected:
    std::ref_vector<StampT> stamps;
    int oracle_size;
    int init_oracle_size(std::ref_vector<StampT> stamps_arg);

    int stored_min;
    int init_stored_min(std::ref_vector<StampT> stamps_arg);

  public:
    StampLottery(std::ref_vector<StampT> stamps_arg): stamps(stamps_arg), oracle_size(init_oracle_size(stamps_arg)), stored_min(init_stored_min(stamps_arg)) {};
    StampLottery(): stored_min(-1) {};

    virtual int  minSize() override;
    virtual int  maxSize() override;
    virtual std::string ExtractStr(Blob &blob) override;
    void Append(StampT & stamp);
};


template<class StampT> int
StampLottery<StampT>::
init_stored_min(std::ref_vector<StampT> stamps_arg)
{
  int min = std::numeric_limits<int>::max();

  for(StampT & stamp : stamps)
  {

    if (min > stamp.minSize())
        min = stamp.minSize();
  }
  return min;
}

template<class StampT> int
StampLottery<StampT>::init_oracle_size(std::ref_vector<StampT> stamps_arg)
{
  unsigned long size = stamps_arg.size();
  if (size < std::numeric_limits<unsigned char>::max())
    return 1;
  if (size < std::numeric_limits<unsigned short int>::max())
    return 2;
  if (size < std::numeric_limits<unsigned int>::max())
    return 4;
  return 8;
}


template<class StampT> int
StampLottery<StampT>::minSize()
{
  return stored_min + oracle_size;
}

template<class StampT> int
StampLottery<StampT>::maxSize()
{
  return -1;  // FIXME this is true only for recurion case. Should fix it somehow if Lottery is used in other cases
}


template<class StampT> std::string
StampLottery<StampT>::ExtractStr(Blob &blob)
{
  unsigned long oracle;
  unsigned long oracle_max;

  switch (oracle_size)
  {
     case 1:
     {
        StampArithm<unsigned char> stamp;
        oracle = stamp.ExtractValue(blob);
        oracle_max = std::numeric_limits<unsigned char>::max();
        break;
     }
     case 2:
     {
        StampArithm<unsigned short> stamp;
        oracle = stamp.ExtractValue(blob);
        oracle_max = std::numeric_limits<unsigned short>::max();
        break;
     }
     case 4:
     {
        StampArithm<unsigned int> stamp;
        oracle = stamp.ExtractValue(blob);
        oracle_max = std::numeric_limits<unsigned int>::max();
        break;
     }
     case 8:
     {
        StampArithm<unsigned long> stamp;
        oracle = stamp.ExtractValue(blob);
        oracle_max = std::numeric_limits<unsigned long>::max();
        break;
     }
     default:
        abort(); // Should never get here
 }

  /* Actually we use only stamps that short enogh to consume blob's available data*/
  std::ref_vector<StampT> actual_stamps;
  for(StampT & stamp : stamps)
  {
    if(blob.Size() < stamp.minSize())  // Skip all stamps that dose not fit
       continue;
    if ( stamp.isUnbounded() ||            // Unbounded is always ok
         stamp.maxSize() > blob.Size()  ||  // Variated that can consume all data is ok
         stamp.minSize() * 2 > blob.Size()  // Fixed or variated stamp that lefts less data then it's min size will also do
       )
    {
      actual_stamps.push_back(stamp);
    }
  }
  if (actual_stamps.empty())
  {
    // Add just everything that fits
    for(StampT & stamp : stamps)
    {
      if(blob.Size() < stamp.minSize())  // Skip all stamps that dose not fit
         continue;
      actual_stamps.push_back(stamp);
    }
  }

  if (actual_stamps.empty())
        throw OutOfData();  // This should not happen

  long long index = ((double) oracle) / oracle_max * actual_stamps.size();
  if ( index == actual_stamps.size())  index--; /* If we hit the boundary step inside a bit*/

  StampT& stamp = actual_stamps[index];
  return stamp.ExtractStr(blob);
}


template<class StampT> void
StampLottery<StampT>::Append(StampT & stamp)
{
  if (stamp.minSize()<stored_min)
  {
    stored_min = stamp.minSize();
  }
  stamps.push_back(stamp);
  oracle_size = init_oracle_size(stamps);
}


class BinaryOp: public StampBaseStr, public GalleySetBase
{
  protected:
    std::string op_name;
    StampBaseStr &stamp1;
    StampBaseStr &stamp2;
  public:
    virtual std::string ExtractStr(Blob &blob) override;
    BinaryOp(std::string arg_op_name, StampBaseStr& arg_stamp1, StampBaseStr& arg_stamp2) :
        GalleySetBase({arg_stamp1, arg_stamp2}),
        op_name(arg_op_name),
        stamp1(arg_stamp1),
        stamp2(arg_stamp2) {};
};

std::string
BinaryOp::ExtractStr(Blob &blob)
{
  std::vector<Blob> blobs = extract_internal(blob);
  return (std::string)"(" +  stamp1.ExtractStr(blobs[0]) + " "+  op_name + " " + stamp2.ExtractStr(blobs[1]) + ")";
}

std::vector<std::string> ops = {"+","-","*","/","^"};

int main()
{
//  char data[] = "abcdef" "abcdef" "ABCDEF" "012345" "sdfaskdlfjalsfjdlasjfaksdjfgkwuergkwhfdaksjdfgaskuyrgfaelkrgfsaldjfgakyefgrkweugyfaksjskdfsd";

char data[] =
 "\x051\x04E\x05A\x018\x043\x00C\x039\x0DC\x069\x0AC\x009\x014\x05A\x0B2\x07F\x078\x021\x09F\x08B\x0B1\x07E\x060\x01F\x04A\x0D1\x071\x05C\x04F\x011\x0D0\x061\x0FB\x037\x077\x081\x00C\x059\x00A\x037\x02F\x061\x04A\x065\x06D"
"\x003\x04A\x0BC\x099\x0F8\x00B\x0F7\x020\x0C9\x074\x065\x008\x0B4\x010\x008\x0B4\x08B\x070\x0E1\x0EF\x026\x04F\x0F9\x0AB\x01C\x06C\x035\x018\x086\x037\x0E7\x02F\x044\x057\x001\x020\x006\x0DD\x0C4\x059\x0D1\x0C5\x0A9\x005"
"\x038\x078\x0E2\x053\x01D\x0F0\x06E\x0E6\x018\x0B6\x048\x0F1\x0DC\x061\x092\x0FB\x0D3\x010\x0B8\x042\x0CA\x0C1\x0E3\x075\x077\x099\x093\x0CC\x063\x0F0\x09E\x044\x03D\x070\x01A\x089\x035\x032\x04A\x0BD\x082\x0BF\x0EA\x002"
"\x043\x071\x079\x0A0\x068\x0B3\x0D9\x029\x0E9\x045\x0A2\x027\x003\x02E\x0E2\x01F\x007\x0BD\x0CF\x00A\x03E\x00D\x044\x024\x0FA\x0DB\x03D\x033\x036\x011\x081\x070\x0B6\x04A\x083\x061\x05F\x0AE\x0F0\x0C5\x0A1\x010\x05B\x003"
"\x061\x0C3\x0D2\x078\x0BD\x0F8\x0E1\x04B\x02F\x0D9\x093\x09F\x00E\x0D6\x03A\x070\x0F8\x052\x013\x0EE\x062\x0C0\x027\x0E5\x07B\x07B\x09E\x05D\x074\x068\x0C6\x0CD\x04E\x022\x03B\x04E\x0E7\x0E7\x0EE\x0EC\x015\x02C\x0FA\x050"
"\x033\x042\x0E6\x0BF\x028\x002\x052\x096\x033\x057\x0D8\x082\x053\x06E\x0BD\x0C6\x0ED\x015\x036\x09E\x03B\x0BE\x0F3\x068\x0BD\x0EC\x0D3\x0E9\x023\x029\x081\x0CF\x0F8\x02D\x081\x049\x007\x0CC\x005\x004\x062\x040\x0E0\x0D0"
"\x0CD\x062\x0D4\x09B\x007\x001\x037\x020\x059\x0AC\x0FC\x0A4\x095\x049\x05F\x04C\x0DA\x02B\x0E8\x0E9\x0BF\x029\x01F\x0D0\x06B\x06E\x0F5\x005\x075\x07B\x036\x0D2\x054\x078\x0D3\x059\x077\x09A\x0D5\x079\x0AC\x034\x030\x0FD"
"\x006\x079\x022\x0F4\x0ED\x059\x080\x081\x08F\x0A6\x08F\x042\x08A\x0CC\x030\x019\x094\x0F3\x062\x00B\x08A\x0D4\x0F8\x0F3\x03B\x049\x0D1\x06D\x0C6\x067\x006\x0D3\x023\x035\x053\x0C1\x0F8\x068\x0EF\x0AD\x0C7\x053\x004\x02C"
"\x092\x087\x075\x0B0\x0F0\x0F7\x0D9\x04C\x0C7\x0A2\x095\x02B\x038\x02E\x0F2\x005\x0BE\x0CD\x02E\x093\x08A\x088\x063\x07D\x0F1\x08A\x002\x0D0\x0B9\x05C\x008\x066\x002\x044\x0B0\x08F\x041\x009\x06F\x0E5\x08B\x068\x0EB\x05A";

  Blob blob(data, strlen(data));

  StampArithm<unsigned char> stampс;

  DictLCAlphaSmall dict;
  StampDict stamp_dict(dict);

  StampLottery<StampBaseStr> stamp_lot({stampс, stamp_dict});

  for(std::string op_name : ops)
  {
    BinaryOp *stamp_bi = new BinaryOp(op_name, stamp_lot, stamp_lot);
    stamp_lot.Append(*stamp_bi);
  }

  for(int i=stamp_lot.minSize(); i<=strlen(data);i++)
  {
    Blob blob2(data, i);
    std::cout << i << " " << stamp_lot.ExtractStr(blob2) <<"\n";
  }
}
