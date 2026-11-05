#pragma once
 
#include <string>
#include <string_view>
 
class SentinelScanner {
public:
    explicit SentinelScanner(std::string sentinel):
    sentinel_(sentinel) {}
 
    struct Out { std::string safe_text; bool sentinel_found; };
    Out feed(std::string_view chunk);
    Out flush();


    //only used for the bounds testing 
    std::size_t pending_size() const noexcept {
         return pending_.size();}
 
private:
    std::string sentinel_;
    std::string pending_;
};