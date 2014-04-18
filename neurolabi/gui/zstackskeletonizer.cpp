#include "zstackskeletonizer.h"

#include <iostream>
#include "c_stack.h"
#include "zswctree.h"
#include "tz_stack_lib.h"
#include "tz_stack_bwmorph.h"
#include "tz_sp_grow.h"
#include "tz_stack_objlabel.h"
#include "zspgrowparser.h"
#include "tz_stack_stat.h"
#include "tz_stack_math.h"
#include "zswcforest.h"
#include "swctreenode.h"
#include "zswcgenerator.h"
#include "tz_error.h"
#include "zstack.hxx"
#include "zobject3dscan.h"
#include "zerror.h"

using namespace std;

ZStackSkeletonizer::ZStackSkeletonizer() : m_lengthThreshold(15.0),
  m_distanceThreshold(-1.0), m_rebase(false), m_interpolating(false),
  m_removingBorder(false), m_minObjSize(0), m_keepingSingleObject(false),
  m_level(0), m_connectingBranch(true)
{
  for (int i = 0; i < 3; ++i) {
    m_resolution[i] = 1.0;
    m_downsampleInterval[i] = 0;
  }
}

ZSwcTree* ZStackSkeletonizer::makeSkeleton(const ZStack &stack)
{
  ZSwcTree *tree = makeSkeleton(stack.c_stack());
  if (tree != NULL) {
    const ZPoint &pt = stack.getOffset();
    tree->translate(pt.x(), pt.y(), pt.z());
  }

  return tree;
}

static double AdjustedDistanceWeight(double v)
{
  return dmax2(0.1, sqrt(v) - 0.5);
}

ZSwcTree* ZStackSkeletonizer::makeSkeleton(const ZObject3dScan &obj)
{
  ZSwcTree *tree = NULL;
  if (!obj.isEmpty()) {
    ZObject3dScan newObj = obj;
    newObj.downsampleMax(m_downsampleInterval[0],
                         m_downsampleInterval[1], m_downsampleInterval[2]);
    ZStack *stack = newObj.toStackObject();
    tree = makeSkeletonWithoutDs(stack->c_stack());
  }

  return tree;
}

ZSwcTree* ZStackSkeletonizer::makeSkeletonWithoutDs(Stack *stackData)
{
  if (m_level > 0) {
    Stack_Level_Mask(stackData, m_level);
  }
  advanceProgress(0.05);

  double maxMaskIntensity = Stack_Max(stackData, NULL);
  if (maxMaskIntensity > 1.0) {
    Stack_Binarize(stackData);
  } else if (maxMaskIntensity == 0.0) {
    cout << "Not a binary image. No skeleton generated." << endl;
    return NULL;
  }

  if (C_Stack::kind(stackData) != GREY) {
    Translate_Stack(stackData, GREY, 1);
  }

  advanceProgress(0.05);

  Stack *out = stackData;

  if (m_removingBorder) {
    cout << "Remove 1-pixel gaps ..." << endl;
    Stack_Not(out, out);
    Stack* solid = Stack_Majority_Filter(out, NULL, 8);
    Kill_Stack(out);

    Stack_Not(solid, solid);
    stackData = solid;
  }
  advanceProgress(0.05);

  if (m_interpolating) {
    cout << "Interpolating ..." << endl;
    out = Stack_Bwinterp(stackData, NULL);
    Kill_Stack(stackData);
    stackData = out;
  }
  advanceProgress(0.05);

#ifdef _DEBUG_
    C_Stack::write("/groups/flyem/home/zhaot/Work/neutube/neurolabi/data/test.tif", stackData);
#endif

  cout << "Label objects ...\n" << endl;
  int nobj = Stack_Label_Large_Objects_N(
        stackData, NULL, 1, 2, m_minObjSize, 26);
  //int nobj = Stack_Label_Objects_N(stackData, NULL, 1, 2, 26);
  if (nobj == 0) {
    cout << "No object found in the image. No skeleton generated." << endl;
    C_Stack::kill(stackData);
    return NULL;
  }

  advanceProgress(0.1);

  if (nobj > 65533) {
    cout << "Too many objects ( > 65533). No skeleton generated." << endl;
    C_Stack::kill(stackData);
    return NULL;
  }

  Swc_Tree *tree = New_Swc_Tree();
  tree->root = Make_Virtual_Swc_Tree_Node();

  size_t voxelNumber = C_Stack::voxelNumber(stackData);

  double step = 0.6 / nobj;
  for (int objIndex = 0; objIndex < nobj; objIndex++) {
    cout << "Skeletonizing object " << objIndex + 1 << "/" << nobj << endl;
    Swc_Tree *subtree = New_Swc_Tree();
    subtree->root = Make_Virtual_Swc_Tree_Node();

    Stack *objstack = Copy_Stack(stackData);
    size_t objSize = Stack_Level_Mask(objstack, 3 + objIndex);

    Translate_Stack(objstack, GREY, 1);

    int objectOffset[3];
    Stack *croppedObjStack = C_Stack::boundCrop(objstack, 0, objectOffset);

    /*
    if (C_Stack::kind(objstack) == GREY16) {
      Translate_Stack(objstack, GREY, 1);
    }
    */

    if (C_Stack::kind(croppedObjStack) == GREY16) {
      Translate_Stack(croppedObjStack, GREY, 1);
    }

    if (objSize == 1) {
      if (m_keepingSingleObject || m_lengthThreshold <= 1) {
        int x = 0;
        int y = 0;
        int z = 0;
        for (size_t offset = 0; offset < voxelNumber; ++offset) {
          if (croppedObjStack->array[offset] == 1) {
            C_Stack::indexToCoord(offset, C_Stack::width(croppedObjStack),
                                  C_Stack::height(croppedObjStack), &x, &y, &z);
            break;
          }
        }
        Swc_Tree_Node *tn = SwcTreeNode::makePointer(
              x + objectOffset[0], y + objectOffset[1],
            z + objectOffset[2], 1.0);
        SwcTreeNode::setParent(tn, subtree->root);
      }
    } else {
      cout << "Build distance map ..." << endl;
      Stack *tmpdist = Stack_Bwdist_L_U16P(croppedObjStack, NULL, 0);

      cout << "Shortest path grow ..." << endl;
      Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
      Sp_Grow_Workspace_Enable_Eucdist_Buffer(sgw);
      sgw->resolution[0] = m_resolution[0];
      sgw->resolution[1] = m_resolution[1];
      sgw->resolution[2] = m_resolution[2];
      sgw->wf = Stack_Voxel_Weight_I;
      size_t max_index;
      Stack_Max(tmpdist, &max_index);

      Stack *mask = Make_Stack(GREY, Stack_Width(tmpdist),
                               Stack_Height(tmpdist),
                               Stack_Depth(tmpdist));
      Zero_Stack(mask);

      size_t nvoxel = Stack_Voxel_Number(croppedObjStack);
      size_t i;
      for (i = 0; i < nvoxel; i++) {
        if (croppedObjStack->array[i] == 0) {
          mask->array[i] = SP_GROW_BARRIER;
        }
      }

      mask->array[max_index] = SP_GROW_SOURCE;
      Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
      Stack_Sp_Grow(tmpdist, NULL, 0, NULL, 0, sgw);

      ZSpGrowParser parser(sgw);

      if (m_rebase) {
        cout << "Replacing start point ..." << endl;
        ZVoxelArray path = parser.extractLongestPath(NULL, false);
        for (i = 0; i < nvoxel; i++) {
          if (mask->array[i] != SP_GROW_BARRIER) {
            mask->array[i] = 0;
          }
        }

        ssize_t seedIndex = path[0].toIndex(
              C_Stack::width(mask), C_Stack::height(mask),
              C_Stack::depth(mask));
        mask->array[seedIndex] = SP_GROW_SOURCE;
        Stack_Sp_Grow(tmpdist, NULL, 0, NULL, 0, sgw);
      }

      double lengthThreshold = m_lengthThreshold;

      std::vector<ZVoxelArray> pathArray =
          parser.extractAllPath(lengthThreshold, tmpdist);

      if (pathArray.empty() && m_keepingSingleObject) {
        pathArray.push_back(parser.extractLongestPath(NULL, false));
      }

      //Make a subtree from a single object
      for (std::vector<ZVoxelArray>::iterator iter = pathArray.begin();
           iter != pathArray.end(); ++iter) {
        (*iter).sample(tmpdist, AdjustedDistanceWeight);
        //(*iter).labelStack(stackData, 255.0);

        ZSwcTree *branchWrapper =
            ZSwcGenerator::createSwc(*iter, ZSwcGenerator::REGION_SAMPLING);
        Swc_Tree *branch  = branchWrapper->data();
        branchWrapper->setData(branch, ZSwcTree::LEAVE_ALONE);
        branchWrapper->translate(objectOffset[0], objectOffset[1],
            objectOffset[2]);

        //branch = (*iter).toSwcTree();
        if (SwcTreeNode::firstChild(branch->root) != NULL) {
          if (SwcTreeNode::radius(branch->root) * 2.0 <
              SwcTreeNode::radius(SwcTreeNode::firstChild(branch->root))) {
            Swc_Tree_Node *oldRoot = branch->root;
            Swc_Tree_Node *newRoot = SwcTreeNode::firstChild(branch->root);
            SwcTreeNode::detachParent(newRoot);
            branch->root = newRoot;
            SwcTreeNode::kill(oldRoot);
          }
        }

        Swc_Tree_Node *leaf = SwcTreeNode::firstChild(branch->root);
        if (leaf != NULL) {
          while (SwcTreeNode::firstChild(leaf) != NULL) {
            leaf = SwcTreeNode::firstChild(leaf);
          }

          if (SwcTreeNode::radius(leaf) * 2.0 <
              SwcTreeNode::radius(SwcTreeNode::parent(leaf))) {
            SwcTreeNode::detachParent(leaf);
            SwcTreeNode::kill(leaf);
          }
        }

        Swc_Tree_Node *tn = Swc_Tree_Connect_Branch(subtree, branch->root);

#ifdef _DEBUG_
        if (SwcTreeNode::length(branch->root) > 100) {
          std::cout << "Potential bug." << std::endl;
        }
#endif

        if (SwcTreeNode::isRegular(SwcTreeNode::parent(tn))) {
          if (SwcTreeNode::hasOverlap(tn, SwcTreeNode::parent(tn))) {
            SwcTreeNode::mergeToParent(tn);
          }
        }

        branch->root = NULL;
        Kill_Swc_Tree(branch);

#ifdef _DEBUG_
        Swc_Tree_Iterator_Start(subtree, SWC_TREE_ITERATOR_DEPTH_FIRST, false);
        Swc_Tree_Node *tmptn = NULL;
        while ((tmptn = Swc_Tree_Next(subtree)) != NULL) {
          if (!SwcTreeNode::isRoot(tmptn)) {
            TZ_ASSERT(SwcTreeNode::length(tmptn) > 0.0, "duplicating nodes");
          }
        }

#endif
      }

      Kill_Stack(mask);
      Kill_Stack(tmpdist);
    }

    C_Stack::kill(croppedObjStack);
    C_Stack::kill(objstack);

    if (Swc_Tree_Regular_Root(subtree) != NULL) {
      cout << Swc_Tree_Node_Fsize(subtree->root) - 1<< " nodes added" << endl;
      Swc_Tree_Merge(tree, subtree);
    }

    Kill_Swc_Tree(subtree);

    advanceProgress(step);
  }

  ZSwcTree *wholeTree = NULL;

  if (Swc_Tree_Regular_Root(tree) != NULL) {
    wholeTree = new ZSwcTree;
    wholeTree->setData(tree);

    if (m_downsampleInterval[0] > 0 || m_downsampleInterval[1] > 0 ||
        m_downsampleInterval[2] > 0) {
      wholeTree->rescale(m_downsampleInterval[0] + 1,
          m_downsampleInterval[1] + 1, m_downsampleInterval[2] + 1);
    }

    wholeTree->resortId();
    if (m_connectingBranch) {
      reconnect(wholeTree);
    }
  }

  advanceProgress(0.05);
  endProgress();

  return wholeTree;
}

ZSwcTree* ZStackSkeletonizer::makeSkeleton(const Stack *stack)
{
  Stack *stackData = Downsample_Stack_Max(stack, m_downsampleInterval[0],
      m_downsampleInterval[1], m_downsampleInterval[2], NULL);
  return makeSkeletonWithoutDs(stackData);
}

void ZStackSkeletonizer::reconnect(ZSwcTree *tree)
{
  if (tree->regularRootNumber() > 1) {
    double z_scale = m_resolution[2];
    if (m_resolution[0] != 1.0) {
      z_scale /= m_resolution[0];
    }
    Swc_Tree_Reconnect(tree->data(), z_scale,
                       m_distanceThreshold / m_resolution[0]);
    tree->resortId();
  }
}

void ZStackSkeletonizer::configure(const ZJsonObject &config)
{
  ZJsonArray array(const_cast<json_t*>(config["downsampleInterval"]), false);
  std::vector<int> interval = array.toIntegerArray();
  if (interval.size() == 3) {
    setDownsampleInterval(interval[0], interval[1], interval[2]);
  } else {
    RECORD_WARNING_UNCOND("Invalid downsample parameter");
  }

  const json_t *value = config["minimalLength"];
  if (ZJsonParser::isNumber(value)) {
    setLengthThreshold(ZJsonParser::numberValue(value));
  }

  value = config["maximalDistance"];
  if (ZJsonParser::isNumber(value)) {
    setDistanceThreshold(ZJsonParser::numberValue(value));
  }

  value = config["keepingSingleObject"];
  if (ZJsonParser::isBoolean(value)) {
    setKeepingSingleObject(ZJsonParser::booleanValue(value));
  }

  value = config["rebase"];
  if (ZJsonParser::isBoolean(value)) {
    setRebase(ZJsonParser::booleanValue(value));
  }
}

void ZStackSkeletonizer::print() const
{
  std::cout << "Minimal length: " << m_lengthThreshold << std::endl;
  std::cout << "Maximal distance: " << m_distanceThreshold << std::endl;
  std::cout << "Rebase: " << m_rebase << std::endl;
  std::cout << "Intepolate: " << m_interpolating << std::endl;
  std::cout << "Remove border: " << m_removingBorder << std::endl;
  std::cout << "Minimal object size: " << m_minObjSize << std::endl;
  std::cout << "Keep short object: " << m_keepingSingleObject << std::endl;
  std::cout << "Level: " << m_level << std::endl;
  std::cout << "Connect branch: " << m_connectingBranch << std::endl;
  std::cout << "Resolution: " << "(" << m_resolution[0] << ", "
            << m_resolution[1] << ", " << m_resolution[2] << ")" << std::endl;
  std::cout << "Downsample interval: (" << m_downsampleInterval[0] << ", "
            << m_downsampleInterval[1] << ", " << m_downsampleInterval[2]
            << ")" << std::endl;
}
