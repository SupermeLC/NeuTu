/**@file zstackdoc.h
 * @brief Stack document
 * @author Ting Zhao
 */
#ifndef _ZSTACKDOC_H_
#define _ZSTACKDOC_H_

/**@file zstackdoc.h
 * @brief Stack document
 * @author Ting Zhao
 */

#include <QString>
#include <QList>
#include <QUrl>
#include <set>
#include <QObject>
#include <QUndoCommand>
#include <QMap>
#include <string>
#include <QMenu>
#include <QPair>
#include <QMap>

#include "neutube.h"
#include "zcurve.h"
#include "zswctree.h"
#include "zobject3d.h"
#include "tz_local_neuroseg.h"
#include "tz_locseg_chain.h"
#include "tz_trace_defs.h"
#include "zpunctum.h"
#include "zprogressreporter.h"
#include "zrescaleswcdialog.h"
#include "zreportable.h"
#include "biocytin/zstackprojector.h"
#include "zstackreadthread.h"
#include "zstackfile.h"
#include "zactionactivator.h"
#include "resolutiondialog.h"
#include "zneurontracer.h"
#include "zdocplayer.h"
#include "z3dgraph.h"

class ZStackFrame;
class ZLocalNeuroseg;
class ZStackObject;
class ZLocsegChain;
class ZLocsegChainConn;
class QXmlStreamReader;
class QProgressBar;
class ZStack;
class ZResolution;
class ZSwcNetwork;
class ZSwcObjsModel;
class ZPunctaObjsModel;
class ZStroke2d;
class QWidget;
class ZSwcNodeObjsModel;
class ZDocPlayerObjsModel;
class ZStackDocReader;
class ZStackFactory;
class ZSparseObject;
class ZSparseStack;
class ZStackBall;
class ZUndoCommand;

/*!
 * \brief The class of stack document
 *
 * Each document has at most one main stack, which defines some context of other
 * data (e.g. resolution).
 */
class ZStackDoc : public QObject, public ZReportable, public ZProgressable
{
  Q_OBJECT

public:
  ZStackDoc(ZStack *stack, QObject *parent);
  virtual ~ZStackDoc();

  //Designed for multi-thread reading

  enum TubeImportOption {
    ALL_TUBE,
    GOOD_TUBE,
    BAD_TUBE
  };

  enum LoadObjectOption {
    REPLACE_OBJECT,
    APPEND_OBJECT
  };

  enum EComponent {
    STACK, STACK_MASK, STACK_SEGMENTATION, SEGMENTATION_OBJECT,
    SEGMENTATION_GRAPH, SEGMENTATION_INDEX_MAP, SPARSE_STACK
  };

  enum EDocumentDataType {
    SWC_DATA, PUNCTA_DATA, STACK_DATA, NETWORK_DATA
  };

  enum EActionItem {
    ACTION_MEASURE_SWC_NODE_LENGTH, ACTION_MEASURE_SCALED_SWC_NODE_LENGTH,
    ACTION_SWC_SUMMARIZE,
    ACTION_CHNAGE_SWC_NODE_SIZE, ACTION_TRANSLATE_SWC_NODE,
    ACTION_SET_SWC_ROOT, ACTION_INSERT_SWC_NODE,
    ACTION_RESET_BRANCH_POINT, ACTION_SET_BRANCH_POINT,
    ACTION_CONNECTED_ISOLATED_SWC,
    ACTION_DELETE_SWC_NODE, ACTION_CONNECT_SWC_NODE,
    ACTION_MERGE_SWC_NODE, ACTION_BREAK_SWC_NODE,
    ACTION_SELECT_DOWNSTREAM, ACTION_SELECT_UPSTREAM,
    ACTION_SELECT_NEIGHBOR_SWC_NODE,
    ACTION_SELECT_SWC_BRANCH, ACTION_SELECT_CONNECTED_SWC_NODE,
    ACTION_SELECT_ALL_SWC_NODE,
    ACTION_CHANGE_SWC_TYPE, ACTION_CHANGE_SWC_SIZE, ACTION_REMOVE_TURN,
    ACTION_RESOLVE_CROSSOVER, ACTION_SWC_Z_INTERPOLATION,
    ACTION_SWC_RADIUS_INTERPOLATION, ACTION_SWC_POSITION_INTERPOLATION,
    ACTION_SWC_INTERPOLATION
  };

public: //attributes
  // isEmpty() returns true iff it has no stack data or object.
  bool isEmpty();

  // hasStackData() returns true iff it has stack array data.
  bool hasStackData() const;
  /*!
   * \brief Test if there is any stack (including virtual)
   */
  bool hasStack() const;
  bool hasStackMask();

  // hasTracable() returns true iff it has tracable data.
  bool hasTracable();

  // hasObject() returns true iff it has an object.
  bool hasObject();
  // hasSwc() returns true iff it has an SWC object.
  bool hasSwc() const;
  // hasDrawable() returns true iff it has a drawable object.
  bool hasDrawable();
  bool hasSparseObject();
  bool hasSparseStack() const;

  bool hasSelectedSwc() const;
  bool hasSelectedSwcNode() const;
  bool hasMultipleSelectedSwcNode() const;

  int stackWidth() const;
  int stackHeight() const;
  int stackChannelNumber() const;

  virtual void deprecateDependent(EComponent component);
  virtual void deprecate(EComponent component);
  virtual bool isDeprecated(EComponent component);


  void clearData();

  /*!
   * \brief The offset from stack space to data space
   */
  ZIntPoint getStackOffset() const;
  void setStackOffset(int x, int y, int z);
  void setStackOffset(const ZIntPoint &offset);
  void setStackOffset(const ZPoint &offset);

  ZIntPoint getStackSize() const;

  /*!
   * \brief Get the data space coordinates of stack coordinates
   */
  ZIntPoint getDataCoord(const ZIntPoint &pt);
  ZIntPoint getDataCoord(int x, int y, int z);

  /*!
   * \brief Map stack coodinates to data space
   */
  void mapToDataCoord(ZPoint *pt);
  void mapToDataCoord(double *x, double *y, double *z);

  /*!
   * \brief Data coordinates to stack coordinates
   */
  void mapToStackCoord(ZPoint *pt);
  void mapToStackCoord(double *x, double *y, double *z);


  // Prefix for tracing project.
  const char *tubePrefix() const;

  inline QList<ZStackObject*>* drawableList() { return &m_objectList; }

  inline QList<ZSwcTree*>* swcList() {return &m_swcList;}
  bool hasSwcList();       //to test swctree
  inline QList<ZLocsegChain*>* chainList() {return &m_chainList;}
  inline QList<ZPunctum*>* punctaList() {return &m_punctaList;}
  inline ZSwcObjsModel* swcObjsModel() {return m_swcObjsModel;}
  inline ZDocPlayerObjsModel* seedObjsModel() { return m_seedObjsModel; }
  inline ZSwcNodeObjsModel* swcNodeObjsModel() {return m_swcNodeObjsModel;}
  inline ZPunctaObjsModel* punctaObjsModel() {return m_punctaObjsModel;}
  inline std::set<ZPunctum*>* selectedPuncta() {return &m_selectedPuncta;}
  inline std::set<ZLocsegChain*>* selectedChains() {return &m_selectedChains;}
  inline std::set<ZSwcTree*>* selectedSwcs() {return &m_selectedSwcs;}
  inline const std::set<ZSwcTree*>* selectedSwcs() const {
    return &m_selectedSwcs;}
  inline std::set<Swc_Tree_Node*>* selectedSwcTreeNodes() {
    return &m_selectedSwcTreeNodes;}
  inline const std::set<Swc_Tree_Node*>* selectedSwcTreeNodes() const {
    return &m_selectedSwcTreeNodes;}

  /*
  inline std::set<ZStroke2d*>* selectedStrokeList() {
    return &m_selectedStroke;}
  inline const std::set<ZStroke2d*>* selectedStrokeList() const {
    return &m_selectedStroke;}
    */

  inline ZSwcNetwork* swcNetwork() { return m_swcNetwork; }
  ZResolution stackResolution() const;
  std::string stackSourcePath() const;
  bool hasChainList();

  //void setStackMask(ZStack *stack);

  void createActions();
  inline QAction* getAction(EActionItem item) const {
    return m_actionMap[item];
  }

  void updateSwcNodeAction();

  void addData(const ZStackDocReader &reader);


  bool isUndoClean() const;
  bool isSwcSavingRequired() const;
  void setSaved(NeuTube::EDocumentableType type, bool state);

  const ZUndoCommand* getLastUndoCommand() const;
  //const ZUndoCommand* getLastCommand() const;

public: //swc tree edit
  // move soma (first root) to new location
  void swcTreeTranslateRootTo(double x, double y, double z);
  // rescale location and radius
  void swcTreeRescale(double scaleX, double scaleY, double scaleZ);
  void swcTreeRescale(double srcPixelPerUmXY, double srcPixelPerUmZ, double dstPixelPerUmXY, double dstPixelPerUmZ);
  // rescale radius of nodes of certain depth
  void swcTreeRescaleRadius(double scale, int startdepth, int enddepth);
  // reduce node number, similar to Swc_Tree_Merge_Close_Node, but only merge Continuation node
  void swcTreeReduceNodeNumber(double lengthThre);
  void updateVirtualStackSize();

  void deleteSelectedSwcNode();
  void addSizeForSelectedSwcNode(double dr);

  void estimateSwcRadius(ZSwcTree *tree, int maxIter = 1);
  void estimateSwcRadius();

public: //swc selection
  //Select connection
  //void selectSwcNodeConnection(); // change to slot
  void selectSwcNodeNeighbor();
  std::string getSwcSource() const;

public:
  void loadFileList(const QList<QUrl> &urlList);
  void loadFileList(const QStringList &filePath);
  bool loadFile(const QString &filePath, bool emitMessage = false);
  virtual void loadStack(Stack *getStack, bool isOwner = true);
  virtual void loadStack(ZStack *zstack);

  /*!
   * \brief The reference of the main stack variable
   */
  virtual ZStack*& stackRef();
  virtual const ZStack *stackRef() const;

  void readStack(const char *filePath, bool newThread = true);
  void readSwc(const char *filePath);

  void saveSwc(QWidget *parentWidget);

  inline ZStackFrame* getParentFrame() { return m_parentFrame; }
  void setParentFrame(ZStackFrame* parent);

  virtual ZStack* getStack() const;
  virtual ZStack *stackMask() const;
  void setStackSource(const char *filePath);
  void setStackSource(const ZStackFile &stackFile);
  void loadSwcNetwork(const QString &filePath);
  void loadSwcNetwork(const char *filePath);
  bool importImageSequence(const char *filePath);

  void loadSwc(const QString &filePath);
  void loadLocsegChain(const QString &filePath);

  void importFlyEmNetwork(const char *filePath);

  //void exportVrml(const char *filePath);
  void exportSvg(const char *filePath);
  void exportBinary(const char *prefix);
  void exportSwcTree(const char *filePath);
  void exportChainFileList(const char *filepath);
  void exportPuncta(const char *filePath);
  void exportObjectMask(const std::string &filePath);

  //Those functions do not notify object modification
  void removeLastObject(bool deleteObject = false);
  void removeAllObject(bool deleteObject = true);
  ZDocPlayer::TRole removeObject(ZStackObject *obj, bool deleteObject = false);
  void removeSelectedObject(bool deleteObject = false);

  /* Remove object with specific roles */
  void removeObject(ZDocPlayer::TRole role, bool deleteObject = false);

  void removeSelectedPuncta(bool deleteObject = false);
  void removeSmallLocsegChain(double thre);   //remove small locseg chain (geolen < thre)
  void removeAllLocsegChain();
  void removeAllObj3d();
  void removeAllSparseObject();
  std::set<ZSwcTree*> removeEmptySwcTree(bool deleteObject = true);
  void removeAllSwcTree(bool deleteObject = true);

  void addObject(ZStackObject *obj);
  void appendSwcNetwork(ZSwcNetwork &network);

  QString toString();
  QStringList toStringList() const;
  virtual QString dataInfo(double cx, double cy, int z) const;

  ZCurve locsegProfileCurve(int option) const;

  void cutLocsegChain(ZLocsegChain *obj, QList<ZLocsegChain*> *pResult = NULL);   //optional output cut result
  void cutSelectedLocsegChain();
  void breakLocsegChain(ZLocsegChain *obj, QList<ZLocsegChain*> *pResult = NULL);  //optional output break result
  void breakSelectedLocsegChain();

  int maxIntesityDepth(int x, int y);
  ZStack* projectBiocytinStack(Biocytin::ZStackProjector &projector);

  void updateStackFromSource();
  void setStackFactory(ZStackFactory *factory);

  void setSparseStack(ZSparseStack *spStack);

  void importSeedMask(const QString &filePath);

public: //Image processing
  static int autoThreshold(Stack* getStack);
  int autoThreshold();
  bool binarize(int threshold);
  bool bwsolid();
  bool enhanceLine();
  bool watershed();
  bool invert();
  int findLoop(int minLoopSize = 100);
  void bwthin();
  bool bwperim();
  void runSeededWatershed();
  void runLocalSeededWatershed();

private:
  void localSeededWatershed();
  void seededWatershed();

public: /* tracing routines */
  ZLocsegChain* fitseg(int x, int y, int z, double r = 3.0);
  ZLocsegChain* fitRpiseg(int x, int y, int z, double r = 3.0);
  ZLocsegChain* fitRect(int x, int y, int z, double r = 3.0);
  ZLocsegChain* fitEllipse(int x, int y, int z, double r = 1.0);
  ZLocsegChain* dropseg(int x, int y, int z, double r = 3.0);
  ZLocsegChain* traceTube(int x, int y, int z, double r = 3.0, int c = 0);
  ZLocsegChain* traceRect(int x, int y, int z, double r = 3.0, int c = 0);

  void refreshTraceMask();

public: /* puncta related methods */
  ZPunctum* markPunctum(int x, int y, int z, double r = 2.0);
  int pickPunctaIndex(int x, int y, int z) const;
  bool selectPuncta(int index);
  bool deleteAllPuncta();
  bool expandSelectedPuncta();
  bool shrinkSelectedPuncta();
  bool meanshiftSelectedPuncta();
  bool meanshiftAllPuncta();
  inline bool hasSelectedPuncta() {return !m_selectedPuncta.empty();}

  void addLocsegChain(ZLocsegChain *chain);
  void addLocsegChain(const QList<ZLocsegChain*> &chainList);

  void addSwcTree(ZSwcTree *obj, bool uniqueSource = true);
  void addSwcTree(ZSwcTree *obj, bool uniqueSource, bool translatingWithStack);
  void addSwcTree(const QList<ZSwcTree*> &swcList, bool uniqueSource = true);
  void addSparseObject(const QList<ZSparseObject*> &objList);
  void addPunctum(ZPunctum *obj);
  void addPunctum(const QList<ZPunctum*> &punctaList);


  void addObj3d(ZObject3d *obj);
  void addStroke(ZStroke2d *obj);
  void addSparseObject(ZSparseObject *obj);

  /*!
   * \brief Add an object
   * \param obj
   * \param type
   * \param role
   * \param uniqueSource Replace the object with the same nonempty source if it
   *        is true. Note that if there are multiple objects with the same source
   *        existing in the doc, only the first one is replaced.
   */
  void addObject(ZStackObject *obj, NeuTube::EDocumentableType type,
                 ZDocPlayer::TRole role = ZDocPlayer::ROLE_NONE,
                 bool uniqueSource = false);

  /*!
   * \brief Add a palyer
   *
   * Nothing will be done if \a role is ZDocPlayer::ROLE_NONE.
   */
  void addPlayer(ZStackObject *obj, NeuTube::EDocumentableType type,
                 ZDocPlayer::TRole role);

  void updateLocsegChain(ZLocsegChain *chain);
  void importLocsegChain(const QStringList &files,
                         TubeImportOption option = ALL_TUBE,
                         LoadObjectOption objopt = APPEND_OBJECT);
  void importGoodTube(const char *dirpath = NULL, const char *prefix = NULL,
                      QProgressBar *pb = NULL);
  void importBadTube(const char *dirpath, const char *prefix = NULL);
  void importSwc(QStringList files, LoadObjectOption objopt = APPEND_OBJECT);
  void importPuncta(const QStringList &files,
                    LoadObjectOption objopt = APPEND_OBJECT);

  bool importPuncta(const char *filePath);

  int pickLocsegChainId(int x, int y, int z) const;
  void holdClosestSeg(int id, int x, int y, int z);
  int selectLocsegChain(int id, int x = -1, int y = -1, int z = -1,
  		bool showProfile = false);
  bool selectSwcTreeBranch(int x, int y, int z);
  bool pushLocsegChain(ZStackObject *obj);
  void pushSelectedLocsegChain();

  bool importSynapseAnnotation(const std::string &filePath);

  ZStackObject *hitTest(double x, double y, double z);
  ZStackObject *hitTest(double x, double y);

  Swc_Tree_Node *swcHitTest(double x, double y) const;
  Swc_Tree_Node *swcHitTest(double x, double y, double z) const;
  Swc_Tree_Node *swcHitTest(const ZPoint &pt) const;
  Swc_Tree_Node *selectSwcTreeNode(int x, int y, int z, bool append = false);
  Swc_Tree_Node *selectSwcTreeNode(const ZPoint &pt, bool append = false);
  void selectSwcTreeNode(Swc_Tree_Node *tn, bool append = false);

  // (de)select objects and emit signals for 3D view and 2D view to sync
  void setPunctumSelected(ZPunctum* punctum, bool select);
  template <class InputIterator>
  void setPunctumSelected(InputIterator first, InputIterator last, bool select);
  void deselectAllPuncta();
  void setChainSelected(ZLocsegChain* chain, bool select);
  void setChainSelected(const std::vector<ZLocsegChain*> &chains, bool select);
  void deselectAllChains();
  //void deselectAllStroke();
  void setSwcSelected(ZSwcTree* tree, bool select);
  template <class InputIterator>
  void setSwcSelected(InputIterator first, InputIterator last, bool select);
  void deselectAllSwcs();
  void setSwcTreeNodeSelected(Swc_Tree_Node* tn, bool select);
  template <class InputIterator>
  void setSwcTreeNodeSelected(InputIterator first, InputIterator last, bool select);
  void deselectAllSwcTreeNodes();
  void deselectAllObject();

  // show/hide objects and emit signals for 3D view and 2D view to sync
  void setPunctumVisible(ZPunctum* punctum, bool visible);
  void setChainVisible(ZLocsegChain* chain, bool visible);
  void setSwcVisible(ZSwcTree* tree, bool visible);

  void setTraceMinScore(double score);
  void setReceptor(int option, bool cone = false);

  //void updateMasterLocsegChain();
  //bool linkChain(int id);
  //bool hookChain(int id, int option = 0);
  bool mergeChain(int id);
  bool connectChain(int id);
  bool disconnectChain(int id);
  //bool isMasterChainId(int id);
  /*
  inline bool isMasterChain(const ZLocsegChain *chain) const {
    return m_masterChain == chain;
  }
  inline void setMasterChain(ZLocsegChain *chain) {
    m_masterChain = chain;
  }
*/
  void eraseTraceMask(const ZLocsegChain *chain);

  //bool chainShortestPath(int id);
  //void chainConnInfo(int id);

  //void extendChain(double x, double y, double z);

  ZStackObject *bringChainToFront();
  ZStackObject* sendChainToBack();
  //void refineSelectedChainEnd();
  //void refineLocsegChainEnd();
  void mergeAllChain();

  //void importLocsegChainConn(const char *filePath);
  //void exportLocsegChainConn(const char *filePath);
  //void exportLocsegChainConnFeat(const char *filePath);

  //void buildLocsegChainConn();
  //void clearLocsegChainConn();
  //void selectNeighbor();
  //void selectConnectedChain();

  void setWorkdir(const QString &filePath);
  void setWorkdir(const char *filePath);
  void setTubePrefix(const char *filePath);
  void setBadChainScreen(const char *screen);

  //void autoTrace();
  //void autoTrace(Stack* stack);
  //void traceFromSwc(QProgressBar *pb = NULL);

  void test(QProgressBar *pb = NULL);

  inline QUndoStack* undoStack() const { return m_undoStack; }
  inline void pushUndoCommand(QUndoCommand *command) { m_undoStack->push(command); }

  inline std::string additionalSource() { return m_additionalSource; }
  inline void setAdditionalSource(const std::string &filePath) {
    m_additionalSource = filePath;
  }

  bool hasObjectSelected();

  inline const QList<ZStackObject*>& getObjectList() const { return m_objectList; }
  inline QList<ZStackObject*>& getObjectList() { return m_objectList; }

  inline const QList<ZObject3d*>& getObj3dList() const { return m_obj3dList; }
  inline QList<ZObject3d*>& getObj3dList() { return m_obj3dList; }

  inline const QList<ZLocsegChain*>& getChainList() const { return m_chainList; }
  inline QList<ZLocsegChain*>& getChainList() { return m_chainList; }

  inline const QList<ZPunctum*>& getPunctaList() const { return m_punctaList; }
  inline QList<ZPunctum*>& getPunctaList() { return m_punctaList; }

  inline const QList<ZStroke2d*>& getStrokeList() const { return m_strokeList; }
  inline QList<ZStroke2d*>& getStrokeList() { return m_strokeList; }

  inline const QList<ZSparseObject*>& getSparseObjectList() const {
    return m_sparseObjectList; }
  inline QList<ZSparseObject*>& getSparseObjectList() {
    return m_sparseObjectList; }

  inline const QList<ZSwcTree*>& getSwcList() const { return m_swcList; }
  inline QList<ZSwcTree*>& getSwcList() { return m_swcList; }
  inline ZSwcTree* getSwcTree(size_t index) {
    if ((int) index >= m_swcList.size()) return NULL;
    return m_swcList[index]; }
  QList<ZSwcTree*>::iterator getSwcIteratorBegin() { return m_swcList.begin(); }
  QList<ZSwcTree*>::iterator getSwcIteratorEnd() { return m_swcList.end(); }
  QList<ZSwcTree*>::const_iterator getSwcIteratorBegin() const {
    return m_swcList.begin(); }
  QList<ZSwcTree*>::const_iterator getSwcIteratorEnd() const {
    return m_swcList.end(); }

  inline const ZDocPlayerList& getPlayerList() const {
    return m_playerList;
  }

  inline const ZSparseStack* getSparseStack() const {
    return m_sparseStack;
  }

  QList<const ZDocPlayer*> getPlayerList(ZDocPlayer::TRole role) const;

  bool hasPlayer(ZDocPlayer::TRole role) const;

  Z3DGraph get3DGraphDecoration() const;

  std::vector<ZSwcTree*> getSwcArray() const;
  bool getLastStrokePoint(int *x, int *y) const;
  bool getLastStrokePoint(double *x, double *y) const;

  void updateModelData(EDocumentDataType type);

  /*!
   * \brief Get all swc trees from the document in a single tree
   *
   * \return The user is responsible of freeing the returned object.
   */
  ZSwcTree *getMergedSwc();

  void setSelected(ZStackObject *obj,  bool selecting = true);
  const std::set<ZStackObject*>& getSelected(ZStackObject::EType type) const;
  std::set<ZStackObject*>& getSelected(ZStackObject::EType type);

  void clearSelectedSet();

  /*
  void setSelected(ZStackObject *obj, NeuTube::EDocumentableType type,
                   bool selecting = true);
                   */

public:
  inline NeuTube::Document::ETag getTag() const { return m_tag; }
  inline void setTag(NeuTube::Document::ETag tag) { m_tag = tag; }
  void setStackBackground(NeuTube::EImageBackground bg);
  inline NeuTube::EImageBackground getStackBackground() const {
    return m_stackBackground;
  }

public:
  inline void deprecateTraceMask() { m_isTraceMaskObsolete = true; }
  void updateTraceWorkspace(int traceEffort, bool traceMasked,
                            double xRes, double yRes, double zRes);
  void updateConnectionTestWorkspace(double xRes, double yRes, double zRes,
                                     char unit, double distThre, bool spTest,
                                     bool crossoverTest);

  inline Trace_Workspace* getTraceWorkspace() const {
    return m_neuronTracer.getTraceWorkspace();
  }

  inline Connection_Test_Workspace* getConnectionTestWorkspace() const {
    return m_neuronTracer.getConnectionTestWorkspace();
  }

  /*
  inline ZSwcTree* previewSwc() { return m_previewSwc; }
  void updatePreviewSwc();
  */
  void notifyObjectModified();

  /*!
   * \brief Notify any connect slots about the modification of SWC objects
   *
   * It explicitly deprecate all intermediate components of all the SWC objects
   */
  void notifySwcModified();

  void notifyPunctumModified();
  void notifyChainModified();
  void notifyObj3dModified();
  void notifySparseObjectModified();
  void notifyStackModified();
  void notifySparseStackModified();
  void notifyVolumeModified();
  void notifyStrokeModified();
  void notifyAllObjectModified();
  void notify3DGraphModified();
  void notifyStatusMessageUpdated(const QString &message);

public:
  inline QAction* getUndoAction() { return m_undoAction; }
  inline QAction* getRedoAction() { return m_redoAction; }

  ZSingleSwcNodeActionActivator* getSingleSwcNodeActionActivator()  {
    return &m_singleSwcNodeActionActivator;
  }

  inline const ZStack* getLabelField() const {
    return m_labelField;
  }

  void setLabelField(ZStack *getStack);

  ZStack* makeLabelStack(ZStack *stack = NULL) const;

  void notifyPlayerChanged(ZDocPlayer::TRole role);

public slots: //undoable commands
  bool executeAddObjectCommand(ZStackObject *obj, NeuTube::EDocumentableType type,
      ZDocPlayer::TRole role = ZDocPlayer::ROLE_NONE);
  bool executeRemoveSelectedObjectCommand();
  //bool executeRemoveUnselectedObjectCommand();
  bool executeMoveObjectCommand(
      double x, double y, double z,
      double punctaScaleX, double punctaScaleY, double punctaScaleZ,
      double swcScaleX, double swcScaleY, double swcScaleZ);

  bool executeTraceTubeCommand(double x, double y, double z, int c = 0);
  bool executeRemoveTubeCommand();
  bool executeAutoTraceCommand();
  bool executeAutoTraceAxonCommand();

  bool executeAddSwcCommand(ZSwcTree *tree);
  bool executeReplaceSwcCommand(
      ZSwcTree *tree, ZDocPlayer::TRole role = ZDocPlayer::ROLE_NONE);
  void executeSwcRescaleCommand(const ZRescaleSwcSetting &setting);
  bool executeSwcNodeExtendCommand(const ZPoint &center);
  bool executeSwcNodeExtendCommand(const ZPoint &center, double radius);
  bool executeSwcNodeSmartExtendCommand(const ZPoint &center);
  bool executeSwcNodeSmartExtendCommand(const ZPoint &center, double radius);
  bool executeSwcNodeChangeZCommand(double z);
  bool executeSwcNodeEstimateRadiusCommand();
  bool executeMoveSwcNodeCommand(double dx, double dy, double dz);
  bool executeScaleSwcNodeCommand(
      double sx, double sy, double sz, const ZPoint &center);
  bool executeRotateSwcNodeCommand(double theta, double psi, bool aroundCenter);
  bool executeTranslateSelectedSwcNode();
  bool executeDeleteSwcNodeCommand();
  bool executeConnectSwcNodeCommand();
  bool executeChangeSelectedSwcNodeSize();
  bool executeConnectSwcNodeCommand(Swc_Tree_Node *tn);
  bool executeConnectSwcNodeCommand(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2);
  bool executeSmartConnectSwcNodeCommand(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2);
  bool executeSmartConnectSwcNodeCommand();
  bool executeBreakSwcConnectionCommand();
  bool executeAddSwcNodeCommand(const ZPoint &center, double radius);
  bool executeSwcNodeChangeSizeCommand(double dr);
  bool executeMergeSwcNodeCommand();
  bool executeTraceSwcBranchCommand(double x, double y, double z, int c = 0);
  bool executeTraceSwcBranchCommand(double x, double y);
  bool executeInterpolateSwcZCommand();
  bool executeInterpolateSwcRadiusCommand();
  bool executeInterpolateSwcPositionCommand();
  bool executeInterpolateSwcCommand();
  bool executeBreakForestCommand();
  bool executeGroupSwcCommand();
  bool executeSetRootCommand();
  bool executeRemoveTurnCommand();
  bool executeResolveCrossoverCommand();
  bool executeInsertSwcNode();
  bool executeSetBranchPoint();
  bool executeConnectIsolatedSwc();
  bool executeResetBranchPoint();

  bool executeMoveAllSwcCommand(double dx, double dy, double dz);
  bool executeScaleAllSwcCommand(double sx, double sy, double sz,
                                 bool aroundCenter = false);
  bool executeRotateAllSwcCommand(
      double theta, double psi, bool aroundCenter = false);

  bool executeBinarizeCommand(int thre);
  bool executeBwsolidCommand();
  bool executeEnhanceLineCommand();
  bool executeWatershedCommand();

  //bool executeAddStrokeCommand(ZStroke2d *stroke);
  //bool executeAddStrokeCommand(const QList<ZStroke2d*> &strokeList);

public slots:
  void selectAllSwcTreeNode();
  void autoSave();
  bool saveSwc(const std::string &filePath);
  void loadReaderResult();
  void selectDownstreamNode();
  void selectSwcNodeConnection(Swc_Tree_Node *lastSelectedNode = NULL);
  void selectSwcNodeFloodFilling(Swc_Tree_Node *lastSelectedNode);
  void selectUpstreamNode();
  void selectBranchNode();
  void selectTreeNode();
  void selectConnectedNode();

  /*!
   * \brief Select neighboring swc nodes.
   *
   * Add the neighbors of the current selected nodes into the selection set.
   */
  void selectNeighborSwcNode();

  void showSeletedSwcNodeLength(double *resolution = NULL);
  void showSeletedSwcNodeScaledLength();
  void showSwcSummary();

  void hideSelectedPuncta();
  void showSelectedPuncta();

  void emptySlot();

  void reloadStack();

/*
public:
  inline void notifyStackModified() {
    emit stackModified();
  }
*/
signals:
  void locsegChainSelected(ZLocsegChain*);
  void stackDelivered(Stack *getStack, bool beOwner);
  void frameDelivered(ZStackFrame *frame);
  void stackModified();
  void sparseStackModified();
  void labelFieldModified();
  void stackReadDone();
  void stackLoaded();
  void punctaModified();
  void swcModified();
  void seedModified();
  void chainModified();
  void obj3dModified();
  void sparseObjectModified();
  void strokeModified();
  void graph3dModified();
  void objectModified();
  void swcNetworkModified();
  void punctaSelectionChanged(QList<ZPunctum*> selected,
                              QList<ZPunctum*> deselected);
  void chainSelectionChanged(QList<ZLocsegChain*> selected,
                             QList<ZLocsegChain*> deselected);
  void swcSelectionChanged(QList<ZSwcTree*> selected,
                           QList<ZSwcTree*> deselected);
  void swcTreeNodeSelectionChanged(QList<Swc_Tree_Node*> selected,
                                   QList<Swc_Tree_Node*> deselected);
  void punctumVisibleStateChanged(ZPunctum* punctum, bool visible);
  void chainVisibleStateChanged(ZLocsegChain* chain, bool visible);
  void swcVisibleStateChanged(ZSwcTree* swctree, bool visible);
  void cleanChanged(bool);
  void holdSegChanged();
  void statusMessageUpdated(QString message) const;

  /*!
   * \brief A signal indicating modification of volume rendering
   */
  void volumeModified();

private:
  void connectSignalSlot();
  void initNeuronTracer();
  //void initTraceWorkspace();
  //void initConnectionTestWorkspace();
  //void loadTraceMask(bool traceMasked);
  int xmlConnNode(QXmlStreamReader *xml, QString *filePath, int *spot);
  int xmlConnMode(QXmlStreamReader *xml);
  ZSwcTree* nodeToSwcTree(Swc_Tree_Node* node) const;
  std::vector<ZStack*> createWatershedMask();
  ResolutionDialog* getResolutionDialog();
  void updateWatershedBoundaryObject(ZStack *out, ZIntPoint dsIntv);


private:
  //Main stack
  ZStack *m_stack;
  ZSparseStack *m_sparseStack; //Serve as main data when m_stack is virtual.


  //Concrete objects
  QList<ZSwcTree*> m_swcList;
  QList<ZPunctum*> m_punctaList;
  QList<ZStroke2d*> m_strokeList;
  QList<ZObject3d*> m_obj3dList;
  QList<ZSparseObject*> m_sparseObjectList;

  ZDocPlayerList m_playerList;

  //Special object
  ZSwcNetwork *m_swcNetwork;

  //Roles
  QList<ZStackObject*> m_objectList;

  //Subset of selected objects
  std::set<ZPunctum*> m_selectedPuncta;
  std::set<ZSwcTree*> m_selectedSwcs;
  std::set<Swc_Tree_Node*> m_selectedSwcTreeNodes;
  //std::set<ZStroke2d*> m_selectedStroke;

  QMap<ZStackObject::EType, std::set<ZStackObject*> > m_selectedObjectMap;

  //model-view structure for obj list and edit
  ZSwcObjsModel *m_swcObjsModel;
  ZSwcNodeObjsModel *m_swcNodeObjsModel;
  ZPunctaObjsModel *m_punctaObjsModel;
  ZDocPlayerObjsModel *m_seedObjsModel;

  //Parent frame
  ZStackFrame *m_parentFrame;
  ZStack *m_labelField;

  /* workspaces */
  bool m_isTraceMaskObsolete;
  ZNeuronTracer m_neuronTracer;

  //Meta information
  ZStackFile m_stackSource;
  std::string m_additionalSource;

  //Thread
  ZStackReadThread m_reader;

  //Actions
  //  Undo/Redo
  QUndoStack *m_undoStack;
  QAction *m_undoAction;
  QAction *m_redoAction;

  //  Action map
  QMap<EActionItem, QAction*> m_actionMap;

  //Context menu
  //QMenu *m_swcNodeContextMenu;

  ZSingleSwcNodeActionActivator m_singleSwcNodeActionActivator;

  //obsolete fields
  QList<ZLocsegChain*> m_chainList;
  std::set<ZLocsegChain*> m_selectedChains;
  //ZLocsegChain *m_masterChain;
  QString m_badChainScreen;

  NeuTube::Document::ETag m_tag;
  NeuTube::EImageBackground m_stackBackground;

  ResolutionDialog *m_resDlg;
  ZStackFactory *m_stackFactory;
};

//   template  //
template <class InputIterator>
void ZStackDoc::setPunctumSelected(InputIterator first, InputIterator last, bool select)
{
  QList<ZPunctum*> selected;
  QList<ZPunctum*> deselected;
  for (InputIterator it = first; it != last; ++it) {
    ZPunctum *punctum = *it;
    if (punctum->isSelected() != select) {
      punctum->setSelected(select);
      if (select) {
        m_selectedPuncta.insert(punctum);
        selected.push_back(punctum);
      } else {
        m_selectedPuncta.erase(punctum);
        deselected.push_back(punctum);
      }
    }
  }
  if (!selected.empty() || !deselected.empty()) {
    emit punctaSelectionChanged(selected, deselected);
  }
}

template <class InputIterator>
void ZStackDoc::setSwcSelected(InputIterator first, InputIterator last, bool select)
{
  QList<ZSwcTree*> selected;
  QList<ZSwcTree*> deselected;
  std::vector<Swc_Tree_Node *> tns;
  for (InputIterator it = first; it != last; ++it) {
    ZSwcTree *tree = *it;
    if (tree->isSelected() != select) {
      tree->setSelected(select);
      if (select) {
        m_selectedSwcs.insert(tree);
        selected.push_back(tree);
        // deselect its nodes
        for (std::set<Swc_Tree_Node*>::iterator it = m_selectedSwcTreeNodes.begin();
             it != m_selectedSwcTreeNodes.end(); ++it) {
          if (tree == nodeToSwcTree(*it))
            tns.push_back(*it);
        }
      } else {
        m_selectedSwcs.erase(tree);
        deselected.push_back(tree);
      }
    }
  }
  setSwcTreeNodeSelected(tns.begin(), tns.end(), false);
  if (!selected.empty() || !deselected.empty()) {
    emit swcSelectionChanged(selected, deselected);
  }
}

template <class InputIterator>
void ZStackDoc::setSwcTreeNodeSelected(
    InputIterator first, InputIterator last, bool select)
{
  QList<Swc_Tree_Node*> selected;
  QList<Swc_Tree_Node*> deselected;

  for (InputIterator it = first; it != last; ++it) {
    Swc_Tree_Node *tn = *it;
    if (select) {
      if ((m_selectedSwcTreeNodes.insert(tn)).second) {
        selected.push_back(tn);
        // deselect its tree
        setSwcSelected(nodeToSwcTree(tn), false);
      }
    } else {
      if (m_selectedSwcTreeNodes.erase(tn) > 0) {
        deselected.push_back(tn);
      }
    }
  }
  if (!selected.empty() || !deselected.empty()) {
    emit swcTreeNodeSelectionChanged(selected, deselected);
  }
}

class ZStackDocReader {
public:
  ZStackDocReader();
  ~ZStackDocReader();

  bool readFile(const QString &filePath);
  void clear();
  void loadSwc(const QString &filePath);
  void loadLocsegChain(const QString &filePath);
  void loadStack(const QString &filePath);
  void loadSwcNetwork(const QString &filePath);
  void loadPuncta(const QString &filePath);

  inline ZStack* getStack() const { return m_stack; }
  inline ZSparseStack* getSparseStack() const { return m_sparseStack; }
  inline const ZStackFile& getStackSource() const { return m_stackSource; }
  inline const QList<ZSwcTree*>& getSwcList() const { return m_swcList; }
  inline const QList<ZPunctum*>& getPunctaList() const { return m_punctaList; }
  inline const QList<ZStroke2d*>& getStrokeList() const { return m_strokeList; }
  inline const QList<ZObject3d*>& getObjectList() const { return m_obj3dList; }
  inline const QList<ZLocsegChain*>& getChainList() const { return m_chainList; }
  inline const QList<ZSparseObject*>& getSparseObjectList() const {
    return m_sparseObjectList; }
  inline const ZDocPlayerList& getPlayerList() const {
    return m_playerList;
  }

  bool hasData() const;
  inline const QString& getFileName() const {
    return m_filePath;
  }

  void addPlayer(ZStackObject *obj, NeuTube::EDocumentableType type,
                 ZDocPlayer::TRole role);
  void addObject(ZStackObject *obj, NeuTube::EDocumentableType type,
                 ZDocPlayer::TRole role = ZDocPlayer::ROLE_NONE);

public:
  void addSwcTree(ZSwcTree *tree);
  void addLocsegChain(ZLocsegChain *chain);
  void addPunctum(ZPunctum *p);
  void setStack(ZStack *stack);
  void setStackSource(const ZStackFile &stackFile);
  void addStroke(ZStroke2d *stroke);
  void addSparseObject(ZSparseObject *obj);
  void setSparseStack(ZSparseStack *spStack);
  void addObj3d(ZObject3d *obj);

  /*
  class ZStackObjectWithRole {
  public:
    ZStackObjectWithRole(ZStackObject *obj, NeuTube::EDocumentableType type,
                         ZDocPlayer::TRole role);
  };
*/
private:
  QString m_filePath;

  //Main stack
  ZStack *m_stack;
  ZSparseStack *m_sparseStack;
  ZStackFile m_stackSource;

  //Concrete objects
  //QList<QPair<ZStackObject*,

  QList<ZSwcTree*> m_swcList;
  QList<ZPunctum*> m_punctaList;
  QList<ZStroke2d*> m_strokeList;
  QList<ZObject3d*> m_obj3dList;
  QList<ZSparseObject*> m_sparseObjectList;
  QList<ZLocsegChain*> m_chainList;

  ZDocPlayerList m_playerList;


  //Special object
  ZSwcNetwork *m_swcNetwork;
};
#endif
