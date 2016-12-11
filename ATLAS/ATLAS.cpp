#include "ATLAS.h"
#include <iostream>
using namespace ATLAS;
#define app make_application
#define abs make_abstraction
#define var make_variable
int main()
{
    //\t.\f.t
    auto tru = abs("t", abs("f", var(1)));
    //\t.\f.f
    auto fls = abs("t", abs("f", var(0)));

    std::cout << tru << "\n";
    std::cout << fls << "\n";

    // \l.\m.\n.lmn
    auto test = abs("l", abs("m", abs("n", app(app(var(2), var(1)), var(0)))));

    std::cout << test << "\n";

    // test tru v w
    std::cout << app(app(app(test, tru), var(3)), var(4)) << "\n";
}