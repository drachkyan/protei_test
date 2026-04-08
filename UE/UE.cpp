#include "include/Menu/Menu.h"
#include "include/Settings/ArgParser.h"


int main(const int argc, char* argv[]) {
    ArgParser p;
    p.parse(argc, argv);
    auto settings = p.build();

    Menu menu(settings);
    menu.run();
    return 0;
}
