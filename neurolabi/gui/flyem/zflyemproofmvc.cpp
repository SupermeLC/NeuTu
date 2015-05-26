#include "zflyemproofmvc.h"

#include <QFuture>
#include <QtConcurrentRun>

#include "flyem/zflyemproofdoc.h"
#include "zstackview.h"
#include "dvid/zdvidtileensemble.h"
#include "zstackpresenter.h"
#include "zdviddialog.h"
#include "dvid/zdvidreader.h"
#include "zstackobjectsourcefactory.h"
#include "dvid/zdvidsparsestack.h"
#include "zprogresssignal.h"
#include "zstackviewlocator.h"
#include "zimagewidget.h"
#include "dvid/zdvidlabelslice.h"
#include "flyem/zflyemproofpresenter.h"

ZFlyEmProofMvc::ZFlyEmProofMvc(QWidget *parent) :
  ZStackMvc(parent), m_splitOn(false), m_dvidDlg(NULL)
{
  qRegisterMetaType<uint64_t>("uint64_t");
}

ZFlyEmProofMvc* ZFlyEmProofMvc::Make(
    QWidget *parent, ZSharedPointer<ZFlyEmProofDoc> doc)
{
  ZFlyEmProofMvc *frame = new ZFlyEmProofMvc(parent);

  BaseConstruct(frame, doc);

  return frame;
}

ZFlyEmProofMvc* ZFlyEmProofMvc::Make(const ZDvidTarget &target)
{
  ZFlyEmProofDoc *doc = new ZFlyEmProofDoc(NULL, NULL);
//  doc->setTag(NeuTube::Document::FLYEM_DVID);
  ZFlyEmProofMvc *mvc =
      ZFlyEmProofMvc::Make(NULL, ZSharedPointer<ZFlyEmProofDoc>(doc));
  mvc->getPresenter()->setObjectStyle(ZStackObject::SOLID);
  mvc->setDvidTarget(target);

  return mvc;
}

ZFlyEmProofDoc* ZFlyEmProofMvc::getCompleteDocument() const
{
  return dynamic_cast<ZFlyEmProofDoc*>(getDocument().get());
}

ZFlyEmProofPresenter* ZFlyEmProofMvc::getCompletePresenter() const
{
  return dynamic_cast<ZFlyEmProofPresenter*>(getPresenter());
}

void ZFlyEmProofMvc::mergeSelected()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->mergeSelected();
  }
}

void ZFlyEmProofMvc::undo()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->undoStack()->undo();
  }
}

void ZFlyEmProofMvc::redo()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->undoStack()->redo();
  }
}

void ZFlyEmProofMvc::setSegmentationVisible(bool visible)
{
  m_showSegmentation = visible;
  if (getCompleteDocument() != NULL) {
    QList<ZDvidLabelSlice*> sliceList =
        getCompleteDocument()->getDvidLabelSliceList();
    foreach (ZDvidLabelSlice *slice, sliceList) {
      slice->setVisible(visible);
      if (visible) {
        slice->update(getView()->getViewParameter(NeuTube::COORD_STACK));
      }
    }
  }
  getView()->redrawObject();
}

void ZFlyEmProofMvc::clear()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->clearData();
    getPresenter()->clearData();
//    getView()->imageWidget();
  }
}

void ZFlyEmProofMvc::setDvidTarget(const ZDvidTarget &target)
{
  if (getCompleteDocument() != NULL) {
    clear();
//    getCompleteDocument()->clearData();
    getCompleteDocument()->setDvidTarget(target);
    getCompleteDocument()->updateTileData();
    QList<ZDvidTileEnsemble*> teList =
        getCompleteDocument()->getDvidTileEnsembleList();
    foreach (ZDvidTileEnsemble *te, teList) {
      te->attachView(getView());
    }
    getView()->reset(false);

    m_splitProject.setDvidTarget(target);
    m_mergeProject.setDvidTarget(target);
    m_mergeProject.syncWithDvid();

    emit dvidTargetChanged(target);
  }
}

void ZFlyEmProofMvc::setDvidTarget()
{
  m_dvidDlg = new ZDvidDialog(this);
  if (m_dvidDlg->exec()) {
    const ZDvidTarget &target = m_dvidDlg->getDvidTarget();
    setDvidTarget(target);
  }
}

ZDvidTarget ZFlyEmProofMvc::getDvidTarget() const
{
  if (m_dvidDlg != NULL) {
    return m_dvidDlg->getDvidTarget();
  }

  return ZDvidTarget();
}

void ZFlyEmProofMvc::createPresenter()
{
  if (getDocument().get() != NULL) {
    m_presenter = new ZFlyEmProofPresenter(this);
  }
}


void ZFlyEmProofMvc::customInit()
{
  connect(getPresenter(), SIGNAL(bodySplitTriggered()),
          this, SLOT(notifySplitTriggered()));
  connect(getPresenter(), SIGNAL(objectVisibleTurnedOn()),
          this, SLOT(processViewChange()));

  connect(getDocument().get(), SIGNAL(activeViewModified()),
          this, SLOT(processViewChange()));
  connect(getCompleteDocument(), SIGNAL(bodyMerged()),
          getView(), SLOT(paintObject()));
  connect(getCompleteDocument(), SIGNAL(bodyUnmerged()),
          getView(), SLOT(paintObject()));
  connect(getCompleteDocument(), SIGNAL(bodyMerged()),
          &m_mergeProject, SLOT(update3DBodyViewDeep()));
  connect(getCompleteDocument(), SIGNAL(bodyUnmerged()),
          &m_mergeProject, SLOT(update3DBodyViewDeep()));


  connect(getCompleteDocument(), SIGNAL(bodyUnmerged()),
          getView(), SLOT(paintObject()));


  m_splitProject.setDocument(getDocument());
  connect(&m_splitProject, SIGNAL(locating2DViewTriggered(const ZStackViewParam&)),
          this->getView(), SLOT(setView(const ZStackViewParam&)));
  connect(&m_splitProject, SIGNAL(messageGenerated(QString, bool)),
          this, SIGNAL(messageGenerated(QString, bool)));

  m_mergeProject.setDocument(getDocument());
  connect(getPresenter(), SIGNAL(labelSliceSelectionChanged()),
          this, SLOT(updateSelection()));
  connect(getCompletePresenter(), SIGNAL(highlightingSelected(bool)),
          &m_mergeProject, SLOT(highlightSelectedObject(bool)));
  connect(&m_mergeProject, SIGNAL(locating2DViewTriggered(ZStackViewParam)),
          this->getView(), SLOT(setView(ZStackViewParam)));
  connect(&m_mergeProject, SIGNAL(messageGenerated(QString, bool)),
          this, SIGNAL(messageGenerated(QString,bool)));

  connect(getDocument().get(), SIGNAL(messageGenerated(QString, bool)),
          this, SIGNAL(messageGenerated(QString, bool)));
  connect(this, SIGNAL(splitBodyLoaded(uint64_t)),
          this, SLOT(presentBodySplit(uint64_t)));

  connect(getCompletePresenter(), SIGNAL(selectingBodyAt(int,int,int)),
          this, SLOT(xorSelectionAt(int, int, int)));

  disableSplit();
}

void ZFlyEmProofMvc::updateSelection()
{
  if (getCompleteDocument() != NULL) {
    ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
    const std::set<uint64_t> &selected = slice->getSelected();
    m_mergeProject.setSelection(selected);
    m_mergeProject.update3DBodyView();
    if (getCompletePresenter()->isHighlight()) {
      m_mergeProject.highlightSelectedObject(true);
    }
  }
}

void ZFlyEmProofMvc::notifySplitTriggered()
{
  ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();

  if (labelSlice->isVisible()) {
    const std::set<uint64_t> &selected = labelSlice->getSelected();

    if (!selected.empty()) {
      uint64_t bodyId = *(selected.begin());

      emit launchingSplit(bodyId);
    }
  }
}

void ZFlyEmProofMvc::launchSplitFunc(uint64_t bodyId)
{
  if (!getCompleteDocument()->isSplittable(bodyId)) {
    emit errorGenerated(QString("%1 is not ready for split.").arg(bodyId));
  } else {
    ZDvidSparseStack *body = dynamic_cast<ZDvidSparseStack*>(
          getDocument()->getObjectGroup().findFirstSameSource(
            ZStackObject::TYPE_DVID_SPARSE_STACK,
            ZStackObjectSourceFactory::MakeSplitObjectSource()));
    ZDvidReader reader;
    if (reader.open(getDvidTarget())) {
      getProgressSignal()->startProgress("Launching split ...");

      if (body != NULL) {
        if (body->getLabel() != bodyId) {
          body = NULL;
        }
      }

      getProgressSignal()->advanceProgress(0.1);

      ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();

      getProgressSignal()->advanceProgress(0.1);

      if (body == NULL) {
        body = reader.readDvidSparseStack(bodyId);
        body->setZOrder(0);
        body->setSource(ZStackObjectSourceFactory::MakeSplitObjectSource());
        body->setMaskColor(labelSlice->getColor(bodyId));
        getDocument()->addObject(body, true);
      }

      m_splitProject.setBodyId(bodyId);

      labelSlice->setVisible(false);
      labelSlice->setHittable(false);
      body->setVisible(true);

      getProgressSignal()->advanceProgress(0.1);

      emit splitBodyLoaded(bodyId);

      getProgressSignal()->endProgress();
    }
  }
}

void ZFlyEmProofMvc::presentBodySplit(uint64_t bodyId)
{
  enableSplit();

  m_mergeProject.closeBodyWindow();

  m_splitProject.setBodyId(bodyId);
  m_splitProject.downloadSeed();
  emit bookmarkUpdated(&m_splitProject);
  getView()->redrawObject();
}

void ZFlyEmProofMvc::enableSplit()
{
  m_splitOn = true;
  getCompletePresenter()->enableSplit();
}

void ZFlyEmProofMvc::disableSplit()
{
  m_splitOn = false;
  getCompletePresenter()->disableSplit();
}

void ZFlyEmProofMvc::launchSplit(uint64_t bodyId)
{
  if (bodyId > 0) {
#ifdef _DEBUG_2
    bodyId = 14742253;
#endif
    const QString threadId = "launchSplitFunc";
    if (!m_futureMap.isAlive(threadId)) {
      m_futureMap.removeDeadThread();
      QFuture<void> future =
          QtConcurrent::run(
            this, &ZFlyEmProofMvc::launchSplitFunc, bodyId);
      m_futureMap[threadId] = future;
    }
  }
}

void ZFlyEmProofMvc::exitSplit()
{
  if (m_splitOn) {
    ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();
    labelSlice->setVisible(true);
    labelSlice->update(getView()->getViewParameter(NeuTube::COORD_STACK));

    labelSlice->setHittable(true);

    getDocument()->removeObject(ZStackObjectRole::ROLE_SEED);
    getDocument()->removeObject(ZStackObjectRole::ROLE_TMP_RESULT);

    ZDvidSparseStack *body = dynamic_cast<ZDvidSparseStack*>(
          getDocument()->getObjectGroup().findFirstSameSource(
            ZStackObject::TYPE_DVID_SPARSE_STACK,
            ZStackObjectSourceFactory::MakeSplitObjectSource()));
    if (body != NULL) {
      body->setVisible(false);
    }

    getView()->redrawObject();
    m_splitProject.clear();

    disableSplit();
  }
}

void ZFlyEmProofMvc::switchSplitBody(uint64_t bodyId)
{
  if (bodyId != m_splitProject.getBodyId()) {
    if (m_splitOn) {
//      exitSplit();
      m_splitProject.clear();
      getDocument()->removeObject(ZStackObjectRole::ROLE_SEED);
      getDocument()->removeObject(ZStackObjectRole::ROLE_TMP_RESULT);
      launchSplit(bodyId);
    }
  }
}

void ZFlyEmProofMvc::processMessageSlot(const QString &message)
{
  ZJsonObject obj;
  obj.decodeString(message.toStdString().c_str());

  if (obj.hasKey("type")) {
    std::string type = ZJsonParser::stringValue(obj["type"]);
    if (type == "locate_view") {
//      viewRoi(int x, int y, int z, int radius);
    }
  }
}

void ZFlyEmProofMvc::showBodyQuickView()
{
  m_splitProject.quickView();
}

void ZFlyEmProofMvc::showSplitQuickView()
{
  m_splitProject.showResult3dQuick();
}

void ZFlyEmProofMvc::showBody3d()
{
  m_splitProject.showDataFrame3d();
}

void ZFlyEmProofMvc::showSplit3d()
{
  m_splitProject.showResult3d();
}

void ZFlyEmProofMvc::showCoarseBody3d()
{
  m_mergeProject.showBody3d();
}

void ZFlyEmProofMvc::setDvidLabelSliceSize(int width, int height)
{
  if (getCompleteDocument() != NULL) {
    ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
    if (slice != NULL) {
      slice->setMaxSize(width, height);
      getView()->paintObject();
    }
  }
}

void ZFlyEmProofMvc::saveSeed()
{
  m_splitProject.saveSeed();
}

void ZFlyEmProofMvc::saveMergeOperation()
{
  getCompleteDocument()->saveMergeOperation();
}

void ZFlyEmProofMvc::commitCurrentSplit()
{
  m_splitProject.commitResult();
}

void ZFlyEmProofMvc::zoomTo(int x, int y, int z, int width)
{
  z -= getDocument()->getStackOffset().getZ();

  ZStackViewLocator locator;
  locator.setCanvasSize(getView()->imageWidget()->canvasSize().width(),
                        getView()->imageWidget()->canvasSize().height());

  QRect viewPort = locator.getRectViewPort(x, y, width);
  getPresenter()->setZoomRatio(
        locator.getZoomRatio(viewPort.width(), viewPort.height()));
  getPresenter()->setViewPortCenter(x, y, z);
}


void ZFlyEmProofMvc::zoomTo(const ZIntPoint &pt)
{
  zoomTo(pt.getX(), pt.getY(), pt.getZ());
}

void ZFlyEmProofMvc::zoomTo(int x, int y, int z)
{
  zoomTo(x, y, z, 400);
}

void ZFlyEmProofMvc::loadBookmark(const QString &filePath)
{
  m_splitProject.loadBookmark(filePath);

  emit bookmarkUpdated(&m_splitProject);
}

void ZFlyEmProofMvc::addSelectionAt(int x, int y, int z)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    uint64_t bodyId = reader.readBodyIdAt(x, y, z);
    if (bodyId > 0) {
      ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
      if (slice != NULL) {
        slice->addSelection(bodyId);
      }
      updateSelection();
    }
  }
}

void ZFlyEmProofMvc::xorSelectionAt(int x, int y, int z)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    uint64_t bodyId = reader.readBodyIdAt(x, y, z);
    if (bodyId > 0) {
      ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
      if (slice != NULL) {
        uint64_t finalBodyId = getMappedBodyId(bodyId);
        QList<uint64_t> labelList =
            getCompleteDocument()->getBodyMerger()->getOriginalLabelList(
              finalBodyId);
        slice->xorSelection(labelList.begin(), labelList.end());
        /*
        foreach (uint64_t label, labelList) {
          slice->xorSelection(label);
        }
        */
      }
      updateSelection();
    }
  }
}

uint64_t ZFlyEmProofMvc::getMappedBodyId(uint64_t bodyId)
{
  return m_mergeProject.getMappedBodyId(bodyId);
}

void ZFlyEmProofMvc::locateBody(uint64_t bodyId)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZObject3dScan body = reader.readCoarseBody(bodyId);

    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();

    ZVoxel voxel = body.getSlice((body.getMinZ() + body.getMaxZ()) / 2).getMarker();
    ZIntPoint pt(voxel.x(), voxel.y(), voxel.z());
    pt -= dvidInfo.getStartBlockIndex();
    pt *= dvidInfo.getBlockSize();
    pt += dvidInfo.getStartCoordinates();

    std::set<uint64_t> bodySet;
    bodySet.insert(bodyId);

    ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
    if (slice != NULL) {
      slice->setSelection(bodySet);
    }
    updateSelection();

    zoomTo(pt);
  }
}

//void ZFlyEmProofMvc::toggleEdgeMode(bool edgeOn)

