#include "TEveBoxSet.h"
#include "Fireworks/Core/interface/FWSimpleProxyBuilderTemplate.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWGeometry.h"
#include "Fireworks/Core/interface/BuilderUtils.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"

class FWCaloClusterProxyBuilder : public FWSimpleProxyBuilderTemplate<reco::CaloCluster>
{
public:
   FWCaloClusterProxyBuilder( void ) {}  
   ~FWCaloClusterProxyBuilder( void ) override {}

   REGISTER_PROXYBUILDER_METHODS();

private:
   FWCaloClusterProxyBuilder( const FWCaloClusterProxyBuilder& ) = delete; 			// stop default
   const FWCaloClusterProxyBuilder& operator=( const FWCaloClusterProxyBuilder& ) = delete; 	// stop default

   using FWSimpleProxyBuilderTemplate<reco::CaloCluster>::build;
   void build( const reco::CaloCluster& iData, unsigned int iIndex, TEveElement& oItemHolder, const FWViewContext* ) override;
};

void
FWCaloClusterProxyBuilder::build( const reco::CaloCluster& iData, unsigned int iIndex, TEveElement& oItemHolder, const FWViewContext* ) 
{
   std::vector<std::pair<DetId, float> > clusterDetIds = iData.hitsAndFractions();

   bool h_hex(false);
   TEveBoxSet *hex_boxset = new TEveBoxSet();
   hex_boxset->UseSingleColor();
   hex_boxset->SetPickable(true);
   hex_boxset->Reset(TEveBoxSet::kBT_Hex, true, 64);
   hex_boxset->SetAntiFlick(true);

   bool h_box(false);
   TEveBoxSet *boxset = new TEveBoxSet();
   boxset->UseSingleColor();
   boxset->SetPickable(true);
   boxset->Reset(TEveBoxSet::kBT_FreeBox, true, 64);
   boxset->SetAntiFlick(true);

   for( std::vector<std::pair<DetId, float> >::iterator it = clusterDetIds.begin(), itEnd = clusterDetIds.end();
        it != itEnd; ++it )
   {
      const uint type = ((it->first >> 28) & 0xF);
      // HGCal
      if (type >= 8 && type <= 10)
      {
         item()->getGeom()->getHGCalRecHits(it->first, hex_boxset, boxset, h_hex, h_box);
      }
      // Not HGCal
      else
      {
         const float* corners = item()->getGeom()->getCorners( it->first );

         if( corners == nullptr ) {
            continue;
         }

         h_box = true;

         std::vector<float> pnts(24);
         fireworks::energyTower3DCorners(corners, (*it).second, pnts);
         boxset->AddBox( &pnts[0]);
      }

   }

   if (h_hex)
   {
      hex_boxset->RefitPlex();
      setupAddElement(hex_boxset, &oItemHolder);
   }

   if (h_box)
   {
      boxset->RefitPlex();
      setupAddElement(boxset, &oItemHolder);
   }
}

REGISTER_FWPROXYBUILDER( FWCaloClusterProxyBuilder, reco::CaloCluster, "Calo Cluster", FWViewType::kISpyBit );
