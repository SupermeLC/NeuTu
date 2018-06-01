#ifndef ZDVIDLABELSLICEHIGHRESTASK_H
#define ZDVIDLABELSLICEHIGHRESTASK_H

#include "ztask.h"
#include "zstackviewparam.h"

class ZStackDoc;

class ZDvidLabelSliceHighresTask : public ZTask
{
  Q_OBJECT
public:
  explicit ZDvidLabelSliceHighresTask(QObject *parent = nullptr);

  void setViewParam(const ZStackViewParam &param);
  void setZoom(int zoom);
  void setCenterCut(int width, int height);
  void setDoc(ZStackDoc *doc);

  void execute();

signals:

public slots:

private:
  ZStackViewParam m_viewParam;
  int m_zoom = 0;
  int m_centerCutWidth = 0;
  int m_centerCutHeight = 0;
  ZStackDoc *m_doc = nullptr;
};

#endif // ZDVIDLABELSLICEHIGHRESTASK_H
