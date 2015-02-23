#include "zwindowfactory.h"
#include <QApplication>
#include <QDesktopWidget>
#include "z3dapplication.h"
#include "neutubeconfig.h"
#include "z3dcompositor.h"
#include "z3dcanvas.h"
#include "z3dvolumeraycaster.h"
#include "zscalablestack.h"
#include "z3dvolume.h"
#include "z3dvolumesource.h"

ZWindowFactory::ZWindowFactory() : m_parentWidget(NULL),
  m_showVolumeBoundBox(false)
{
}


Z3DWindow* ZWindowFactory::make3DWindow(
    ZStackDoc *doc, Z3DWindow::EInitMode mode)
{
  ZSharedPointer<ZStackDoc> sharedDoc(doc);

  return make3DWindow(sharedDoc, mode);
}

Z3DWindow* ZWindowFactory::make3DWindow(ZSharedPointer<ZStackDoc> doc,
                                        Z3DWindow::EInitMode mode)
{
  Z3DWindow *window = NULL;

  if (Z3DApplication::app()->is3DSupported() && doc) {
    window = new Z3DWindow(doc, mode, false, m_parentWidget);
    if (m_windowTitle.isEmpty()) {
      window->setWindowTitle("3D View");
    } else {
      window->setWindowTitle(m_windowTitle);
    }
    //connect(window, SIGNAL(destroyed()), m_hostFrame, SLOT(detach3DWindow()));
    if (GET_APPLICATION_NAME == "Biocytin") {
      window->getCompositor()->setBackgroundFirstColor(glm::vec3(1, 1, 1));
      window->getCompositor()->setBackgroundSecondColor(glm::vec3(1, 1, 1));
    }
    if (!NeutubeConfig::getInstance().getZ3DWindowConfig().isBackgroundOn()) {
      window->getCompositor()->setShowBackground(false);
    }
    if (doc->getTag() == NeuTube::Document::FLYEM_BODY ||
        doc->getTag() == NeuTube::Document::FLYEM_SPLIT ||
        doc->getTag() != NeuTube::Document::SEGMENTATION_TARGET) {
      window->getVolumeRaycasterRenderer()->setCompositeMode(
            "Direct Volume Rendering");
    }
    if (doc->getTag() != NeuTube::Document::FLYEM_SPLIT &&
        doc->getTag() != NeuTube::Document::SEGMENTATION_TARGET) {
      window->getCanvas()->disableKeyEvent();
    }
    if (!m_showVolumeBoundBox) {
      window->getVolumeRaycaster()->hideBoundBox();
    }

    if (m_windowGeometry.isEmpty()) {
      QRect screenRect = QApplication::desktop()->screenGeometry();
      window->setGeometry(screenRect.width() / 10, screenRect.height() / 10,
                          screenRect.width() - screenRect.width() / 5,
                          screenRect.height() - screenRect.height() / 5);
    } else {
      window->setGeometry(m_windowGeometry);
    }
  }

  return window;
}

Z3DWindow* ZWindowFactory::make3DWindow(ZScalableStack *stack)
{
  if (stack == NULL) {
    return NULL;
  }

  Z3DWindow *window = NULL;

  if (Z3DApplication::app()->is3DSupported()) {
    ZStackDoc *doc = new ZStackDoc(stack->getStack(), NULL);
    window = make3DWindow(doc);
    window->getVolumeSource()->getVolume(0)->setScaleSpacing(
          glm::vec3(stack->getXScale(), stack->getYScale(), stack->getZScale()));
    ZPoint offset = stack->getOffset();
    window->getVolumeSource()->getVolume(0)->setOffset(
          glm::vec3(offset.x(), offset.y(), offset.z()));
    window->updateVolumeBoundBox();
    window->updateOverallBoundBox();
    //window->resetCameraClippingRange();
    window->resetCamera();
    stack->releaseStack();
    delete stack;
  }

  return window;
}

void ZWindowFactory::setWindowTitle(const QString &title)
{
  m_windowTitle = title;
}

void ZWindowFactory::setParentWidget(QWidget *parentWidget)
{
  m_parentWidget = parentWidget;
}
