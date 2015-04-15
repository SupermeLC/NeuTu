#include "zdvidtile.h"

#include <QTransform>
#include <QtConcurrentRun>
#include "zimagewidget.h"
#include "neutubeconfig.h"
#include "zstack.hxx"
#include "zstackfactory.h"
#include "zdvidreader.h"
#include "zimage.h"
#include "zpainter.h"
#include "zdvidbufferreader.h"
#include "zdvidurl.h"
#include "zdvidtileinfo.h"
#include "zdvidreader.h"
#include "zstackview.h"
#include "zrect2d.h"

ZDvidTile::ZDvidTile() : m_ix(0), m_iy(0), m_z(0), m_view(NULL)
{
  setTarget(ZStackObject::OBJECT_CANVAS);
  m_type = ZStackObject::TYPE_DVID_TILE;
}

ZDvidTile::~ZDvidTile()
{
  clear();
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidTile)

void ZDvidTile::clear()
{
  m_dvidTarget.clear();
}

void ZDvidTile::loadDvidPng(const QByteArray &buffer, int z)
{
  bool loading = true;
  if (m_view != NULL) {
    if (m_view->getZ(NeuTube::COORD_STACK) != z) {
      loading = false;
    }
  }
  if (loading) {
#ifdef _DEBUG_
      std::cout << z << " Loaded." << std::endl;
#endif
    m_image.loadFromData(buffer);
//    m_image.setOffset();
    m_z = z;
  }

//  m_image.save((GET_TEST_DATA_DIR + "/test.tif").c_str());
}

void ZDvidTile::display(
    ZPainter &painter, int slice, EDisplayStyle /*option*/) const
{
  //if (!m_image.isNull()) {
    int z = painter.getZOffset() + slice;
    m_latestZ = z;

    const_cast<ZDvidTile&>(*this).update(z);

    if (z == m_z && !m_image.isNull()) {
#ifdef _DEBUG_
      std::cout << "Display " << z << std::endl;
#endif
//      ZImage image = getImage();
      //int dx = getX() - painter.getOffset().x();
      //int dy = getY() - painter.getOffset().y();

//      QRect sourceRect = QRect(0, 0, m_image.width(), m_image.height());
//      QRect targetRect = QRect(getX(), getY(), m_image.width() * m_res.getScale(),
//                         m_image.height() * m_res.getScale());
#if 0
      if (m_res.getScale() == 1) {
        m_image.save((GET_DATA_DIR + "/test.tif").c_str());
      }
#endif

      painter.drawImage(getX(), getY(), m_image);

//      ZIntPoint pt = m_offset - painter.getOffset().toIntPoint();

//      painter.save();

//      QTransform transform;

//      transform.scale(m_res.getScale(), m_res.getScale());
//      transform.translate(getX(), getY());

//      //transform.translate(pt.x(), pt.y());
//      painter.setTransform(transform);
//      painter.drawImage(m_image);

//      painter.restore();
    //}
  }
}
#if 0
void ZDvidTile::update(int x, int y, int z, int width, int height)
{

  bool updating = false;
  if (m_stack == NULL) {
    m_stack = ZStackFactory::makeZeroStack(GREY, width, height, 1);
    m_stack->setOffset(x, y, z);
    updating = true;
  } else if (m_stack->getOffset().getZ() != z ||
             m_stack->getOffset().getX() != x ||
             m_stack->getOffset().getZ() != z ||
             m_stack->width() != width || m_stack->height() != height) {
    updating = true;
  }

  if (updating) {
    ZDvidReader reader;
    if (reader.open(m_dvidTarget)) {
      Stack *stack = reader.readTile(x, y, z, width, heigth, m_res.getLevel());
    }
  }

}
#endif
void ZDvidTile::setTileIndex(int ix, int iy)
{
  m_ix = ix;
  m_iy = iy;
}

void ZDvidTile::update(int z)
{
  if (m_z != z || m_image.isNull()) {
    ZDvidUrl dvidUrl(getDvidTarget());
    ZDvidBufferReader bufferReader;

    /*
    QFuture<void> result = QtConcurrent::run(
          &bufferReader, &ZDvidBufferReader::read,
          QString(dvidUrl.getTileUrl("graytiles", m_res.getLevel(), m_ix, m_iy, z).c_str()));
    result.waitForFinished();
    */

    bufferReader.read(
          dvidUrl.getTileUrl(getDvidTarget().getMultiscale2dName(),
                             m_res.getLevel(), m_ix, m_iy, z).c_str());
    QByteArray buffer = bufferReader.getBuffer();

//    ZDvidTileInfo tileInfo = readTileInfo("graytiles");

    if (!buffer.isEmpty()) {
      loadDvidPng(buffer, z);
      m_image.setScale(1.0 / m_res.getScale(), 1.0 / m_res.getScale());
      m_image.setOffset(-getX(), -getY());
      //      setResolutionLevel(m_res.getLevel());
    }
  }
  //m_z = z;
}
#if 0
void ZDvidTile::setTileOffset(int x, int y, int z)
{
  m_offset.set(x, y, z);
}
#endif

void ZDvidTile::printInfo() const
{
  std::cout << "Dvid tile: " << std::endl;
  m_res.print();
  std::cout << "Offset: " << getX() << ", " << getY() << ", " << getZ() << std::endl;
  std::cout << "Size: " << m_image.width() << " x " << m_image.height()
            << std::endl;

}

void ZDvidTile::setResolutionLevel(int level)
{
  m_res.setLevel(level);
}

void ZDvidTile::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  if (!m_tilingInfo.isValid()) {
    ZDvidReader reader;
    if (reader.open(target)) {
      m_tilingInfo = reader.readTileInfo(target.getMultiscale2dName());
    }
  }
}

int ZDvidTile::getX() const
{
  return m_ix * m_tilingInfo.getWidth() * m_res.getScale();
}

int ZDvidTile::getWidth() const
{
  return m_tilingInfo.getWidth() * m_res.getScale();
}

int ZDvidTile::getHeight() const
{
  return m_tilingInfo.getHeight() * m_res.getScale();
}

int ZDvidTile::getY() const
{
  return m_iy * m_tilingInfo.getHeight() * m_res.getScale();
}

int ZDvidTile::getZ() const
{
  return m_z;
}

void ZDvidTile::attachView(ZStackView *view)
{
  m_view = view;
}

ZRect2d ZDvidTile::getBoundBox() const
{
  ZRect2d rect;
  rect.set(getX(), getY(), getWidth(), getHeight());

  return rect;
}
