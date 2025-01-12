#include <mpi.h>
#include <iostream>
#include <vector>
#include <numeric>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Example: each process has a different number of values
    std::vector<int> local_data;
    if (rank == 0) local_data = {1, 2, 3};
    if (rank == 1) local_data = {4, 5};
    if (rank == 2) local_data = {6, 7, 8, 9};

    // Step 1: Compute local prefix sum
    std::vector<int> local_prefix_sum(local_data.size());
    std::partial_sum(local_data.begin(), local_data.end(), local_prefix_sum.begin());

    // Step 2: Gather local sums at root
    int local_sum = local_prefix_sum.back();
    std::vector<int> all_local_sums(size);
    MPI_Gather(&local_sum, 1, MPI_INT, all_local_sums.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Step 3: Compute offsets
    std::vector<int> offsets(size, 0);
    if (rank == 0) {
        std::partial_sum(all_local_sums.begin(), all_local_sums.end(), offsets.begin());
        offsets.insert(offsets.begin(), 0);
        offsets.pop_back();
    }

    // Broadcast offsets to all processes
    int my_offset = 0;
    MPI_Scatter(offsets.data(), 1, MPI_INT, &my_offset, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Step 4: Adjust local prefix sums with offset
    for (auto& value : local_prefix_sum) {
        value += my_offset;
    }

    // Step 5: Print results
    for (int proc = 0; proc < size; ++proc) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == proc) {
            std::cout << "Rank " << rank << ": ";
            for (const auto& value : local_prefix_sum) {
                std::cout << value << " ";
            }
            std::cout << std::endl;
        }
    }

    MPI_Finalize();
    return 0;
}
