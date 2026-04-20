#pragma once

#include "ephemeris.hpp"
#include <array>
#include <cstddef>
#include <vector>

namespace physics {

struct Octree {

    struct Node {
        // top left back
        double x, y, z, width;
        double total_mass;
        double com_x, com_y, com_z;

        std::array<int, 8> children;
        int body_idx = -1; 

        Node(double x, double y, double z, double w);

        inline bool isLeaf() const;
        inline bool isEmpty() const;
    };

    std::vector<Node> nodes;
    int root_idx;

    Ephemeris *eph;

    Octree(Ephemeris *eph);

    void divide(int node_idx);
    int allocNode(double x, double y, double z, double width);
    int getOctant(int node_idx, int b_idx);
    void build();
    void insert(int b_idx);
    void insert(int node_idx, int b_idx);
    void computeMass();
    void computeMass(int node_idx);
    void computeAccel(int b_idx, double theta);
    void computeAccel(int node_idx, int b_idx, double theta);
    void computeAccelIt(int b_idx, double theta);
    void reset();
};

}
