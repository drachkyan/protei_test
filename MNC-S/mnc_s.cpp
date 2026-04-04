#include "network/include/Gateway.h"
#include "ENODE/include/ENodeB.h"

int main(int argc, char* argv[]) {
    std::string pythonPath = "generate_tmsi.py";
    MME mme(pythonPath);
    std::thread mme_t(&MME::run, &mme);

    auto ENode1 = ENodeB(1, -100, 1, 120, mme);
    auto ENode2 = ENodeB(2, 100, 1, 120, mme);

    std::unordered_map<int, ENodeB*> ENodes;
    ENodes[1] = &ENode1;
    ENodes[2] = &ENode2;

    std::thread t1(&ENodeB::run, &ENode1);
    std::thread t2(&ENodeB::run, &ENode2);

    auto gw = Gateway(8085, ENodes);
    gw.run();
    t1.join();
    t2.join();
    mme_t.join();
    return 0;
}