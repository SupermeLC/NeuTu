#ifndef ZARRAY_H
#define ZARRAY_H

#include "mylib/array.h"
#include <vector>

class ZArray
{
public:
  typedef mylib::Value_Type Value_Type;
  typedef mylib::Dimn_Type Dimn_Type;

public:
  ZArray();
  ZArray(Value_Type type, int ndims, mylib::Dimn_Type *dims);
  ZArray(const ZArray &array); //always deep copy
  ~ZArray();

  inline int ndims() const { return m_data->ndims; }
  inline int dim(int index) const { return m_data->dims[index]; }
  inline Value_Type valueType() const { return m_data->type; }

  size_t getElementNumber() const;
  size_t getByteNumber() const;

  bool isEmpty() const;

  /*!
   * \brief Set all array elements to 0
   */
  void setZero();

  /*!
   * \brief Copy data from memory buffer
   *
   * Note the function does not check if \a data is valid or properly allocated.
   * It does nothing if \a data is NULL.
   */
  void copyDataFrom(const void *data);

  void printInfo() const;

  static size_t getValueTypeSize(Value_Type valueType);

  void print() const;

private:
  mylib::Array *m_data;
  std::vector<int> m_startCoordinates;
};

#endif // ZARRAY_H
