// src/policy/policies.cpp — YOU IMPLEMENT. Spec §5.2.

#include "policy/policies.h"

#include <utility>

AllowlistPolicy::AllowlistPolicy(const std::vector<std::string> &programs) : programs_(programs.begin(), programs.end())
{
}

Decision AllowlistPolicy::evaluate(const CommandRequest &req)
{

    if (programs_.count(req.program) > 0)
    {
        return Decision::allow("Program is found", name());
    }
    else
        return Decision::deny("Program not found", name());
}

ArgumentPatternPolicy::ArgumentPatternPolicy(std::vector<std::string> patterns)
    : patterns_(std::move(patterns)) {}

Decision ArgumentPatternPolicy::evaluate(const CommandRequest &req)
{
    std::string line = req.command_line();
    for (const std::string &p : patterns_)
    {
        if (p.empty())
        {
            continue;
        }
        if (line.find(p) != std::string::npos)
        {
            return Decision::deny("forbidden pattern", name());
        }
    }
    return Decision::allow("no pattern matched", name());
}

ConfirmPolicy::ConfirmPolicy(const std::vector<std::string> &programs)
    : programs_(programs.begin(), programs.end())
{
}

Decision ConfirmPolicy::evaluate(const CommandRequest &req)
{
  if (programs_.count(req.program) > 0){
    return Decision::ask("check listed programs", name());
  }
  else return Decision::allow("Not listed program", name());
}

RateLimitPolicy::RateLimitPolicy(std::size_t max_commands, double window_seconds, const Clock &clock)
    : max_(max_commands), window_(window_seconds), clock_(clock) {}

Decision RateLimitPolicy::evaluate(const CommandRequest &req)
{
    (void) req;
   double now = clock_.now_seconds();
   while(!times_.empty() && (now -times_.front() >= window_)){
        times_.pop();
   }
   if (times_.size() >=max_){
    return Decision::deny("Rate limit reached", name());
   }
   times_.push(now);
   return Decision::allow("Within rate limit", name());
}

std::size_t RateLimitPolicy::tracked() const noexcept { return times_.size(); }
