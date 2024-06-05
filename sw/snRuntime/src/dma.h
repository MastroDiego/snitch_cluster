// Copyright 2023 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#pragma once

/// A DMA transfer identifier.
typedef uint32_t snrt_dma_txid_t;

/// Initiate an asynchronous 1D DMA transfer with wide 64-bit pointers.
inline snrt_dma_txid_t snrt_dma_start_1d_wideptr(uint64_t dst, uint64_t src,
                                                 size_t size) {
    register uint32_t reg_txid;  // 10
    asm volatile("dmsrc   %[sl], %[sh]"     :: [sh]"r"(src >> 32), [sl]"r"(src));
    asm volatile("dmdst   %[dl], %[dh]"     :: [dh]"r"(dst >> 32), [dl]"r"(dst));
    asm volatile("dmcpyi  %[id], %[sz], 0"  : [id]"=r"(reg_txid) : [sz]"r"(size));
    return reg_txid;
}

/// Initiate an asynchronous 1D DMA transfer.
inline snrt_dma_txid_t snrt_dma_start_1d(void *dst, const void *src,
                                         size_t size) {
    return snrt_dma_start_1d_wideptr((size_t)dst, (size_t)src, size);
}

/// Initiate an asynchronous 2D DMA transfer with wide 64-bit pointers.
inline snrt_dma_txid_t snrt_dma_start_2d_wideptr(uint64_t dst, uint64_t src,
                                                 size_t size, size_t dst_stride,
                                                 size_t src_stride,
                                                 size_t repeat) {
    register uint32_t reg_txid;  // 10
    asm volatile("dmsrc   %[sl], %[sh]"     :: [sh]"r"(src >> 32), [sl]"r"(src));
    asm volatile("dmdst   %[dl], %[dh]"     :: [dh]"r"(dst >> 32), [dl]"r"(dst));
    asm volatile("dmstr   %[rd], %[rs]"     :: [rd]"r"(dst_stride), [rs]"r"(src_stride));
    asm volatile("dmrep   %[rp]"            :: [rp]"r"(repeat));
    asm volatile("dmcpyi  %[id], %[sz], 2"  : [id]"=r"(reg_txid) : [sz]"r"(size));
    return reg_txid;
}

/// Initiate an asynchronous 2D DMA transfer.
inline snrt_dma_txid_t snrt_dma_start_2d(void *dst, const void *src,
                                         size_t size, size_t dst_stride,
                                         size_t src_stride, size_t repeat) {
    return snrt_dma_start_2d_wideptr((size_t)dst, (size_t)src, size, dst_stride,
                                     src_stride, repeat);
}

/// Initiate an asynchronous 1D DMA transfer with wide 64-bit pointers and a specific channel.
inline snrt_dma_txid_t snrt_dma_start_1d_channel_wideptr(uint64_t dst, uint64_t src,
                                                         size_t size, uint32_t channel) {
        register uint32_t reg_txid;  // 10
        register uint32_t cfg = channel << 2;
        asm volatile("dmsrc   %[sl], %[sh]"         :: [sh]"r"(src >> 32), [sl]"r"(src));
        asm volatile("dmdst   %[dl], %[dh]"         :: [dh]"r"(dst >> 32), [dl]"r"(dst));
        asm volatile("dmcpy  %[id], %[sz], %[cfg]"  : [id]"=r"(reg_txid) : [sz]"r"(size), [cfg]"r"(cfg));
        return reg_txid;
}

/// Initiate an asynchronous 1D DMA transfer and a specific channel.
inline snrt_dma_txid_t snrt_dma_start_1d_channel(void *dst, const void *src,
                                         size_t size, uint32_t channel) {
    return snrt_dma_start_1d_channel_wideptr((size_t)dst, (size_t)src, size, channel);
}

/// Initiate an asynchronous 1D DMA transfer with wide 64-bit pointers and a specific channel.
inline snrt_dma_txid_t snrt_dma_start_2d_channel_wideptr(uint64_t dst, uint64_t src,
                                                         size_t size, uint32_t channel) {
        register uint32_t reg_txid;  // 10
        register uint32_t cfg = channel << 2 | 2;
        asm volatile("dmsrc   %[sl], %[sh]"         :: [sh]"r"(src >> 32), [sl]"r"(src));
        asm volatile("dmdst   %[dl], %[dh]"         :: [dh]"r"(dst >> 32), [dl]"r"(dst));
        asm volatile("dmcpy  %[id], %[sz], %[cfg]"  : [id]"=r"(reg_txid) : [sz]"r"(size), [cfg]"r"(cfg));
        return reg_txid;
}

/// Initiate an asynchronous 2D DMA transfer and a specific channel.
inline snrt_dma_txid_t snrt_dma_start_2d_channel(void *dst, const void *src,
                                         size_t size, uint32_t channel) {
    return snrt_dma_start_2d_channel_wideptr((size_t)dst, (size_t)src, size, channel);
}

/// Block until a transfer finishes.
inline void snrt_dma_wait(snrt_dma_txid_t tid) {
    register uint32_t tmp;
    // dmstati t0, 0  # 2=status.completed_id
    asm volatile(
        "1: \n"
        "dmstati  %[tmp], 0 \n"
        "sub t0, t0, %0 \n"
        "blez t0, 1b \n"
        : [tmp]"=&r"(tmp) :"r"(tid) : "t0");
}

/// Block until all operation on the DMA ceases.
inline void snrt_dma_wait_all() {
    register uint32_t tmp;
    // dmstati t0, 2  # 2=status.busy
    asm volatile(
        "1: \n"
        "dmstati  %[tmp], 2 \n"
        "bne %[tmp], zero, 1b \n"
        : [tmp]"=&r"(tmp) :: "t0");
}

/**
 * @brief start tracking of dma performance region. Does not have any
 * implications on the HW. Only injects a marker in the DMA traces that can be
 * analyzed
 *
 */
inline void snrt_dma_start_tracking() { asm volatile("dmstati zero, 1"); }

/**
 * @brief stop tracking of dma performance region. Does not have any
 * implications on the HW. Only injects a marker in the DMA traces that can be
 * analyzed
 *
 */
inline void snrt_dma_stop_tracking() { asm volatile("dmstati zero, 3"); }

/**
 * @brief fast memset function performed by DMA
 *
 * @param ptr pointer to the start of the region
 * @param value value to set
 * @param len number of bytes, must be multiple of DMA bus-width
 */
inline void snrt_dma_memset(void *ptr, uint8_t value, uint32_t len) {
    // set first 64bytes to value
    // memset(ptr, value, 64);
    uint8_t *p = ptr;
    uint32_t nbytes = 64;
    while (nbytes--) {
        *p++ = value;
    }

    // DMA copy the the rest
    snrt_dma_txid_t memset_txid =
        snrt_dma_start_2d(ptr, ptr, 64, 64, 0, len / 64);
    snrt_dma_wait_all();
}

/// Load a 1D-tile of size tile_size from a 1D array. The specific tile is
/// selected by tile_idx. Every element in the src and dst arrays has prec
/// bytes.
inline snrt_dma_txid_t snrt_dma_load_1d_tile(void *dst, void *src,
                                             size_t tile_idx, size_t tile_size,
                                             uint32_t prec) {
    size_t tile_nbytes = tile_size * prec;
    return snrt_dma_start_1d(dst, src + tile_idx * tile_nbytes, tile_nbytes);
}

/// Store a 1D-tile of size tile_size to a 1D array. The specific tile is
/// selected by tile_idx. Every element in the src and dst arrays has prec
/// bytes.
inline snrt_dma_txid_t snrt_dma_store_1d_tile(void *dst, void *src,
                                              size_t tile_idx, size_t tile_size,
                                              uint32_t prec) {
    size_t tile_nbytes = tile_size * prec;
    return snrt_dma_start_1d(dst + tile_idx * tile_nbytes, src, tile_nbytes);
}

/// Load a 2D-tile of shape (tile_x1_size, tile_x0_size) from the 2D array
/// of shape (full_x1_size, full_x0_size). The specific tile is selected
/// by the (tile_x1_idx, tile_x0_idx) tuple. Every element in the src and
/// destination arrays has prec bytes.
inline snrt_dma_txid_t snrt_dma_load_2d_tile(
    void *dst, void *src, size_t tile_x1_idx, size_t tile_x0_idx,
    size_t tile_x1_size, size_t tile_x0_size, size_t full_x0_size,
    uint32_t prec) {
    size_t src_offset = 0;
    // Advance src array in x0 and x1 dimensions, and convert to byte offset
    src_offset += tile_x0_idx * tile_x0_size;
    src_offset += tile_x1_idx * tile_x1_size * full_x0_size;
    src_offset *= prec;
    // Initiate transfer
    return snrt_dma_start_2d(dst,                  // dst
                             src + src_offset,     // src
                             tile_x0_size * prec,  // size
                             tile_x0_size * prec,  // dst_stride
                             full_x0_size * prec,  // src_stride
                             tile_x1_size          // repeat
    );
}

/// Store a 2D-tile of shape (tile_x1_size, tile_x0_size) to the 2D array
/// of shape (full_x1_size, full_x0_size). The specific tile is selected
/// by the (tile_x1_idx, tile_x0_idx) tuple. Every element in the src and
/// destination arrays has prec bytes.
inline snrt_dma_txid_t snrt_dma_store_2d_tile(
    void *dst, void *src, size_t tile_x1_idx, size_t tile_x0_idx,
    size_t tile_x1_size, size_t tile_x0_size, size_t full_x0_size,
    uint32_t prec) {
    size_t dst_offset = 0;
    // Advance dst array in x0 and x1 dimensions, and convert to byte offset
    dst_offset += tile_x0_idx * tile_x0_size;
    dst_offset += tile_x1_idx * tile_x1_size * full_x0_size;
    dst_offset *= prec;
    // Initiate transfer
    return snrt_dma_start_2d(dst + dst_offset,     // dst
                             src,                  // src
                             tile_x0_size * prec,  // size
                             full_x0_size * prec,  // dst_stride
                             tile_x0_size * prec,  // src_stride
                             tile_x1_size          // repeat
    );
}
