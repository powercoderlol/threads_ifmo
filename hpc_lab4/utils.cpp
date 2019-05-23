#include "utils.hpp"

void write_large_file_clusters(
    std::string filename, const points<double>& clusters) {
    FILE* pfile = fopen(filename.c_str(), "w");
    setvbuf(pfile, NULL, _IONBF, 0);
    fwrite(&clusters, sizeof(points<double>), clusters.size(), pfile);
    fclose(pfile);
}

//
// http://cs.joensuu.fi/sipu/datasets/
// Synthetic 2-d data with N=5000 vectors and k=15 Gaussian clusters
// with different degree of cluster overlap
//
void read_dataset_v1(std::string filename, points<double>& storage) {
    std::fstream dataset_file(filename);
    double x, y;
    while(dataset_file) {
        dataset_file >> x >> y;
        storage.push_back(std::make_pair(x, y));
    }
    dataset_file.close();
}

void serialize_to_python(
    const points<double>& storage, const std::vector<size_t>& assignments,
    size_t cluster_number, std::string filename) {
    std::ofstream file;
    file.rdbuf()->pubsetbuf(0, 0);
    file.open(filename);

    file << '{' << "\"cluster_size\": " << cluster_number << ", \"data\": [";

    size_t loop = assignments.size();
    bool first = false;
    for(size_t i = 0; i < loop; ++i) {
        if(first)
            file << ", ";
        else
            first = true;
        file << storage[i].first << ", " << storage[i].second << ", "
             << assignments[i];
    }
    file << "]}";
    file.close();
}
