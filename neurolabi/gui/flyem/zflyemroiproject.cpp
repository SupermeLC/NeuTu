#include "zflyemroiproject.h"
#include <QStringList>
#include "neutubeconfig.h"
#include "zstackframe.h"
#include "zswcgenerator.h"
#include "zflyemutilities.h"
#include "zstack.hxx"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidreader.h"
#include "swctreenode.h"
#include "zobject3dscan.h"
#include "zstroke2d.h"
#include "zstackfactory.h"

ZFlyEmRoiProject::ZFlyEmRoiProject(QObject *parent) :
  QObject(parent), m_z(-1), m_dataFrame(NULL)
{
}

ZFlyEmRoiProject::~ZFlyEmRoiProject()
{
  clear();
}

void ZFlyEmRoiProject::clear()
{
  if (m_dataFrame != NULL) {
    m_dataFrame->hide();
    delete m_dataFrame;
    m_dataFrame = NULL;
  }

  for (std::vector<ZClosedCurve*>::const_iterator iter = m_curveArray.begin();
       iter != m_curveArray.end(); ++iter) {
    delete *iter;
  }
  m_curveArray.clear();

  shallowClear();
}

void ZFlyEmRoiProject::shallowClear()
{
  m_dataFrame = NULL;
}

void ZFlyEmRoiProject::setDvidTarget(const ZDvidTarget &target)
{
  clear();

  m_dvidTarget = target;
  ZDvidReader reader;
  if (reader.open(target)) {
    m_dvidInfo = reader.readGrayScaleInfo();
    downloadAllRoi();
  }
}

void ZFlyEmRoiProject::downloadAllRoi()
{
  //Download All ROIs
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    QStringList roiIdArray = reader.readKeys(
          "roi_curve", QString("%1").arg(m_dvidInfo.getMinZ()),
          QString("%1").arg(99999));
    foreach (const QString &roiId, roiIdArray) {
      downloadRoi(roiId.toInt());
    }
  }
}

void ZFlyEmRoiProject::showDataFrame() const
{
  if (m_dataFrame != NULL) {
    m_dataFrame->show();
  }
}

bool ZFlyEmRoiProject::hasDataFrame() const
{
  return m_dataFrame != NULL;
}

void ZFlyEmRoiProject::setDataFrame(ZStackFrame *frame)
{
  if (m_dataFrame != NULL) {
    frame->setObjectStyle(m_dataFrame->getObjectStyle());

    m_dataFrame->hide();
    delete m_dataFrame;
    m_dataFrame = NULL;
  }

  m_dataFrame = frame;
}

void ZFlyEmRoiProject::shallowClearDataFrame()
{
  shallowClear();
}

void ZFlyEmRoiProject::setRoi(ZClosedCurve *roi, int z)
{
  if (roi != NULL && z >= 0) {
    if (z >= (int) m_curveArray.size()) {
      m_curveArray.resize(z + 1, NULL);
    }
    if (m_curveArray[z] != NULL) {
      delete m_curveArray[z];
    }
    m_curveArray[z] = roi;
    setRoiUploaded(z, false);
  }
}

int ZFlyEmRoiProject::getDataZ() const
{
  if (m_dataFrame == NULL) {
    return -1;
  }

  return m_dataFrame->document()->getStackOffset().getZ();
}
int ZFlyEmRoiProject::getCurrentZ() const
{
  return m_z;
}

void ZFlyEmRoiProject::setZ(int z)
{
  m_z = z;
}

bool ZFlyEmRoiProject::addRoi()
{
  if (m_dataFrame != NULL) {
    QList<ZSwcTree*>& swcList = m_dataFrame->document()->getSwcList();
    if (swcList.size() == 1) {
      ZClosedCurve curve = swcList.front()->toClosedCurve();
      if(!curve.isEmpty()) {
        ZClosedCurve *roi = new ZClosedCurve(curve);
        setRoi(roi, getDataZ());

#ifdef _DEBUG_2
      ZObject3dScan *obj = getFilledRoi(getDataZ(), NULL);
      obj->save(GET_DATA_DIR + "/test.sobj");
      delete obj;
#endif

        return true;
      }
    }
  }

  return false;
}

const ZClosedCurve* ZFlyEmRoiProject::getRoi(int z) const
{
  const ZClosedCurve *curve = NULL;
  if (z >= 0 && z < (int) m_curveArray.size()) {
    curve = m_curveArray[z];
  }

  return curve;
}

const ZClosedCurve* ZFlyEmRoiProject::getRoi() const
{
  return getRoi(getDataZ());
}

double ZFlyEmRoiProject::getMarkerRadius() const
{
  double s = 0;

  if (m_dataFrame != NULL) {
    s = imin2(m_dataFrame->document()->getStack()->width(),
              m_dataFrame->document()->getStack()->height());
  }

  return FlyEm::GetFlyEmRoiMarkerRadius(s);
}

ZSwcTree* ZFlyEmRoiProject::getRoiSwc() const
{
  return getRoiSwc(getDataZ());
}

ZSwcTree* ZFlyEmRoiProject::getRoiSwc(int z) const
{
  ZSwcTree *tree = NULL;

  const ZClosedCurve *curve = getRoi(z);
  if (curve != NULL) {
    tree = ZSwcGenerator::createSwc(*curve, getMarkerRadius());
  }

  return tree;
}

ZSwcTree* ZFlyEmRoiProject::getAllRoiSwc() const
{
  ZSwcTree *tree = new ZSwcTree();

  for (size_t i = 0; i < m_curveArray.size(); ++i) {
    const ZClosedCurve *curve = m_curveArray[i];
    if (curve != NULL) {
      ZSwcTree *curveTree = ZSwcGenerator::createSwc(*curve, getMarkerRadius());
      if (curveTree != NULL) {
        tree->merge(curveTree, true);
      }
    }
  }

  if (tree->isEmpty()) {
    delete tree;
    tree = NULL;
  }

  return tree;
}

bool ZFlyEmRoiProject::hasOpenedRoi() const
{
  return getRoi(getDataZ()) != NULL;
}

int ZFlyEmRoiProject::uploadRoi(int z)
{
  ZDvidWriter writer;
  int count = 0;
  if (writer.open(getDvidTarget())) {
    const ZClosedCurve *curve = m_curveArray[z];
    if (curve != NULL) {
      if (!isRoiCurveUploaded(z)) {
        writer.writeRoiCurve(*curve, z);
        setRoiUploaded(z, true);
        ++count;
      }
    }
  }

  return count;
}

int ZFlyEmRoiProject::uploadRoi()
{
  int count = 0;
  for (size_t i = 0; i < m_curveArray.size(); ++i) {
    count += uploadRoi(i);
  }

  return count;
}

void ZFlyEmRoiProject::downloadRoi()
{
  downloadRoi(getDataZ());
}

void ZFlyEmRoiProject::downloadRoi(int z)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZClosedCurve *curve = reader.readRoiCurve(z, NULL);
    if (curve != NULL) {
      setRoi(curve, z);
      setRoiUploaded(z, true);
    }
  }
}

ZClosedCurve* ZFlyEmRoiProject::estimateRoi(int z, ZClosedCurve *result) const
{
  if (result != NULL) {
    result->clear();
  }

  int minZ = -1;
  int maxZ = -1;
  for (int tz = z + 1; tz < (int) m_curveArray.size(); ++tz) {
    if (m_curveArray[tz] != NULL) {
      maxZ = tz;
      break;
    }
  }

  for (int tz = imin2(z - 1, (int) m_curveArray.size() - 1); tz >= 0; --tz) {
    if (m_curveArray[tz] != NULL) {
      minZ = tz;
      break;
    }
  }

  if (minZ >= 0 || maxZ >= 0) {
    if (minZ < 0) {
      minZ = maxZ;
      maxZ = -1;
      for (int tz = minZ + 1; tz < (int) m_curveArray.size(); ++tz) {
        if (m_curveArray[tz] != NULL) {
          maxZ = tz;
          break;
        }
      }
    } else if (maxZ < 0) {
      maxZ = minZ;
      minZ = -1;
      for (int tz = maxZ - 1; tz >= 0; --tz) {
        if (m_curveArray[tz] != NULL) {
          minZ = tz;
          break;
        }
      }
    }
  }

  if (minZ >= 0 && maxZ >= 0) {
    const ZClosedCurve *lowerCurve = getRoi(minZ);
    const ZClosedCurve *upperCurve = getRoi(maxZ);
    int sampleNumber = imax3(100, lowerCurve->getLandmarkNumber(),
                             upperCurve->getLandmarkNumber());
    ZClosedCurve curve1 = lowerCurve->resampleF(sampleNumber);
    ZClosedCurve curve2 = upperCurve->resampleF(sampleNumber);

    if (curve1.computeDirection().dot(curve2.computeDirection()) < 0) {
      curve2.reverse();
    }
    int shift = curve1.findMatchShift(curve2);

    result = curve1.interpolate(
          curve2, ((double) maxZ - z) / (maxZ - minZ), shift, result);
    if (result != NULL) {
      result->resampleF(50);
    }
  }

  return result;

}

ZClosedCurve ZFlyEmRoiProject::estimateRoi(int z)
{
  ZClosedCurve roiCurve;

  estimateRoi(z, &roiCurve);

  return roiCurve;
}

void ZFlyEmRoiProject::estimateRoi()
{
  if (m_dataFrame != NULL) {
    ZClosedCurve roiCurve = estimateRoi(getDataZ());
    if (!roiCurve.isEmpty()) {
      m_dataFrame->document()->removeObject(ZDocPlayer::ROLE_ROI, true);
      ZSwcTree *tree = ZSwcGenerator::createSwc(roiCurve, getMarkerRadius());
      m_dataFrame->document()->addObject(tree, NeuTube::Documentable_SWC,
                                         ZDocPlayer::ROLE_ROI);
    }
  }
}

void ZFlyEmRoiProject::setRoiUploaded(int z, bool uploaded)
{
  if (z >= (int) m_isRoiCurveUploaded.size()) {
    m_isRoiCurveUploaded.resize(z + 1, false);
  }

  m_isRoiCurveUploaded[z] = uploaded;
}

bool ZFlyEmRoiProject::isRoiCurveUploaded(int z) const
{
  if (z >= (int) m_isRoiCurveUploaded.size()) {
    return false;
  }

  return m_isRoiCurveUploaded[z];
}

int ZFlyEmRoiProject::findSliceToCreateRoi() const
{
  int minZ = (getDvidInfo().getMinZ() + getDataZ()) / 2;
  int maxZ = (getDvidInfo().getMaxZ() + getDataZ()) / 2;
  std::pair<int, int> bestSeg(0, 1);

  int length = 0;

  int startZ = minZ;
  int z = startZ;
  for (; z <= maxZ; ++z) {
    const ZClosedCurve *curve = getRoi(z);
    if (curve == NULL) {
      ++length;
    } else {
      int maxLength = bestSeg.second - bestSeg.first - 1;
      if (length > maxLength) {
        length = 0;
        bestSeg.first = startZ;
        bestSeg.second = z;
      }
      startZ = z;
    }
  }

  if (z < maxZ) {
    length = maxZ - z - 1;
    int maxLength = bestSeg.second - bestSeg.first - 1;
    if (length > maxLength) {
      bestSeg.first = z;
      bestSeg.second = maxZ;
    }
  }

  int targetZ = -1;
  if (bestSeg.second - bestSeg.first > 1) {
    targetZ = (bestSeg.second + bestSeg.first) / 2;
  }

  return targetZ;
}

ZObject3dScan* ZFlyEmRoiProject::getFilledRoi(
    const ZClosedCurve *curve, int z, ZObject3dScan *result) const
{
  if (result != NULL) {
    result->clear();
  }

  if (curve != NULL) {
    ZStroke2d stroke;
    stroke.setZ(z);
    stroke.setWidth(1.0);
    for (size_t i = 0; i < curve->getLandmarkNumber(); ++i) {
      ZPoint pt = curve->getLandmark(i);
      stroke.append(pt.x(), pt.y());
    }
    ZStack *stack = ZStackFactory::makePolygonPicture(stroke);
    if (stack != NULL) {
      if (result == NULL) {
        result = new ZObject3dScan;
      }

      result->loadStack(*stack);
      delete stack;
    }
  }

  return result;
}

ZObject3dScan* ZFlyEmRoiProject::getFilledRoi(int z, ZObject3dScan *result) const
{
  const ZClosedCurve *curve = getRoi(z);

  return getFilledRoi(curve, z, result);
}

ZObject3dScan ZFlyEmRoiProject::getFilledRoi(int z) const
{
  ZObject3dScan obj;

  getFilledRoi(z, &obj);

  return obj;
}

int ZFlyEmRoiProject::getFirstRoiZ() const
{
  for (int z = 0; z < (int) m_curveArray.size(); ++z) {
    if (getRoi(z) != NULL) {
      return z;
    }
  }

  return -1;
}

int ZFlyEmRoiProject::getLastRoiZ() const
{
  for (int z = (int) m_curveArray.size(); z >= 0; --z) {
    if (getRoi(z) != NULL) {
      return z;
    }
  }

  return -1;
}

ZObject3dScan ZFlyEmRoiProject::getRoiObject() const
{
  ZObject3dScan obj;

  ZObject3dScan sliceObj;

  int minZ = getFirstRoiZ();
  int maxZ = getLastRoiZ();

  for (int z = minZ; z <= maxZ; ++z) {
    const ZClosedCurve *roiCurve = getRoi(z);
    if (roiCurve == NULL) {
      roiCurve = estimateRoi(z, NULL);
    }
    if (!roiCurve->isEmpty()) {
      getFilledRoi(roiCurve, z, &sliceObj);
      obj.concat(sliceObj);
    }
    if (getRoi(z) == NULL) {
      delete roiCurve;
    }
  }

  return obj;
}

void ZFlyEmRoiProject::translateRoiSwc(double dx, double dy)
{
  if (m_dataFrame != NULL) {
    ZSwcTree *tree = m_dataFrame->document()->getSwcTree(0);
    if (tree != NULL) {
      tree->translate(dx, dy, 0.0);
      m_dataFrame->document()->notifySwcModified();
    }
  }
}

void ZFlyEmRoiProject::scaleRoiSwc(double sx, double sy)
{
  if (m_dataFrame != NULL) {
    ZSwcTree *tree = m_dataFrame->document()->getSwcTree(0);
    if (tree != NULL) {
      ZPoint center = tree->computeCentroid();
      tree->translate(-center);
      tree->scale(sx, sy, 1.0);
      tree->translate(center);
      m_dataFrame->document()->notifySwcModified();
    }
  }
}

void ZFlyEmRoiProject::rotateRoiSwc(double theta)
{
  if (m_dataFrame != NULL) {
    ZSwcTree *tree = m_dataFrame->document()->getSwcTree(0);
    if (tree != NULL) {
      ZPoint center = tree->computeCentroid();
      tree->rotate(0.0, theta, center);
      m_dataFrame->document()->notifySwcModified();
    }
  }
}

ZIntCuboid ZFlyEmRoiProject::estimateBoundBox(const ZStack &stack)
{
  const static uint8_t fgValue = 255;
  int width = stack.width();
  int height = stack.height();
  const uint8_t *array = stack.array8();

  ZIntCuboid cuboid = stack.getBoundBox();
  //Left bound
  for (int y = 0; y < height; y += 100) {
    size_t offset = width * y;
    for (int x = 0; x < width; ++x) {
      if (array[offset++] != fgValue) {
        if (x > cuboid.getFirstCorner().getX()) {
          cuboid.setFirstX(x);
        }
        break;
      }
    }
  }

  //Right bound
  for (int y = 0; y < height; y += 100) {
    size_t offset = width * y + width - 1;
    for (int x = width - 1; x >= 0; --x) {
      if (array[offset--] != fgValue) {
        if (x < cuboid.getLastCorner().getX()) {
          cuboid.setLastX(x);
        }
        break;
      }
    }
  }

  //Up bound
  for (int x = 0; x < width; x += 100) {
    size_t offset =x;
    for (int y = 0; y < height; ++y) {
      if (array[offset] != fgValue) {
        if (y > cuboid.getFirstCorner().getY()) {
          cuboid.setFirstY(y);
        }
        break;
      }
      offset += width;
    }
  }

  //Down bound
  size_t area = width * height;
  for (int x = 0; x < width; x += 100) {
    size_t offset = area - 1 - x;
    for (int y = height - 1; y >= 0; --y) {
      if (array[offset] != fgValue) {
        if (y < cuboid.getLastCorner().getY()) {
          cuboid.setLastY(y);
        }
        break;
      }
      offset -= width;
    }
  }

  return cuboid;
}