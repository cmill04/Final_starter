#include "core/conversation.h"
#include <stdexcept>

Conversation::Conversation() : data_(nullptr), size_(0), capacity_(0) {}

Conversation::~Conversation() {
    delete[] data_;
}

Conversation::Conversation(const Conversation& other) :
data_(new Message[other.capacity_]), size_(other.size_), capacity_(other.capacity_)
{
    for (std::size_t i=0; i <size_; i++){
        data_[i] = other.data_[i];
    }
}

Conversation& Conversation::operator=(const Conversation& other)
{
    if (this == &other) return *this;
    Message* new_data_ = new Message[other.capacity_];  //creates the copied array
    for (std::size_t i = 0; i < other.size_; i++){  //copies the data over
        new_data_[i] = other.data_[i];
    }

    delete[] data_;
    data_ = new_data_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    return *this;
}

Conversation::Conversation(Conversation&& other) noexcept :
data_(other.data_), size_(other.size_), capacity_(other.capacity_)
{
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
}

Conversation& Conversation::operator=(Conversation&& other) noexcept
{
    if (this == &other) return *this;
    delete [] data_;

    //copying it over
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;

    //clearing what was used to copy
    other.data_ = nullptr;
    other.size_ =0;
    other.capacity_=0;
    return *this;

}

void Conversation::append(Message m){
    if (capacity_ == size_){
        std::size_t new_cap = (capacity_== 0) ? 1 : capacity_ * 2;
        Message* new_data_ = new Message[new_cap];
        for (std::size_t i = 0; i < size_; i++){
            new_data_[i] = data_[i];
        }

        delete [] data_;
        data_ = new_data_;
        capacity_= new_cap;

    }

    data_[size_] = m;
    size_++;

}

std::size_t Conversation::size() const noexcept {return size_;}

const Message& Conversation::at(std::size_t i) const{
    if (i < size_){
        return data_[i];
    }
    else {
        throw std::out_of_range("Index out of bounds");
    }
}

const Message* Conversation::begin() const noexcept {return data_;}
const Message* Conversation::end()   const noexcept {return data_ + size_;}