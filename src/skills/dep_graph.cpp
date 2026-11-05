// src/skills/dep_graph.cpp — YOU IMPLEMENT. Spec §6.2.

#include "skills/dep_graph.h"

bool DepGraph::add_node(const std::string& name) {
    // TODO
    (void)name;
    return false;
}

bool DepGraph::add_edge(const std::string& from, const std::string& to) {
    // TODO
    (void)from;
    (void)to;
    return false;
}

bool DepGraph::has_node(const std::string& name) const {
    // TODO
    (void)name;
    return false;
}

std::size_t DepGraph::node_count() const noexcept {
    // TODO
    return 0;
}

const std::vector<std::string>& DepGraph::requirements(const std::string& name) const {
    // TODO: return the stored list; for an unknown name return a reference
    // to an empty vector that outlives the call (e.g. a function-local
    // static const).
    (void)name;
    static const std::vector<std::string> kNone;
    return kNone;
}

bool DepGraph::order_from(const std::string& root, std::vector<std::string>* out,
                          std::string* error) const {
    // TODO: DFS with three colors (white / gray = on the current path /
    // black = finished). Spec §6.2 gives the exact error strings, and
    // Appendix D traces the diamond.
    // Clues:
    //  * Keep a vector<string> path of the gray nodes: push_back when a node
    //    turns gray, pop_back when it turns black. When an edge reaches a
    //    gray node x, the cycle is the part of path from x to the end, then
    //    x again. Check yourself with a>b, a>c, c>a: the answer is
    //    "cycle: a -> c -> a" (b is already finished and must not appear).
    //  * Append a node to *out when it turns BLACK (after its requirements),
    //    never when it turns gray.
    //  * Order of checks for each edge: missing node, then gray (cycle),
    //    then white (visit). Black: skip, that is not a cycle.
    //  * Separators are exactly " -> " (space, dash, greater-than, space).
    (void)root;
    (void)out;
    *error = "not implemented";
    return false;
}

std::vector<std::string> DepGraph::dependents(const std::string& name) const {
    // TODO: BFS over the reversed edges.
    // Clues:
    //  * Mark `name` itself as seen before you start, so a cycle can't add
    //    it to the result.
    //  * The order is by (distance, name). Easiest: sort each BFS level
    //    alphabetically before appending it. Sorting the whole result by
    //    name alone loses the nearest-first order.
    //  * std::queue<std::pair<std::string, int>> (node, distance) works, or
    //    process one level (a vector) at a time.
    (void)name;
    return {};
}
