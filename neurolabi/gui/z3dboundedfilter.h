#ifndef Z3DBOUNDEDFILTER_H
#define Z3DBOUNDEDFILTER_H

#include "z3dfilter.h"
#include "z3dtransferfunction.h"
#include "z3dlinerenderer.h"
#include "z3dmeshrenderer.h"
#include "z3dsphererenderer.h"
#include "z3darrowrenderer.h"

// child class should implement two pure virtual function and call updateBoundBox whenever its
// own parameter affect bound box
class Z3DBoundedFilter : public Z3DFilter
{
Q_OBJECT
public:
  explicit Z3DBoundedFilter(Z3DGlobalParameters& globalPara, QObject* parent = nullptr);

  void setSelected(bool v);

  bool isSelected() const
  { return m_isSelected; }

  bool isTransformEnabled() const
  { return m_transformEnabled; }

  virtual void setViewport(glm::uvec2 viewport)
  { m_rendererBase.setViewport(viewport); }

  virtual void setViewport(glm::uvec4 viewport)
  { m_rendererBase.setViewport(viewport); }

  inline Z3DCamera& camera()
  { return m_rendererBase.camera(); }

  inline Z3DCamera& globalCamera()
  { return m_rendererBase.globalCamera(); }

  inline Z3DCameraParameter& globalCameraPara()
  { return m_rendererBase.globalCameraPara(); }

  inline Z3DPickingManager& pickingManager()
  { return m_rendererBase.globalParas().pickingManager; }

  inline Z3DTrackballInteractionHandler& interactionHandler()
  { return m_rendererBase.globalParas().interactionHandler; }

  virtual void setShaderHookType(Z3DRendererBase::ShaderHookType t)
  { m_rendererBase.setShaderHookType(t); }

  virtual void setShaderHookParaDDPDepthBlenderTexture(const Z3DTexture* t)
  { m_rendererBase.setShaderHookParaDDPDepthBlenderTexture(t); }

  virtual void setShaderHookParaDDPFrontBlenderTexture(const Z3DTexture* t)
  { m_rendererBase.setShaderHookParaDDPFrontBlenderTexture(t); }

  inline Z3DRendererBase::ShaderHookParameter& shaderHookPara()
  { return m_rendererBase.shaderHookPara(); }

  const ZBBox<glm::dvec3>& axisAlignedBoundBox() const
  { return m_axisAlignedBoundBox; }

  const ZBBox<glm::dvec3>& notTransformedBoundBox() const
  { return m_notTransformedBoundBox; }

  // Useful coordinate L->Left U->Up F->Front R->Right D->Down B->Back
  glm::vec3 physicalLUF() const
  { return glm::vec3(m_notTransformedBoundBox.minCorner()); }

  glm::vec3 physicalRDB() const
  { return glm::vec3(m_notTransformedBoundBox.maxCorner()); }

  glm::vec3 physicalLDF() const
  { return glm::vec3(physicalLUF().x, physicalRDB().y, physicalLUF().z); }

  glm::vec3 physicalRDF() const
  { return glm::vec3(physicalRDB().x, physicalRDB().y, physicalLUF().z); }

  glm::vec3 physicalRUF() const
  { return glm::vec3(physicalRDB().x, physicalLUF().y, physicalLUF().z); }

  glm::vec3 physicalLUB() const
  { return glm::vec3(physicalLUF().x, physicalLUF().y, physicalRDB().z); }

  glm::vec3 physicalLDB() const
  { return glm::vec3(physicalLUF().x, physicalRDB().y, physicalRDB().z); }

  glm::vec3 physicalRUB() const
  { return glm::vec3(physicalRDB().x, physicalLUF().y, physicalRDB().z); }

  // bound voxels in world coordinate
  glm::vec3 worldLUF() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalLUF()); }

  glm::vec3 worldRDB() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalRDB()); }

  glm::vec3 worldLDF() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalLDF()); }

  glm::vec3 worldRDF() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalRDF()); }

  glm::vec3 worldRUF() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalRUF()); }

  glm::vec3 worldLUB() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalLUB()); }

  glm::vec3 worldLDB() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalLDB()); }

  glm::vec3 worldRUB() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalRUB()); }

  virtual bool hasOpaque(Z3DEye) const
  { return m_rendererBase.opacity() == 1.f; }

  virtual void renderOpaque(Z3DEye)
  {}

  virtual bool hasTransparent(Z3DEye) const
  { return m_rendererBase.opacity() < 1.f; }

  virtual void renderTransparent(Z3DEye)
  {}

  void renderSelectionBox(Z3DEye eye);

  virtual void rotateX() override;

  virtual void rotateY() override;

  virtual void rotateZ() override;

  virtual void rotateXM() override;

  virtual void rotateYM() override;

  virtual void rotateZM() override;

signals:

  void boundBoxChanged();

  void objSelected(bool append);

  void objDeselected();

  void objVisibleChanged(bool v);

protected:
  void updateBoundBox();

  // implement this to empty function if clip planes are not needed
  virtual void setClipPlanes();

  void initializeCutRange();

  void initializeRotationCenter();

  void setTransformEnabled(bool v)
  { m_transformEnabled = v; }

  void renderBoundBox(Z3DEye eye);

  void appendBoundboxLines(const ZBBox<glm::dvec3>& bound, std::vector<glm::vec3>& lines);

  // output v1 is start point of ray, v2 is a point on the ray, v2-v1 is normalized
  // x and y are input screen point, width and height are input screen dimension
  void rayUnderScreenPoint(glm::vec3& v1, glm::vec3& v2, int x, int y, int width, int height);

  void rayUnderScreenPoint(glm::dvec3& v1, glm::dvec3& v2, int x, int y, int width, int height);

  // implement these to support bound box
  virtual void updateAxisAlignedBoundBoxImpl();

  virtual void updateNotTransformedBoundBoxImpl()
  {}

  // besides the big selection box, other additional lines can be added through this function
  virtual void addSelectionLines()
  {}

  // reimplement this if cut range has different behavior
  virtual void expandCutRange();

private:
  void updateAxisAlignedBoundBox();

  void updateNotTransformedBoundBox();

  void onBoundBoxModeChanged();

  void updateBoundBoxLineColors();

  void updateSelectionLineColors();

  void onBoundBoxLineWidthChanged();

  void onSelectionBoundBoxLineWidthChanged();

  void makeSelectionGeometries();

protected:
  Z3DRendererBase m_rendererBase;

  Z3DLineRenderer m_baseBoundBoxRenderer;
  Z3DLineRenderer m_selectionBoundBoxRenderer;
  Z3DMeshRenderer m_selectionCornerRenderer;

  ZFloatSpanParameter m_xCut;
  ZFloatSpanParameter m_yCut;
  ZFloatSpanParameter m_zCut;
  ZStringIntOptionParameter m_boundBoxMode;
  ZIntParameter m_boundBoxLineWidth;
  ZVec4Parameter m_boundBoxLineColor;
  //Z3DTransferFunctionParameter m_boundBoxLineColor;
  ZIntParameter m_selectionLineWidth;
  ZVec4Parameter m_selectionLineColor;

  ZBBox<glm::dvec3> m_axisAlignedBoundBox;
  glm::vec3 m_center;
  ZBBox<glm::dvec3> m_notTransformedBoundBox;

  std::vector<glm::vec3> m_normalBoundBoxLines;
  std::vector<glm::vec3> m_axisAlignedBoundBoxLines;
  std::vector<glm::vec4> m_boundBoxLineColors;

  std::vector<glm::vec3> m_selectionLines;
  ZMesh m_selectionCornerCubes;
  std::vector<ZMesh*> m_selectionCornerCubesWrapper;
  std::vector<glm::vec4> m_selectionLineColors;

  bool m_canUpdateClipPlane;

  bool m_isSelected;

private:
  ZBBox<glm::dvec3> m_selectionBoundBox;

  glm::ivec2 m_lastMousePosition;
  glm::vec3 m_startMouseWorldPos;
  glm::vec3 m_startTrans;
  float m_startDepth;

  bool m_transformEnabled;
};

#endif // Z3DBOUNDEDFILTER_H
