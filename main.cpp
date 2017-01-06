#include <iostream>
#include <fstream>
#include <regex>

#include <boost/coroutine2/all.hpp>

#include "graph.h"

int main(int argc, char **argv) {
    using namespace std;

    if (argc != 2) {
        cerr << "usage: day22 input.txt\n";
        return 1;
    }

    ifstream input(argv[1]);
    if (!input.is_open()) {
        cerr << "error opening " << argv[1] << "\n";
        return 1;
    }

    regex df_re(R"(^/dev/grid/node-x(\d+)-y(\d+)\s+(\d+)T\s+(\d+)T\s.*)");
    vector<server_t> servers;
    vector<int>      usages;
    while (!input.eof()) {
        string instr;
        getline(input, instr);
        match_results<string::iterator> matches;
        if (regex_match(instr.begin(), instr.end(), matches, df_re)) {
            // collect info on this server
            servers.push_back({
                    stoi(matches.str(1)),
                    stoi(matches.str(2)),
                    stoi(matches.str(3))});
            usages.push_back(stoi(matches.str(4)));
        }        
    }        

    cout << "found " << servers.size() << " server definitions\n";

    // now see how many viable pairs there are
    // create a generator from the pair calculation:

    using namespace boost::coroutines2;
    using viable_pair_coro_t = asymmetric_coroutine<std::pair<size_t, size_t>>;
    auto viable_pair_generator =
        [&](viable_pair_coro_t::push_type & sink) {
        // faster way is to sort by capacity but this is good enough for now
        for (size_t i = 0; i < servers.size(); ++i) {
            for (size_t j = 0; j < servers.size(); ++j) {
                if (usages[i] == 0) {
                    continue;
                }

                if (i == j) {
                    continue;
                }

                if (usages[i] <= (servers[j].capacity - usages[j])) {
                    // room to move there, if there is a path
                    sink(std::make_pair(i, j));
                }
            }
        }};

    // create a sequence from the generator
    viable_pair_coro_t::pull_type viable_pairs(viable_pair_generator);

    // count pairs in sequence
    size_t viable_pair_count = 0;
    for (auto const& v : viable_pairs) {
        (void)v;
        ++viable_pair_count;
    }

    cout << viable_pair_count << " viable pairs\n";

}
