#include "gtest/gtest.h"

#include "LoggerUtil.h"

#include "CallgraphManager.h"
#include "IPCGEstimatorPhase.h"

class IPCGEstimatorPhaseBasic : public ::testing::Test {
 protected:
  void SetUp() override { loggerutil::getLogger(); }
};

TEST_F(IPCGEstimatorPhaseBasic, EmptyCG) {
  Config cfg;
  //CallgraphManager cm(&cfg);
  auto &cm = CallgraphManager::get();
  cm.clear();
  cm.setConfig(&cfg);
  cm.setNoOutput();
  StatementCountEstimatorPhase sce(10);
  cm.registerEstimatorPhase(&sce);
  ASSERT_DEATH(cm.applyRegisteredPhases(), "CallgraphManager: Cannot find main function.");
}

TEST_F(IPCGEstimatorPhaseBasic, OneNodeCG) {
  Config cfg;
  //CallgraphManager cm(&cfg);
  auto &cm = CallgraphManager::get();
  cm.clear();
  cm.setConfig(&cfg);
  cm.setNoOutput();
  auto mainNode = cm.findOrCreateNode("main", 1.4);
  StatementCountEstimatorPhase sce(10);
  sce.setNoReport();
  cm.registerEstimatorPhase(&sce);
  cm.applyRegisteredPhases();
  auto graph = cm.getCallgraph(&cm);
  ASSERT_EQ(mainNode, graph.findMain());
  ASSERT_EQ(0, sce.getNumStatements(mainNode));
}

TEST_F(IPCGEstimatorPhaseBasic, TwoNodeCG) {
  Config cfg;
  //CallgraphManager cm(&cfg);
  auto &cm = CallgraphManager::get();
  cm.clear();
  cm.setConfig(&cfg);
  cm.setNoOutput();
  auto mainNode = cm.findOrCreateNode("main", 1.4);
  auto childNode = cm.findOrCreateNode("child1", 2.0);
  cm.putEdge("main", "main.c", 1, "child1", 100, 1.2, 0, 0);
  StatementCountEstimatorPhase sce(10);
  sce.setNoReport();
  cm.registerEstimatorPhase(&sce);
  cm.applyRegisteredPhases();
  auto graph = cm.getCallgraph(&cm);
  ASSERT_EQ(mainNode, graph.findMain());
  ASSERT_EQ(0, sce.getNumStatements(mainNode));
}

TEST_F(IPCGEstimatorPhaseBasic, OneNodeCGwStmt) {
  Config cfg;
  //CallgraphManager cm(&cfg);
  auto &cm = CallgraphManager::get();
  cm.clear();
  cm.setConfig(&cfg);
  cm.setNoOutput();
  auto mainNode = cm.findOrCreateNode("main", 1.4);
  cm.putNumberOfStatements("main", 12, true);
  StatementCountEstimatorPhase sce(10);
  sce.setNoReport();
  cm.registerEstimatorPhase(&sce);
  cm.applyRegisteredPhases();
  auto graph = cm.getCallgraph(&cm);
  ASSERT_EQ(mainNode, graph.findMain());
  ASSERT_EQ(12, sce.getNumStatements(mainNode));
}

TEST_F(IPCGEstimatorPhaseBasic, TwoNodeCGwStmt) {
  Config cfg;
  //CallgraphManager cm(&cfg);
  auto &cm = CallgraphManager::get();
  cm.clear();
  cm.setConfig(&cfg);
  cm.setNoOutput();
  auto mainNode = cm.findOrCreateNode("main", 1.4);
  cm.putNumberOfStatements("main", 12, true);
  auto childNode = cm.findOrCreateNode("child1", 2.0);
  cm.putNumberOfStatements("child1", 7, true);
  childNode->setReachable();
  cm.putEdge("main", "main.c", 1, "child1", 100, 1.2, 0, 0);

  StatementCountEstimatorPhase sce(10);
  sce.setNoReport();
  cm.registerEstimatorPhase(&sce);
  cm.applyRegisteredPhases();
  auto graph = cm.getCallgraph(&cm);
  ASSERT_EQ(mainNode, graph.findMain());
  ASSERT_EQ(19, sce.getNumStatements(mainNode));
  ASSERT_EQ(7, sce.getNumStatements(childNode));
}

TEST_F(IPCGEstimatorPhaseBasic, ThreeNodeCGwStmt) {
  Config cfg;
  //CallgraphManager cm(&cfg);
  auto &cm = CallgraphManager::get();
  cm.clear();
  cm.setConfig(&cfg);
  cm.setNoOutput();
  auto mainNode = cm.findOrCreateNode("main", 1.4);
  cm.putNumberOfStatements("main", 12, true);

  auto childNode = cm.findOrCreateNode("child1", 2.0);
  cm.putNumberOfStatements("child1", 7, true);
  cm.putEdge("main", "main.c", 1, "child1", 100, 1.2, 0, 0);
  childNode->setReachable();

  auto childNode2 = cm.findOrCreateNode("child2", 23.0);
  cm.putNumberOfStatements("child2", 42, true);
  cm.putEdge("main", "main.c", 1, "child2", 10, 1.9, 0, 0);
  childNode2->setReachable();

  StatementCountEstimatorPhase sce(10);
  sce.setNoReport();
  cm.registerEstimatorPhase(&sce);
  cm.applyRegisteredPhases();
  auto graph = cm.getCallgraph(&cm);
  ASSERT_EQ(mainNode, graph.findMain());
  ASSERT_EQ(61, sce.getNumStatements(mainNode));
  ASSERT_EQ(7, sce.getNumStatements(childNode));
  ASSERT_EQ(42, sce.getNumStatements(childNode2));
}

TEST_F(IPCGEstimatorPhaseBasic, ThreeNodeCycleCGwStmt) {
  Config cfg;
  //CallgraphManager cm(&cfg);
  auto &cm = CallgraphManager::get();
  cm.clear();
  cm.setConfig(&cfg);
  cm.setNoOutput();
  auto mainNode = cm.findOrCreateNode("main", 1.4);
  cm.putNumberOfStatements("main", 12, true);

  auto childNode = cm.findOrCreateNode("child1", 2.0);
  cm.putNumberOfStatements("child1", 7, true);
  cm.putEdge("main", "main.c", 1, "child1", 100, 1.2, 0, 0);
  childNode->setReachable();

  auto childNode2 = cm.findOrCreateNode("child2", 23.0);
  cm.putNumberOfStatements("child2", 42, true);
  cm.putEdge("child1", "main.c", 1, "child2", 10, 1.9, 0, 0);
  cm.putEdge("child2", "main.c", 1, "child1", 10, 1.9, 0, 0);
  childNode2->setReachable();

  StatementCountEstimatorPhase sce(10);
  sce.setNoReport();
  cm.registerEstimatorPhase(&sce);
  cm.applyRegisteredPhases();
  auto graph = cm.getCallgraph(&cm);
  ASSERT_EQ(mainNode, graph.findMain());
  ASSERT_EQ(61, sce.getNumStatements(mainNode));
  ASSERT_EQ(49, sce.getNumStatements(childNode));
  ASSERT_EQ(49, sce.getNumStatements(childNode2));
}

TEST_F(IPCGEstimatorPhaseBasic, FourNodeCGwStmt) {
  Config cfg;
  //CallgraphManager cm(&cfg);
  auto &cm = CallgraphManager::get();
  cm.clear();
  cm.setConfig(&cfg);
  cm.setNoOutput();
  auto mainNode = cm.findOrCreateNode("main", 1.4);
  cm.putNumberOfStatements("main", 12, true);

  auto childNode = cm.findOrCreateNode("child1", 2.0);
  cm.putNumberOfStatements("child1", 7, true);
  cm.putEdge("main", "main.c", 1, "child1", 100, 1.2, 0, 0);
  childNode->setReachable();

  auto childNode2 = cm.findOrCreateNode("child2", 23.0);
  cm.putNumberOfStatements("child2", 42, true);
  cm.putEdge("main", "main.c", 1, "child2", 10, 1.9, 0, 0);
  childNode2->setReachable();

  auto childNode3 = cm.findOrCreateNode("child3", 14.0);
  cm.putNumberOfStatements("child3", 22, true);
  cm.putEdge("child1", "main.c", 1, "child3", 10, 1.9, 0, 0);
  childNode3->setReachable();

  StatementCountEstimatorPhase sce(10);
  sce.setNoReport();
  cm.registerEstimatorPhase(&sce);
  cm.applyRegisteredPhases();
  auto graph = cm.getCallgraph(&cm);
  ASSERT_EQ(mainNode, graph.findMain());
  ASSERT_EQ(83, sce.getNumStatements(mainNode));
  ASSERT_EQ(29, sce.getNumStatements(childNode));
  ASSERT_EQ(42, sce.getNumStatements(childNode2));
  ASSERT_EQ(22, sce.getNumStatements(childNode3));
}

TEST_F(IPCGEstimatorPhaseBasic, FourNodeDiamondCGwStmt) {
  Config cfg;
  //CallgraphManager cm(&cfg);
  auto &cm = CallgraphManager::get();
  cm.clear();
  cm.setConfig(&cfg);
  cm.setNoOutput();
  auto mainNode = cm.findOrCreateNode("main", 1.4);
  cm.putNumberOfStatements("main", 12, true);

  auto childNode = cm.findOrCreateNode("child1", 2.0);
  cm.putNumberOfStatements("child1", 7, true);
  cm.putEdge("main", "main.c", 1, "child1", 100, 1.2, 0, 0);
  childNode->setReachable();

  auto childNode2 = cm.findOrCreateNode("child2", 23.0);
  cm.putNumberOfStatements("child2", 42, true);
  cm.putEdge("main", "main.c", 1, "child2", 10, 1.9, 0, 0);
  childNode2->setReachable();

  auto childNode3 = cm.findOrCreateNode("child3", 14.0);
  cm.putNumberOfStatements("child3", 22, true);
  cm.putEdge("child1", "main.c", 1, "child3", 10, 1.9, 0, 0);
  cm.putEdge("child2", "main.c", 1, "child3", 10, 1.9, 0, 0);
  childNode3->setReachable();

  StatementCountEstimatorPhase sce(10);
  sce.setNoReport();
  cm.registerEstimatorPhase(&sce);
  cm.applyRegisteredPhases();
  auto graph = cm.getCallgraph(&cm);
  ASSERT_EQ(mainNode, graph.findMain());
  ASSERT_EQ(105, sce.getNumStatements(mainNode));
  ASSERT_EQ(29, sce.getNumStatements(childNode));
  ASSERT_EQ(64, sce.getNumStatements(childNode2));
  ASSERT_EQ(22, sce.getNumStatements(childNode3));
}

/*
   o
  / \
  o  o
  \  /
   o
   |
   o
*/
TEST_F(IPCGEstimatorPhaseBasic, FiveNodeDiamondCGwStmt) {
  Config cfg;
  //CallgraphManager cm(&cfg);
  auto &cm = CallgraphManager::get();
  cm.clear();
  cm.setConfig(&cfg);
  cm.setNoOutput();
  auto mainNode = cm.findOrCreateNode("main", 1.4);
  cm.putNumberOfStatements("main", 12 ,true);

  auto childNode = cm.findOrCreateNode("child1", 2.0);
  cm.putNumberOfStatements("child1", 7, true);
  cm.putEdge("main", "main.c", 1, "child1", 100, 1.2, 0, 0);
  childNode->setReachable();

  auto childNode2 = cm.findOrCreateNode("child2", 23.0);
  cm.putNumberOfStatements("child2", 42, true);
  cm.putEdge("main", "main.c", 1, "child2", 10, 1.9, 0, 0);
  childNode2->setReachable();

  auto childNode3 = cm.findOrCreateNode("child3", 14.0);
  cm.putNumberOfStatements("child3", 22, true);
  cm.putEdge("child1", "main.c", 1, "child3", 10, 1.9, 0, 0);
  cm.putEdge("child2", "main.c", 1, "child3", 10, 1.9, 0, 0);
  childNode3->setReachable();

  auto childNode4 = cm.findOrCreateNode("child4", 11.2);
  cm.putNumberOfStatements("child4", 4, true);
  cm.putEdge("child3", "main.c", 1, "child4", 12, 1.4, 0, 0);
  childNode4->setReachable();

  StatementCountEstimatorPhase sce(10);
  sce.setNoReport();
  cm.registerEstimatorPhase(&sce);
  cm.applyRegisteredPhases();
  auto graph = cm.getCallgraph(&cm);
  ASSERT_EQ(mainNode, graph.findMain());
  ASSERT_EQ(113, sce.getNumStatements(mainNode));
  ASSERT_EQ(33, sce.getNumStatements(childNode));
  ASSERT_EQ(68, sce.getNumStatements(childNode2));
  ASSERT_EQ(26, sce.getNumStatements(childNode3));
  ASSERT_EQ(4, sce.getNumStatements(childNode4));
}

class IPCGEstimatorPhaseTest : public ::testing::Test {
 protected:
  IPCGEstimatorPhaseTest() {}
  /* Construct a call graph that has the following structure
   o
  / \ \
  o  o o
  \  / |
   o   o
   |   |
   o   o
*/
  void createCalleeNode(std::string name, std::string caller, int numStatements, double runtime, int numCalls) {
    auto &cm = CallgraphManager::get();
    auto nodeCaller = cm.findOrCreateNode(caller, runtime);
    cm.putNumberOfStatements(name, numStatements, true);
    cm.putEdge(caller, "main.c", 1, name, numCalls, runtime, 0, 0);
  }

  void SetUp() override {
    loggerutil::getLogger();
  auto &cm = CallgraphManager::get();
  cm.clear();
  cm.setConfig(new Config());
    cm.setNoOutput();
    auto mainNode = cm.findOrCreateNode("main", 1.4);
    cm.putNumberOfStatements("main", 12, true);

    createCalleeNode("child1", "main", 7, 2.0, 100);
    createCalleeNode("child2", "main", 42, 23.0, 10);
    createCalleeNode("child3", "child1", 22, 14.0, 10);
    createCalleeNode("child3", "child2", 22, 14.0, 10);
    createCalleeNode("child4", "child3", 4, 11.2, 12);
    createCalleeNode("child5", "main", 15, 3.0, 12);
    createCalleeNode("child6", "child5", 2, 0.8, 1000);
    createCalleeNode("child7", "child6", 1, 20.2, 1002);
  }

};

TEST_F(IPCGEstimatorPhaseTest, ValidateBasics) {
  auto &cm = CallgraphManager::get();
  auto graph = cm.getCallgraph(&cm);
  ASSERT_NE(nullptr, graph.findMain());

  auto mainNode = graph.findMain();
  EXPECT_EQ(3, mainNode->getChildNodes().size());
  auto childNodes = mainNode->getChildNodes();
  auto nodeIter = childNodes.begin();
  EXPECT_EQ("child1", (*(nodeIter))->getFunctionName());
  EXPECT_EQ("child2", (*(++nodeIter))->getFunctionName());
  EXPECT_EQ("child5", (*(++nodeIter))->getFunctionName());
}

// TODO: THis seems more like a test for CallgraphManager?
TEST_F(IPCGEstimatorPhaseTest, InitiallyNoneReachable) {
  auto &cm = CallgraphManager::get();
  ASSERT_NE(0, cm.size());
  int count = 0;
  for (const auto n : cm) {
    EXPECT_EQ(false, n->isReachable());
    count++;
  }
  EXPECT_EQ(count, cm.size());
}

TEST_F(IPCGEstimatorPhaseTest, ApplyPhaseFinalizesGraph) {
  auto &cm = CallgraphManager::get();
  auto nep = std::make_unique<NopEstimatorPhase>();
  ASSERT_EQ(false, nep->didRun);
  cm.registerEstimatorPhase(nep.get());
  ASSERT_EQ(false, nep->didRun);
  cm.applyRegisteredPhases();
  ASSERT_EQ(true, nep->didRun);
  ASSERT_NE(0, cm.size());
  for (const auto n : cm) {
    EXPECT_EQ(true, n->isReachable());
  }
}
