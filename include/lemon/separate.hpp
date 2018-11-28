#ifndef LEMON_SEPARATE_HPP
#define LEMON_SEPARATE_HPP

#include <set>
#include <unordered_set>

#include "chemfiles/Topology.hpp"
#include "chemfiles/Frame.hpp"

namespace lemon {

inline void separate_residues(const chemfiles::Frame& input,
                              const std::set<size_t>& accepted_residues,
                              chemfiles::Frame& new_frame) {

    const auto& residues  = input.topology().residues();
    const auto& positions = input.positions();
    const auto& old_bonds = input.topology().bonds();
    const auto& bond_ord  = input.topology().bond_orders();

    std::unordered_map<size_t, size_t> old_to_new;
    std::unordered_set<size_t> accepted_atoms;
    for (auto res_id : accepted_residues) {
        const auto& res = residues[res_id];

        auto res_new = chemfiles::Residue(res.name(), *(res.id()));

        for (size_t res_atom : res) {
            new_frame.add_atom(input[res_atom], positions[res_atom]);
            res_new.add_atom(new_frame.size() - 1);
            old_to_new.insert({res_atom, new_frame.size() - 1});
            accepted_atoms.insert(res_atom);
        }

        res_new.set("chainid", res.get("chainid")->as_string());
        new_frame.add_residue(std::move(res_new));
    }

    for (size_t bond_idx = 0; bond_idx < old_bonds.size(); ++bond_idx) {
        if (accepted_atoms.count(old_bonds[bond_idx][0]) &&
            accepted_atoms.count(old_bonds[bond_idx][1])) {

            new_frame.add_bond(old_to_new[old_bonds[bond_idx][0]],
                    old_to_new[old_bonds[bond_idx][1]],
                    bond_ord[bond_idx]);
        }
    }
}

inline void separate_protein_and_ligand(const chemfiles::Frame& input,
                                 size_t ligand_id,
                                 chemfiles::Frame& protein,
                                 chemfiles::Frame& ligand, double pocket_size) {
    const auto& topo = input.topology();
    const auto& positions = input.positions();
    const auto& residues = topo.residues();
    const auto& ligand_residue = residues[ligand_id];

    std::set<size_t> accepted_residues;
    for (size_t res_id = 0; res_id < residues.size(); ++res_id) {
        if (res_id == ligand_id) {
            continue;
        }

        const auto& res = residues[res_id];
        for (auto prot_atom : res) {
            for (auto lig_atom : ligand_residue) {
                if (input.distance(prot_atom, lig_atom) < pocket_size) {
                    accepted_residues.insert(res_id);
                    goto found_interaction;
                }
            }
        }
        found_interaction:;
    }

    separate_residues(input, accepted_residues, protein);
    separate_residues(input, {ligand_id}, ligand);

    ligand.set("name", ligand_residue.name());
}
}

#endif
