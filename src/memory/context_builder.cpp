// src/memory/context_builder.cpp — YOU IMPLEMENT. Spec §4.4.

#include "memory/context_builder.h"
#include "util/min_heap.h"

ContextPlan select_context(const std::vector<ContextItem> &items,
                           std::size_t budget, std::size_t keep_recent)
{
    std::size_t n = items.size();
    std::size_t total = 0;
    ContextPlan plan;
    std::size_t i;
    for (i = 0; i < n; i++)
    {
        total += items[i].tokens;
    }
    if (total <= budget)
    {
        for (std::size_t j = 0; j < n; j++)
        {
            plan.keep.push_back(j);
        }
        plan.total_tokens = total;
        plan.over_budget = false;
        return plan;
    }
    std::size_t recent_start;
    if (keep_recent >= n)
    {
        recent_start = 0;
    }
    else
        recent_start = n - keep_recent;
    MinHeap<std::pair<int, std::size_t>> heap;
    for (i = 0; i < n; i++)
    {
        if (items[i].role == Role::System || items[i].pinned || (i >= recent_start))
        {
            continue;
        }
        heap.push({drop_rank(items[i].role), i});
    }
    std::vector<bool> kept(n, true);
    while (total > budget && !heap.empty())
    {
        std::pair<int, std::size_t> p = heap.pop();
        std::size_t idx = p.second;
        kept[idx] = false;
        total -= items[idx].tokens;
    }
    for (i=0; i<n; i++){
        if (kept[i]){
            plan.keep.push_back(i);
        }
    }
        plan.total_tokens =total;
        plan.over_budget = (total > budget);
        return plan;
}
