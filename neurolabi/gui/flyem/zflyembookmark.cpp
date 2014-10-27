#include "zflyembookmark.h"
#include <iostream>

ZFlyEmBookmark::ZFlyEmBookmark() : m_bodyId(-1)
{
}

void ZFlyEmBookmark::print() const
{
  std::cout << "Body ID: " << m_bodyId << std::endl;
  std::cout << "Location: " << m_location.toString() << std::endl;
}
