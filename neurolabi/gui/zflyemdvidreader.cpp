#include "zflyemdvidreader.h"
#include <QStringList>
#include "dvid/zdvidbufferreader.h"
#include "dvid/zdvidurl.h"

ZObject3dScan ZFlyEmDvidReader::readCoarseBody(int bodyId)
{
  ZDvidBufferReader reader;
  ZDvidUrl dvidUrl(m_dvidTarget);
  reader.read(dvidUrl.getCoarseSparsevolUrl(
                bodyId, m_dvidTarget.getBodyLabelName()).c_str());

  ZObject3dScan obj;
  obj.importDvidObjectBuffer(
        reader.getBuffer().data(), reader.getBuffer().size());
  obj.setLabel(bodyId);

  return obj;
}

ZFlyEmBodyAnnotation ZFlyEmDvidReader::readAnnotation(int bodyId)
{
  QByteArray data = readKeyValue("annotations", QString("%1").arg(bodyId));
  ZFlyEmBodyAnnotation annotation;
  annotation.loadJsonString(QString(data).toStdString());

  annotation.setBodyId(bodyId);

  return annotation;
}

QStringList ZFlyEmDvidReader::readSynapseList()
{
  ZDvidBufferReader reader;
  ZDvidUrl dvidUrl(m_dvidTarget);

  reader.read(dvidUrl.getSynapseListUrl().c_str());
  QByteArray buffer = reader.getBuffer();

  ZJsonObject obj;
  obj.decodeString(buffer.data());

  QStringList synapseList;
  if (obj.hasKey("synapses")) {
    ZJsonArray synapseArrayJson(
          obj["synapses"], ZJsonValue::SET_INCREASE_REF_COUNT);
    for (size_t i = 0; i < synapseArrayJson.size(); ++i) {
      const char *value = ZJsonParser::stringValue(synapseArrayJson.at(i));
      if (value != NULL) {
        synapseList.append(value);
      }
    }
  }

  return synapseList;
}

ZJsonObject ZFlyEmDvidReader::readSynapseAnnotation(const QString &name)
{
  ZDvidBufferReader reader;
  ZDvidUrl dvidUrl(m_dvidTarget);

  reader.read(dvidUrl.getSynapseAnnotationUrl(name.toStdString()).c_str());
  QByteArray buffer = reader.getBuffer();
  ZJsonObject obj;
  obj.decodeString(buffer.data());

  return obj;
}
