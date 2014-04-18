#ifndef ZTILEMANAGER_H
#define ZTILEMANAGER_H

#include <QGraphicsScene>
#include <QVector>
#include "zprogressable.h"

class ZStackFrame;
class ZTileGraphicsItem;

class ZTileManager : public QGraphicsScene, public ZProgressable
{
  Q_OBJECT
public:
  explicit ZTileManager(QObject *parent = 0);
  ~ZTileManager();

  /*!
   * \brief Load tiles from a json file
   *
   * \return true iff the tile is loaded successfully.
   */
  bool importJsonFile(const QString &filePath);

  ZStackFrame *getParentFrame() const;

  /*!
   * \brief getFirstTile
   * \return
   */
  ZTileGraphicsItem* getFirstTile();

  void preselectItem(ZTileGraphicsItem *item);
  void selectItem(ZTileGraphicsItem *item);
  void updateTileStack();

signals:

public slots:

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent * mouseEvent);
  void initDecoration();
  void clearPreselected();

private:
  ZTileGraphicsItem *m_selectedTileItem;
  ZTileGraphicsItem *m_preselected;

  static const QColor m_preselectionColor;
  static const QColor m_selectionColor;
  //QGraphicsRectItem *m_selectDecoration; //always owned by the scene
};

#endif // ZTILEMANAGER_H
