// Driver program for solution to Advent of Code, Day 22
// Jeff Trull <edaskel@att.net>

#include <iostream>
#include <fstream>
#include <regex>
#include <algorithm>

#include <boost/coroutine2/all.hpp>
#include <boost/graph/astar_search.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/properties.hpp>

#include "graph.h"

struct goal_reached {
    server_state_t state;
};

// specialize an astar visitor to detect when we've reached our goal
struct goal_state_finder : public boost::default_astar_visitor {

    void examine_vertex( server_state_t state, move_graph_t const& g) {
        if ((g.servers()[state.data_offset()].x == 0) &&
            (g.servers()[state.data_offset()].y == 0)) {
            throw goal_reached{state};
        }
    }
};


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
    using capacity_t = server_t::capacity_t;
    std::vector<capacity_t> usages;
    while (!input.eof()) {
        string instr;
        getline(input, instr);
        match_results<string::iterator> matches;
        if (regex_match(instr.begin(), instr.end(), matches, df_re)) {
            // collect info on this server
            int capacity = stoi(matches.str(3));
            assert(capacity <= std::numeric_limits<capacity_t>::max());
            servers.push_back({
                    stoi(matches.str(1)),
                    stoi(matches.str(2)),
                    static_cast<capacity_t>(capacity)});
            int usage = stoi(matches.str(4));
            assert(usage <= std::numeric_limits<capacity_t>::max());
            usages.push_back(usage);
        }        
    }        
    server_state_t   initial_state(servers.begin(), servers.end(),
                                   usages.begin(), usages.end());


    // now see how many viable pairs there are
    // create a generator from the pair calculation:

    using namespace boost::coroutines2;
    using viable_pair_coro_t = boost::coroutines2::coroutine<std::pair<int, int>>;
    auto viable_pair_generator =
        [&](viable_pair_coro_t::push_type & sink) {
        // faster way is to sort by capacity but this is good enough for now
        for (size_t i = 0; i < servers.size(); ++i) {
            for (size_t j = 0; j < servers.size(); ++j) {
                if (initial_state.usage(i) == 0) {
                    continue;
                }

                if (i == j) {
                    continue;
                }

                if (initial_state.usage(i) <= (servers[j].capacity - initial_state.usage(j))) {
                    // room to move there, if there is a path
                    sink(make_pair(i, j));
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

    // next, find a sequence of moves of data that will result in the data in the
    // upper right being in the upper left

    move_graph_t move_graph(servers);

    // requirements for A* search
    using vertex_t = move_graph_t::vertex_t;
    map<vertex_t, size_t> vertex_index_map;
    map<vertex_t, size_t> rank_map;
    map<vertex_t, boost::default_color_type> color_map;
    map<vertex_t, vertex_t> predecessor_map;  // results (path) storage
    // the distance map needs to default to a large number instead of 0
    // because initially all vertices have unknown paths to them
    map<vertex_t, size_t> distance_map;
    auto distance_lookup =
        [&distance_map](vertex_t const& v) -> size_t& {
        if (distance_map.find(v) == distance_map.end()) {
            distance_map[v] = numeric_limits<size_t>::max();
        }
        return distance_map[v];
    };
    auto distance_pmap =
        boost::make_function_property_map<vertex_t, size_t&, decltype(distance_lookup)>(
            distance_lookup);

    // set up initial state
    distance_map[initial_state] = 0;
    predecessor_map[initial_state] = initial_state;

    using namespace boost;
    try {
        // no_init is the appropriate variant for implicit graphs like ours
        astar_search_no_init(
            move_graph,
            initial_state,
            [&](const vertex_t& v) {

                // Heuristic plan:
                // We need enough steps to move the original data to the origin
                // That ends up being about 5 times the Manhattan distance due to the need to move
                // the "blank tile" (server with sufficient capacity) back into place between the
                // target data and the origin each time.
                // In addition, we need to move the "blank tile" into position in the first place.

                // Manhattan distance to goal
                // we must make at least this many moves to get the original data home
                server_t current_server = servers[v.data_offset()];
                int mdist = current_server.x + current_server.y;

                // if mdist is 0, we are at the target, so simply return 0
                if (mdist == 0) {
                    return 0;
                }

                // Finding the distance to the "blank tile"
                // First, find the nearest (to the origin) server of sufficient reserve capacity
                // to hold the target data

                vector<server_t> eligible_servers;
                for (size_t i = 0; i < servers.size(); ++i) {
                    if ((servers[i].capacity - v.usage(i)) >= v.usage(v.data_offset())) {
                        eligible_servers.push_back(servers[i]);
                    }
                }
                // take the one with the minimum Manhattan distance to the server with our data
                auto min_it = min_element(eligible_servers.begin(), eligible_servers.end(),
                                          [&current_server](server_t const& a, server_t const& b) {
                                              return ((abs(current_server.x - a.x) + abs(current_server.y - a.y)) <
                                                      (abs(current_server.x - b.x) + abs(current_server.y - b.y)));
                                               });
                assert(min_it != eligible_servers.end());   // insoluble!

                // calculate distance to point above or to left of target data, whichever is shorter
                int hole_dist = std::numeric_limits<int>::max();
                if (current_server.y > 0) {
                    // distance to point above target
                    hole_dist = abs((current_server.y - 1) - min_it->y) + abs(current_server.x - min_it->x);
                }
                if (current_server.x > 0) {
                    // distance to point left of target
                    hole_dist = min(hole_dist,
                                    abs((current_server.x - 1) - min_it->x) + abs(current_server.y - min_it->y));
                }

                // describe our cost calculation
                cerr << "vertex with original data at (" << current_server.x << ", " << current_server.y << ") est cost " << mdist << "+" << hole_dist << "=" << (5*(mdist-1) + 1) + hole_dist << "\n";

                // each move of the target data requires 5 moves overall, except for the last one
                return (5*(mdist-1)+ 1) + hole_dist;
            },
            // named params
            weight_map(make_static_property_map<vertex_t, size_t>(1)).
            vertex_index_map(associative_property_map<map<vertex_t, size_t>>(vertex_index_map)).
            rank_map(associative_property_map<map<vertex_t, size_t>>(rank_map)).
            distance_map(distance_pmap).
            color_map(associative_property_map<map<vertex_t, default_color_type>>(color_map)).
            visitor(goal_state_finder()).
            predecessor_map(associative_property_map<map<vertex_t, vertex_t>>(predecessor_map))
            );
    } catch (goal_reached const& e) {
        // reverse path for display
        vector<server_state_t> soln_path;
        auto next_state = e.state;
        do {
            soln_path.push_back(next_state);
            next_state = predecessor_map[next_state];
        } while (!(soln_path.back() == next_state));
        reverse(soln_path.begin(), soln_path.end());

        // describe the path
        cout << "solution: " << (soln_path.size() - 1) << " steps to goal state:\n";
        copy(soln_path.begin(), soln_path.end(),
                  ostream_iterator<server_state_t>(cout, "\n"));

        return 0;
    }
    cerr << "could not find solution\n";
    return 1;
}
