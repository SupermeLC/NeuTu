#ifndef ZSTROKE2D_H
#define ZSTROKE2D_H

#include <QPointF>
#include <QVector>
#include <vector>
#include <QColor>
#include <QImage>

#include "zdocumentable.h"
#include "zstackdrawable.h"
#include "c_stack.h"

class ZStack;

class ZStroke2d : public ZDocumentable, public ZStackDrawable {
public:
  ZStroke2d();
  ZStroke2d(const ZStroke2d &stroke);
  virtual ~ZStroke2d();

public:
  virtual void save(const char *filePath);
  virtual void load(const char *filePath);

  virtual void display(ZPainter &painter, int z = 0, Display_Style option = NORMAL) const;

  void labelBinary(Stack *stack) const;

  /*!
   * \brief Label a stack with the internal label value.
   */
  void labelGrey(Stack *stack) const;

  /*!
   * \brief Label a stack with a given label value.
   *
   * The label is automatically set to 0 if it is negative or 255 if it is
   * larger than 255. \a stack must have GREY kind; otherwise nothing will be
   * done.
   */
  void labelGrey(Stack *stack, int label) const;

  virtual const std::string& className() const;

  inline void setWidth(double width) { m_width = width; }
  void append(double x, double y);
  void set(const QPoint &pt);
  void set(double x, double y);
  void setLabel(int label);

  void clear();

  bool isEmpty() const;

  ZStroke2d* clone();

  void addWidth(double dw);

  void setEraser(bool enabled);
  inline bool isEraser() const { return m_label == 0; }
  inline void setFilled(bool isFilled) {
    m_isFilled = isFilled;
  }
  inline void setZ(int z) { m_z = z; }

  double inline getWidth() { return m_width; }

  bool getLastPoint(int *x, int *y) const;
  bool getLastPoint(double *x, double *y) const;

  inline size_t getPointNumber() const { return m_pointArray.size(); }

  void print() const;

  /*!
   * \brief Translate the stroke
   */
  void translate(const ZPoint offset);

  /*!
   * \brief Convert the stroke to a stack.
   *
   * Only GREY type is supported. If m_label is bigger than 255, label % 255 is
   * taken.
   */
  ZStack *toStack() const;

private:
  static QVector<QColor> constructColorTable();
  const QColor& getLabelColor() const;
  void labelImage(QImage *image) const;


private:
  std::vector<QPointF> m_pointArray;
  double m_width;

  int m_label; //Label = 0 is reserved for eraser
  int m_z;

  //bool m_isEraser;
  bool m_isFilled;

  static const double m_minWidth;
  static const double m_maxWidth;

  const static QVector<QColor> m_colorTable;
  const static QColor m_blackColor;
};

#endif // ZSTROKE2D_H
