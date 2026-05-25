#include "solvers/barnes_hut_tbb.hpp"
#include "ephemeris.hpp"
#include "octree.hpp"

#include <tbb/tbb.h>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>
#include <limits>


void solvers::BarnesHutTBB::resetAccel(physics::Ephemeris &s) {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, s.n),
                      [&](const tbb::blocked_range<size_t> &r) {
                          for (size_t i = r.begin(); i != r.end(); ++i) {
                              s.ax[i] = 0.0;
                              s.ay[i] = 0.0;
                              s.az[i] = 0.0;
                          }
                      }
                      );
}

void solvers::BarnesHutTBB::computeBounds(physics::Ephemeris &s) {
    struct BoundsReducer {
        double max_x = -std::numeric_limits<double>::infinity();
        double max_y = -std::numeric_limits<double>::infinity();
        double max_z = -std::numeric_limits<double>::infinity();
        double min_x = std::numeric_limits<double>::infinity();
        double min_y = std::numeric_limits<double>::infinity();
        double min_z = std::numeric_limits<double>::infinity();

        const physics::Ephemeris& s;

        BoundsReducer(const physics::Ephemeris& s) : s(s) {}

        BoundsReducer(const BoundsReducer& other, tbb::split) 
        : s(other.s) {}

        void operator()(const tbb::blocked_range<size_t>& range) {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                double x = s.x[i];
                double y = s.y[i];
                double z = s.z[i];
                max_x = std::max(max_x, x);
                max_y = std::max(max_y, y);
                max_z = std::max(max_z, z);
                min_x = std::min(min_x, x);
                min_y = std::min(min_y, y);
                min_z = std::min(min_z, z);
            }
        }

        void join(const BoundsReducer& other) {
            max_x = std::max(max_x, other.max_x);
            max_y = std::max(max_y, other.max_y);
            max_z = std::max(max_z, other.max_z);
            min_x = std::min(min_x, other.min_x);
            min_y = std::min(min_y, other.min_y);
            min_z = std::min(min_z, other.min_z);
        }
    };

    BoundsReducer reducer(s);
    tbb::parallel_reduce(tbb::blocked_range<size_t>(0, s.n), reducer);

    s.max_x = reducer.max_x;
    s.max_y = reducer.max_y;
    s.max_z = reducer.max_z;
    s.min_x = reducer.min_x;
    s.min_y = reducer.min_y;
    s.min_z = reducer.min_z;
}

void solvers::BarnesHutTBB::computeAccel(physics::Ephemeris &s) {
    physics::Octree tree = physics::Octree(&s);
    tree.build();
    tree.computeMass();
    resetAccel(s);
    
    tbb::parallel_for(tbb::blocked_range<size_t>(0, s.n),
        [this, &tree](const tbb::blocked_range<size_t>& range) {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                tree.computeAccelIt(i, theta);
            }
        });
}
