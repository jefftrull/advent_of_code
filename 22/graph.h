#ifndef GRAPH_H
#define GRAPH_H

#include <boost/graph/graph_utility.hpp>
#include <boost/property_map/property_map.hpp>

struct server_t {
    using capacity_t = std::int16_t;

    int        x;
    int        y;
    capacity_t capacity;
};

struct server_state_t {
    using capacity_t = server_t::capacity_t;

    server_state_t();

    template<typename ServIt, typename UsageIt>
    server_state_t(ServIt sbegin, ServIt send,
                   UsageIt ubegin, UsageIt uend) : usages(ubegin, uend) {
        // to find the "upper right" element, first find the largest x (column) value
        using namespace std;
        int largest_x = max_element(sbegin, send,
                                    [](server_t const& a, server_t const& b) {
                                        return a.x < b.x;
                                    })->x;

        // then locate the y=0 matching that x, and calculate its offset among the servers
        original_data_location =
            distance(sbegin,
                     find_if(sbegin, send,
                             [largest_x](server_t const& s) {
                                 return ((s.x == largest_x) && (s.y == 0));
                             }));
    }

    capacity_t usage(size_t idx) const;

    bool operator<(server_state_t const& other) const;
    bool operator==(server_state_t const& other) const;
    bool operator!=(server_state_t const& other) const;
    size_t data_offset() const;

    server_state_t state_if_move(size_t, size_t) const;

    // helper for edge printing
    friend std::ostream& operator<<(std::ostream &, server_state_t const&);

private:
    std::vector<capacity_t> usages;                  // usage per server at a moment in time
    size_t                  original_data_location;  // where desired data is
};

// there is essentially no edge property because it either exists or does not exist
// and all edges are weight 1 - a constant property map may be appropriate here

// for astar_search_no_init (appropriate for implicit graphs) we need to model IncidenceGraph

struct move_graph_t {
    using vertex_t = server_state_t;
    using edge_t   = std::pair<vertex_t, vertex_t>;

    move_graph_t(std::vector<server_t> servers);

    struct out_edge_iterator_t
        : boost::iterator_facade<out_edge_iterator_t,
                                 edge_t,
                                 boost::forward_traversal_tag,
                                 edge_t> {

        // default constructor for "end of edges"
        out_edge_iterator_t();
        // another for beginning the range
        out_edge_iterator_t(move_graph_t const *      g,
                            std::shared_ptr<vertex_t> source);

        // requirements for iterator_facade
        edge_t dereference() const;

        bool equal(out_edge_iterator_t const& other) const;

        void increment();

    private:

        void ensure_valid();

        move_graph_t const *      move_graph_;
        std::shared_ptr<vertex_t> source_;
        bool                      sentinel_;
        size_t                    src_server_, dst_server_;

    };

    std::vector<server_t> const & servers() const;

private:
    std::vector<server_t> const servers_;

};

// Concept type requirements
namespace boost {

template<>
struct graph_traits<move_graph_t> {
    // for Graph
    using vertex_descriptor      = move_graph_t::vertex_t;
    using edge_descriptor        = move_graph_t::edge_t;
    using directed_category      = directed_tag;
    using edge_parallel_category = disallow_parallel_edge_tag;


    // for IncidenceGraph
    using traversal_category = incidence_graph_tag;
    using out_edge_iterator  = move_graph_t::out_edge_iterator_t;
    using degree_size_type   = size_t;

};


}

// Free function requirements for IncidenceGraph
move_graph_t::vertex_t source(move_graph_t::edge_t const&, move_graph_t const&);
move_graph_t::vertex_t target(move_graph_t::edge_t const&, move_graph_t const&);

std::pair<move_graph_t::out_edge_iterator_t, move_graph_t::out_edge_iterator_t>
out_edges(move_graph_t::vertex_t const&, move_graph_t const&);

size_t
out_degree(move_graph_t::vertex_t const&, move_graph_t const&);

#endif // GRAPH_H
