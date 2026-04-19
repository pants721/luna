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
    for (int i = 0; i < 8; i++) {
        children[i] = nullptr;
    }
}

inline bool physics::Octree::Node::isLeaf() const {
    return children[0] == nullptr;
}

inline bool physics::Octree::Node::isEmpty() const {
    return body_idx == -1;
}

void physics::Octree::Node::divide() {
    double half_width = width / 2.0;

    children[0] = new Node(x, y, z, half_width);
    children[1] = new Node(x + half_width, y, z, half_width);
    children[2] = new Node(x, y + half_width, z, half_width);
    children[3] = new Node(x + half_width, y + half_width, z, half_width);
    children[4] = new Node(x, y, z + half_width, half_width);
    children[5] = new Node(x + half_width, y, z + half_width, half_width);
    children[6] = new Node(x, y + half_width, z + half_width, half_width);
    children[7] = new Node(x + half_width, y + half_width, z + half_width, half_width);
}

physics::Octree::Octree(physics::Ephemeris *eph) : eph(eph) {}

// returns 3 bits representing quadrants
// 0bXXX
//   ^^^
//   |||-X: 0 for far left, 1 for far right
//   ||--Y: 0 for top, 1 for bottom
//   |---Z: 0 for far, 1 for close
//
int physics::Octree::getOctant(physics::Octree::Node *node, int b_idx) {
    int octant = 0b000;

    if (eph->x[b_idx] >= node->center_x) octant |= 0b001;
    if (eph->y[b_idx] >= node->center_y) octant |= 0b010;
    if (eph->z[b_idx] >= node->center_z) octant |= 0b100;

    return octant;
}

int physics::Octree::allocNode(physics::Octree::Node node) {
    nodes.emplace_back(node);
    return nodes.size() - 1;
}

void physics::Octree::build() {
    double size = std::max({eph->max_x - eph->min_x, eph->max_y - eph->min_y, eph->max_z - eph->min_z});
    root = new Node(eph->min_x, eph->min_y, eph->min_z, size);

    for (int i = 0; i < eph->n; ++i) {
        insert(i);
    }
}

void physics::Octree::clear() {
    clear(root);
}

void physics::Octree::clear(physics::Octree::Node *node) {
    if (node == nullptr) {
        return;
    }

    for (int i = 0; i < 8; ++i) {
        clear(node->children[i]);
    }

    delete node;
}

void physics::Octree::insert(int b_idx) {
    insert(root, b_idx);
}

void physics::Octree::insert(physics::Octree::Node *node, int b_idx) {
    // empty
    if (node->isEmpty() && node->isLeaf()) {
        node->body_idx = b_idx;
        return;
    }

    // node is leaf, but not empty
    if (node->isLeaf()) {
        int existing_idx = node->body_idx; 

        if (node->width < 1e-10) {
            return;
        }

        node->divide();
        // its empty now 
        node->body_idx = -1;

        // re-insert old particle
        int oct_old = getOctant(node, existing_idx);
        insert(node->children[oct_old], existing_idx);

        int oct_new = getOctant(node, b_idx);
        insert(node->children[oct_new], b_idx);
    } else {
        int oct = getOctant(node, b_idx);
        insert(node->children[oct], b_idx);
    }
}

void physics::Octree::computeMass() {
    computeMass(root);
}

void physics::Octree::computeMass(physics::Octree::Node *node) {
    if (node == nullptr) {
        return;
    }

    // base case
    if (node->isLeaf()) {
        if (node->body_idx != -1) {
            int idx = node->body_idx;
            node->total_mass = eph->mass[idx];
            node->com_x = eph->x[idx];
            node->com_y = eph->y[idx];
            node->com_z = eph->z[idx];
        } else {
            node->total_mass = 0;
        }

        return;
    }

    double mass_sum = 0;
    double weighted_x = 0, weighted_y = 0, weighted_z = 0;

    for (int i = 0; i < 8; ++i) {
        Node *child = node->children[i];

        if (child != nullptr) {
            computeMass(child);
            double m = child->total_mass;
            mass_sum += m;
            weighted_x += child->com_x * m;
            weighted_y += child->com_y * m;
            weighted_z += child->com_z * m;
        }
    }

    node->total_mass = mass_sum;

    if (mass_sum > 0) {
        node->com_x = weighted_x / mass_sum;
        node->com_y = weighted_y / mass_sum;
        node->com_z = weighted_z / mass_sum;
    }
}

void physics::Octree::computeNetForce(int b_idx, double theta) {
    computeNetForce(root, b_idx, theta);
}

void physics::Octree::computeNetForce(physics::Octree::Node *node, int b_idx, double theta) {
    double jx, jy, jz;
    if (node->isLeaf()) {
        if (node->body_idx == -1 || node->body_idx == b_idx) return;
        jx = eph->x[node->body_idx];
        jy = eph->y[node->body_idx];
        jz = eph->z[node->body_idx];
    } else {
        jx = node->com_x;
        jy = node->com_y;
        jz = node->com_z;
    }

    double dx = jx - eph->x[b_idx];
    double dy = jy - eph->y[b_idx];
    double dz = jz - eph->z[b_idx];

    double dist_sq = dx * dx + dy * dy + dz * dz + SOFTENING;
    double curr_theta = node->width / sqrt(dist_sq);

    // node is far enough
    if (curr_theta <= theta || node->isLeaf()) {
        double inv_dist = 1.0 / sqrt(dist_sq);
        double inv_dist_cub = inv_dist * inv_dist * inv_dist;
        double common_factor = G * node->total_mass * inv_dist_cub;

        eph->ax[b_idx] += dx * common_factor;
        eph->ay[b_idx] += dy * common_factor;
        eph->az[b_idx] += dz * common_factor;
    } else { // node is too close
        for (int i = 0; i < 8; ++i) {
            Node *child = node->children[i];
            if (child != nullptr) {
                computeNetForce(child, b_idx, theta);
            }
        }
    }
}
