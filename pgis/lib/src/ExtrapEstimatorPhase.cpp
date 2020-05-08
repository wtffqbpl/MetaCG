//
// Created by jp on 23.04.19.
//

#include "ExtrapEstimatorPhase.h"
#include "CgHelper.h"

#include "IPCGEstimatorPhase.h"

#include "EXTRAP_Function.hpp"
#include "EXTRAP_Model.hpp"

#include "spdlog/spdlog.h"

#include <algorithm>
#include <sstream>

namespace pira {

void ExtrapLocalEstimatorPhaseBase::modifyGraph(CgNodePtr mainNode) {
  auto console = spdlog::get("console");
  console->info("Running ExtrapLocalEstimatorPhaseBase::modifyGraph");
  for (const auto n : *graph) {
    auto [shouldInstr, funcRtVal] = shouldInstrument(n);
    if (shouldInstr) {
      n->setState(CgNodeState::INSTRUMENT_WITNESS);
      kernels.push_back({funcRtVal, n});

      if (allNodesToMain) {
        auto nodesToMain = CgHelper::allNodesToMain(n, mainNode);
        console->trace("Node {} has {} nodes on paths to main.", n->getFunctionName(), nodesToMain.size());
        for (const auto ntm : nodesToMain) {
          ntm->setState(CgNodeState::INSTRUMENT_WITNESS);
        }
      }
    }
  }
}

void ExtrapLocalEstimatorPhaseBase::printReport() {
  auto console = spdlog::get("console");

  std::stringstream ss;

  std::sort(std::begin(kernels), std::end(kernels), [&](auto &e1, auto &e2) { return e1.first > e2.first; });
  for (auto [t, k] : kernels) {
    ss << "- " << k->getFunctionName() << "\n  * Time: " << t << "\n";
    ss << "\t" << k->getFilename() << ":" << k->getLineNumber() << "\n\n";
  }

  console->info("$$ Identified Kernels (w/ Runtime) $$\n{}$$ End Kernels $$", ss.str());
}

std::pair<bool, double> ExtrapLocalEstimatorPhaseBase::shouldInstrument(CgNodePtr node) const {
  assert(false && "Base class should not be instantiated.");
  return {false, -1};
}

std::pair<bool, double> ExtrapLocalEstimatorPhaseSingleValueFilter::shouldInstrument(CgNodePtr node) const {
  if (!node->get<PiraTwoData>()->getExtrapModelConnector().isModelSet()) {
    spdlog::get("console")->warn("Model not set for {}", node->getFunctionName());
    return {false, -1};
  }

  auto modelValue = node->get<PiraTwoData>()->getExtrapModelConnector().getEpolator().getExtrapolationValue();

  auto fVal = evalModelWValue(node, modelValue);

  spdlog::get("console")->debug("Model value for {} is calcuated as {}", node->getFunctionName(), fVal);

  return {fVal > threshold, fVal};
}

void ExtrapLocalEstimatorPhaseSingleValueExpander::modifyGraph(CgNodePtr mainNode) {
  std::unordered_map<CgNodePtr, CgNodePtrUnorderedSet> pathsToMain;

  for (const auto n : *graph) {
    auto console = spdlog::get("console");
    console->info("Running ExtrapLocalEstimatorPhaseExpander::modifyGraph on {}", n->getFunctionName());
    auto [shouldInstr, funcRtVal] = shouldInstrument(n);
    if (shouldInstr) {
      n->setState(CgNodeState::INSTRUMENT_WITNESS);
      kernels.push_back({funcRtVal, n});

      if (allNodesToMain) {
        if (pathsToMain.find(n) == pathsToMain.end()) {
          auto nodesToMain = CgHelper::allNodesToMain(n, mainNode, pathsToMain);
          pathsToMain[n] = nodesToMain;
        }
        auto nodesToMain = pathsToMain[n];
        spdlog::get("console")->trace("Found {} nodes to main.", nodesToMain.size());
        for (const auto ntm : nodesToMain) {
          ntm->setState(CgNodeState::INSTRUMENT_WITNESS);
        }
      }

      std::unordered_set<CgNodePtr> totalToMain;
      for (const auto c : n->getChildNodes()) {
        if (!c->get<PiraTwoData>()->getExtrapModelConnector().hasModels()) {
          // We use our heuristic to deepen the instrumentation
          StatementCountEstimatorPhase scep(200);  // TODO we use some threshold value here?
          scep.estimateStatementCount(c);
          if (allNodesToMain) {
            if (pathsToMain.find(c) == pathsToMain.end()) {
              auto cLocal = c;
              auto nodesToMain = CgHelper::allNodesToMain(cLocal, mainNode, pathsToMain);
              pathsToMain[cLocal] = nodesToMain;
              totalToMain.insert(nodesToMain.begin(), nodesToMain.end());
            }
          }
        }
      }
    }
  }
}

}  // namespace pira
