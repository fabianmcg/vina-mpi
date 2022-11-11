//============================================================================
// Name        : string_serializer.hh
// Author      : Fabian Mora
// Version     : 1.0
// License     : MIT license
// Description :
//============================================================================

#ifndef __STRING_SERIALIZER_HH__
#define __STRING_SERIALIZER_HH__

#include <string>
#include <unordered_map>
#include <tuple>
#include <vector>
#include "../definitions.hh"

namespace MPIBatch {
class StringSerializer {
public:
    using data_t = typename std::string::value_type;
    static constexpr MPI_Datatype mpi_data_type = MPI_CHAR;
private:
    std::unordered_map<int64_t, std::string>* __data__ = nullptr;
    int64_t __next_slot__ = 0;
public:
    StringSerializer();
    ~StringSerializer();
    StringSerializer(StringSerializer &&other);
    StringSerializer& operator=(StringSerializer &&other);

    std::tuple <void*, int, MPI_Datatype, int64_t> operator()(std::string data);

    void free(const std::tuple <void*, int, MPI_Datatype, int64_t>& data);
    void free(int64_t id);
    MPI_Datatype mpi_type() const;
    std::tuple <void*, int, MPI_Datatype, int64_t> serialize(std::string data);
    std::string deserialize(std::vector <data_t> &buffer);
};

}
#endif

