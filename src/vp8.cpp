// SPDX-License-Identifier: MIT
#include "internal.hpp"

namespace irisva {
std::vector<uint8_t> vp8_bitstream(const Vp8Picture &picture) {
    check(picture.has_params, "VP8 picture parameters missing", VA_STATUS_ERROR_INVALID_BUFFER);
    check(picture.slices.size() == 1 && picture.pending_slices.empty(),
          "VP8 requires one complete frame per picture", VA_STATUS_ERROR_INVALID_BUFFER);
    const auto &slice = picture.slices[0];
    const auto &p = picture.params;
    const auto &s = slice.params;

    // VA clients hand over only the partition data: the uncompressed header is
    // described by the picture/slice parameters and has to be rebuilt. A stateful
    // decoder needs the original frame bytes.
    //   first_part_size covers the whole control partition, i.e. what the client
    //   already parsed plus the remainder it kept in partition_size[0].
    check(s.num_of_partitions >= 1 && s.num_of_partitions <= 9,
          "invalid VP8 partition count", VA_STATUS_ERROR_INVALID_BUFFER);
    // macroblock_offset counts the bits of the control partition the client has
    // already consumed; round up to whole bytes.
    const uint64_t first_part_size =
        uint64_t{s.partition_size[0]} + (uint64_t{s.macroblock_offset} + 7) / 8;
    check(first_part_size && first_part_size <= 0x7ffff && first_part_size <= slice.bytes.size(),
          "invalid VP8 partition size", VA_STATUS_ERROR_INVALID_BUFFER);

    const unsigned frame_type = p.pic_fields.bits.key_frame; // 0 == key frame
    const unsigned version = p.pic_fields.bits.version;
    const unsigned show_frame = 1;
    const uint32_t tag = (frame_type & 1) | ((version & 7) << 1) | ((show_frame & 1) << 4) |
                         (uint32_t(first_part_size) << 5);

    std::vector<uint8_t> out;
    out.reserve(slice.bytes.size() + (frame_type ? 3 : 10));
    out.push_back(uint8_t(tag));
    out.push_back(uint8_t(tag >> 8));
    out.push_back(uint8_t(tag >> 16));
    if (!frame_type) {
        out.push_back(0x9d);
        out.push_back(0x01);
        out.push_back(0x2a);
        const unsigned width = p.frame_width & 0x3fff;
        const unsigned height = p.frame_height & 0x3fff;
        out.push_back(uint8_t(width));
        out.push_back(uint8_t(width >> 8));
        out.push_back(uint8_t(height));
        out.push_back(uint8_t(height >> 8));
    }
    out.insert(out.end(), slice.bytes.begin(), slice.bytes.end());
    return out;
}
} // namespace irisva
