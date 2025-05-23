// Copyright 2010-2021, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef MOZC_CONVERTER_LATTICE_H_
#define MOZC_CONVERTER_LATTICE_H_

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "absl/log/check.h"
#include "absl/strings/string_view.h"
#include "converter/node.h"
#include "converter/node_allocator.h"

namespace mozc {

class Lattice {
 public:
  Lattice()
      : history_end_pos_(0),
        node_allocator_(std::make_unique<NodeAllocator>()) {}

  NodeAllocator *node_allocator() const { return node_allocator_.get(); }

  // set key and initializes lattice with key.
  void SetKey(std::string key);

  // return key.
  absl::string_view key() const { return key_; }

  // Set history end position.
  // For cache, we have to reset lattice when the history size is changed.
  void set_history_end_pos(size_t pos) { history_end_pos_ = pos; }

  size_t history_end_pos() const { return history_end_pos_; }

  // allocate new node.
  Node *NewNode() { return node_allocator_->NewNode(); }

  // return nodes (linked list) starting with |pos|.
  // To traverse all nodes, use Node::bnext member.
  Node *begin_nodes(size_t pos) const { return begin_nodes_[pos]; }

  // return nodes (linked list) ending at |pos|.
  // To traverse all nodes, use Node::enext member.
  Node *end_nodes(size_t pos) const { return end_nodes_[pos]; }

  // return bos nodes.
  // alias of end_nodes(0).
  Node *bos_nodes() const { return end_nodes_[0]; }

  // return eos nodes.
  // alias of begin_nodes(key.size()).
  Node *eos_nodes() const { return begin_nodes_[key_.size()]; }

  // inset nodes (linked list) to the position |pos|.
  void Insert(size_t pos, Node *node);

  // clear all lattice and nodes allocated with NewNode method.
  void Clear();

  // return true if this instance has a valid lattice.
  bool has_lattice() const { return !begin_nodes_.empty(); }

  // set key with cache information kept
  void UpdateKey(absl::string_view new_key);

  // add suffix_key to the end of a current key
  void AddSuffix(absl::string_view suffix_key);

  // erase the suffix of a key so that the length of the key becomes new_len
  void ShrinkKey(size_t new_len);

  // getter
  size_t cache_info(const size_t pos) const {
    CHECK_LE(pos, key_.size());
    return cache_info_[pos];
  }

  // setter
  void SetCacheInfo(const size_t pos, const size_t len) {
    CHECK_LE(pos, key_.size());
    cache_info_[pos] = len;
  }

  // revert the wcost of nodes if it has ENABLE_CACHE attribute.
  // This function is needed for wcost may be changed during conversion
  // process for some heuristic methods.
  void ResetNodeCost();

  // Dump the best path and the path that contains the designated string.
  std::string DebugString() const;

  // Set the node info that should be used in DebugString() (For debug use).
  static void SetDebugDisplayNode(size_t begin_pos, size_t end_pos,
                                  std::string str);

  // Reset the debug info.
  static void ResetDebugDisplayNode();

 private:
  // TODO(team): Splitting the cache module may make this module simpler.
  std::string key_;
  size_t history_end_pos_;
  std::vector<Node *> begin_nodes_;
  std::vector<Node *> end_nodes_;
  std::unique_ptr<NodeAllocator> node_allocator_;

  // cache_info_ holds cache information about lookup.
  // If cache_info_[pos] equals to len, it means key.substr(pos, k)
  // (1 <= k <= len) is already looked up.
  std::vector<size_t> cache_info_;
};

}  // namespace mozc

#endif  // MOZC_CONVERTER_LATTICE_H_
