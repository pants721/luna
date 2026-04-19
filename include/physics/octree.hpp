#pragma once

#include "ephemeris.hpp"
#include <array>
#include <cstddef>
#include <vector>

namespace physics {

struct Octree {

    struct Node {
        std::array<Node *, 8> children = {nullptr};
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
        void divide();
    };

    Node *root;
    Ephemeris *eph;

    Octree(Ephemeris *eph);

    int getOctant(Node *node, int b_idx);
    void build();
    void clear();
    void clear(Node *node);
    void insert(int b_idx);
    void insert(Node *node, int b_idx);
    void computeMass();
    void computeMass(Node *node);
    void computeNetForce(int b_idx, double theta);
    void computeNetForce(Node *node, int b_idx, double theta);
    void reset();
};

}
