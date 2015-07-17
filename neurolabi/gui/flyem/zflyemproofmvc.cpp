#include "zflyemproofmvc.h"

#include <QFuture>
#include <QtConcurrentRun>
#include <QMessageBox>
#include <QInputDialog>
#include <QPushButton>
#include <QMainWindow>

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
#include "widgets/zimagewidget.h"
#include "dvid/zdvidlabelslice.h"
#include "flyem/zflyemproofpresenter.h"
#include "zwidgetmessage.h"
#include "dialogs/zspinboxdialog.h"
#include "zdialogfactory.h"
#include "flyem/zflyembodyannotationdialog.h"
#include "zflyembodyannotation.h"
#include "flyem/zflyemsupervisor.h"
#include "dvid/zdvidwriter.h"
#include "zstring.h"
#include "flyem/zpaintlabelwidget.h"
#include "zwidgetfactory.h"
#include "flyem/zflyemcoordinateconverter.h"
#include "flyem/zflyembookmarkannotationdialog.h"

ZFlyEmProofMvc::ZFlyEmProofMvc(QWidget *parent) :
  ZStackMvc(parent)
{
  m_dvidDlg = new ZDvidDialog(this);
  m_supervisor = new ZFlyEmSupervisor(this);
//  m_splitProject.attachBookmarkArray(&m_bookmarkArray);
//  m_mergeProject.attachBookmarkArray(&m_bookmarkArray);

  qRegisterMetaType<ZDvidTarget>("ZDvidTarget");
}

ZFlyEmProofMvc::~ZFlyEmProofMvc()
{
  exitCurrentDoc();
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
  ZFlyEmProofDoc *doc = new ZFlyEmProofDoc;
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
    getCompleteDocument()->mergeSelected(getSupervisor());
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

void ZFlyEmProofMvc::exitCurrentDoc()
{
  if (getCompleteDocument() != NULL) {
    getCompleteDocument()->saveCustomBookmark();
  }
}

void ZFlyEmProofMvc::setDvidTargetFromDialog()
{
  getProgressSignal()->startProgress("Loading data ...");
  setDvidTarget(m_dvidDlg->getDvidTarget());
  getProgressSignal()->endProgress();
}

void ZFlyEmProofMvc::setDvidTarget(const ZDvidTarget &target)
{
  exitCurrentDoc();

  getProgressSignal()->startProgress("Loading data ...");

  if (getCompleteDocument() != NULL) {
#if 1
//    QByteArray geometry;
    bool isMaximized = false;
    if (getMainWindow() != NULL) {
      isMaximized = getMainWindow()->isMaximized();
//      geometry = parentWidget()->saveGeometry();
    }
#endif

    clear();
    getProgressSignal()->advanceProgress(0.1);
//    getCompleteDocument()->clearData();
    getCompleteDocument()->setDvidTarget(target);
    getCompleteDocument()->beginObjectModifiedMode(
          ZStackDoc::OBJECT_MODIFIED_CACHE);
    getCompleteDocument()->updateTileData();
    getCompleteDocument()->endObjectModifiedMode();
    QList<ZDvidTileEnsemble*> teList =
        getCompleteDocument()->getDvidTileEnsembleList();
    foreach (ZDvidTileEnsemble *te, teList) {
      te->attachView(getView());
    }
    getView()->reset(false);
    getProgressSignal()->advanceProgress(0.1);

#if 1
    if (getMainWindow() != NULL) {
//      parentWidget()->hide();
//      parentWidget()->restoreGeometry(geometry);
//      parentWidget()->show();
      if (isMaximized) {
        getMainWindow()->showNormal();
        getMainWindow()->showMaximized();
      }
    }
#endif

    m_splitProject.setDvidTarget(target);
    m_mergeProject.setDvidTarget(target);
    m_mergeProject.syncWithDvid();
    getProgressSignal()->advanceProgress(0.2);

    if (getSupervisor() != NULL) {
      getSupervisor()->setDvidTarget(target);
    }

    getCompleteDocument()->downloadSynapse();
    getCompleteDocument()->downloadBookmark();
    getProgressSignal()->advanceProgress(0.5);


    emit dvidTargetChanged(target);
  }
  getProgressSignal()->endProgress();

  emit messageGenerated(ZWidgetMessage("Database loaded.",
                                       NeuTube::MSG_INFORMATION,
                                       ZWidgetMessage::TARGET_STATUS_BAR));
}

void ZFlyEmProofMvc::setDvidTarget()
{
//  m_dvidDlg = new ZDvidDialog(this);
  if (m_dvidDlg->exec()) {
    const ZDvidTarget &target = m_dvidDlg->getDvidTarget();
    setDvidTarget(target);
    /*
    const QString threadId = "setDvidTarget";
    if (!m_futureMap.isAlive(threadId)) {
      m_futureMap.removeDeadThread();
      QFuture<void> future =
          QtConcurrent::run(this, &ZFlyEmProofMvc::setDvidTargetFromDialog);
      m_futureMap[threadId] = future;
    }
    */


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
    m_presenter = ZFlyEmProofPresenter::Make(this);
  }
}

void ZFlyEmProofMvc::customInit()
{
  connect(getPresenter(), SIGNAL(bodySplitTriggered()),
          this, SLOT(notifySplitTriggered()));
  connect(getPresenter(), SIGNAL(bodyAnnotationTriggered()),
          this, SLOT(annotateBody()));
  connect(getPresenter(), SIGNAL(bodyCheckinTriggered()),
          this, SLOT(checkInSelectedBody()));
  connect(getPresenter(), SIGNAL(bodyCheckoutTriggered()),
          this, SLOT(checkOutBody()));
  connect(getPresenter(), SIGNAL(objectVisibleTurnedOn()),
          this, SLOT(processViewChange()));
  connect(getCompletePresenter(), SIGNAL(goingToBody()),
          this, SLOT(goToBody()));

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

  m_mergeProject.getProgressSignal()->connectProgress(getProgressSignal());
  m_splitProject.getProgressSignal()->connectProgress(getProgressSignal());


  connect(getCompleteDocument(), SIGNAL(bodyUnmerged()),
          getView(), SLOT(paintObject()));
  connect(getDocument().get(),
          SIGNAL(objectSelectorChanged(ZStackObjectSelector)),
          this, SLOT(processSelectionChange(ZStackObjectSelector)));


  m_splitProject.setDocument(getDocument());
  connect(&m_splitProject, SIGNAL(locating2DViewTriggered(const ZStackViewParam&)),
          this->getView(), SLOT(setView(const ZStackViewParam&)));
  /*
  connect(&m_splitProject, SIGNAL(messageGenerated(QString, bool)),
          this, SIGNAL(messageGenerated(QString, bool)));
          */

  m_mergeProject.setDocument(getDocument());
  connect(getPresenter(), SIGNAL(labelSliceSelectionChanged()),
          this, SLOT(updateBodySelection()));
  connect(getCompletePresenter(), SIGNAL(highlightingSelected(bool)),
          &m_mergeProject, SLOT(highlightSelectedObject(bool)));
  connect(&m_mergeProject, SIGNAL(locating2DViewTriggered(ZStackViewParam)),
          this->getView(), SLOT(setView(ZStackViewParam)));
  connect(&m_mergeProject, SIGNAL(dvidLabelChanged()),
          this->getCompleteDocument(), SLOT(updateDvidLabelObject()));
  connect(&m_mergeProject, SIGNAL(checkingInBody(uint64_t)),
          this, SLOT(checkInBody(uint64_t)));
  /*
  connect(&m_mergeProject, SIGNAL(messageGenerated(QString, bool)),
          this, SIGNAL(messageGenerated(QString,bool)));
  connect(getDocument().get(), SIGNAL(messageGenerated(QString, bool)),
          this, SIGNAL(messageGenerated(QString, bool)));
          */

  ZWidgetMessage::ConnectMessagePipe(&m_splitProject, this, false);
  ZWidgetMessage::ConnectMessagePipe(&m_mergeProject, this, false);
//  ZWidgetMessage::ConnectMessagePipe(&getDocument().get(), this, false);


  connect(this, SIGNAL(splitBodyLoaded(uint64_t)),
          this, SLOT(presentBodySplit(uint64_t)));

  connect(getCompletePresenter(), SIGNAL(selectingBodyAt(int,int,int)),
          this, SLOT(xorSelectionAt(int, int, int)));
  connect(getCompletePresenter(), SIGNAL(deselectingAllBody()),
          this, SLOT(deselectAllBody()));
  connect(getCompletePresenter(), SIGNAL(runningSplit()), this, SLOT(runSplit()));
  connect(getCompletePresenter(), SIGNAL(bookmarkAdded(ZFlyEmBookmark*)),
          this, SLOT(annotateBookmark(ZFlyEmBookmark*)));
  connect(getCompletePresenter(), SIGNAL(annotatingBookmark(ZFlyEmBookmark*)),
          this, SLOT(annotateBookmark(ZFlyEmBookmark*)));
//  connect(getCompletePresenter(), SIGNAL(goingToBody()), this, SLOT());

  disableSplit();


  /*
  QPushButton *button = new QPushButton(this);
  button->setCheckable(true);
  button->setChecked(true);
  button->setIcon(QIcon(":/images/synapse.png"));
  connect(button, SIGNAL(toggled(bool)),
          this, SLOT(showSynapseAnnotation(bool)));
          */

//  getView()->addHorizontalWidget(button);

//  getView()->addHorizontalWidget(ZWidgetFactory::makeHSpacerItem());

  getView()->addHorizontalWidget(new ZPaintLabelWidget());
}

void ZFlyEmProofMvc::goToBody()
{
  bool ok;

  QString text = QInputDialog::getText(this, tr("Go To"),
                                       tr("Body:"), QLineEdit::Normal,
                                       "", &ok);
  if (ok) {
    if (!text.isEmpty()) {
      ZString str = text.toStdString();
      std::vector<int> bodyArray = str.toIntegerArray();
      if (bodyArray.size() == 1) {
        locateBody((uint64_t) bodyArray[0]);
//        emit locatingBody();
      }
    }
  }
}

void ZFlyEmProofMvc::selectBody()
{
  bool ok;

  QString text = QInputDialog::getText(this, tr("Go To"),
                                       tr("Body:"), QLineEdit::Normal,
                                       "", &ok);
  if (ok) {
    if (!text.isEmpty()) {
      ZString str = text.toStdString();
      std::vector<int> bodyArray = str.toIntegerArray();
      if (bodyArray.size() == 1) {
        selectBody((uint64_t) bodyArray[0]);
      }
    }
  }
}

void ZFlyEmProofMvc::processSelectionChange(const ZStackObjectSelector &selector)
{
  const std::vector<ZStackObject*>& objList =
      selector.getSelectedList(ZStackObject::TYPE_FLYEM_BOOKMARK);
  if (!objList.empty()) {
    const ZStackObject *obj = objList.back();
    const ZFlyEmBookmark *bookmark = dynamic_cast<const ZFlyEmBookmark*>(obj);
    if (bookmark != NULL) {
      emit messageGenerated(
            ZWidgetMessage(bookmark->toJsonObject(true).dumpString(0).c_str(),
                           NeuTube::MSG_INFORMATION,
                           ZWidgetMessage::TARGET_STATUS_BAR));
    }
  } else {
    emit messageGenerated(
          ZWidgetMessage("---",
                         NeuTube::MSG_INFORMATION,
                         ZWidgetMessage::TARGET_STATUS_BAR));
  }
}

void ZFlyEmProofMvc::runSplitFunc()
{
  getProgressSignal()->startProgress(1.0);
  m_splitProject.runSplit();
  getProgressSignal()->endProgress();
}

void ZFlyEmProofMvc::runSplit()
{
//  getProgressSignal()->startProgress("Running split ...");
  runSplitFunc();
//  getProgressSignal()->endProgress();

#if 0 //has strange crash caused by Make_Stack
  const QString threadId = "runSplit";
  if (!m_futureMap.isAlive(threadId)) {
    m_futureMap.removeDeadThread();
    QFuture<void> future =
        QtConcurrent::run(
          this, &ZFlyEmProofMvc::runSplitFunc);
    m_futureMap[threadId] = future;
  }
#endif
//  m_splitProject.runSplit();
}

void ZFlyEmProofMvc::updateBodySelection()
{
  if (getCompleteDocument() != NULL) {
    ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
    const std::set<uint64_t> &selected = slice->getSelectedOriginal();
    m_mergeProject.setSelection(selected, NeuTube::BODY_LABEL_ORIGINAL);
    m_mergeProject.update3DBodyView();
    if (getCompletePresenter()->isHighlight()) {
      m_mergeProject.highlightSelectedObject(true);
    } else {
      getCompleteDocument()->processObjectModified(slice);
    }
  }
}

bool ZFlyEmProofMvc::checkInBody(uint64_t bodyId)
{
  if (getSupervisor() != NULL) {
    return getSupervisor()->checkIn(bodyId);
  }

  return true;
}

bool ZFlyEmProofMvc::checkOutBody(uint64_t bodyId)
{
  if (getSupervisor() != NULL) {
    return getSupervisor()->checkOut(bodyId);
  }

  return true;
}

void ZFlyEmProofMvc::checkInSelectedBody()
{
  if (getSupervisor() != NULL) {
    std::set<uint64_t> bodyIdArray =
        getCurrentSelectedBodyId(NeuTube::BODY_LABEL_ORIGINAL);
    for (std::set<uint64_t>::const_iterator iter = bodyIdArray.begin();
         iter != bodyIdArray.end(); ++iter) {
      uint64_t bodyId = *iter;
      if (bodyId > 0) {
        if (getSupervisor()->checkIn(bodyId)) {
          emit messageGenerated(QString("Body %1 is unlocked.").arg(bodyId));
        } else {
          emit errorGenerated(QString("Failed to unlock body %1.").arg(bodyId));
        }
      }
    }
  } else {
    emit messageGenerated(QString("Body lock service is not available."));
  }

#if 0
  std::set<uint64_t> bodyIdArray =
      getCurrentSelectedBodyId(NeuTube::BODY_LABEL_MAPPED);
  if (bodyIdArray.size() == 1) {
    uint64_t bodyId = *(bodyIdArray.begin());
    if (bodyId > 0) {
      if (getSupervisor() != NULL) {
        if (getSupervisor()->checkIn(bodyId)) {
          emit messageGenerated(QString("Body %1 is unlocked.").arg(bodyId));
        } else {
          emit errorGenerated(QString("Failed to check in body %1.").arg(bodyId));
        }
      }
    }
  }
#endif
}

void ZFlyEmProofMvc::checkOutBody()
{
  if (getSupervisor() != NULL) {
    std::set<uint64_t> bodyIdArray =
        getCurrentSelectedBodyId(NeuTube::BODY_LABEL_ORIGINAL);
    for (std::set<uint64_t>::const_iterator iter = bodyIdArray.begin();
         iter != bodyIdArray.end(); ++iter) {
      uint64_t bodyId = *iter;
      if (bodyId > 0) {
        if (getSupervisor()->checkOut(bodyId)) {
          emit messageGenerated(QString("Body %1 is locked.").arg(bodyId));
        } else {
          emit errorGenerated(QString("Failed to lock body %1.").arg(bodyId));
        }
      }
    }
  } else {
    emit messageGenerated(QString("Body lock service is not available."));
  }
#if 0
  std::set<uint64_t> bodyIdArray =
      getCurrentSelectedBodyId(NeuTube::BODY_LABEL_MAPPED);
  if (bodyIdArray.size() == 1) {
    uint64_t bodyId = *(bodyIdArray.begin());
    if (bodyId > 0) {
      if (getSupervisor() != NULL) {
        if (getSupervisor()->checkOut(bodyId)) {
          emit messageGenerated(QString("Body %1 is locked.").arg(bodyId));
        } else {
          emit errorGenerated(QString("Failed to check out body %1.").arg(bodyId));
        }
      }
    }
  }
#endif
}

void ZFlyEmProofMvc::annotateBody()
{
  std::set<uint64_t> bodyIdArray =
      getCurrentSelectedBodyId(NeuTube::BODY_LABEL_ORIGINAL);
  if (bodyIdArray.size() == 1) {
    uint64_t bodyId = *(bodyIdArray.begin());
    if (bodyId > 0) {
      if (checkOutBody(bodyId)) {
        ZFlyEmBodyAnnotationDialog *dlg = new ZFlyEmBodyAnnotationDialog(this);
        dlg->setBodyId(bodyId);
        ZDvidReader reader;
        if (reader.open(getDvidTarget())) {
          ZFlyEmBodyAnnotation annotation = reader.readBodyAnnotation(bodyId);

          if (!annotation.isEmpty()) {
            dlg->loadBodyAnnotation(annotation);
          }
        }

        if (dlg->exec() && dlg->getBodyId() == bodyId) {
          ZDvidWriter writer;
          if (writer.open(getDvidTarget())) {
            writer.writeAnnotation(bodyId, dlg->getBodyAnnotation().toJsonObject());
          }
          if (writer.getStatusCode() == 200) {
            emit messageGenerated(QString("Body %1 is annotated.").arg(bodyId));
          } else {
            qDebug() << writer.getStandardOutput();
            emit errorGenerated("Cannot save annotation.");
          }
        }

        checkInBody(bodyId);
      } else {
        if (getSupervisor() != NULL) {
          std::string owner = getSupervisor()->getOwner(bodyId);
          if (owner.empty()) {
            owner = "unknown user";
          }
          emit messageGenerated(
                ZWidgetMessage(
                  QString("Failed to start annotation. %1 has been locked by %2").
                  arg(bodyId).arg(owner.c_str()), NeuTube::MSG_ERROR));
        }
      }
    } else {
      qDebug() << "Unexpected body ID: 0";
    }
  } else {
    QString msg;
    if (getCurrentSelectedBodyId(NeuTube::BODY_LABEL_MAPPED).size() == 1) {
      msg = "The annotation cannot be done because "
          "the merged body has not be uploaded.";
    } else {
      msg = "The annotation cannot be done because "
          "one and only one body has to be selected.";
    }
    if (!msg.isEmpty()) {
      emit messageGenerated(ZWidgetMessage(msg, NeuTube::MSG_WARING));
    }
  }


//  emit messageGenerated(
//        ZWidgetMessage("The function of annotating body is still under testing.",
//                       NeuTube::MSG_WARING));
}

void ZFlyEmProofMvc::notifySplitTriggered()
{
  const std::set<uint64_t> &selected =
      getCurrentSelectedBodyId(NeuTube::BODY_LABEL_ORIGINAL);

  if (selected.size() == 1) {
    uint64_t bodyId = *(selected.begin());

    emit launchingSplit(bodyId);
  } else {
    QString msg;
    if (getCurrentSelectedBodyId(NeuTube::BODY_LABEL_MAPPED).size() == 1) {
      msg = "The split cannot be launched because "
          "the merged body has not been uploaded.";
    } else {
      msg = "The split cannot be launched because "
          "one and only one body has to be selected.";
    }
    emit messageGenerated(ZWidgetMessage(msg, NeuTube::MSG_WARING));
  }

  /*
  ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();

  if (labelSlice->isVisible()) {
    const std::set<uint64_t> &selected = labelSlice->getSelectedOriginal();

    if (selected.size() == 1) {
      uint64_t bodyId = *(selected.begin());

      emit launchingSplit(bodyId);
    } else {
      emit messageGenerated("The split cannot be launched because "
                            "one and only one body has to be selected.");
    }
  }
  */
}

void ZFlyEmProofMvc::launchSplitFunc(uint64_t bodyId)
{
  if (bodyId == 0) {
    emit errorGenerated(QString("Invalid body id: %1").arg(bodyId));
    return;
  }

  if (!getCompleteDocument()->isSplittable(bodyId)) {
    QString msg = QString("%1 is not ready for split.").arg(bodyId);
    emit messageGenerated(
          ZWidgetMessage(msg, NeuTube::MSG_ERROR, ZWidgetMessage::TARGET_DIALOG));
    emit errorGenerated(msg);
  } else {
    ZDvidSparseStack *body = dynamic_cast<ZDvidSparseStack*>(
          getDocument()->getObjectGroup().findFirstSameSource(
            ZStackObject::TYPE_DVID_SPARSE_STACK,
            ZStackObjectSourceFactory::MakeSplitObjectSource()));
    ZDvidReader reader;
    if (reader.open(getDvidTarget())) {
      getProgressSignal()->startProgress("Launching split ...");

      getCompletePresenter()->setHighlightMode(false);
      m_mergeProject.highlightSelectedObject(false);

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
      }

      if (body->isEmpty()) {
        delete body;
        body = NULL;

        QString msg = QString("Invalid body id: %1").arg(bodyId);
        emit messageGenerated(
              ZWidgetMessage(msg, NeuTube::MSG_ERROR, ZWidgetMessage::TARGET_DIALOG));
        emit errorGenerated(msg);
      } else {
        body->setZOrder(0);
        body->setSource(ZStackObjectSourceFactory::MakeSplitObjectSource());
        body->setMaskColor(labelSlice->getColor(
                             bodyId, NeuTube::BODY_LABEL_ORIGINAL));
        body->setSelectable(false);
        getDocument()->addObject(body, true);
        m_splitProject.setBodyId(bodyId);

        labelSlice->setVisible(false);
        labelSlice->setHittable(false);
        body->setVisible(true);

        getProgressSignal()->advanceProgress(0.1);

        emit splitBodyLoaded(bodyId);
      }

//      getDocument()->setVisible(ZStackObject::TYPE_PUNCTA, true);

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
//  m_splitOn = true;
  getCompletePresenter()->enableSplit();
}

void ZFlyEmProofMvc::disableSplit()
{
//  m_splitOn = false;
  getCompletePresenter()->disableSplit();
}

void ZFlyEmProofMvc::launchSplit(uint64_t bodyId)
{
  if (bodyId > 0) {
    if (checkOutBody(bodyId)) {
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
    } else {
      std::string owner = getSupervisor()->getOwner(bodyId);
      if (owner.empty()) {
        owner = "unknown user";
      }
      emit messageGenerated(
            ZWidgetMessage(
              QString("Failed to launch split. %1 has been locked by %2").
              arg(bodyId).arg(owner.c_str()), NeuTube::MSG_ERROR));
    }
  }
}

void ZFlyEmProofMvc::exitSplit()
{
  if (getCompletePresenter()->isSplitWindow()) {
    emit messageGenerated("Exiting split ...");
//    emitMessage("Exiting split ...");
    ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();
    labelSlice->setVisible(true);
    labelSlice->update(getView()->getViewParameter(NeuTube::COORD_STACK));

    labelSlice->setHittable(true);

    //m_splitProject.clearBookmarkDecoration();
    getDocument()->removeObject(ZStackObjectRole::ROLE_SEED);
    getDocument()->removeObject(ZStackObjectRole::ROLE_TMP_RESULT);
//    getDocument()->removeObject(ZStackObjectRole::ROLE_TMP_BOOKMARK);

    getDocument()->setVisible(ZStackObject::TYPE_DVID_SPARSE_STACK, false);

    checkInBody(m_splitProject.getBodyId());
//    getDocument()->setVisible(ZStackObject::TYPE_PUNCTA, false);


    /*
    ZDvidSparseStack *body = dynamic_cast<ZDvidSparseStack*>(
          getDocument()->getObjectGroup().findFirstSameSource(
            ZStackObject::TYPE_DVID_SPARSE_STACK,
            ZStackObjectSourceFactory::MakeSplitObjectSource()));
    if (body != NULL) {
      body->setVisible(false);
    }

    getView()->redrawObject();
    */

    m_splitProject.clear();

    disableSplit();
  }
}

void ZFlyEmProofMvc::switchSplitBody(uint64_t bodyId)
{
  if (bodyId != m_splitProject.getBodyId()) {
    if (getCompletePresenter()->isSplitWindow()) {
//      exitSplit();
      QMessageBox msgBox;
       msgBox.setText("Changing to another body to split.");
       msgBox.setInformativeText("Do you want to save your seeds?");
       msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
       msgBox.setDefaultButton(QMessageBox::Save);
       int ret = msgBox.exec();
       if (ret != QMessageBox::Cancel) {
         if (ret == QMessageBox::Save) {
           m_splitProject.saveSeed(false);
         }
         m_splitProject.clear();
         getDocument()->removeObject(ZStackObjectRole::ROLE_SEED);
         getDocument()->removeObject(ZStackObjectRole::ROLE_TMP_RESULT);
         launchSplit(bodyId);
       }
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
  m_splitProject.showBodyQuickView();
}

void ZFlyEmProofMvc::showSplitQuickView()
{
  m_splitProject.showResultQuickView();
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
  m_mergeProject.showCoarseBody3d();
}

void ZFlyEmProofMvc::showFineBody3d()
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

void ZFlyEmProofMvc::showFullSegmentation()
{
  if (getCompleteDocument() != NULL) {
    ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
    if (slice != NULL) {
      slice->updateFullView(getView()->getViewParameter());
      getView()->paintObject();
    }
  }
}

void ZFlyEmProofMvc::saveSeed()
{
  m_splitProject.saveSeed(true);
}

void ZFlyEmProofMvc::saveMergeOperation()
{
  getCompleteDocument()->saveMergeOperation();
}

void ZFlyEmProofMvc::commitMerge()
{
  if (ZDialogFactory::Ask("Upload Confirmation",
                          "Do you want to upload the merging result now? "
                          "It cannot be undone. ",
                          this)) {
    m_mergeProject.uploadResult();
  }
}

void ZFlyEmProofMvc::commitCurrentSplit()
{
  if (!getDocument()->isSegmentationReady()) {
//    emit messageGenerated("test");
    emit messageGenerated(
          ZWidgetMessage("Failed to save results: The split has not been updated."
                         "Please Run full split (shift+space) first.",
                         NeuTube::MSG_ERROR, ZWidgetMessage::TARGET_TEXT_APPENDING));
    return;
  }

  if (ZDialogFactory::Ask("Upload Confirmation",
                          "Do you want to upload the splitting results now? "
                          "It cannot be undone. "
                          "***IMPORTANT**** Please make sure you have run"
                          " the full split.***",
                          this)) {
    const QString threadId = "ZFlyEmBodySplitProject::commitResult";
    if (!m_futureMap.isAlive(threadId)) {
      m_futureMap.removeDeadThread();
      QFuture<void> future =
          QtConcurrent::run(
            &m_splitProject, &ZFlyEmBodySplitProject::commitResult);
      m_futureMap[threadId] = future;
    }
//    m_splitProject.commitResult();
  }
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

void ZFlyEmProofMvc::syncDvidBookmark()
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    TStackObjectList &objList =
        getDocument()->getObjectList(ZStackObject::TYPE_FLYEM_BOOKMARK);
    for (TStackObjectList::iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      ZStackObject *obj = *iter;
      ZFlyEmBookmark *bookmark = dynamic_cast<ZFlyEmBookmark*>(obj);
      if (bookmark != NULL) {
        const QByteArray &bookmarkData = reader.readKeyValue(
              ZDvidData::GetName(ZDvidData::ROLE_BOOKMARK), bookmark->getDvidKey());
        if (!bookmarkData.isEmpty()) {
          ZJsonObject obj;
          obj.decodeString(bookmarkData.data());
          if (obj.hasKey("checked")) {
            bookmark->setChecked(ZJsonParser::booleanValue(obj["checked"]));
          }
        }
      }
    }
  }
}

void ZFlyEmProofMvc::notifyBookmarkUpdated()
{
  syncDvidBookmark();

//  m_splitProject.updateBookmarkDecoration();
//  m_mergeProject.updateBookmarkDecoration();

  emit bookmarkUpdated(&m_mergeProject);
  emit bookmarkUpdated(&m_splitProject);
}

void ZFlyEmProofMvc::loadBookmarkFunc(const QString &filePath)
{
  getProgressSignal()->startProgress("Importing bookmarks ...");
  //  m_splitProject.loadBookmark(filePath);

    ZDvidReader reader;
  //  ZFlyEmCoordinateConverter converter;
    if (reader.open(getDvidTarget())) {
  //    ZDvidInfo info = reader.readGrayScaleInfo();
  //    converter.configure(info);
      getProgressSignal()->advanceProgress(0.1);
      getCompleteDocument()->importFlyEmBookmark(filePath.toStdString());
      getProgressSignal()->advanceProgress(0.5);
  //    m_bookmarkArray.importJsonFile(filePath.toStdString(), NULL/*&converter*/);
    }

    notifyBookmarkUpdated();

    getProgressSignal()->advanceProgress(0.3);

    getProgressSignal()->endProgress();
  //  m_bookmarkArray.importJsonFile(filePath);

  //  emit bookmarkUpdated(&m_splitProject);
}

void ZFlyEmProofMvc::loadBookmark(const QString &filePath)
{
  QtConcurrent::run(this, &ZFlyEmProofMvc::loadBookmarkFunc, filePath);
}

void ZFlyEmProofMvc::loadSynapse()
{
  QString fileName = ZDialogFactory::GetOpenFileName("Load Synapses", "", this);
  if (!fileName.isEmpty()) {
    getCompleteDocument()->loadSynapse(fileName.toStdString());
  }
}

void ZFlyEmProofMvc::loadBookmark()
{
  QString fileName = ZDialogFactory::GetOpenFileName("Load Bookmarks", "", this);
  if (!fileName.isEmpty()) {
    loadBookmark(fileName);
  }
}


void ZFlyEmProofMvc::showSynapseAnnotation(bool visible)
{
  getCompleteDocument()->setVisible(ZStackObject::TYPE_PUNCTA, visible);
}

void ZFlyEmProofMvc::showBookmark(bool visible)
{
  getCompleteDocument()->setVisible(ZStackObject::TYPE_FLYEM_BOOKMARK, visible);
//  m_splitProject.setBookmarkVisible(visible);
//  m_mergeProject.setBookmarkVisible(visible);
}

void ZFlyEmProofMvc::showSegmentation(bool visible)
{
  getCompleteDocument()->setVisible(ZStackObject::TYPE_DVID_LABEL_SLICE, visible);
}

void ZFlyEmProofMvc::addSelectionAt(int x, int y, int z)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    uint64_t bodyId = reader.readBodyIdAt(x, y, z);
    if (bodyId > 0) {
      ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
      if (slice != NULL) {
        slice->addSelection(
              slice->getMappedLabel(bodyId, NeuTube::BODY_LABEL_ORIGINAL),
              NeuTube::BODY_LABEL_ORIGINAL);
      }
      updateBodySelection();
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
//        uint64_t finalBodyId = getMappedBodyId(bodyId);
        slice->xorSelection(
              slice->getMappedLabel(bodyId, NeuTube::BODY_LABEL_ORIGINAL),
              NeuTube::BODY_LABEL_MAPPED);
#if 0
        QList<uint64_t> labelList =
            getCompleteDocument()->getBodyMerger()->getOriginalLabelList(
              finalBodyId);
        slice->xorSelectionGroup(labelList.begin(), labelList.end());
        /*
        foreach (uint64_t label, labelList) {
          slice->xorSelection(label);
        }
        */
#endif
      }
      updateBodySelection();
    }
  }
}

void ZFlyEmProofMvc::deselectAllBody()
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
    if (slice != NULL) {
      slice->deselectAll();
      updateBodySelection();
    }
  }
}

void ZFlyEmProofMvc::selectSeed()
{
  ZSpinBoxDialog *dlg = ZDialogFactory::makeSpinBoxDialog(this);
  dlg->setValueLabel("Label");
  dlg->getButton(ZButtonBox::ROLE_SKIP)->hide();
  dlg->setValue(1);
  if (dlg->exec()) {
    int label = dlg->getValue();
   int nSelected = m_splitProject.selectSeed(label);
   getView()->paintObject();
   emit messageGenerated(QString("%1 seed(s) are selected.").arg(nSelected));
  }
  delete dlg;
}

void ZFlyEmProofMvc::selectAllSeed()
{
  int nSelected = m_splitProject.selectAllSeed();
  getView()->paintObject();
  emit messageGenerated(QString("%1 seed(s) are selected.").arg(nSelected));
}

void ZFlyEmProofMvc::recoverSeed()
{
  if (ZDialogFactory::Ask("Recover Seed", "All current seeds might be lost. "
                          "Do you want to continue?", this)) {
    m_splitProject.recoverSeed();
  }
}

void ZFlyEmProofMvc::exportSeed()
{
  QString fileName = ZDialogFactory::GetSaveFileName("Export Seed", "", this);
  if (!fileName.isEmpty()) {
    m_splitProject.exportSeed(fileName);
  }
}

void ZFlyEmProofMvc::importSeed()
{
  if (ZDialogFactory::Ask("Import Seed",
                          "All current seeds might be lost after import. "
                          "Do you want to continue?", this)) {
    QString fileName = ZDialogFactory::GetOpenFileName("Import Seed", "", this);
    if (!fileName.isEmpty()) {
      m_splitProject.importSeed(fileName);
    }
  }
}

uint64_t ZFlyEmProofMvc::getMappedBodyId(uint64_t bodyId)
{
  return m_mergeProject.getMappedBodyId(bodyId);
}

std::set<uint64_t> ZFlyEmProofMvc::getCurrentSelectedBodyId(
    NeuTube::EBodyLabelType type) const
{
  return m_mergeProject.getSelection(type);
#if 0
  std::set<uint64_t> idSet;

  if (getCompletePresenter()->isHighlight()) {

//    idSet = m_mergeProject.getSelectedBodyId();
  } else {
    ZDvidLabelSlice *labelSlice = getCompleteDocument()->getDvidLabelSlice();
    if (labelSlice != NULL) {
      if (labelSlice->isVisible()) {
        idSet = labelSlice->getSelected(type);
      }
    }
  }

  return idSet;
#endif
}

void ZFlyEmProofMvc::locateBody(uint64_t bodyId)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZObject3dScan body = reader.readCoarseBody(bodyId);
    if (body.isEmpty()) {
      emit messageGenerated(
            ZWidgetMessage(QString("Cannot go to body: %1. No such body.").
            arg(bodyId), NeuTube::MSG_ERROR));
    } else {
      ZDvidInfo dvidInfo = reader.readGrayScaleInfo();

      ZVoxel voxel = body.getSlice((body.getMinZ() + body.getMaxZ()) / 2).getMarker();
      ZIntPoint pt(voxel.x(), voxel.y(), voxel.z());
      pt -= dvidInfo.getStartBlockIndex();
      pt *= dvidInfo.getBlockSize();
      pt += dvidInfo.getStartCoordinates();

      //    std::set<uint64_t> bodySet;
      //    bodySet.insert(bodyId);

      ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
      if (slice != NULL) {
        slice->clearSelection();
        slice->addSelection(
              slice->getMappedLabel(bodyId, NeuTube::BODY_LABEL_ORIGINAL),
              NeuTube::BODY_LABEL_MAPPED);
      }
      updateBodySelection();

      zoomTo(pt);
    }
  }
}

void ZFlyEmProofMvc::selectBody(uint64_t bodyId)
{
  ZDvidLabelSlice *slice = getCompleteDocument()->getDvidLabelSlice();
  if (slice != NULL) {
    slice->addSelection(bodyId, NeuTube::BODY_LABEL_MAPPED);
  }
  updateBodySelection();
}

void ZFlyEmProofMvc::processViewChangeCustom(const ZStackViewParam &/*viewParam*/)
{
  m_mergeProject.update3DBodyViewPlane();
  m_splitProject.update3DViewPlane();
}

void ZFlyEmProofMvc::recordCheckedBookmark(const QString &key, bool checking)
{
//  ZFlyEmBookmark *bookmark = m_bookmarkArray.findFirstBookmark(key);
  ZFlyEmBookmark *bookmark = getCompleteDocument()->findFirstBookmark(key);
  if (bookmark != NULL) {
    bookmark->setChecked(checking);
    ZDvidWriter writer;
    if (writer.open(getDvidTarget())) {
      writer.writeBookmark(*bookmark);
      if (writer.getStatusCode() != 200) {
        emit messageGenerated(ZWidgetMessage("Failed to record bookmark.",
                                             NeuTube::MSG_WARING));
      }
    }
  }
}

void ZFlyEmProofMvc::enhanceTileContrast(bool state)
{
  ZDvidTileEnsemble *tile = getCompleteDocument()->getDvidTileEnsemble();
  if (tile != NULL) {
    if (state) {
      tile->addVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST);
    } else {
      tile->removeVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST);
    }
    getCompleteDocument()->processObjectModified(tile->getTarget());
  }
}

ZFlyEmSupervisor* ZFlyEmProofMvc::getSupervisor() const
{
  if (getDvidTarget().isSupervised()) {
    return m_supervisor;
  }

  return NULL;
}

void ZFlyEmProofMvc::annotateBookmark(ZFlyEmBookmark *bookmark)
{
  if (bookmark != NULL) {
    ZFlyEmBookmarkAnnotationDialog dlg(this);
    dlg.setFrom(bookmark);
    if (dlg.exec()) {
      dlg.annotate(bookmark);
    }
  }
}

//void ZFlyEmProofMvc::toggleEdgeMode(bool edgeOn)


