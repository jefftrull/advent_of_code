#ifndef GRAPH_H
#define GRAPH_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/filtered_graph.hpp>

struct server_t {
    int x;
    int y;
    int capacity;
};

struct vertex_property_t {
    // capacity and usage for each server
    std::vector<server_t> server_state;
};

// there is essentially no edge property because it either exists or does not exist
// should really be implicit

using graph_t =  boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                       vertex_property_t>; // no edge property

// filter edges so we only get the ones that are possible
/*
std::pair<graph_t::out_edge_iterator, graph_t::out_edge_iterator>
out_edges(graph_t::vertex_descriptor u, graph_t const& g) {
    // return special edge iterator
*/
    
#endif // GRAPH_H
