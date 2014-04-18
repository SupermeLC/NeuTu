#include "ztileinfo.h"
#include "zjsonparser.h"

ZTileInfo::ZTileInfo()
{
  for (int i = 0; i < 3; ++i) {
    m_dim[i] = 0;
  }
}

bool ZTileInfo::loadJsonObject(const ZJsonObject &obj)
{
  if (obj.hasKey("source")) {
    m_source = ZJsonParser::stringValue(obj["source"]);
  }

  if (obj.hasKey("swc")) {
    m_swcSource = ZJsonParser::stringValue(obj["swc"]);
  }

  if (obj.hasKey("offset")) {
    const json_t *value = obj["offset"];
    if (ZJsonParser::isArray(value)) {
      if (ZJsonParser::arraySize(value)  == 3) {
        m_offset.set(ZJsonParser::numberValue(value, 0),
                     ZJsonParser::numberValue(value, 1),
                     ZJsonParser::numberValue(value, 2));
      } else {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }

  if (obj.hasKey("size")) {
    const json_t *value = obj["size"];
    if (ZJsonParser::isArray(value)) {
      if (ZJsonParser::arraySize(value)  == 3) {
        for (int i = 0; i < 3; ++i) {
          m_dim[i] = ZJsonParser::integerValue(value, i);
        }
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  return true;
}
