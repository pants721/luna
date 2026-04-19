#pragma once

#include "ephemeris.hpp"
#include <array>
#include <cstddef>
#include <vector>

namespace physics {

struct Octree {

    struct Node {
        std::array<int, 8> children;
        int body_idx = -1; 

        // top left back
        double x, y, z;
        double width;
        double center_x, center_y, center_z;

        double total_mass;
        double com_x, com_y, com_z;

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
    void computeNetForce(int b_idx, double theta);
    void computeNetForce(int node_idx, int b_idx, double theta);
    void reset();
};

}
