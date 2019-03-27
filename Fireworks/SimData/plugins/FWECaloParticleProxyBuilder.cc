#include "Fireworks/Core/interface/FWSimpleProxyBuilderTemplate.h"
#include "Fireworks/Core/interface/FWProxyBuilderConfiguration.h"
#include "Fireworks/Core/interface/Context.h"
#include "Fireworks/Core/interface/FWGeometry.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

#include "TEveBoxSet.h"

class FWECaloParticleProxyBuilder : public FWSimpleProxyBuilderTemplate<CaloParticle>
{
 public:
   FWECaloParticleProxyBuilder(void) {}
   ~FWECaloParticleProxyBuilder(void) override {}

   REGISTER_PROXYBUILDER_METHODS();

 private:
   // Disable default copy constructor
   FWECaloParticleProxyBuilder(const FWECaloParticleProxyBuilder &) = delete;
   // Disable default assignment operator
   const FWECaloParticleProxyBuilder &operator=(const FWECaloParticleProxyBuilder &) = delete;

   void setItem(const FWEventItem *iItem) override;
   void build(const FWEventItem *iItem, TEveElementList *product, const FWViewContext *vc) override;
   void build(const CaloParticle &iData, unsigned int iIndex, TEveElement &oItemHolder, const FWViewContext *) override;

   static constexpr uint8_t gradient_steps = 9;
#define C(x) static_cast<uint8_t>(x)
   static constexpr uint8_t gradient[3][gradient_steps] = {
       {C(0.2082 * 255), C(0.0592 * 255), C(0.0780 * 255), C(0.0232 * 255), C(0.1802 * 255), C(0.5301 * 255), C(0.8186 * 255), C(0.9956 * 255), C(0.9764 * 255)},
       {C(0.1664 * 255), C(0.3599 * 255), C(0.5041 * 255), C(0.6419 * 255), C(0.7178 * 255), C(0.7492 * 255), C(0.7328 * 255), C(0.7862 * 255), C(0.9832 * 255)},
       {C(0.5293 * 255), C(0.8684 * 255), C(0.8385 * 255), C(0.7914 * 255), C(0.6425 * 255), C(0.4662 * 255), C(0.3499 * 255), C(0.1968 * 255), C(0.0539 * 255)}};
#undef C
   std::map<DetId, const EcalRecHit *> hitmap;

   double saturation_energy;
   bool heatmap;
};

void FWECaloParticleProxyBuilder::setItem(const FWEventItem *iItem)
{
   FWProxyBuilderBase::setItem(iItem);
   if (iItem)
   {
      iItem->getConfig()->keepEntries(true);
      iItem->getConfig()->assertParam("EnergyCutOff", 0.5, 0.2, 5.0);
      iItem->getConfig()->assertParam("Heatmap", true);
   }
}

void FWECaloParticleProxyBuilder::build(const FWEventItem *iItem, TEveElementList *product, const FWViewContext *vc)
{
   if (item()->getConfig()->template value<bool>("Heatmap"))
   {
      edm::Handle<EcalRecHitCollection> recHitHandleEB;
      edm::Handle<EcalRecHitCollection> recHitHandleEE;
      edm::Handle<EcalRecHitCollection> recHitHandleES;

      const edm::EventBase *event = iItem->getEvent();

      event->getByLabel(edm::InputTag("reducedEcalRecHitsEB"), recHitHandleEB);
      event->getByLabel(edm::InputTag("reducedEcalRecHitsEE"), recHitHandleEE);
      event->getByLabel(edm::InputTag("reducedEcalRecHitsES"), recHitHandleES);

      hitmap.clear();

      if (recHitHandleEB.isValid())
      {
         const auto &rechitsEB = *recHitHandleEB;

         for (unsigned int i = 0; i < rechitsEB.size(); ++i)
         {
            hitmap[rechitsEB[i].detid().rawId()] = &rechitsEB[i];
         }
      }

      if (recHitHandleEE.isValid())
      {
         const auto &rechitsEE = *recHitHandleEE;

         for (unsigned int i = 0; i < rechitsEE.size(); ++i)
         {
            hitmap[rechitsEE[i].detid().rawId()] = &rechitsEE[i];
         }
      }

      if (recHitHandleES.isValid())
      {
         const auto &rechitsES = *recHitHandleES;

         for (unsigned int i = 0; i < rechitsES.size(); ++i)
         {
            hitmap[rechitsES[i].detid().rawId()] = &rechitsES[i];
         }
      }
   }

   saturation_energy = item()->getConfig()->value<double>("EnergyCutOff");
   heatmap = item()->getConfig()->value<bool>("Heatmap");

   FWSimpleProxyBuilder::build(iItem, product, vc);
}

void FWECaloParticleProxyBuilder::build(const CaloParticle &iData, unsigned int iIndex, TEveElement &oItemHolder, const FWViewContext *)
{
   TEveBoxSet *boxset = new TEveBoxSet();
   if (!heatmap)
      boxset->UseSingleColor();
   boxset->SetPickable(true);
   boxset->Reset(TEveBoxSet::kBT_FreeBox, true, 64);
   boxset->SetAntiFlick(true);

   for (const auto &c : iData.simClusters())
   {
      for (const auto &it : (*c).hits_and_fractions())
      {
         if (DetId(it.first).det() != DetId::Detector::Ecal)
         {
            std::cerr << "this proxy should be used only for ECAL";
            return;
         }

         if (heatmap && hitmap.find(it.first) == hitmap.end())
            continue;

         const float *corners = item()->getGeom()->getCorners(it.first);
         if (corners == nullptr)
            continue;

         boxset->AddBox(corners);
         if (heatmap)
         {
            const uint8_t colorFactor = gradient_steps * (fmin(hitmap[it.first]->energy() / saturation_energy, 1.0f));
            boxset->DigitColor(gradient[0][colorFactor], gradient[1][colorFactor], gradient[2][colorFactor]);
         }
      }
   }

   boxset->RefitPlex();

   boxset->CSCTakeAnyParentAsMaster();
   if (!heatmap)
   {
      boxset->CSCApplyMainColorToMatchingChildren();
      boxset->CSCApplyMainTransparencyToMatchingChildren();
      boxset->SetMainColor(item()->modelInfo(iIndex).displayProperties().color());
      boxset->SetMainTransparency(item()->defaultDisplayProperties().transparency());
   }
   oItemHolder.AddElement(boxset);
}

REGISTER_FWPROXYBUILDER(FWECaloParticleProxyBuilder, CaloParticle, "ECaloParticle", FWViewType::kAll3DBits | FWViewType::kAllRPZBits);
