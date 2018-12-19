#ifndef LEMON_LAUNCH_HPP
#define LEMON_LAUNCH_HPP

#include "lemon/options.hpp"
#include "lemon/parallel.hpp"

namespace lemon {

template<typename Map1, typename Map2>
struct map_combine {
    void operator()(Map1& map1, const Map2& map2) const {
        for (const auto& sc : map2) {
            map1[sc.first] += sc.second;
        }
    }
};

template<typename OS, typename Map>
struct print_combine {
    void operator()(OS& os, const Map& map) const {
        os << map;
    }
};

template< template<typename C, typename R> class Combiner, typename Function,
         typename Collector>
int launch(const Options& o, Function&& worker, Collector& collect) {
    auto p = o.work_dir();
    auto threads = o.ncpu();
    auto entries = read_entry_file(o.entries());

    using ret = typename std::result_of<Function&(chemfiles::Frame,
                                                  const std::string&)>::type;
    Combiner<Collector, ret> combiner;

    try {
        lemon::run_parallel(worker, combiner, p, collect, threads, entries);
    } catch(std::runtime_error& e){
        std::cerr << e.what() << "\n";
        return 1;
    }
}

}; // namespace lemon

#endif
