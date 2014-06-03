#ifndef ZDVIDREQUEST_H
#define ZDVIDREQUEST_H

#include <QVariant>
#include <QString>

class ZDvidRequest
{
public:
  ZDvidRequest();

  enum EDvidRequest {
    DVID_NULL_REQUEST,
    DVID_GET_OBJECT, DVID_SAVE_OBJECT, DVID_UPLOAD_SWC, DVID_GET_SWC,
    DVID_GET_GRAY_SCALE, DVID_GET_SUPERPIXEL_INFO,
    DVID_GET_SP2BODY_STRING, DVID_GET_KEYVALUE, DVID_GET_BODY_LABEL
  };

  void setGetSwcRequest(int bodyId);
  void setGetObjectRequest(int bodyId);
  void setGetImageRequest(int x0, int y0, int z0, int width, int height);
  void setGetImageRequest(
      int x0, int y0, int z0, int width, int height, int depth);
  void setGetBodyLabelRequest(
      int x0, int y0, int z0, int width, int height, int depth);
  void setGetInfoRequest(const QString &dataType);
  void setGetStringRequest(const QString &dataType);
  void setGetKeyValueRequest(const QString &dataName, const QString key);

  void setParameter(const QVariant &parameter);

  inline EDvidRequest getType() const { return m_requestType; }
  inline const QVariant& getParameter() const { return m_parameter; }

private:
  EDvidRequest m_requestType;
  QVariant m_parameter;
};

#endif // ZDVIDREQUEST_H
