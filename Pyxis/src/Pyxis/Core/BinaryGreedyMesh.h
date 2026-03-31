#pragma once
#include "Pyxis/Core/Log.h"
#include <Pyxis/Core/Core.h>

namespace Pyxis {
struct BGMQuad {
  public:
    float halfWidth;
    float halfHeight;
    glm::vec2 center;

    BGMQuad(float half_width, float half_height, glm::vec2 center_pos) {
        halfWidth = half_width;
        halfHeight = half_height;
        center = center_pos;
    }
};

// Processes a 2D binary mask where each column is a uint64_t (up to 64 columns)
// bitArray: span of columns, each a uint64_t mask (1 = solid/opaque, 0 =
// air) LSB is bottom, so bottom left would be LSB of first column, top right is
// MSB of last column
inline std::vector<BGMQuad>
BinaryGreedyMesh(std::span<const uint64_t> bitArray) {
    std::vector<BGMQuad> quads;
    if (bitArray.empty())
        return quads;

    // copy bits as we destroy as we go.
    std::vector<uint64_t> map(bitArray.begin(), bitArray.end());
    const size_t columns = bitArray.size();
    const size_t bitLength = 64;

    // iterate over each column, iterate over each continuous set of bits, and
    // grow vertically then horizontally.
    for (int x = 0; x < columns; x++) {
        while (map[x] != 0) // iterate while we still have bits in this column
        {
            // how many 0's there are at the beginning of the int.
            // this is basically the starting index of the first 1.
            int zero_count = std::countr_zero(map[x]);

            // iterate to find how many continuous 1's there are
            uint64_t mask = map[x] >> zero_count;
            int height = 0;
            while ((mask & 1) == 1) {
                height++;
                mask >>= 1;
            }
            // now, height is how many bits we have in a run, and zero_count is
            // the start index. in order to isolate the bits of this run, i can
            // bit shift the remaining bits above away.
            int invRunLength = bitLength - (zero_count + height);
            mask = map[x] << invRunLength;
            mask >>= invRunLength;
            // mask is now only the bits we want!
            map[x] ^= mask;
            int width = 1;
            while ((x + width) < columns && (map[x + width] & mask) == mask) {
                // we want to remove these bits from the mask as we go.
                map[x + width] ^= mask;
                width++;
            }
            // we have width & height!
            float halfWidth = (float)width / 2.0f;
            float halfHeight = (float)height / 2.0f;
            quads.push_back(BGMQuad(halfWidth, halfHeight,
                                    glm::vec2((float)x + halfWidth,
                                              (float)zero_count + halfHeight)));
        }
    }

    return quads;
}

inline std::vector<BGMQuad>
BinaryGreedyMeshAI(std::span<const uint64_t> bitArray) {
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
            float halfWidth = (float)width_max / 2.0f;
            float halfHeight = (float)height_max / 2.0f;
            glm::vec2 center = glm::ivec2(x_start + halfWidth, y + halfHeight);
            BGMQuad quad = BGMQuad(halfWidth, halfHeight, center);

            quads.push_back(quad);
        }
    }

    return quads;
}

} // namespace Pyxis
