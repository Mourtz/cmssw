#include "TEveBoxSet.h"
#include "Fireworks/Core/interface/FWSimpleProxyBuilderTemplate.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWGeometry.h"
#include "Fireworks/Core/interface/BuilderUtils.h"
#include "DataFormats/ParticleFlowReco/interface/HGCalMultiCluster.h"

class FWHGCalMultiClusterProxyBuilder : public FWSimpleProxyBuilderTemplate<reco::HGCalMultiCluster>
{
public:
   FWHGCalMultiClusterProxyBuilder( void ) {}
   ~FWHGCalMultiClusterProxyBuilder( void ) override {}

   REGISTER_PROXYBUILDER_METHODS();

private:
   FWHGCalMultiClusterProxyBuilder( const FWHGCalMultiClusterProxyBuilder& ) = delete; 			// stop default
   const FWHGCalMultiClusterProxyBuilder& operator=( const FWHGCalMultiClusterProxyBuilder& ) = delete; 	// stop default

   using FWSimpleProxyBuilderTemplate<reco::HGCalMultiCluster>::build;
   void build( const reco::HGCalMultiCluster& iData, unsigned int iIndex, TEveElement& oItemHolder, const FWViewContext* ) override;
};

void
FWHGCalMultiClusterProxyBuilder::build(const reco::HGCalMultiCluster &iData, unsigned int iIndex, TEveElement &oItemHolder, const FWViewContext *)
{
   const auto &clusters = iData.clusters();

   bool h_hex(false);
   TEveBoxSet *hex_boxset = new TEveBoxSet("Silicon");
   hex_boxset->UseSingleColor();
   hex_boxset->SetPickable(true);
   hex_boxset->Reset(TEveBoxSet::kBT_Hex, true, 64);
   hex_boxset->SetAntiFlick(kTRUE);

   bool h_box(false);
   TEveBoxSet *boxset = new TEveBoxSet("Scintillator");
   boxset->UseSingleColor();
   boxset->SetPickable(true);
   boxset->Reset(TEveBoxSet::kBT_FreeBox, true, 64);
   boxset->SetAntiFlick(kTRUE);

   for (const auto &c : clusters)
   {
      std::vector<std::pair<DetId, float>> clusterDetIds = c->hitsAndFractions();

      for (std::vector<std::pair<DetId, float>>::iterator it = clusterDetIds.begin(), itEnd = clusterDetIds.end();
           it != itEnd; ++it)
      {
         const float *corners = item()->getGeom()->getCorners(it->first);
         item()->getGeom()->getHGCalRecHits(it->first, hex_boxset, boxset, h_hex, h_box);
      }
   }
   
   if (h_hex)
   {
      hex_boxset->RefitPlex();
      setupAddElement(hex_boxset, &oItemHolder, true);
   }

   if (h_box)
   {
      boxset->RefitPlex();
      setupAddElement(boxset, &oItemHolder, true);
   }
}

REGISTER_FWPROXYBUILDER( FWHGCalMultiClusterProxyBuilder, reco::HGCalMultiCluster, "HGCal MultiCluster", FWViewType::kISpyBit );
