#include "benchmarker/prune.hpp"
#include <chemfiles.hpp>
#include "benchmarker/select.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("Select and prune P30 from 4XUF") {
    auto traj = chemfiles::Trajectory("files/4XUF.mmtf.gz", 'r');
    auto frame = traj.read();

    auto res = benchmarker::select_small_molecule(frame);
    CHECK(res.size() == 2);

    benchmarker::remove_identical_residues(frame, res);
    CHECK(res.size() == 1);
}

TEST_CASE("Select and prune 1PG from 1D7D") {
    auto traj = chemfiles::Trajectory("files/1D7D.mmtf.gz", 'r');
    auto frame = traj.read();

    auto res = benchmarker::select_small_molecule(frame);
    CHECK(res.size() == 3);  // Two hemes and 1PG

    benchmarker::remove_common_cofactors(frame, res);
    CHECK(res.size() == 1);
}

TEST_CASE("Remove non-nucleic acid interactions") {
    auto traj = chemfiles::Trajectory("files/4XUF.mmtf.gz", 'r');
    auto frame = traj.read();

    auto res = benchmarker::select_small_molecule(frame);
    benchmarker::find_nucleic_acid_interactions(frame, res);
    CHECK(res.size() == 0);  // It got removed

    traj = chemfiles::Trajectory("files/entry_10/1/0/100D.mmtf.gz", 'r');
    frame = traj.read();

    res = benchmarker::select_small_molecule(frame);
    benchmarker::find_nucleic_acid_interactions(frame, res);
    CHECK(res.size() == 1);  // Not removed
}
