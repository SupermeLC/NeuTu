#include <QtGui>

#include <numeric>
#include <algorithm>
#include <sstream>

#include "zpunctum.h"
#include "zrandom.h"
#include "zstackball.h"

#define INIT_PUNCTUM m_score(1.0)

ZPunctum::ZPunctum()
  : INIT_PUNCTUM
{
  setColor(255, 255, 0, 255);
  setCenter(-1, -1, -1);
  setRadius(2.0);
  setMaxIntensity(255);
  setMeanIntensity(255);
  setSDevOfIntensity(0);
  updateVolSize();
  updateMass();
  setVisualEffect(ZStackBall::VE_OUT_FOCUS_DIM);
  m_type = ZStackObject::TYPE_PUNCTUM;
}

ZPunctum::ZPunctum(double x, double y, double z, double r)
  : INIT_PUNCTUM
{
  setColor(255, 255, 0, 255);
  setCenter(x, y, z);
  setRadius(r);
  setMaxIntensity(255);
  setMeanIntensity(255);
  setSDevOfIntensity(0);
  updateVolSize();
  updateMass();
  setVisualEffect(ZStackBall::VE_OUT_FOCUS_DIM);
  m_type = ZStackObject::TYPE_PUNCTUM;
}

ZPunctum::~ZPunctum()
{
}

//void ZPunctum::display(QImage *image, int n, Display_Style style) const
//{
//  if (!isVisible())
//    return;

//  if (style == NORMAL) {
//    style = SOLID;
//  }

//  QPainter painter(image);
//  if (m_selected == true) {
//    painter.setPen(QPen(selectingColor(m_color)));
//  } else {
//    painter.setPen(QPen(m_color));
//  }

//  switch (style) {
//  case SOLID: {
//    if ((iround(m_z) == n) || (n == -1)) {
//      int half_size = iround(m_radius - 0.5);
//      int cx = iround(m_x);
//      int cy = iround(m_y);
//      painter.drawRect(cx - half_size, cy - half_size,
//                       half_size * 2 + 1, half_size * 2 + 1);
//    }
//    break;
//  }
//  case SKELETON:
//    break;
//  case BOUNDARY: {
//    double r = m_radius;
//    bool visible = false;

//    if ((iround(m_z) == n) || (n == -1)) {
//      visible = true;
//    } else if (fabs(m_z - n) < r) {
//      r = sqrt(r * r - (m_z - n) * (m_z - n));
//      visible = true;
//    }

//    if (visible) {
//      if (m_selected == true) {
//        painter.setPen(QPen(selectingColor(m_color)));
//      } else {
//        painter.setPen(QPen(m_color));
//      }
//      painter.drawEllipse(QPointF(m_x, m_y), r, r);
//    }
//    break;
//  }
//  default:
//    break;
//  }
//}

#if 0
void ZPunctum::display(ZPainter &painter, int n, ZStackObject::Display_Style style) const
{
  if (!isVisible())
    return;

  bool isVisible = true;

  double dataFocus = n - painter.getOffset().z();

  if (!ZStackBall::isCuttingPlane(m_z, getRadius(), dataFocus, m_zScale)) {
    isVisible = false;
  }

  if (isVisible) {
    ZStackBall circle(m_x, m_y, m_z, m_radius);
    circle.setZScale(m_zScale);
    circle.setColor(m_color);
    circle.useCosmeticPen(m_usingCosmeticPen);
    circle.setVisualEffect(ZStackBall::VE_OUT_FOCUS_DIM);

    /*
    if (style == SOLID) {
      circle.setVisualEffect(ZCircle::VE_GRADIENT_FILL);
    }
    */
    circle.display(painter, n, style);
  }

#if 0
  if (n >= 0) {
    if (fabs(dataFocus - m_z) > m_radius) {
      isVisible = false;
    }
  }

  if (isVisible) {
    if (style == NORMAL) {
      style = SOLID;
    }

    if (m_selected == true) {
      painter.setPen(QPen(selectingColor(m_color), 1.5));
    } else {
      painter.setPen(QPen(m_color, .7));
    }

    switch (style) {
    case SOLID: {
      if ((iround(m_z) == dataFocus) || (n == -1)) {
        int half_size = iround(m_radius - 0.5);
        int cx = iround(m_x);
        int cy = iround(m_y);
        painter.drawRect(cx - half_size, cy - half_size,
                         half_size * 2 + 1, half_size * 2 + 1);
      }
      break;
    }
    case SKELETON:
      break;
    case BOUNDARY: {
      double r = m_radius;
      bool visible = false;

      if ((iround(m_z) == iround(dataFocus)) || (n == -1)) {
        visible = true;
      } else if (fabs(m_z - dataFocus) < r) {
        r = sqrt(r * r - (m_z - dataFocus) * (m_z - dataFocus));
        visible = true;
      }

      if (visible) {
        if (m_selected == true) {
          painter.setPen(QPen(selectingColor(m_color), 1.5));
        } else {
          painter.setPen(QPen(m_color, .7));
        }
        painter.drawEllipse(QPointF(m_x, m_y), r, r);
      }
      break;
    }
    default:
      break;
    }
  }
#endif
}
#endif


QList<ZPunctum *> ZPunctum::deepCopyPunctaList(const QList<ZPunctum *> &src)
{
  QList<ZPunctum*> des;
  for (int i=0; i<src.size(); ++i) {
    des.push_back(new ZPunctum(*(src[i])));
  }
  return des;
}

void ZPunctum::setSelected(bool selected)
{
  ZStackObject::setSelected(selected);
}

QColor ZPunctum::highlightingColor(const QColor &color) const
{
  QColor highlight;

  highlight.setRed(imin2(255, color.red() + 96));
  highlight.setGreen(imin2(255, color.green() + 96));
  highlight.setBlue(imin2(255, color.blue() + 96));

  return highlight;
}

QColor ZPunctum::selectingColor(const QColor &color) const
{
  QColor select;

  select.setHsv((color.hue() + 60) % 360,
                /*color.saturation()*/255, 255/*color.value()*/);

  return select;
}

std::string ZPunctum::toString()
{
  std::ostringstream stream;

  stream << "Puncta(" << m_name.toStdString() << "): "
         << "(" << getX() << ", " << getY() << ", " << getZ() << ")";

  return stream.str();
}

void ZPunctum::setFromMarker(const ZVaa3dMarker &marker)
{
  set(marker.x(), marker.y(), marker.z(), marker.radius());
  setColor(marker.colorR(), marker.colorG(), marker.colorB());
  setComment(marker.comment().c_str());
  setName(marker.name().c_str());
  setSource(QString("%1").arg(marker.type()).toStdString());
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZPunctum)
