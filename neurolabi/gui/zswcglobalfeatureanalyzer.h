#ifndef ZSWCGLOBALFEATUREANALYZER_H
#define ZSWCGLOBALFEATUREANALYZER_H

#include <map>
#include <vector>
#include <string>
#include "zswctree.h"

class ZSwcGlobalFeatureAnalyzer
{
public:
  ZSwcGlobalFeatureAnalyzer();

  enum EFeatureSet{
    NGF1, //Number of leaves, number of branch points,
          //box volume, maximum segment length, maximum path length
          //average radius, radius variance, lateral/vertical ratio
          //Average curvature
    NGF2, //NGF1, most spreaded layer, dentritic arbor spread,
          //average brancing angle,
    NGF3,
    UNDEFINED_NGF
  };

public:
  static double computeLateralVerticalRatio(const ZSwcTree &tree);
  static std::vector<double> computeFeatureSet(ZSwcTree &tree,
                                               EFeatureSet setName);
  static int getFeatureNumber(EFeatureSet setName);

  static const std::string& getFeatureName(EFeatureSet setName, int index);

private:
  static std::vector<std::string> m_ngf1FeatureName;
  static std::string m_emptyFeatureName;
};

#endif // ZSWCGLOBALFEATUREANALYZER_H
