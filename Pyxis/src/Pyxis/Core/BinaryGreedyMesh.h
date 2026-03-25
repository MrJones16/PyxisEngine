#pragma once
#include <Pyxis/Core/Core.h>

namespace Pyxis {
struct BGMQuad {
    float halfWidth;
    float halfHeight;
    glm::vec2 center;
};

// Processes a 2D binary mask where each row is a uint64_t (up to 64 columns)
// bitArray: span of height rows, each a uint64_t mask (1 = solid/opaque, 0 =
// air) Assumes the grid width <= 64 (common for binary greedy meshing) Returns
// quads covering maximal rectangles of 1-bits
inline std::vector<BGMQuad>
BinaryGreedyMesh(std::span<const uint64_t> bitArray) {
    std::vector<BGMQuad> quads;
    if (bitArray.empty())
        return quads;

    const size_t height = bitArray.size();
    const size_t width =
        64; // fixed for uint64_t; change if you use smaller masks

    // We'll mutate a working copy so we can clear bits as we consume quads
    std::vector<uint64_t> mask(bitArray.begin(), bitArray.end());

    for (size_t y = 0; y < height; ++y) {
        uint64_t &row = mask[y];

        while (row != 0) {
            // Find leftmost solid bit (first 1 in the row)
            int x_start = std::countr_zero(row);

            // Find how far this horizontal run goes (greedily take max width)
            // One way: isolate lowest bit, then keep clearing until we hit a 0
            uint64_t run_mask = row >> x_start;
            int width_max = 1;
            run_mask >>= 1;
            while ((run_mask & 1) != 0) {
                ++width_max;
                run_mask >>= 1;
            }
            // Now try to expand this rectangle downward as far as possible
            int height_max = 1;
            for (size_t y2 = y + 1; y2 < height; ++y2) {
                uint64_t lower_row = mask[y2];
                uint64_t needed = ((uint64_t(1) << width_max) - 1) << x_start;

                if ((lower_row & needed) != needed) {
                    break; // cannot extend — missing some bits
                }

                ++height_max;
                // Clear the bits in the lower row (consume them)
                mask[y2] &= ~needed;
            }

            // Clear the bits in the starting row too
            uint64_t needed = ((uint64_t(1) << width_max) - 1) << x_start;
            row &= ~needed;

            // Create the quad (bottom-left origin, assuming +x right, +y up or
            // down — adjust as needed)
            BGMQuad quad;
            quad.halfWidth = (float)width_max / 2.0f;
            quad.halfHeight = (float)height_max / 2.0f;
            quad.center =
                glm::ivec2(x_start + quad.halfWidth, y + quad.halfHeight);

            quads.push_back(quad);
        }
    }

    return quads;
}

} // namespace Pyxis
