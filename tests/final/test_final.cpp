// tests/final/test_final.cpp — YOUR TEST SUITE (spec §8).
//
// You need at least 20 tests. The 8 examples below are given and count:
//   Memory  (4): heap, lru, bst, ctx
//   Policy  (2): pol x2
//   Skills  (2): graph, index
// Write at least 12 more of your own, covering the list in spec §8.
// Name tests <area>_<what> with the areas heap, lru, bst, ctx, pol, graph,
// index, so the summary groups them.
//
//   ./build/test_final              run everything
//   ./build/test_final lru_         only tests whose names start with lru_
//   ./build/test_final =lru_evicts_least_recently_used    one test, exactly
//   ./build/test_final --list       list test names
//
// If a sanitizer error (ASan/UBSan) aborts the run, the tests after it do
// not run: rerun the failing test alone with =its_name and read the FIRST
// stack frame that is in your own file.
//
// CHECK_EQ(a, b) prints both values when it fails (vectors too), so prefer
// it over CHECK(a == b).
//
// Helpers you can use (tests/test_helpers.h): TempDir, write_text,
// write_skill, FixedPolicy, ScriptedConfirmer, LinesInput, StringOutput,
// parse_ok, args1. ManualClock is in policy/policy.h.

#include "../mini_test.h"
#include "../test_helpers.h"

#include "memory/context_builder.h"
#include "memory/memory_index.h"
#include "memory/memory_store.h"
#include "policy/policies.h"
#include "policy/policy_chain.h"
#include "skills/dep_graph.h"
#include "skills/skill_catalog.h"
#include "skills/skill_index.h"
#include "util/lru_cache.h"
#include "util/min_heap.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include <chrono>
#include <cstdio>

// =========================================================================
// Memory — 4 examples
// =========================================================================

// Example 1 (heap): items come back smallest first, duplicates included.
TEST(heap_pops_in_order)
{
    MinHeap<int> h;
    for (int x : {5, 1, 4, 1, 3})
        h.push(x);
    CHECK_EQ(h.size(), std::size_t{5});
    CHECK_EQ(h.top(), 1);
    std::vector<int> got;
    while (!h.empty())
        got.push_back(h.pop());
    CHECK_EQ(got, (std::vector<int>{1, 1, 3, 4, 5}));
}

// Example 2 (lru): the least recently USED entry is evicted, and get()
// counts as a use.
TEST(lru_evicts_least_recently_used)
{
    LruCache<std::string, int> c(2);
    c.put("a", 1);
    c.put("b", 2);
    REQUIRE(c.get("a") != nullptr); // "a" is now the most recent
    c.put("c", 3);                  // full: "b" must go
    CHECK(c.contains("a"));
    CHECK(!c.contains("b"));
    CHECK_EQ(c.keys_mru_to_lru(), (std::vector<std::string>{"c", "a"}));
    CHECK_EQ(c.stats().evictions, std::size_t{1});
}

// Example 3 (bst): removing a node with two children keeps the tree sorted,
// keeps every other record, and lowers size() by exactly one.
TEST(bst_remove_two_children)
{
    MemoryIndex t;
    for (std::uint64_t id : {50, 30, 70, 60, 80})
        REQUIRE(t.insert(MemoryRecord{id, "m" + std::to_string(id)}));
    REQUIRE(t.remove(70));
    std::vector<std::uint64_t> ids;
    t.for_each_in_order([&](const MemoryRecord &r)
                        { ids.push_back(r.id); });
    CHECK_EQ(ids, (std::vector<std::uint64_t>{30, 50, 60, 80}));
    CHECK(t.find(70) == nullptr);
    REQUIRE(t.find(80) != nullptr);
    CHECK_EQ(t.find(80)->text, std::string("m80"));
    CHECK_EQ(t.size(), std::size_t{4});
}

// Example 4 (ctx): the worked example in spec §4.4.
TEST(ctx_spec_example)
{
    std::vector<ContextItem> items{
        {Role::System, 10, false},     // 0  protected
        {Role::User, 10, false},       // 1
        {Role::Assistant, 10, false},  // 2  dropped 3rd (oldest assistant)
        {Role::Tool, 10, false},       // 3  dropped 1st
        {Role::Assistant, 10, false},  // 4
        {Role::Tool, 10, false},       // 5  dropped 2nd
        {Role::User, 10, false},       // 6  recent: protected
        {Role::Assistant, 10, false}}; // 7  recent: protected
    ContextPlan p = select_context(items, 50, 2);
    CHECK_EQ(p.keep, (std::vector<std::size_t>{0, 1, 4, 6, 7}));
    CHECK_EQ(p.total_tokens, std::size_t{50});
    CHECK(!p.over_budget);
}

// MY MEMORY TESTS

TEST(heap_empty_throws)
{
    MinHeap<int> h;
    CHECK(h.empty());
    CHECK_THROWS(h.top(), std::out_of_range);
    h.push(3);
    CHECK_EQ(h.pop(), 3);
    CHECK(h.empty());
    CHECK_THROWS(h.pop(), std::out_of_range);
}

TEST(heap_custom_comparator)
{
    MinHeap<int, std::greater<int>> h;
    for (int x : {12, 8, 9, 5, 4, 1, 0, 3, 7, 10, 11, 7, 2, 6})
    {
        h.push(x);
    }
    CHECK_EQ(h.top(), 12);
    std::vector<int> test;
    while (!h.empty())
    {
        test.push_back(h.pop());
    }
    CHECK_EQ(test, (std::vector<int>{12, 11, 10, 9, 8, 7, 7, 6, 5, 4, 3, 2, 1, 0}));
}

TEST(heap_topk_edge_cases)
{
    std::vector<int> r0 = top_k(std::vector<int>{3, 1, 2}, 0);
    CHECK(r0.empty());
    std::vector<int> r1 = top_k(std::vector<int>{}, 3);
    CHECK(r1.empty());
    std::vector<int> r3 = top_k(std::vector<int>{4, 8}, 5);
    CHECK_EQ(r3, (std::vector<int>{8, 4}));
    std::vector<int> r4 = top_k(std::vector<int>{4, 8, 1}, 3);
    CHECK_EQ(r4, (std::vector<int>{8, 4, 1}));
    std::vector<int> r5 = top_k(std::vector<int>{5, 1, 9, 3, 7}, 2);
    CHECK_EQ(r5, (std::vector<int>{9, 7}));
}

TEST(heap_topk_matches_sort)
{
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> value(0, 20);
    std::uniform_int_distribution<int> length(0, 50);

    for (int trial = 0; trial < 200; trial++)
    {
        std::vector<int> v;
        int n = length(rng);
        for (int i = 0; i < n; i++)
            v.push_back(value(rng));

        for (std::size_t k : {0, 1, 2, 5, 10, 60})
        {
            std::vector<int> got = top_k(v, k);

            std::vector<int> expected = v;
            std::sort(expected.begin(), expected.end(), std::greater<int>());
            if (expected.size() > k)
                expected.resize(k);
            CHECK_EQ(got, expected);
        }
    }
}

TEST(heap_topk_timing)
{
    std::mt19937 rng(7);
    std::uniform_int_distribution<int> value(0, 1000000000);
    std::vector<int> v;
    // YOUR LINE: a for loop that push_backs 100000 values from value(rng)
    for (int i = 0; i < 100000; i++)
    {
        v.push_back(value(rng));
    }
    auto t0 = std::chrono::steady_clock::now();
    std::vector<int> got = top_k(v, 10);
    auto t1 = std::chrono::steady_clock::now();
    double ms_topk = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::vector<int> sorted = v;
    t0 = std::chrono::steady_clock::now();
    std::sort(sorted.begin(), sorted.end(), std::greater<int>());
    t1 = std::chrono::steady_clock::now();
    double ms_sort = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::printf("top_k(k=10) on 100000: %.3f ms\n", ms_topk);
    std::printf("std::sort on 100000:    %.3f ms\n", ms_sort);
    sorted.resize(10);
    CHECK_EQ(got, sorted);
}

TEST(lru_refresh_recency)
{
    LruCache<std::string, int> c(3);

    c.put("a", 1);
    c.put("b", 2);
    c.put("c", 3);
    CHECK_EQ(c.keys_mru_to_lru(), (std::vector<std::string>{"c", "b", "a"}));
    const int *v = c.get("a");
    REQUIRE(v != nullptr);
    CHECK_EQ(c.keys_mru_to_lru(), (std::vector<std::string>{"a", "c", "b"}));
    CHECK_EQ(*v, 1);
    c.put("d", 4);
    CHECK_EQ(c.keys_mru_to_lru(), (std::vector<std::string>{"d", "a", "c"}));
    CHECK(c.contains("a"));
    CHECK(!c.contains("b"));
    CHECK_EQ(c.stats().evictions, std::size_t{1});
}

TEST(lru_put_existing_updates)
{
    LruCache<std::string, int> c(2);
    c.put("a", 1);
    c.put("b", 2);
    CHECK_EQ(c.keys_mru_to_lru(), (std::vector<std::string>{"b", "a"}));
    c.put("a", 10);
    CHECK_EQ(c.keys_mru_to_lru(), (std::vector<std::string>{"a", "b"}));
    CHECK_EQ(c.size(), std::size_t{2});
    CHECK_EQ(c.stats().evictions, std::size_t{0});
    const int *v = c.get("a");
    REQUIRE(v != nullptr);
    CHECK_EQ(*v, 10);
    CHECK_EQ(c.stats().hits, std::size_t{1});
    CHECK_EQ(c.stats().misses, std::size_t{0});
}

TEST(lru_capacity_0_1)
{
    LruCache<int, int> z(0);
    z.put(1, 100);
    CHECK_EQ(z.size(), std::size_t{0});
    CHECK(!z.contains(1));
    CHECK(z.get(1) == nullptr);
    CHECK_EQ(z.stats().evictions, std::size_t{0});

    LruCache<int, int> o(1);
    o.put(1, 10);
    CHECK_EQ(o.keys_mru_to_lru(), (std::vector<int>{1}));
    o.put(2, 20);
    CHECK(!o.contains(1));
    CHECK_EQ(o.keys_mru_to_lru(), (std::vector<int>{2}));
    CHECK_EQ(o.stats().evictions, std::size_t{1});

    o.put(2, 25);
    CHECK_EQ(o.keys_mru_to_lru(), (std::vector<int>{2}));
    CHECK_EQ(o.stats().evictions, std::size_t{1});
    const int *v = o.get(2);
    REQUIRE(v != nullptr);
    CHECK_EQ(*v, 25);
}

TEST(lru_erase_head_middle_tail)
{
    LruCache<int, int> c(5);
    c.put(1, 1);
    c.put(2, 2);
    c.put(3, 3);
    c.put(4, 4);
    c.put(5, 5);
    CHECK_EQ(c.keys_mru_to_lru(), (std::vector<int>{5, 4, 3, 2, 1}));
    CHECK(c.erase(5));
    CHECK_EQ(c.keys_mru_to_lru(), (std::vector<int>{4, 3, 2, 1}));
    CHECK(c.erase(3));
    CHECK_EQ(c.keys_mru_to_lru(), (std::vector<int>{4, 2, 1}));
    CHECK(c.erase(1));
    CHECK_EQ(c.keys_mru_to_lru(), (std::vector<int>{4, 2}));
    CHECK(!c.erase(9));
    CHECK_EQ(c.keys_mru_to_lru(), (std::vector<int>{4, 2}));
    CHECK_EQ(c.size(), std::size_t{2});
    CHECK_EQ(c.stats().evictions, std::size_t{0});
    c.put(6, 60);
    CHECK_EQ(c.keys_mru_to_lru(), (std::vector<int>{6, 4, 2}));
}

static std::vector<std::uint64_t> in_order_ids(const MemoryIndex &t)
{
    std::vector<std::uint64_t> ids;
    t.for_each_in_order([&](const MemoryRecord &r)
                        { ids.push_back(r.id); });
    return ids;
}

TEST(bst_remove_leaf_1child_root)
{
    MemoryIndex t;
    for (std::uint64_t id : {50, 30, 70, 20, 40, 60})
        REQUIRE(t.insert(MemoryRecord{id, "m" + std::to_string(id)}));
    CHECK_EQ(in_order_ids(t), (std::vector<std::uint64_t>{20, 30, 40, 50, 60, 70}));
    CHECK_EQ(t.size(), std::size_t{6});
    CHECK(t.remove(20));
    CHECK_EQ(in_order_ids(t), (std::vector<std::uint64_t>{30, 40, 50, 60, 70}));
    CHECK_EQ(t.size(), std::size_t{5});
    CHECK(t.find(20) == nullptr);
    CHECK(t.remove(70));
    CHECK_EQ(in_order_ids(t), (std::vector<std::uint64_t>{30, 40, 50, 60}));
    CHECK_EQ(t.size(), std::size_t{4});
    CHECK(t.find(70) == nullptr);
    CHECK(t.remove(50));
    CHECK_EQ(in_order_ids(t), (std::vector<std::uint64_t>{30, 40, 60}));
    CHECK_EQ(t.size(), std::size_t{3});
    CHECK(t.find(50) == nullptr);
    CHECK(t.remove(60));
    CHECK_EQ(in_order_ids(t), (std::vector<std::uint64_t>{30, 40}));
    CHECK_EQ(t.size(), std::size_t{2});
    CHECK(t.find(60) == nullptr);
    CHECK(t.remove(30));
    CHECK_EQ(in_order_ids(t), (std::vector<std::uint64_t>{40}));
    CHECK_EQ(t.size(), std::size_t{1});
    CHECK(t.find(30) == nullptr);
    CHECK(t.remove(40));
    CHECK(in_order_ids(t).empty());
    CHECK_EQ(t.size(), std::size_t{0});
    CHECK(t.find(40) == nullptr);
    CHECK_EQ(t.height(), std::size_t{0});
    CHECK(!t.remove(40));
}

TEST(bst_dup_inorder_height)
{
    MemoryIndex t;
    CHECK(t.height() == 0);
    CHECK(t.insert(MemoryRecord{50, "m50"}));
    CHECK(t.height() == 1);
    CHECK(t.insert(MemoryRecord{30, "m30"}));
    CHECK(t.insert(MemoryRecord{70, "m70"}));
    CHECK(t.height() == 2);
    CHECK_EQ(in_order_ids(t), (std::vector<std::uint64_t>{30, 50, 70}));
    CHECK(!t.insert(MemoryRecord{30, "30dup"}));
    const MemoryRecord *r = t.find(30);
    REQUIRE(r != nullptr);
    CHECK_EQ(r->text, std::string("m30"));
    CHECK_EQ(t.size(), std::size_t{3});
    CHECK_EQ(in_order_ids(t), (std::vector<std::uint64_t>{30, 50, 70}));
    CHECK(t.height() == 2);
}

TEST(bst_height_seq_shuffle)
{
    MemoryIndex seq;
    for (std::uint64_t id = 1; id <= 1000; id++)
        seq.insert(MemoryRecord{id, "m"});
    std::vector<std::uint64_t> ids;
    for (std::uint64_t id = 1; id <= 1000; id++)
        ids.push_back(id);
    std::mt19937 rng(42);
    std::shuffle(ids.begin(), ids.end(), rng);
    MemoryIndex shuf;
    for (std::uint64_t id : ids)
        shuf.insert(MemoryRecord{id, "m"});
    std::printf("BST height, 1000 sequential ids: %zu\n", seq.height());
    std::printf("BST height, 1000 shuffled ids:   %zu\n", shuf.height());
    CHECK_EQ(seq.size(), std::size_t{1000});
    CHECK_EQ(shuf.size(), std::size_t{1000});
    CHECK_EQ(seq.height(), std::size_t{1000});
    CHECK(shuf.height() < 100);
}

TEST(ctx_protect_over_budget)
{
    std::vector<ContextItem> items{
        {Role::System, 10, false},
        {Role::Tool, 10, true},
        {Role::Tool, 10, false},
        {Role::Assistant, 10, false},
        {Role::User, 10, false},
        {Role::Tool, 10, false}};
    ContextPlan a = select_context(items, 0, 1);
    CHECK_EQ(a.keep, (std::vector<std::size_t>{0, 1, 5}));
    CHECK_EQ(a.total_tokens, std::size_t{30});
    CHECK(a.over_budget);

    ContextPlan b = select_context(items, 30, 1);
    CHECK_EQ(b.keep, (std::vector<std::size_t>{0, 1, 5}));
    CHECK_EQ(b.total_tokens, std::size_t{30});
    CHECK(!b.over_budget);
}
// =========================================================================
// Policy — 2 examples
// =========================================================================

// Example 5 (pol): the sliding window. A request exactly `window` seconds
// after an allowed one no longer counts it.
TEST(pol_rate_limit_window_boundary)
{
    ManualClock clock(100);
    RateLimitPolicy p(2, 10.0, clock); // at most 2 per 10 s
    CommandRequest req;
    req.program = "ls";
    CHECK(p.evaluate(req).kind == Decision::Kind::Allow); // t = 100
    clock.advance(1);
    CHECK(p.evaluate(req).kind == Decision::Kind::Allow); // t = 101
    clock.advance(1);
    CHECK(p.evaluate(req).kind == Decision::Kind::Deny); // t = 102: two in the window
    clock.advance(8);
    CHECK(p.evaluate(req).kind == Decision::Kind::Allow); // t = 110: 100 has left (110 - 100 = 10)
    CHECK(p.tracked() <= 2u);
}

// Example 6 (pol): first Deny wins and later policies are not asked; with no
// Deny, the FIRST Ask is returned unchanged.
TEST(pol_chain_first_deny_stops_first_ask_wins)
{
    auto ask1 = std::make_unique<FixedPolicy>(Decision::Kind::Ask, "ask1");
    auto ask2 = std::make_unique<FixedPolicy>(Decision::Kind::Ask, "ask2");
    PolicyChain chain;
    REQUIRE(chain.add(std::make_unique<FixedPolicy>(Decision::Kind::Allow, "ok")));
    REQUIRE(chain.add(std::move(ask1)));
    REQUIRE(chain.add(std::move(ask2)));
    CommandRequest req;
    req.program = "git";
    Decision d = chain.evaluate(req);
    CHECK(d.kind == Decision::Kind::Ask);
    CHECK_EQ(d.policy, std::string("ask1"));

    auto deny = std::make_unique<FixedPolicy>(Decision::Kind::Deny, "no");
    auto after = std::make_unique<FixedPolicy>(Decision::Kind::Allow, "after");
    FixedPolicy *after_ptr = after.get(); // still valid: the chain owns it now
    REQUIRE(chain.add(std::move(deny)));
    REQUIRE(chain.add(std::move(after)));
    d = chain.evaluate(req);
    CHECK(d.kind == Decision::Kind::Deny);
    CHECK_EQ(d.policy, std::string("no"));
    CHECK_EQ(after_ptr->calls, 0); // never evaluated: the chain stopped at the Deny
}

// MY POLICY TESTS

TEST(pol_each_policy)
{

    AllowlistPolicy allow({"ls", "git"});
    Decision d = allow.evaluate(CommandRequest{"ls", {}, ""});
    CHECK(d.kind == Decision::Kind::Allow);
    CHECK_EQ(d.policy, std::string("allowlist"));
    CHECK(!d.reason.empty());
    CHECK(allow.evaluate(CommandRequest{"rm", {}, ""}).kind == Decision::Kind::Deny);
    CHECK(allow.evaluate(CommandRequest{"gitk", {}, ""}).kind == Decision::Kind::Deny);
    CHECK(allow.evaluate(CommandRequest{"git", {}, ""}).kind == Decision::Kind::Allow);
    ArgumentPatternPolicy patt({";", "rm -rf"});
    d = patt.evaluate(CommandRequest{"cat", {"a;", "rm", "b"}, ""});
    CHECK(d.kind == Decision::Kind::Deny);
    CHECK_EQ(d.policy, std::string("patterns"));
    CHECK(!d.reason.empty());
    CHECK(patt.evaluate(CommandRequest{"rm", {"-rf", "/"}, ""}).kind == Decision::Kind::Deny);
    CHECK(patt.evaluate(CommandRequest{"ls", {"-l"}, ""}).kind == Decision::Kind::Allow);
    ConfirmPolicy conf({"git"});
    d = conf.evaluate(CommandRequest{"git", {"status"}, ""});
    CHECK(d.kind == Decision::Kind::Ask);
    CHECK_EQ(d.policy, std::string("confirm"));
    CHECK(!d.reason.empty());
    CHECK(conf.evaluate(CommandRequest{"ls", {}, ""}).kind == Decision::Kind::Allow);
    ManualClock clock(100);
    RateLimitPolicy one(1, 10.0, clock);
    d = one.evaluate(CommandRequest{"ls", {}, ""});
    CHECK(d.kind == Decision::Kind::Allow);
    CHECK_EQ(d.policy, std::string("ratelimit"));
    CHECK(!d.reason.empty());
    CHECK(one.evaluate(CommandRequest{"ls", {}, ""}).kind == Decision::Kind::Deny);
    RateLimitPolicy zero(0, 10.0, clock);
    CHECK(zero.evaluate(CommandRequest{"ls", {}, ""}).kind == Decision::Kind::Deny);
}

TEST(pol_nested_chain)
{
    auto inner = std::make_unique<PolicyChain>();
    REQUIRE(inner->add(std::make_unique<AllowlistPolicy>(std::vector<std::string>{"ls", "git"})));
    PolicyChain outer;
    REQUIRE(outer.add(std::move(inner)));
    REQUIRE(outer.add(std::make_unique<ConfirmPolicy>(std::vector<std::string>{"git"})));
    Decision d = outer.evaluate(CommandRequest{"rm", {}, ""});
    CHECK(d.kind == Decision::Kind::Deny);
    CHECK_EQ(d.policy, std::string("allowlist"));
    d = outer.evaluate(CommandRequest{"git", {"status"}, ""});
    CHECK(d.kind == Decision::Kind::Ask);
    CHECK_EQ(d.policy, std::string("confirm"));
    d = outer.evaluate(CommandRequest{"ls", {}, ""});
    CHECK(d.kind == Decision::Kind::Allow);
    CHECK_EQ(d.policy, std::string("chain"));
}

// =========================================================================
// Skills — 2 examples
// =========================================================================

// Example 7 (graph): post-order load order, and a cycle message that lists
// only the nodes still on the DFS path (b is already finished).
TEST(graph_order_and_cycle_message)
{
    DepGraph g;
    for (const char *n : {"a", "b", "c", "d"})
        g.add_node(n);
    g.add_edge("a", "b"); // a requires b
    g.add_edge("a", "c");
    g.add_edge("b", "d");
    g.add_edge("c", "d"); // diamond: d is reached twice, but it is not a cycle
    std::vector<std::string> order;
    std::string error;
    REQUIRE(g.order_from("a", &order, &error));
    CHECK_EQ(order, (std::vector<std::string>{"d", "b", "c", "a"}));

    g.add_edge("c", "a"); // now a -> c -> a is a cycle
    CHECK(!g.order_from("a", &order, &error));
    CHECK_EQ(error, std::string("cycle: a -> c -> a"));
}

// Example 8 (index): rank by number of distinct query words; ties go to the
// smaller doc id.
TEST(index_ranking_and_ties)
{
    SkillIndex ix;
    ix.add(0, "format numbers");
    ix.add(1, "csv report with table and numbers");
    ix.add(2, "table format");
    ix.add(3, "unrelated");
    CHECK_EQ(ix.search("csv table numbers", 10), (std::vector<std::size_t>{1, 0, 2}));
    CHECK_EQ(ix.search("format table", 1), (std::vector<std::size_t>{2}));
    REQUIRE(ix.postings("numbers") != nullptr);
    CHECK_EQ(*ix.postings("numbers"), (std::vector<std::size_t>{0, 1}));
}

// =========================================================================
// Your tests (at least 12 more). Spec §8 lists what they must cover.
// =========================================================================

int main(int argc, char **argv) { return mt::run_all(argc, argv); }
