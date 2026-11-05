#include "core/sentinel_scanner.h"

SentinelScanner::Out SentinelScanner::feed(std::string_view chunk) 
{
    std::string safe_text; 
    pending_.append(chunk); 
    if(pending_.find(sentinel_)==std::string::npos){
        if(pending_.size()<(sentinel_.size()-1)){
          pending_ =  pending_.substr(0,sentinel_.size()-1);
         
        }
        else {
        safe_text = pending_.substr(0, pending_.size()-(sentinel_.size()-1));
       pending_ = pending_.substr(pending_.size()-(sentinel_.size()-1),sentinel_.size()-1);
        }
         return Out{safe_text, false};
    }
        else{

            safe_text= pending_.substr(0,pending_.find(sentinel_));
            pending_.clear();
            return Out{safe_text, true}; 
        }
    

}

SentinelScanner::Out SentinelScanner::flush() 
{
    std::string safe_text;
    safe_text = pending_; 
    pending_.clear();
    return Out {safe_text, false};
}