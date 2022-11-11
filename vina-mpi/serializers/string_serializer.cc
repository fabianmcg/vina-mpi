//============================================================================
// Name        : string_serializer.cc
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================
#include "string_serializer.hh"

namespace MPIBatch {
//constexpr MPI_Datatype StringSerializer::mpi_data_type = MPI_CHAR;

StringSerializer::StringSerializer() :
        __data__(new std::unordered_map <int64_t, std::string>()) {
}

StringSerializer::~StringSerializer() {
    if (__data__) {
        __data__->clear();
        delete __data__;
    }
    __next_slot__ = -1;
}

StringSerializer::StringSerializer(StringSerializer &&other) :
        __data__(other.__data__) {
    __next_slot__ = other.__next_slot__;
    other.__next_slot__ = -1;
    other.__data__ = nullptr;
}

StringSerializer& StringSerializer::operator=(StringSerializer &&other) {
    __data__ = other.__data__;
    __next_slot__ = other.__next_slot__;
    other.__next_slot__ = -1;
    other.__data__ = nullptr;
    return *this;
}

std::tuple <void*, int, MPI_Datatype, int64_t> StringSerializer::operator()(std::string data) {
    return serialize(data);
}

void StringSerializer::free(const std::tuple <void*, int, MPI_Datatype, int64_t> &data) {
    auto it = __data__->find(std::get <3>(data));
    if (it != __data__->end()) {
        __data__->erase(it);
    }
}

void StringSerializer::free(int64_t id) {
    auto it = __data__->find(id);
    if (it != __data__->end()) {
        __data__->erase(it);
    }
}

MPI_Datatype StringSerializer::mpi_type() const {
    return mpi_data_type;
}

std::tuple <void*, int, MPI_Datatype, int64_t> StringSerializer::serialize(std::string data) {
    int64_t id = __next_slot__;
    (*__data__)[id] = data;
    void *ptr = data.empty() ? nullptr : reinterpret_cast <void*>((*__data__)[id].data());
    return std::tuple <void*, int, MPI_Datatype, int64_t>(ptr, data.size(), mpi_data_type, id);
}

std::string StringSerializer::deserialize(std::vector <data_t> &buffer) {
    std::string result = std::string(buffer.size(), ' ');
    for (size_t i = 0; i < buffer.size(); ++i)
        result[i] = buffer[i];
    return result;
}
}
