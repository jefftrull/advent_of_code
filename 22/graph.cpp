// Implementation of implicit move graph class for solving Advent of Code, Day 22
// Jeff Trull <edaskel@att.net>

#include "graph.h"

move_graph_t::move_graph_t(std::vector<server_t> servers) : servers_(std::move(servers)) {}

// IncidenceGraph free functions

move_graph_t::vertex_t source(move_graph_t::edge_t const& e, move_graph_t const&) {
    return e.first;
}

move_graph_t::vertex_t target(move_graph_t::edge_t const& e, move_graph_t const&) {
    return e.second;
}

std::vector<server_t> const &
move_graph_t::servers() const {
    return servers_;
}

std::pair<move_graph_t::out_edge_iterator_t, move_graph_t::out_edge_iterator_t>
out_edges(move_graph_t::vertex_t const& u, move_graph_t const& g) {

    return std::make_pair(
        move_graph_t::out_edge_iterator_t(&g, std::make_shared<move_graph_t::vertex_t>(u)),
        move_graph_t::out_edge_iterator_t());
            
}

// Implementations for internal iterator class

move_graph_t::out_edge_iterator_t::out_edge_iterator_t() : sentinel_(true) {}

move_graph_t::out_edge_iterator_t::out_edge_iterator_t(
    move_graph_t const *      g,
    std::shared_ptr<vertex_t> source)
    : move_graph_(g), source_(std::move(source)),
      sentinel_(false),
      src_server_(0), dst_server_(0) {

    ensure_valid();                          // move forward to valid move, if needed
}
      
move_graph_t::edge_t
move_graph_t::out_edge_iterator_t::dereference() const {
    // assuming user has not tried to dereference the end iterator
    return std::make_pair(*source_,
                          move_graph_t::state_if_move(*source_, src_server_, dst_server_));
}

bool
move_graph_t::out_edge_iterator_t::equal(out_edge_iterator_t const& other) const {
    if (sentinel_ && other.sentinel_) {
        // both "end of sequence" so other fields irrelevant
        return true;
    }
    return ((sentinel_    == other.sentinel_) &&
            (source_      == other.source_) &&
            (src_server_  == other.src_server_) &&
            (dst_server_  == other.dst_server_));

}            
    
void
move_graph_t::out_edge_iterator_t::increment() {
    if (!sentinel_) {
        // push out of current state, then look for next valid
        ++dst_server_;
        ensure_valid();
    }
}

// code for searching for a valid move.  Used to keep iterators legit (either valid or "end")
void
move_graph_t::out_edge_iterator_t::ensure_valid() {
    if (sentinel_) {
        return;           // end of sequence is always fine
    }

    std::vector<server_t> const& servers = move_graph_->servers();

    // if the current src/dst pair is not valid, advance it to one that is
    // if there is no such pair, set the end sentinel

    for (; src_server_ < servers.size(); ++src_server_) {
        for (; dst_server_ < servers.size(); ++dst_server_) {
            if (((source_->usages)[src_server_] == 0) ||                              // no source data
                (src_server_ == dst_server_) ||                                       // same src, dst
                ((source_->usages)[src_server_] >
                 (servers[dst_server_].capacity - (source_->usages)[dst_server_]))) { // insufficient space?

                continue;
            }
            // are source and dest neighbors in the grid?
            if (((servers[src_server_].x == servers[dst_server_].x) &&
                 ((servers[src_server_].y == (servers[dst_server_].y + 1)) ||
                  (servers[dst_server_].y == (servers[src_server_].y + 1)))) ||
                ((servers[src_server_].y == servers[dst_server_].y) &&
                 ((servers[src_server_].x == (servers[dst_server_].x + 1)) ||
                  (servers[dst_server_].x == (servers[src_server_].x + 1))))) {

                // all checks pass;  we can use this pair
                return;
            }
        }
        dst_server_ = 0;   // resume search at beginning of next row
    }        

    if (src_server_ >= servers.size()) {
        // we have run out of valid moves
        sentinel_ = true;
    }
}

// Utility function for producing a new state from a move
move_graph_t::vertex_t
move_graph_t::state_if_move(vertex_t initial,
                            size_t src, size_t dst) {
    // turn the source -> dest move into a new usage state
    initial.usages[dst] += initial.usages[src];
    initial.usages[src] = 0;
    if (src == initial.original_data_location) {
        // moving the target data
        initial.original_data_location = dst;
    }
    return initial;
}

bool
server_state_t::operator<(server_state_t const& other) const {
    return (original_data_location < other.original_data_location) ||
        ((original_data_location == other.original_data_location) &&
         (usages < other.usages));
}

bool
server_state_t::operator==(server_state_t const& other) const {
    return (original_data_location == other.original_data_location) &&
        (usages == other.usages);
}

std::ostream&
operator<<(std::ostream & os, server_state_t s) {
    os << "original data at " << s.original_data_location << "\n";
    os << "capacities: ";
    std::copy(s.usages.begin(), s.usages.end(),
              std::ostream_iterator<int>(os, ", "));
    return os;
}
