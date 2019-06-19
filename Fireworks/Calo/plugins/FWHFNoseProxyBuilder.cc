#include <iostream>

#include "Fireworks/Core/interface/FWSimpleProxyBuilderTemplate.h"
#include "Fireworks/Core/interface/FWProxyBuilderConfiguration.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWGeometry.h"

#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "SimDataFormats/CaloHit/interface/PCaloHitContainer.h"

#include "TEveBoxSet.h"

class FWHFNoseProxyBuilder : public FWSimpleProxyBuilderTemplate<PCaloHit> {
public:
  FWHFNoseProxyBuilder() {}
  ~FWHFNoseProxyBuilder(void) override {}

  REGISTER_PROXYBUILDER_METHODS();
private:
  FWHFNoseProxyBuilder(const FWHFNoseProxyBuilder &) = delete;
  const FWHFNoseProxyBuilder &operator=(const FWHFNoseProxyBuilder &) = delete;

  void build(const PCaloHit &iData, unsigned int iIndex, TEveElement &oItemHolder, const FWViewContext *) override;
};

void FWHFNoseProxyBuilder::build(const PCaloHit &iData, unsigned int iIndex, TEveElement &oItemHolder, const FWViewContext *) {
    #define detId iData.id()
   
    TEveBoxSet *boxset = new TEveBoxSet();
    boxset->UseSingleColor();
    boxset->SetPickable(true);
    boxset->Reset(TEveBoxSet::kBT_Hex, true, 64);
    boxset->SetAntiFlick(true);

    const float *corners = item()->getGeom()->getCorners(detId);
    const float *parameters = item()->getGeom()->getParameters(detId);
    const float *shapes = item()->getGeom()->getShapePars(detId);

    if (corners == nullptr || parameters == nullptr || shapes == nullptr) {
      return;
    }

    const int total_points = parameters[0];
    const bool isScintillator = (total_points == 4);

    if(isScintillator) {
      std::cout << "Scintillator detected!" << std::endl; 
      // exit(0);
      return;
    }
    
    {
      constexpr int offset = 9;
      float centerX = (corners[6] + corners[6 + offset]) / 2;
      float centerY = (corners[7] + corners[7 + offset]) / 2;
      float radius = fabs(corners[6] - corners[6 + offset]) / 2;
      boxset->AddHex(TEveVector(centerX, centerY, corners[2]), radius, 90.0, shapes[3]);
    
      // std::cout << "new " << (isScintillator ? "Scintillator" : "Silicon") << "( " << total_points << " )" << std::endl;
      // for(int i = 0; i < 24;) std::cout << corners[i++] << ", " << corners[i++] << ", " << corners[i++] << " - " << i/3 <<  std::endl;
      // std::cout << "hex:\t" << centerX << ", " << centerY << ", " << corners[2] << ", " << shapes[3] << std::endl;
    }


    boxset->RefitPlex();
    boxset->CSCTakeAnyParentAsMaster();
    boxset->CSCApplyMainColorToMatchingChildren();
    boxset->CSCApplyMainTransparencyToMatchingChildren();
    boxset->SetMainColor(item()->modelInfo(iIndex).displayProperties().color());
    boxset->SetMainTransparency(item()->defaultDisplayProperties().transparency());
    
    oItemHolder.AddElement(boxset);
}

REGISTER_FWPROXYBUILDER(FWHFNoseProxyBuilder, PCaloHit, "HFNose Hits", FWViewType::kAll3DBits | FWViewType::kAllRPZBits);
