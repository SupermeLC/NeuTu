#ifndef ZDVIDINFO_H
#define ZDVIDINFO_H

#include <vector>
#include <string>
#include "zintpoint.h"
#include "zintpointarray.h"
#include "zobject3dscan.h"
#include "zresolution.h"

class ZDvidInfo
{
public:
  ZDvidInfo();

  /*!
   * \brief Set info from a json string
   *
   * Properties without correspondng fields will be set to default values.
   *
   * \param str Input json string.
   */
  void setFromJsonString(const std::string &str);

  void print() const;

  /*!
   * \brief Get block index of a certain point.
   *
   * \param x X coordinate of the point.
   * \param y Y coordinate of the point.
   * \param z Z coordinate of the point.
   *
   * \return Empty array if the point is out of range.
   */
  ZIntPoint getBlockIndex(double x, double y, double z) const;

  ZIntPoint getBlockIndex(int x, int y, int z) const;

  /*!
   * \brief Get the indices of all blocks containing at least one voxl of an object
   */
  ZObject3dScan getBlockIndex(const ZObject3dScan &obj) const;

  inline const ZResolution& getVoxelResolution() const {
    return m_voxelResolution;
  }

  inline const ZIntPoint& getStartCoordinates() const {
    return m_startCoordinates;
  }

  inline const std::vector<int>& getStackSize() const {
    return m_stackSize;
  }

  inline const ZIntPoint& getStartBlockIndex() const {
    return m_startBlockIndex;
  }

  inline const ZIntPoint& getEndBlockIndex() const {
    return m_endBlockIndex;
  }

  /*!
   * \brief Get the bound box of a block
   *
   * (\a x, \a y, \a z) is the block index.
   */
  ZIntCuboid getBlockBox(int x, int y, int z) const;

  ZIntPoint getBlockSize() const;
  ZIntPoint getGridSize() const;

  bool isValidBlockIndex(const ZIntPoint &pt);

  int getMinZ() const;
  int getMaxZ() const;

  int getMinX() const;
  int getMaxX() const;

  int getMinY() const;
  int getMaxY() const;

private:
  std::vector<int> m_stackSize;
  //std::vector<double> m_voxelResolution;
  ZResolution m_voxelResolution;
  ZIntPoint m_startCoordinates;
  ZIntPoint m_startBlockIndex;
  ZIntPoint m_endBlockIndex;
  std::vector<int> m_blockSize;

  std::string m_dvidAddress;
  int m_dvidPort;
  std::string m_dvidUuid;

  const static int m_defaultBlockSize;

  //Json keys
  const static char* m_minPointKey;
  const static char* m_maxPointKey;
  const static char* m_blockSizeKey;
  const static char* m_voxelSizeKey;
  const static char* m_voxelUnitKey;
  const static char* m_blockMinIndexKey;
  const static char* m_blockMaxIndexKey;
};

#endif // ZDVIDINFO_H
