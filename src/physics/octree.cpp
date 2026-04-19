#include "physics/octree.hpp"
#include "constants.hpp"
#include "ephemeris.hpp"
#include <algorithm>
#include <cmath>

physics::Octree::Node::Node(double x, double y, double z, double w) 
    : x(x), y(y), z(z), 
    width(w),
    body_idx(-1),
    center_x(x + width / 2), center_y(y + width / 2), center_z(z + width / 2) {
    for (int i = 0; i < children.size(); i++) {
        children[i] = -1;
    }
}

inline bool physics::Octree::Node::isLeaf() const {
    return children[0] == -1;
}

inline bool physics::Octree::Node::isEmpty() const {
    return body_idx == -1;
}

physics::Octree::Octree(physics::Ephemeris *eph) : eph(eph) {}

void physics::Octree::divide(int node_idx) {
    Node &node = nodes[node_idx];
    double x = node.x;
    double y = node.y;
    double z = node.z;
    double half_width = node.width / 2.0;

    node.children[0] = allocNode(x, y, z, half_width);
    node.children[1] = allocNode(x + half_width, y, z, half_width);
    node.children[2] = allocNode(x, y + half_width, z, half_width);
    node.children[3] = allocNode(x + half_width, y + half_width, z, half_width);
    node.children[4] = allocNode(x, y, z + half_width, half_width);
    node.children[5] = allocNode(x + half_width, y, z + half_width, half_width);
    node.children[6] = allocNode(x, y + half_width, z + half_width, half_width);
    node.children[7] = allocNode(x + half_width, y + half_width, z + half_width, half_width);
}

int physics::Octree::allocNode(double x, double y, double z, double width) {
    nodes.emplace_back(x, y, z, width);
    return nodes.size() - 1;
}

// returns 3 bits representing quadrants
// 0bXXX
//   ^^^
//   |||-X: 0 for far left, 1 for far right
//   ||--Y: 0 for top, 1 for bottom
//   |---Z: 0 for far, 1 for close
//
int physics::Octree::getOctant(int node_idx, int b_idx) {
    Node &node = nodes[node_idx];
    int octant = 0b000;

    if (eph->x[b_idx] >= node.center_x) octant |= 0b001;
    if (eph->y[b_idx] >= node.center_y) octant |= 0b010;
    if (eph->z[b_idx] >= node.center_z) octant |= 0b100;

    return octant;
}

void physics::Octree::build() {
    nodes.reserve(eph->n * 8);

    double size = std::max({
        eph->max_x - eph->min_x, 
        eph->max_y - eph->min_y, 
        eph->max_z - eph->min_z
    });
    root_idx = allocNode(eph->min_x, eph->min_y, eph->min_z, size);

    for (int i = 0; i < eph->n; ++i) {
        insert(i);
    }
}

void physics::Octree::insert(int b_idx) {
    insert(root_idx, b_idx);
}

void physics::Octree::insert(int node_idx, int b_idx) {
    Node &node = nodes[node_idx];
    // empty
    if (node.isEmpty() && node.isLeaf()) {
        node.body_idx = b_idx;
        return;
    }

    // node is leaf, but not empty
    if (node.isLeaf()) {
        int existing_idx = node.body_idx; 

        if (node.width < 1e-10) {
            return;
        }

        divide(node_idx);

        // divide call ^ may reallocate nodes so we gotta refresh the ref

        // its empty now 
        nodes[node_idx].body_idx = -1;

        // re-insert old particle
        int oct_old = getOctant(node_idx, existing_idx);
        insert(nodes[node_idx].children[oct_old], existing_idx);
    }
    int oct = getOctant(node_idx, b_idx);
    insert(nodes[node_idx].children[oct], b_idx);
}

void physics::Octree::computeMass() {
    computeMass(root_idx);
}

void physics::Octree::computeMass(int node_idx) {
    if (node_idx == -1) {
        return;
    }

    Node &node = nodes[node_idx];

    // base case
    if (node.isLeaf()) {
        if (node.body_idx != -1) {
            int idx = node.body_idx;
            node.total_mass = eph->mass[idx];
            node.com_x = eph->x[idx];
            node.com_y = eph->y[idx];
            node.com_z = eph->z[idx];
        } else {
            node.total_mass = 0;
        }

        return;
    }

    double mass_sum = 0;
    double weighted_x = 0, weighted_y = 0, weighted_z = 0;

    for (int i = 0; i < 8; ++i) {
        int child_idx = node.children[i];

        if (child_idx != -1) {
            computeMass(child_idx);
            Node &child = nodes[child_idx];
            double m = child.total_mass;
            mass_sum += m;
            weighted_x += child.com_x * m;
            weighted_y += child.com_y * m;
            weighted_z += child.com_z * m;
        }
    }

    node.total_mass = mass_sum;

    if (mass_sum > 0) {
        node.com_x = weighted_x / mass_sum;
        node.com_y = weighted_y / mass_sum;
        node.com_z = weighted_z / mass_sum;
    }
}

void physics::Octree::computeNetForce(int b_idx, double theta) {
    computeNetForce(root_idx, b_idx, theta);
}

void physics::Octree::computeNetForce(int node_idx, int b_idx, double theta) {
    double jx, jy, jz;
    Node &node = nodes[node_idx];

    if (node.isLeaf()) {
        if (node.body_idx == -1 || node.body_idx == b_idx) return;
        jx = eph->x[node.body_idx];
        jy = eph->y[node.body_idx];
        jz = eph->z[node.body_idx];
    } else {
        jx = node.com_x;
        jy = node.com_y;
        jz = node.com_z;
    }

    double dx = jx - eph->x[b_idx];
    double dy = jy - eph->y[b_idx];
    double dz = jz - eph->z[b_idx];

    double dist_sq = dx * dx + dy * dy + dz * dz + SOFTENING;
    double curr_theta = node.width / sqrt(dist_sq);

    // node is far enough
    if (curr_theta <= theta || node.isLeaf()) {
        double inv_dist = 1.0 / sqrt(dist_sq);
        double inv_dist_cub = inv_dist * inv_dist * inv_dist;
        double common_factor = G * node.total_mass * inv_dist_cub;

        eph->ax[b_idx] += dx * common_factor;
        eph->ay[b_idx] += dy * common_factor;
        eph->az[b_idx] += dz * common_factor;
    } else { // node is too close
        for (int i = 0; i < 8; ++i) {
            int child_idx = node.children[i];
            if (child_idx != -1) {
                computeNetForce(child_idx, b_idx, theta);
            }
        }
    }
}
