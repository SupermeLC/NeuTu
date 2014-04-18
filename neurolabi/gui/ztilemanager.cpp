#include "ztilemanager.h"
#include <QGraphicsSceneMouseEvent>
#include <QApplication>

#include "neutubeconfig.h"
#include "zjsonobject.h"
#include "zjsonparser.h"
#include "ztileinfo.h"
#include "ztilegraphicsitem.h"
#include "zstackframe.h"
#include "zstackdoc.h"
#include "zstack.hxx"
#include "zstackpresenter.h"
#include "zstackview.h"

const QColor ZTileManager::m_selectionColor = QColor(255, 0, 0);
const QColor ZTileManager::m_preselectionColor = QColor(0, 255, 0);

ZTileManager::ZTileManager(QObject *parent) : QGraphicsScene(parent),
  m_selectedTileItem(NULL), m_preselected(NULL)/*, m_selectDecoration(NULL)*/
{
}

ZTileManager::~ZTileManager()
{
}

void ZTileManager::initDecoration()
{
  /*
  m_selectDecoration = new QGraphicsRectItem;
  m_selectDecoration->setFlag(QGraphicsItem::ItemIsSelectable, false);
  m_selectDecoration->setZValue(-1.0); //always on top
  m_selectDecoration->setPen(QPen(QColor(255, 0, 0)));
  addItem(m_selectDecoration);
  */
}

ZTileGraphicsItem* ZTileManager::getFirstTile()
{
  QList<QGraphicsItem*> itemList = items();
  foreach (QGraphicsItem *item, itemList) {
    ZTileGraphicsItem *firstItem = dynamic_cast<ZTileGraphicsItem*>(item);
    if (firstItem != NULL) {
      return firstItem;
    }
  }

  return NULL;
}

bool ZTileManager::importJsonFile(const QString &filePath)
{
  clear();

  bool succ = false;
  if (!filePath.isEmpty()) {
    ZJsonObject obj;
    obj.load(filePath.toStdString());

    if (obj.hasKey("Tiles")) {
      json_t *value = obj["Tiles"];
      if (ZJsonParser::isArray(value)) {
        ZJsonArray array(value, false);
        for (size_t i = 0; i < array.size(); ++i) {
          ZJsonObject tileObj(array.at(i), false);
          if (!tileObj.isEmpty()) {
            ZTileGraphicsItem *tileItem = new ZTileGraphicsItem;
            if (tileItem->loadJsonObject(tileObj)) {
              //tileItem->setFlag(QGraphicsItem::ItemIsSelectable);
              addItem(tileItem);
              succ = true;
            } else {
              delete tileItem;
            }
          }
        }
      }
    }
  }

  if (succ) {
    if (getFirstTile() != NULL) {
      m_selectedTileItem = NULL;
      m_preselected = NULL;
      initDecoration();
    } else {
      succ = false;
    }
  }

  return succ;
}

ZStackFrame* ZTileManager::getParentFrame() const
{
  return dynamic_cast<ZStackFrame*>(parent());
}

void ZTileManager::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  ZTileGraphicsItem* hitItem =
      dynamic_cast<ZTileGraphicsItem*>(
        itemAt(event->scenePos().x(), event->scenePos().y(), QTransform()));

  if (hitItem != NULL) {
    preselectItem(hitItem);
  }
}

void ZTileManager::clearPreselected()
{
  if (m_preselected != NULL) {
    m_preselected->setPen(QPen(QColor(0, 0, 0)));
    m_preselected = NULL;
  }
}

void ZTileManager::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  clearPreselected();

  ZTileGraphicsItem* hitItem =
      dynamic_cast<ZTileGraphicsItem*>(
        itemAt(event->scenePos().x(), event->scenePos().y(), QTransform()));

  if (hitItem != NULL) {
    selectItem(hitItem);
  }
}

void ZTileManager::updateTileStack()
{
  ZStackFrame *frame = getParentFrame();
  if (frame != NULL && m_selectedTileItem != NULL) {
    std::string source = m_selectedTileItem->getTileInfo().getSource();
    if (source != std::string(frame->document()->stackSourcePath())) {
      startProgress();
      advanceProgress(0.5);
      QApplication::processEvents();
      frame->document()->readStack(source.c_str(), false);
      if (GET_APPLICATION_NAME == "Biocytin") {
        frame->autoBcAdjust();
        frame->loadRoi();
      }
      frame->setWindowTitle(source.c_str());
      endProgress();
    }
  }
}

void ZTileManager::preselectItem(ZTileGraphicsItem *item)
{
  if (m_preselected != item && m_selectedTileItem != item && item != NULL) {
    m_preselected = item;
    m_preselected->setPen(QPen(m_preselectionColor));
  }
}

void ZTileManager::selectItem(ZTileGraphicsItem *item)
{
  if (m_selectedTileItem != item && item != NULL) {
    if (m_selectedTileItem != NULL) {
      m_selectedTileItem->setPen(QPen(QColor(0, 0, 0)));
    }
    m_selectedTileItem = item;
    m_selectedTileItem->setPen(QPen(m_selectionColor));
    if (m_selectedTileItem == m_preselected) {
      m_preselected = NULL;
    }
    //m_selectDecoration->setRect(m_selectedTileItem->rect());
    //qDebug() << m_selectDecoration->rect();
    updateTileStack();
  }
}
