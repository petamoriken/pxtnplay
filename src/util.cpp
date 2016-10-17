#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <iconv.h>

#include "util.h"

namespace pxtnplay
{
namespace util
{
std::string MS932toUTF8(const char *src)
{
  // input string check
  if (src == nullptr) return std::string();
  if (std::strlen(src) == 0) return std::string();

  auto cd = iconv_open("UTF8//TRANSLIT", "MS932");
  if (cd == (iconv_t)-1) {
    return std::string();
  }

  // make src buffer
  std::string _src(src);
  std::vector<char> srcbuf(_src.begin(), _src.end());
  srcbuf.push_back('\0');

  // make dst buffer
  std::vector<char> dstbuf(0);
  size_t srclen = srcbuf.size() - 1;
  size_t dstlen = srclen * 6;
  dstbuf.resize(dstlen + 1, 0);
  auto srcbuf_ptr = srcbuf.data();
  auto dstbuf_ptr = dstbuf.data();

  auto ret = iconv(cd, &srcbuf_ptr, &srclen, &dstbuf_ptr, &dstlen);
  iconv_close(cd);  // close anyway.
  if (ret == (size_t)-1) return std::string();

  return std::string(dstbuf.data());
}
}
}