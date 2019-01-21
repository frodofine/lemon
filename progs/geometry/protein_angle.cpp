#include <iostream>
#include <sstream>
#include <unordered_map>

#include "lemon/lemon.hpp"

// typedefs for binned data
typedef std::pair<std::string, size_t> BondAngleBin;
typedef std::map<BondAngleBin, size_t> AngleCounts;

using lemon::geometry::protein::angle_name;

int main(int argc, char* argv[]) {
    lemon::Options o;
    auto bin_size = 0.01;
    o.add_option("bin_size,b", bin_size, "Size of the angle bin.");
    o.parse_command_line(argc, argv);

    auto worker = [bin_size](chemfiles::Frame complex, const std::string&) {
        AngleCounts bins;

        // Selection phase
        chemfiles::Frame protein_only;
        std::list<size_t> peptides;

        if (lemon::select::specific_residues(complex, peptides,
                                             lemon::common_peptides) == 0) {
            return bins;
        }

        lemon::separate::residues(complex, peptides, protein_only);

        const auto& angles = protein_only.topology().angles();

        for (const auto& angle : angles) {
            auto anglenm = angle_name(protein_only, angle);

            auto theta = protein_only.angle(angle[0], angle[1], angle[2]);
            size_t bin = static_cast<size_t>(std::floor(theta / bin_size));

            BondAngleBin sbin = {anglenm, bin};
            auto bin_iterator = bins.find(sbin);

            if (bin_iterator == bins.end()) {
                bins[sbin] = 1;
                continue;
            }

            ++(bin_iterator->second);
        }

        return bins;
    };

    AngleCounts sc_total;
    lemon::launch<lemon::map_combine>(o, worker, sc_total);

    for (const auto& i : sc_total) {
        std::cout << i.first.first << "\t"
                  << static_cast<double>(i.first.second) * bin_size << "\t"
                  << i.second << "\n";
    }

    return 0;
}
