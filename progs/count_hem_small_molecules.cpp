#include <iostream>
#include <sstream>

#include <boost/filesystem.hpp>

#include <chemfiles.hpp>

#include "lemon/entries.hpp"
#include "lemon/parse.hpp"
#include "lemon/prune.hpp"
#include "lemon/run.hpp"
#include "lemon/select.hpp"

using namespace boost::filesystem;

int main(int argc, char* argv[]) {
    path entries(argc > 1 ? argv[1] : "entries.idx");
    path p(argc > 2 ? argv[2] : ".");
    double dist_cutoff = argc > 3 ? std::strtod(argv[3], nullptr) : 6.0;
    size_t ncpu = argc > 4 ? std::strtoul(argv[4], nullptr, 0) : 1;
    size_t chun = argc > 5 ? std::strtoul(argv[5], nullptr, 0) : 1;

    if (!is_regular_file(entries)) {
        std::cerr << "You must supply a valid entries file" << std::endl;
        return 1;
    }

    if (!is_directory(p)) {
        std::cerr << "You must supply a valid directory" << std::endl;
        return 2;
    }

    std::vector<std::array<char, 4>> vec;
    lemon::read_entry_file(entries.string(), vec);

    auto worker = [dist_cutoff](const chemfiles::Frame& complex,
                                const std::string& pdbid) {

        auto hemegs = lemon::select_specific_residues(
            complex, {"HEM", "HEA", "HEB", "HEC"});
        auto smallm = lemon::select_small_molecule(complex);
        lemon::remove_identical_residues(complex, smallm);
        lemon::remove_common_cofactors(complex, smallm);
        lemon::find_interactions(complex, smallm, hemegs, dist_cutoff);

        if (smallm.empty()) {
            return;
        }

        std::unordered_map<std::string, size_t> small_molecules;
        const auto& residues = complex.topology().residues();
        for (auto res_id : smallm) {
            auto iter = small_molecules.find(residues[res_id].name());
            if (iter == small_molecules.end()) {
                small_molecules[residues[res_id].name()] = 1;
                continue;
            }
            ++iter->second;
        }

        std::stringstream ss;
        ss << pdbid;
        for (const auto iter : small_molecules) {
            ss << " " << iter.first << " " << iter.second;
        }
        ss << "\n";

        std::cout << ss.str();
    };

    current_path(p);
    lemon::call_multithreaded(worker, vec, ncpu, chun);
}