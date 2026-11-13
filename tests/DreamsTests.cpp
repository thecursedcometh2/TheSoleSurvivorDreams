#include "Dreams.h"
#include <iostream>
#include <set>
#include <string>
#include <limits>
#include <cstdlib>
void check(bool ok) { if (!ok) { std::cerr << "Failed check\n"; std::exit(1); } }
int main() {
    check(dreams::catalog.size() == 28);
    unsigned hits = 0;
    for(unsigned roll = 0; roll < 100; ++roll) hits += dreams::select(roll, 0).has_value();
    check(hits == 15); // exact boundary, not a statistical approximation
    check(dreams::select(14, 27) == 27);
    check(!dreams::select(15, 0));
    check(!dreams::select(0, 28));
    std::set<std::string> stats;
    for(std::size_t i = 0; i < 28; i += 2) {
        const auto& a = dreams::catalog[i]; const auto& b = dreams::catalog[i+1];
        check(std::string(a.actorValue) == b.actorValue);
        check(a.amount == -b.amount);
        check(a.amount == (i < 14 ? 1 : 10));
        check(std::string(a.text).find("TODO") != std::string::npos);
        stats.insert(a.actorValue);
    }
    check(stats.size() == 14);
    dreams::Active active{27, 100.0 + dreams::durationHours};
    check(!active.expired(111.99)); check(active.expired(112)); check(active.expired(150));
    check(!dreams::Active{}.valid());
    check(!dreams::Active{28, 10}.valid());
    check(!dreams::Active{0, std::numeric_limits<double>::infinity()}.valid());
    std::cout << "Passed: catalog, 15% boundary, selection, magnitudes, expiry, invalid state\n";
}
