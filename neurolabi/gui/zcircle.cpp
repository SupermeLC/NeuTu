#if defined(_QT_GUI_USED_)
#include <QPainter>
#include <QDebug>
#endif

#include <math.h>
#include "zcircle.h"
#include "tz_math.h"


const ZCircle::TVisualEffect ZCircle::VE_NONE = 0;
const ZCircle::TVisualEffect ZCircle::VE_DASH_PATTERN = 1;
const ZCircle::TVisualEffect ZCircle::VE_BOUND_BOX = 2;
const ZCircle::TVisualEffect ZCircle::VE_NO_CIRCLE = 4;
const ZCircle::TVisualEffect ZCircle::VE_NO_FILL = 8;

ZCircle::ZCircle() : m_visualEffect(ZCircle::VE_NONE)
{
  set(0, 0, 0, 1);
}

ZCircle::ZCircle(double x, double y, double z, double r) :
  m_visualEffect(ZCircle::VE_NONE)
{
  set(x, y, z, r);
}

void ZCircle::set(double x, double y, double z, double r)
{
  m_center.set(x, y, z);
  m_r = r;
}

//void ZCircle::display(QImage *image, int n, Display_Style style) const
//{
//#if defined(_QT_GUI_USED_)
//  QPainter painter(image);
//  painter.setPen(m_color);
//  display(&painter, n, style);
//#endif
//}

void ZCircle::display(ZPainter &painter, int n,
                      ZStackDrawable::Display_Style style) const
{
  UNUSED_PARAMETER(style);
#if _QT_GUI_USED_
  QPen pen(m_color, m_defaultPenWidth);
  if (hasVisualEffect(VE_DASH_PATTERN)) {
    pen.setStyle(Qt::DotLine);
  }

  painter.setPen(pen);

  //qDebug() << "Internal color: " << m_color;
  const QBrush &oldBrush = painter.brush();
  if (hasVisualEffect(VE_NO_FILL)) {
    painter.setBrush(Qt::NoBrush);
  }
  display(&painter, n, style);
  if (hasVisualEffect(VE_NO_FILL)) {
    painter.setBrush(oldBrush);
  }
#endif
}

bool ZCircle::isCuttingPlane(double z, double r, double n)
{
  double h = fabs(z - n);
  if (r > h) {
    return true;
  } else if (iround(z) == iround(n)) {
    return true;
  }

  return false;
}

void ZCircle::display(ZPainter *painter, int stackFocus, Display_Style style) const
{
  UNUSED_PARAMETER(style);
#if defined(_QT_GUI_USED_)
  double adjustedRadius = m_r + m_defaultPenWidth * 0.5;
  double dataFocus = stackFocus - painter->getOffset().z();

  QRectF rect;
  if (hasVisualEffect(VE_BOUND_BOX)) {
    rect.setLeft(m_center.x() - adjustedRadius);
    rect.setTop(m_center.y() - adjustedRadius);
    rect.setWidth(adjustedRadius + adjustedRadius);
    rect.setHeight(adjustedRadius + adjustedRadius);
  }

  bool visible = false;

  if (stackFocus == -1) {
    visible = true;
  } else {
    if (isCuttingPlane(m_center.z(), m_r, dataFocus)) {
      double h = fabs(m_center.z() - dataFocus);
      if (m_r > h) {
        double r = sqrt(m_r * m_r - h * h);
        adjustedRadius = r + m_defaultPenWidth * 0.5;
        visible = true;
      } else { //too small, show at least one plane
        adjustedRadius = m_defaultPenWidth * 0.5;
        visible = true;
      }
    }
  }

  if (visible) {
    if (!hasVisualEffect(VE_NO_CIRCLE)) {
      //qDebug() << painter->brush().color();
      painter->drawEllipse(QPointF(m_center.x(), m_center.y()),
                           adjustedRadius, adjustedRadius);
    }
  }

  if (hasVisualEffect(VE_BOUND_BOX)) {
    const QBrush &oldBrush = painter->brush();
    const QPen &oldPen = painter->pen();
    painter->setBrush(Qt::NoBrush);
#if 0 //for future versions
    QPen pen = oldPen;
    QVector<qreal> pattern;
    pattern << 1 << 2;
    pen.setDashPattern(pattern);
    painter->setPen(pen);
    painter->drawRect(rect);

    pen.setColor(Qt::black);
    pen.setDashOffset(1.5);
    painter->setPen(pen);
#endif

    //QPainter::CompositionMode oldMode = painter->compositionMode();
    //painter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter->drawRect(rect);

    //painter->setCompositionMode(oldMode);
    painter->setBrush(oldBrush);
    painter->setPen(oldPen);
  }
#endif
}

void ZCircle::save(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

void ZCircle::load(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

ZINTERFACE_DEFINE_CLASS_NAME(ZCircle)
