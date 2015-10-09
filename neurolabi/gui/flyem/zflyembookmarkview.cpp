#include "zflyembookmarkview.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QSortFilterProxyModel>

#include "zflyembookmarklistmodel.h"

ZFlyEmBookmarkView::ZFlyEmBookmarkView(QWidget *parent) :
  QTableView(parent)
{
  init();
}

void ZFlyEmBookmarkView::init()
{
  m_bookmarkModel = NULL;
  m_proxy = NULL;

  setFocusPolicy(Qt::NoFocus);

  createMenu();
  connectSignalSlot();
}

void ZFlyEmBookmarkView::connectSignalSlot()
{
  connect(this, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(processDouleClick(QModelIndex)));
}

void ZFlyEmBookmarkView::processDouleClick(const QModelIndex &index)
{
  const ZFlyEmBookmark *bookmark = getBookmark(index);
  if (bookmark != NULL) {
    emit locatingBookmark(bookmark);
  }
}

void ZFlyEmBookmarkView::setBookmarkModel(ZFlyEmBookmarkListModel *model)
{
  m_bookmarkModel = model;

  if (m_proxy == NULL) {
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    setModel(m_proxy);
    setSortingEnabled(true);
  }

  m_proxy->setSourceModel(m_bookmarkModel);
}

void ZFlyEmBookmarkView::createMenu()
{
  m_contextMenu = new QMenu(this);
  QAction *checkAction = new QAction("Set Checked", this);
  m_contextMenu->addAction(checkAction);
  connect(checkAction, SIGNAL(triggered()), this, SLOT(checkCurrentBookmark()));

  QAction *unCheckAction = new QAction("Uncheck", this);
  m_contextMenu->addAction(unCheckAction);
  connect(unCheckAction, SIGNAL(triggered()),
          this, SLOT(uncheckCurrentBookmark()));
}

void ZFlyEmBookmarkView::contextMenuEvent(QContextMenuEvent *event)
{
#ifdef _DEBUG_2
  std::cout << "Context menu triggered." << std::endl;
#endif

  if (m_contextMenu != NULL) {
    m_contextMenu->popup(event->globalPos());
  }
}

void ZFlyEmBookmarkView::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton) {
    QTableView::mousePressEvent(event);
  }
}

void ZFlyEmBookmarkView::keyPressEvent(QKeyEvent *event)
{
  event->ignore();
}

ZFlyEmBookmarkListModel* ZFlyEmBookmarkView::getModel() const
{
  return m_bookmarkModel;
//  return qobject_cast<ZFlyEmBookmarkListModel*>(model());
}

const ZFlyEmBookmark* ZFlyEmBookmarkView::getBookmark(
    const QModelIndex &viewIndex) const
{
  QModelIndex index = viewIndex;
  if (m_proxy != NULL) {
    index = m_proxy->mapToSource(viewIndex);
  }

  if (getModel() != NULL) {
    return getModel()->getBookmark(index.row());
  }

  return NULL;
}

void ZFlyEmBookmarkView::checkCurrentBookmark(bool checking)
{
  QItemSelectionModel *sel = selectionModel();
  QItemSelection sourceSelection = m_proxy->mapSelectionToSource(sel->selection());

  QModelIndexList selected = sourceSelection.indexes();

  foreach (const QModelIndex &index, selected) {
    ZFlyEmBookmark *bookmark = getModel()->getBookmark(index.row());
    bookmark->setChecked(checking);
    getModel()->update(index.row());

    emit bookmarkChecked(bookmark);
//    emit bookmarkChecked(bookmark.getDvidKey(), checking);
  }
}

void ZFlyEmBookmarkView::checkCurrentBookmark()
{
  checkCurrentBookmark(true);
}

void ZFlyEmBookmarkView::uncheckCurrentBookmark()
{
  checkCurrentBookmark(false);
}

void ZFlyEmBookmarkView::sort()
{
  if (m_proxy != NULL) {
    m_proxy->sort(m_proxy->sortColumn(), m_proxy->sortOrder());
  }
}
