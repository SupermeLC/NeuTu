#include "zflyembodymerger.h"
#include <iostream>

ZFlyEmBodyMerger::ZFlyEmBodyMerger()
{
}

uint64_t ZFlyEmBodyMerger::getFinalLabel(uint64_t label) const
{
  return mapLabel(m_mapList, label);
}

ZFlyEmBodyMerger::TLabelMap ZFlyEmBodyMerger::getFinalMap() const
{
  ZFlyEmBodyMerger::TLabelMap labelMap;
  std::cout << "Body merger:" << std::endl;
  for (TLabelMapList::const_iterator iter = m_mapList.begin();
       iter != m_mapList.end(); ++iter) {
    const TLabelMap &currentLabelMap = *iter;
    for (TLabelMap::const_iterator mapIter = currentLabelMap.begin();
         mapIter != currentLabelMap.end(); ++mapIter) {
      if (!labelMap.contains(mapIter.key())) {
        labelMap[mapIter.key()] = getFinalLabel(mapIter.key());
      }
    }
  }

  return labelMap;
}

uint64_t ZFlyEmBodyMerger::mapLabel(const TLabelMap &labelMap, uint64_t label)
{
  if (labelMap.contains(label)) {
    return labelMap[label];
  }

  return label;
}

uint64_t ZFlyEmBodyMerger::mapLabel(
    const TLabelMapList &labelMap, uint64_t label)
{
  uint64_t finalLabel = label;
  for (TLabelMapList::const_iterator iter = labelMap.begin();
       iter != labelMap.end(); ++iter) {
    const TLabelMap &currentLabelMap = *iter;
    finalLabel = mapLabel(currentLabelMap, finalLabel);
  }

  return finalLabel;
}

void ZFlyEmBodyMerger::pushMap(uint64_t label1, uint64_t label2)
{
  TLabelMap labelMap;
  labelMap[label1] = label2;
  pushMap(labelMap);
}

void ZFlyEmBodyMerger::pushMap(const TLabelMap &map)
{
  if (!map.isEmpty()) {
    m_mapList.append(map);
  }
}

void ZFlyEmBodyMerger::pushMap(const TLabelSet &labelSet)
{
  if (labelSet.size() > 1) {
    uint64_t minLabel = 0;
    for (TLabelSet::const_iterator iter = labelSet.begin();
         iter != labelSet.end(); ++iter) {
      if (minLabel == 0 || minLabel > *iter) {
        minLabel = *iter;
      }
    }

    TLabelMap labelMap;
    if (minLabel > 0) {
      for (TLabelSet::const_iterator iter = labelSet.begin();
           iter != labelSet.end(); ++iter) {
        if (*iter != minLabel) {
          labelMap[*iter] = minLabel;
        }
      }
      pushMap(labelMap);
    }
  }
}

void ZFlyEmBodyMerger::undo()
{
  if (!m_mapList.isEmpty()) {
    TLabelMap labelMap = m_mapList.takeLast();
    m_undoneMapStack.push(labelMap);
  }
}

void ZFlyEmBodyMerger::redo()
{
  if (!m_undoneMapStack.isEmpty()) {
    TLabelMap labelMap = m_undoneMapStack.pop();
    pushMap(labelMap);
  }
}

void ZFlyEmBodyMerger::print() const
{
  int index = 1;
  std::cout << "Body merger:" << std::endl;
  for (TLabelMapList::const_iterator iter = m_mapList.begin();
       iter != m_mapList.end(); ++iter, ++index) {
    const TLabelMap &currentLabelMap = *iter;
    std::cout << "--" << index << "--" << std::endl;
    for (TLabelMap::const_iterator mapIter = currentLabelMap.begin();
         mapIter != currentLabelMap.end(); ++mapIter) {
      std::cout << mapIter.key() << " -> " << mapIter.value() << std::endl;
    }
  }
}
