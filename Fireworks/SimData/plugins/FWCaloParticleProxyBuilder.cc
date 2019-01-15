/*
 *  FWCaloParticleProxyBuilder.cc
 *  FWorks
 *
 *  Created by Marco Rovere 13/09/2018
 *
 */

#include "Fireworks/Core/interface/FWSimpleProxyBuilderTemplate.h"
#include "Fireworks/Core/interface/Context.h"
#include "Fireworks/Core/interface/FWEventItem.h"
#include "Fireworks/Core/interface/FWGeometry.h"
#include "Fireworks/Core/interface/BuilderUtils.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"

#include "FWCore/Common/interface/EventBase.h"

#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "DataFormats/DetId/interface/DetId.h"

#include "Fireworks/Core/interface/FWProxyBuilderConfiguration.h"

#include "TEveBoxSet.h"
#include "TEveCompound.h"

class FWCaloParticleProxyBuilder : public FWSimpleProxyBuilderTemplate<CaloParticle>
{
 public:
   FWCaloParticleProxyBuilder(void) {}
   ~FWCaloParticleProxyBuilder(void) override {}

   REGISTER_PROXYBUILDER_METHODS();

 private:
   // Disable default copy constructor
   FWCaloParticleProxyBuilder(const FWCaloParticleProxyBuilder &) = delete;
   // Disable default assignment operator
   const FWCaloParticleProxyBuilder &operator=(const FWCaloParticleProxyBuilder &) = delete;

   // using FWSimpleProxyBuilderTemplate<CaloParticle>::build;
   void build(const CaloParticle &iData, unsigned int iIndex, TEveElement &oItemHolder, const FWViewContext *) override;
};

void FWCaloParticleProxyBuilder::build(const CaloParticle &iData, unsigned int iIndex, TEveElement &oItemHolder, const FWViewContext *)
{
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

   for (const auto &c : iData.simClusters())
   {
      for (const auto &it : (*c).hits_and_fractions())
      {
         item()->getGeom()->getHGCalRecHits(it.first, hex_boxset, boxset, h_hex, h_box);
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

REGISTER_FWPROXYBUILDER(FWCaloParticleProxyBuilder, CaloParticle, "CaloParticle", FWViewType::kAll3DBits | FWViewType::kAllRPZBits);