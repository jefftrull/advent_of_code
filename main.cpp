#include <iostream>
#include <fstream>
#include <regex>

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
    while (!input.eof()) {
        string instr;
        getline(input, instr);
        match_results<string::iterator> matches;
        if (regex_match(instr.begin(), instr.end(), matches, df_re)) {
            // collect info on this server
            servers.push_back({
                    stoi(matches.str(1)),
                    stoi(matches.str(2)),
                    stoi(matches.str(3)),
                    stoi(matches.str(4))});
        }        
    }        

    cout << "found " << servers.size() << " server definitions\n";

    // now see how many viable pairs there are
    // faster way is to sort by capacity but this is good enough for now
    size_t viable_pair_count = 0;
    for (size_t i = 0; i < servers.size(); ++i) {
        for (size_t j = 0; j < servers.size(); ++j) {
            if (servers[i].usage == 0) {
                continue;
            }

            if (i == j) {
                continue;
            }

            if (servers[i].usage <= (servers[j].capacity - servers[j].usage)) {
                // room to move there, if there is a path
                viable_pair_count++;
            }
        }
    }
        
    cout << viable_pair_count << " viable pairs\n";

}
