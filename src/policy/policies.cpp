// src/policy/policies.cpp — YOU IMPLEMENT. Spec §5.2.

#include "policy/policies.h"

#include <utility>

AllowlistPolicy::AllowlistPolicy(const std::vector<std::string>& programs) {
    // TODO: copy the names into programs_ (an std::unordered_set).
    // Clue: sets use insert(x), not push_back(x). One-liner: in the member
    // initializer list,  programs_(programs.begin(), programs.end()).
    (void)programs;
}

Decision AllowlistPolicy::evaluate(const CommandRequest& req) {
    // TODO
    // Clue: test membership with  programs_.count(x) > 0  or
    // programs_.find(x) != programs_.end().  (contains() is C++20; this
    // project is C++17.)  Build results with Decision::allow(reason, name())
    // and Decision::deny(reason, name()). Any non-empty reason is fine;
    // naming the program ("'rm' is not on the allowlist") helps the user.
    (void)req;
    return Decision::deny("not implemented", name());
}

ArgumentPatternPolicy::ArgumentPatternPolicy(std::vector<std::string> patterns)
    : patterns_(std::move(patterns)) {}

Decision ArgumentPatternPolicy::evaluate(const CommandRequest& req) {
    // TODO: search req.command_line() for each pattern.
    // Clue: std::string::find returns std::string::npos when it finds
    // nothing, NOT 0 or false. "if (line.find(p))" is true for almost
    // everything. Compare with npos.
    // Clue: skip empty patterns: every string "contains" the empty string.
    (void)req;
    return Decision::deny("not implemented", name());
}

ConfirmPolicy::ConfirmPolicy(const std::vector<std::string>& programs) {
    // TODO
    (void)programs;
}

Decision ConfirmPolicy::evaluate(const CommandRequest& req) {
    // TODO: Decision::ask(...) for listed programs, Decision::allow(...) otherwise.
    (void)req;
    return Decision::deny("not implemented", name());
}

RateLimitPolicy::RateLimitPolicy(std::size_t max_commands, double window_seconds, const Clock& clock)
    : max_(max_commands), window_(window_seconds), clock_(clock) {}

Decision RateLimitPolicy::evaluate(const CommandRequest& req) {
    // TODO: drop timestamps that left the window, then decide.
    // Clues, in order:
    //  1. now = clock_.now_seconds().
    //  2. While the queue is NOT empty and its front timestamp has left the
    //     window, pop it. A timestamp s is still INSIDE when now - s < window_,
    //     so it has left when now - s >= window_ (exactly window_ seconds old
    //     counts as gone). Calling front() on an empty queue is undefined
    //     behavior (UBSan says "misaligned address"): check empty() first.
    //  3. If the queue already holds max_ timestamps: Deny, and do NOT push.
    //  4. Otherwise push(now) and Allow. Only allowed requests are recorded.
    (void)req;
    (void)max_;
    (void)window_;
    (void)clock_;
    return Decision::deny("not implemented", name());
}

std::size_t RateLimitPolicy::tracked() const noexcept { return times_.size(); }
