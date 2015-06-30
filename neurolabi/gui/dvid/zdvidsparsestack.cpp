#include "zdvidsparsestack.h"
#include <QImage>

#include "zdvidinfo.h"
#include "zdvidreader.h"
#include "zpainter.h"
#include "zimage.h"

ZDvidSparseStack::ZDvidSparseStack()
{
  setTarget(ZStackObject::TARGET_OBJECT_CANVAS);
  m_type = ZStackObject::TYPE_DVID_SPARSE_STACK;
  m_isValueFilled = false;
}

ZDvidSparseStack::~ZDvidSparseStack()
{
#ifdef _DEBUG_
  std::cout << "Deleting dvid sparsestack: " << ": " << getSource() << std::endl;
#endif
}

ZStack* ZDvidSparseStack::getSlice(int z) const
{
  ZStack *stack = NULL;

  const ZObject3dScan *objectMask = m_sparseStack.getObjectMask();
  if (objectMask != NULL) {
    ZObject3dScan slice = objectMask->getSlice(z);
    ZIntCuboid box = slice.getBoundBox();
    stack = new ZStack(GREY, box, 1);
    stack->setZero();

    size_t stripeNumber = slice.getStripeNumber();
    for (size_t i = 0; i < stripeNumber; ++i) {
      const ZObject3dStripe &stripe = slice.getStripe(i);
      int y = stripe.getY();
      int z = stripe.getZ();
      int segmentNumber = stripe.getSegmentNumber();
      for (int j = 0; j < segmentNumber; ++j) {
        int x0 = stripe.getSegmentStart(j);
        int x1 = stripe.getSegmentEnd(j);

        for (int x = x0; x <= x1; ++x) {
          int v = getValue(x, y, z);
          stack->setIntValue(x, y, z, 0, v);
        }
      }
    }
  }

  return stack;
}

void ZDvidSparseStack::initBlockGrid()
{

  if (m_dvidReader.open(getDvidTarget())) {
    ZDvidInfo dvidInfo = m_dvidReader.readGrayScaleInfo();
    ZStackBlockGrid *grid = new ZStackBlockGrid;
    m_sparseStack.setGreyScale(grid);
    grid->setMinPoint(dvidInfo.getStartCoordinates());
    grid->setBlockSize(dvidInfo.getBlockSize());
    grid->setGridSize(dvidInfo.getGridSize());
  }
}

void ZDvidSparseStack::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  initBlockGrid();
}

int ZDvidSparseStack::getValue(int x, int y, int z) const
{
  int v = 0;
  ZStackBlockGrid *stackGrid = const_cast<ZStackBlockGrid*>(
        m_sparseStack.getStackGrid());
  if (stackGrid != NULL) {
    ZStackBlockGrid::Location location = stackGrid->getLocation(x, y, z);
    const ZIntPoint& blockIndex = location.getBlockIndex();
    if (stackGrid->getStack(blockIndex) == NULL) {
      ZIntCuboid box = stackGrid->getBlockBox(blockIndex);
      ZStack *stack = m_dvidReader.readGrayScale(box);
      stackGrid->consumeStack(blockIndex, stack);
    }

    ZStack *blockStack = stackGrid->getStack(blockIndex);
    v = blockStack->getIntValueLocal(location.getLocalPosition().getX(),
                                     location.getLocalPosition().getY(),
                                     location.getLocalPosition().getZ());
  }

  return v;
}

void ZDvidSparseStack::display(
    ZPainter &painter, int slice, EDisplayStyle option) const
{
  if (slice < 0) {
    if (m_sparseStack.getObjectMask() != NULL) {
      m_sparseStack.getObjectMask()->display(painter, slice, option);
    }
  } else {
#if 0
    int z = painter.getZOffset() + slice;
    ZStack *stack = getSlice(z);
    if (stack != NULL) {
      ZImage image(stack->width(), stack->height());
      image.fill(0);
      image.setData(stack, z, true, false);
      painter.drawImage(stack->getOffset().getX(), stack->getOffset().getY(),
                        image);
    }
#endif

    if (m_sparseStack.getObjectMask() != NULL) {
      m_sparseStack.getObjectMask()->display(painter, slice, option);
    }
  }
}

ZIntCuboid ZDvidSparseStack::getBoundBox() const
{
  ZIntCuboid box;
  if (m_sparseStack.getObjectMask() != NULL) {
    box = m_sparseStack.getObjectMask()->getBoundBox();
  }

  return box;
}

void ZDvidSparseStack::loadBody(int bodyId)
{
  ZObject3dScan *obj = new ZObject3dScan;
  m_dvidReader.readBody(bodyId, obj);
  m_sparseStack.setObjectMask(obj);

  m_isValueFilled = false;
}

void ZDvidSparseStack::setMaskColor(const QColor &color)
{
  if (m_sparseStack.getObjectMask() != NULL) {
    m_sparseStack.getObjectMask()->setColor(color);
  }
}

const ZIntPoint& ZDvidSparseStack::getDownsampleInterval() const
{
  return m_sparseStack.getDownsampleInterval();
}

bool ZDvidSparseStack::fillValue(const ZIntCuboid &box)
{
  if (m_isValueFilled) {
    return true;
  }

//  bool changed = false;
  int blockCount = 0;
  if (!m_sparseStack.getObjectMask()->isEmpty()) {
    qDebug() << "Downloading grayscale ...";
//    tic();
    ZDvidInfo dvidInfo;
    dvidInfo.setFromJsonString(
          m_dvidReader.readInfo(getDvidTarget().getGrayScaleName().c_str()).
          toStdString());
    ZObject3dScan blockObj =
        dvidInfo.getBlockIndex(*(m_sparseStack.getObjectMask()));
    ZStackBlockGrid *grid = m_sparseStack.getStackGrid();

    size_t stripeNumber = blockObj.getStripeNumber();
    ZIntCuboid blockBox;
    blockBox.setFirstCorner(dvidInfo.getBlockIndex(box.getFirstCorner()));
    blockBox.setLastCorner(dvidInfo.getBlockIndex(box.getLastCorner()));


    for (size_t s = 0; s < stripeNumber; ++s) {
      const ZObject3dStripe &stripe = blockObj.getStripe(s);
      int segmentNumber = stripe.getSegmentNumber();
      int y = stripe.getY();
      int z = stripe.getZ();
      for (int i = 0; i < segmentNumber; ++i) {
        int x0 = stripe.getSegmentStart(i);
        int x1 = stripe.getSegmentEnd(i);

        for (int x = x0; x <= x1; ++x) {
          if (blockBox.contains(x, y, z)) {
            const ZIntPoint blockIndex =
                ZIntPoint(x, y, z) - dvidInfo.getStartBlockIndex();

            if (grid->getStack(blockIndex) == NULL) {
              ZIntCuboid box = grid->getBlockBox(blockIndex);
              ZStack *stack = m_dvidReader.readGrayScale(box);
              grid->consumeStack(blockIndex, stack);
              ++blockCount;
//              changed = true;
            }
          }
        }
      }
    }
//    ptoc();

    qDebug() << blockCount << " blocks downloaded.";
  }

  return (blockCount > 0);
}

bool ZDvidSparseStack::fillValue()
{
//  bool changed = false;
  int blockCount = 0;
  if (!m_sparseStack.getObjectMask()->isEmpty()) {
//    std::cout << "Downloading grayscale ..." << std::end;
    qDebug() << "Downloading grayscale ...";
    ZDvidInfo dvidInfo;
    dvidInfo.setFromJsonString(
          m_dvidReader.readInfo(getDvidTarget().getGrayScaleName().c_str()).
          toStdString());
    ZObject3dScan blockObj =
        dvidInfo.getBlockIndex(*(m_sparseStack.getObjectMask()));
    ZStackBlockGrid *grid = m_sparseStack.getStackGrid();

    size_t stripeNumber = blockObj.getStripeNumber();
    for (size_t s = 0; s < stripeNumber; ++s) {
      const ZObject3dStripe &stripe = blockObj.getStripe(s);
      int segmentNumber = stripe.getSegmentNumber();
      int y = stripe.getY();
      int z = stripe.getZ();
      for (int i = 0; i < segmentNumber; ++i) {
        int x0 = stripe.getSegmentStart(i);
        int x1 = stripe.getSegmentEnd(i);

        for (int x = x0; x <= x1; ++x) {
          const ZIntPoint blockIndex =
              ZIntPoint(x, y, z) - dvidInfo.getStartBlockIndex();

          if (grid->getStack(blockIndex) == NULL) {
            ZIntCuboid box = grid->getBlockBox(blockIndex);
            ZStack *stack = m_dvidReader.readGrayScale(box);
            grid->consumeStack(blockIndex, stack);
            ++blockCount;
          }
        }
      }
    }

    m_isValueFilled = true;

    qDebug() << blockCount << " blocks downloaded.";
  }

  return blockCount > 0;
//  return changed;
}

ZStack* ZDvidSparseStack::getStack()
{
  if (fillValue()) {
    m_sparseStack.deprecate(ZSparseStack::STACK);
  }

  return m_sparseStack.getStack();
}

ZStack* ZDvidSparseStack::getStack(const ZIntCuboid &updateBox)
{
  if (fillValue(updateBox)) {
    m_sparseStack.deprecate(ZSparseStack::STACK);
  }

  return m_sparseStack.getStack();
}

uint64_t ZDvidSparseStack::getLabel() const
{
  if (getObjectMask() != NULL) {
    return getObjectMask()->getLabel();
  }

  return 0;
}

const ZObject3dScan* ZDvidSparseStack::getObjectMask() const
{
  return m_sparseStack.getObjectMask();
}

ZObject3dScan* ZDvidSparseStack::getObjectMask()
{
  return const_cast<ZObject3dScan*>(
        static_cast<const ZDvidSparseStack&>(*this).getObjectMask());
}

const ZSparseStack* ZDvidSparseStack::getSparseStack() const
{
  return &m_sparseStack;
}

ZSparseStack* ZDvidSparseStack::getSparseStack()
{
  fillValue();

  return &m_sparseStack;
}

void ZDvidSparseStack::downloadBodyMask()
{

  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZObject3dScan *obj = new ZObject3dScan;
    reader.readBody(getObjectMask()->getLabel(), obj);
    obj->setColor(getObjectMask()->getColor());
    m_sparseStack.setObjectMask(obj);
//    obj->set
  }
}

bool ZDvidSparseStack::hit(double x, double y, double z)
{
  if (getObjectMask() != NULL) {
    if (getObjectMask()->hit(x, y, z)) {
      m_hitPoint = getObjectMask()->getHitPoint();
      return true;
    }
  }

  return false;
}

bool ZDvidSparseStack::hit(double x, double y)
{
  if (getObjectMask() != NULL) {
    if (getObjectMask()->hit(x, y)) {
      m_hitPoint = getObjectMask()->getHitPoint();
      return true;
    }
  }

  return false;
}

bool ZDvidSparseStack::isEmpty() const
{
  return m_sparseStack.isEmpty();
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidSparseStack)
