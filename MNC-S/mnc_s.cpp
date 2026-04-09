#include "network/include/UEconnection.h"
#include "ENODE/include/ENodeB.h"
#include "Utils/Parser.h"
#include "Utils/ArgParcerMNC.h"
#include "Utils/utils.h"

int main(int argc, char* argv[]) {
    ArgParserMNC argParser;
    argParser.parse(argc, argv);
    if (!argParser.isValid()) {
        throw std::runtime_error("Неверные аргументы командной строки");
        return 1;
    }
    const std::string enodePath = argParser.getBasesJsonPath();
    const std::string epcPath   = argParser.getEpcJsonPath();
    int port = argParser.getPort();

    EPCConfig epc = parseEPC(epcPath);
    auto enodeConfigs = parseBases(enodePath);

    setupLogger(epc.xdr_provider, epc.xdr_path, epc.xdr_format);

    std::string pythonPath = epc.tmsi_script;
    MME mme(pythonPath, epc.ttl_sms, epc.hlr_path);
    std::thread mme_t(&MME::run, &mme);

    std::unordered_map<int, ENodeB*> ENodes;


    UEconnection gw(port, ENodes);

    std::vector<std::unique_ptr<ENodeB>> enodeObjects;
    std::vector<std::thread> threads;

    for (const auto& cfg : enodeConfigs) {
        enodeObjects.push_back(std::make_unique<ENodeB>(
            cfg.id,
            cfg.x,
            cfg.power,
            cfg.radius,
            mme,
            mme.getENodes(),
            gw
        ));

        ENodes[cfg.id] = enodeObjects.back().get();
    }

    for (auto& enode : enodeObjects) {
        threads.emplace_back(&ENodeB::run, enode.get());
    }

    gw.run();

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    if (mme_t.joinable()) mme_t.join();

    return 0;
}
