// src/skills/dep_graph.cpp — YOU IMPLEMENT. Spec §6.2.

#include "skills/dep_graph.h"
#include <unordered_set>
#include <algorithm>

static const int WHITE = 0, GRAY = 1, BLACK = 2;

bool DepGraph::add_node(const std::string &name)
{
    if (requires_.count(name) > 0)
    {
        return false;
    }
    requires_[name];
    return true;
}

bool DepGraph::add_edge(const std::string &from, const std::string &to)
{
    auto it = requires_.find(from);
    if (it == requires_.end())
        return false;
    std::vector<std::string> &reqs = it->second;
    for (const std::string &r : reqs)
    {
        if (r == to)
            return false;
    }
    reqs.push_back(to);
    required_by_[to].push_back(from);
    return true;
}

bool DepGraph::has_node(const std::string &name) const
{
    return (requires_.count(name));
}

std::size_t DepGraph::node_count() const noexcept
{
    return requires_.size();
}

const std::vector<std::string> &DepGraph::requirements(const std::string &name) const
{
    static const std::vector<std::string> kNone;
    auto it = requires_.find(name);
    if (it != requires_.end())
        return it->second;
    return kNone;
}

bool DepGraph::order_from(const std::string &root, std::vector<std::string> *out,
                          std::string *error) const
{
    if (!has_node(root))
    {
        *error = "unknown: " + root;
        return false;
    }
    out->clear();
    std::unordered_map<std::string, int> color;
    std::vector<std::string> path;
    return dfs(root, color, path, out, error);
}

bool DepGraph::dfs(const std::string &node, std::unordered_map<std::string, int> &color,
                   std::vector<std::string> &path, std::vector<std::string> *out,
                   std::string *error) const
{
    color[node] = GRAY;
    path.push_back(node);
    for (const std::string &x : requirements(node))
    {
        if (!has_node(x))
        {
            *error = "missing: " + x + " (required by " + node + ")";
            return false;
        }
        if (color[x] == GRAY)
        {
            std::size_t start = 0;
            std::string msg = "cycle: ";
            for (std::size_t k = 0; k < path.size(); k++)
            {
                if (path[k] == x)
                {
                    start = k;
                    break;
                }
            }
            for (std::size_t k = start; k < path.size(); k++)
            {
                msg += path[k] + " -> ";
            }
            msg += x;
            *error = msg;

            return false;
        }
        if (color[x] == WHITE)
        {
            if (!dfs(x, color, path, out, error))
            {
                return false;
            }
        }
    }
    color[node] = BLACK;
    path.pop_back();
    out->push_back(node);
    return true;
}

std::vector<std::string> DepGraph::dependents(const std::string &name) const
{
    std::vector<std::string> result, level, next;
    std::unordered_set<std::string> seen;
    result = {};
    seen.insert(name);
    level.push_back(name);

    while (!level.empty())
    {
        next.clear();
        for (const std::string &u : level)
        {
            auto it = required_by_.find(u);
            if (it == required_by_.end())
                continue;
            for (const std::string &v : it->second)
            {
                if (seen.count(v) == 0)
                {
                    seen.insert(v);
                    next.push_back(v);
                }
            }
        }
        std::sort(next.begin(), next.end());
        result.insert(result.end(), next.begin(), next.end());
        level = next;
    }
    return result;
}
