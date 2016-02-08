#include "zflyemorthomvc.h"
#include "zflyemorthodoc.h"
#include "zstackpresenter.h"
#include "zflyemsupervisor.h"
#include "zstackview.h"

ZFlyEmOrthoMvc::ZFlyEmOrthoMvc(QWidget *parent) :
  ZFlyEmProofMvc(parent)
{
  init();
}

void ZFlyEmOrthoMvc::init()
{
  m_dvidDlg = NULL;
  m_bodyInfoDlg = NULL;
  m_supervisor = new ZFlyEmSupervisor(this);
  m_splitCommitDlg = NULL;

  qRegisterMetaType<ZDvidTarget>("ZDvidTarget");

  m_objectWindow = NULL;
}

ZFlyEmOrthoMvc* ZFlyEmOrthoMvc::Make(
    QWidget *parent, ZSharedPointer<ZFlyEmOrthoDoc> doc, NeuTube::EAxis axis)
{
  ZFlyEmOrthoMvc *frame = new ZFlyEmOrthoMvc(parent);

  BaseConstruct(frame, doc, axis);
  frame->getPresenter()->setObjectStyle(ZStackObject::SOLID);
  frame->getView()->layout()->setContentsMargins(0, 0, 0, 0);
  frame->getView()->setContentsMargins(0, 0, 0, 0);
  frame->getView()->hideThresholdControl();

  return frame;
}

ZFlyEmOrthoMvc* ZFlyEmOrthoMvc::Make(
    const ZDvidTarget &target, NeuTube::EAxis axis)
{
  ZFlyEmOrthoDoc *doc = new ZFlyEmOrthoDoc;
//  doc->setTag(NeuTube::Document::FLYEM_DVID);
  ZFlyEmOrthoMvc *mvc =
      ZFlyEmOrthoMvc::Make(NULL, ZSharedPointer<ZFlyEmOrthoDoc>(doc), axis);

  mvc->setDvidTarget(target);

  return mvc;
}

ZFlyEmOrthoDoc* ZFlyEmOrthoMvc::getCompleteDocument() const
{
  return qobject_cast<ZFlyEmOrthoDoc*>(getDocument().get());
}

void ZFlyEmOrthoMvc::setDvidTarget(const ZDvidTarget &/*target*/)
{
  if (getCompleteDocument() != NULL) {
    getView()->reset(false);
  }
}

ZDvidTarget ZFlyEmOrthoMvc::getDvidTarget() const
{
  return getCompleteDocument()->getDvidTarget();
}

void ZFlyEmOrthoMvc::updateStack(const ZIntPoint &center)
{
  getCompleteDocument()->updateStack(center);
  getView()->updateViewBox();
}
