/**
 * License: MIT
 * Copyright (c) 2013 Gerard Krol
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

typedef struct {
    uint8_t version;
    uint8_t header_length;
    uint8_t message_type;
    uint8_t hop_limit;
    uint8_t flags;
    uint8_t flow_id1;
    uint8_t flow_id2;
    uint8_t flow_id3;
    uint8_t src_pubkey[32];
    uint8_t dst_pubkey[32];
    uint8_t src_location[16];
    uint8_t dst_location[16];
    uint8_t signature[64];
} wanderlust_header_t;

enum {
    WANDERLUST_TYPE_DATA=0,
    WANDERLUST_TYPE_SWAP_REQUEST,
    WANDERLUST_TYPE_SWAP_RESPONSE,
    WANDERLUST_TYPE_SWAP_CONFIRMATION,
    WANDERLUST_TYPE_LOCATION_QUERY,
    WANDERLUST_TYPE_LOCATION_ANSWER
};
