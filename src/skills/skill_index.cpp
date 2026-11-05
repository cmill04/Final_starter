// src/skills/skill_index.cpp — YOU IMPLEMENT. Spec §6.3.

#include "skills/skill_index.h"
#include "util/min_heap.h"
#include "util/text.h"

void SkillIndex::add(std::size_t doc_id, std::string_view text) {
    // TODO
    // Clue: use unique_tokens(text), not tokenize(text): a word that appears
    // twice in one document must post its doc_id only once.
    (void)doc_id;
    (void)text;
}

const std::vector<std::size_t>* SkillIndex::postings(std::string_view word) const {
    // TODO
    // Clue: the map's keys are std::string, so find(word) with a
    // string_view does not compile in C++17: use find(std::string(word)).
    // Return the address of the vector inside the map (&it->second).
    (void)word;
    return nullptr;
}

std::size_t SkillIndex::word_count() const noexcept {
    // TODO
    return 0;
}

std::vector<std::size_t> SkillIndex::search(std::string_view query, std::size_t k) const {
    // TODO: score documents using only the posting lists of the query's
    // words, then rank with top_k().
    // Clues:
    //  * Count with  std::unordered_map<std::size_t, std::size_t> score;
    //    walking the posting list of each word of unique_tokens(query)
    //    (a repeated query word counts once).
    //  * Put (score, doc_id) pairs in a vector and call top_k with a
    //    comparator meaning "a is WORSE than b": lower score is worse; with
    //    equal scores the LARGER doc_id is worse.
    //  * Don't copy the lambda in skill_index_linear.cpp: that one is a
    //    std::sort order ("a comes first"), the opposite direction.
    (void)query;
    (void)k;
    return {};
}
