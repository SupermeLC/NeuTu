#ifndef ZDVIDTARGET_H
#define ZDVIDTARGET_H

#include <string>
#include "zjsonobject.h"

/*!
 * \brief The class of representing the location of a dvid server
 */
class ZDvidTarget
{
public:
  ZDvidTarget();
  ZDvidTarget(const std::string &address, const std::string &uuid, int port = -1);

  void set(const std::string &address, const std::string &uuid, int port = -1);

  /*!
   * \brief Set dvid target from source string
   *
   * The old settings will be cleared no matter what.
   *
   * \param sourceString Must start with "http:".
   */
  void set(const std::string &sourceString);

  inline const std::string& getAddress() const {
    return m_address;
  }

  std::string getAddressWithPort() const;

  inline const std::string& getUuid() const {
    return m_uuid;
  }

  inline const std::string& getComment() const {
    return m_comment;
  }

  inline const std::string& getName() const {
    return m_name;
  }

  inline int getPort() const {
    return m_port;
  }

  inline void setName(const std::string &name) {
    m_name = name;
  }

  std::string getUrl() const;
  std::string getUrl(const std::string &dataName) const;

  /*!
   * \brief Get a single string to represent the target
   *
   * \return "http:address:port:uuid"
   */
  std::string getSourceString(bool withHttpPrefix = true) const;

  /*!
   * \brief getBodyPath
   *
   * The functions does not check if a body exists.
   *
   * \return The path of a certain body.
   */
  std::string getBodyPath(int bodyId) const;

  /*!
   * \brief Test if the target is valid
   *
   * \return true iff the target is valid.
   */
  bool isValid() const;

  /*!
   * \brief Load json object
   */
  void loadJsonObject(const ZJsonObject &obj);

  void print() const;

private:
  std::string m_address;
  std::string m_uuid;
  int m_port;
  std::string m_name;
  std::string m_comment;

  const static char* m_addressKey;
  const static char* m_portKey;
  const static char* m_uuidKey;
  const static char* m_commentKey;
  const static char* m_nameKey;
};

#endif // ZDVIDTARGET_H
