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

enum class grid_neighbor { North, South, East, West, Invalid };
grid_neighbor& operator++(grid_neighbor&);

struct server_state_t {
    using capacity_t = server_t::capacity_t;

    server_state_t();

    template<typename UsageIt>
    server_state_t(size_t target_data_offset,
                   UsageIt ubegin, UsageIt uend) :
        usages(std::make_shared<std::vector<capacity_t>>(ubegin, uend)),
        original_data_location(target_data_offset) {}

    capacity_t usage(size_t idx) const;

    bool operator<(server_state_t const& other) const;
    bool operator==(server_state_t const& other) const;
    bool operator!=(server_state_t const& other) const;
    size_t data_offset() const;

    server_state_t state_if_move(size_t, size_t) const;

    // helper for vertex printing
    friend std::ostream& operator<<(std::ostream &, server_state_t const&);

private:
    std::shared_ptr<std::vector<capacity_t>> usages; // original usage per server
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
        size_t dst_offset() const;

        move_graph_t const *      move_graph_;
        std::shared_ptr<vertex_t> source_;
        bool                      sentinel_;
        size_t                    src_server_;
        grid_neighbor             dst_server_;
    };

    std::vector<server_t> const & servers()    const;
    size_t                        col_stride() const;
    size_t                        ur_corner()  const;

private:
    std::vector<server_t> const servers_;
    size_t                      ur_corner_;
    size_t                      col_stride_;
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
