//===-- sanitizer_deadlock_detector.h ---------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is a part of Sanitizer runtime.
// The deadlock detector maintains a directed graph of lock acquisitions.
// When a lock event happens, the detector checks if the locks already held by
// the current thread are reachable from the newly acquired lock.
//
// The detector can handle only a fixed amount of simultaneously live locks
// (a lock is alive if it has been locked at least once and has not been
// destroyed). When the maximal number of locks is reached the entire graph
// is flushed and the new lock epoch is started. The node ids from the old
// epochs can not be used with any of the detector methods except for
// nodeBelongsToCurrentEpoch().
//
// FIXME: this is work in progress, nothing really works yet.
//
//===----------------------------------------------------------------------===//

#ifndef SANITIZER_DEADLOCK_DETECTOR_H
#define SANITIZER_DEADLOCK_DETECTOR_H

#include "sanitizer_common.h"
#include "sanitizer_bvgraph.h"

namespace __sanitizer {

// Thread-local state for DeadlockDetector.
// It contains the locks currently held by the owning thread.
template <class BV>
class DeadlockDetectorTLS {
 public:
  // No CTOR.
  void clear() {
    bv_.clear();
    epoch_ = 0;
  }

  bool empty() const { return bv_.empty(); }

  void ensureCurrentEpoch(uptr current_epoch) {
    if (epoch_ == current_epoch) return;
    bv_.clear();
    epoch_ = current_epoch;
  }

  uptr getEpoch() const { return epoch_; }

  void addLock(uptr lock_id, uptr current_epoch) {
    // Printf("addLock: %zx %zx\n", lock_id, current_epoch);
    CHECK_EQ(epoch_, current_epoch);
    CHECK(bv_.setBit(lock_id));
  }

  void removeLock(uptr lock_id) {
    // Printf("remLock: %zx %zx\n", lock_id, current_epoch);
    CHECK(bv_.clearBit(lock_id));
  }

  const BV &getLocks(uptr current_epoch) const {
    CHECK_EQ(epoch_, current_epoch);
    return bv_;
  }

 private:
  BV bv_;
  uptr epoch_;
};

// DeadlockDetector.
// For deadlock detection to work we need one global DeadlockDetector object
// and one DeadlockDetectorTLS object per evey thread.
// This class is not thread safe, all concurrent accesses should be guarded
// by an external lock.
// Most of the methods of this class are not thread-safe (i.e. should
// be protected by an external lock) unless explicitly told otherwise.
template <class BV>
class DeadlockDetector {
 public:
  typedef BV BitVector;

  uptr size() const { return g_.size(); }

  // No CTOR.
  void clear() {
    current_epoch_ = 0;
    available_nodes_.clear();
    recycled_nodes_.clear();
    g_.clear();
  }

  // Allocate new deadlock detector node.
  // If we are out of available nodes first try to recycle some.
  // If there is nothing to recycle, flush the graph and increment the epoch.
  // Associate 'data' (opaque user's object) with the new node.
  uptr newNode(uptr data) {
    if (!available_nodes_.empty())
      return getAvailableNode(data);
    if (!recycled_nodes_.empty()) {
      CHECK(available_nodes_.empty());
      // removeEdgesFrom was called in removeNode.
      g_.removeEdgesTo(recycled_nodes_);
      available_nodes_.setUnion(recycled_nodes_);
      recycled_nodes_.clear();
      return getAvailableNode(data);
    }
    // We are out of vacant nodes. Flush and increment the current_epoch_.
    current_epoch_ += size();
    recycled_nodes_.clear();
    available_nodes_.setAll();
    g_.clear();
    return getAvailableNode(data);
  }

  // Get data associated with the node created by newNode().
  uptr getData(uptr node) const { return data_[nodeToIndex(node)]; }

  bool nodeBelongsToCurrentEpoch(uptr node) {
    return node && (node / size() * size()) == current_epoch_;
  }

  void removeNode(uptr node) {
    uptr idx = nodeToIndex(node);
    CHECK(!available_nodes_.getBit(idx));
    CHECK(recycled_nodes_.setBit(idx));
    g_.removeEdgesFrom(idx);
  }

  void ensureCurrentEpoch(DeadlockDetectorTLS<BV> *dtls) {
    dtls->ensureCurrentEpoch(current_epoch_);
  }

  // Handles the lock event, returns true if there is a cycle.
  // FIXME: handle RW locks, recursive locks, etc.
  bool onLock(DeadlockDetectorTLS<BV> *dtls, uptr cur_node) {
    ensureCurrentEpoch(dtls);
    uptr cur_idx = nodeToIndex(cur_node);
    bool is_reachable = g_.isReachable(cur_idx, dtls->getLocks(current_epoch_));
    g_.addEdges(dtls->getLocks(current_epoch_), cur_idx);
    dtls->addLock(cur_idx, current_epoch_);
    return is_reachable;
  }

  // Handles the try_lock event, returns false.
  // When a try_lock event happens (i.e. a try_lock call succeeds) we need
  // to add this lock to the currently held locks, but we should not try to
  // change the lock graph or to detect a cycle.  We may want to investigate
  // whether a more aggressive strategy is possible for try_lock.
  bool onTryLock(DeadlockDetectorTLS<BV> *dtls, uptr cur_node) {
    ensureCurrentEpoch(dtls);
    uptr cur_idx = nodeToIndex(cur_node);
    dtls->addLock(cur_idx, current_epoch_);
    return false;
  }

  // Returns true iff dtls is empty (no locks are currently held) and we can
  // add the node to the currently held locks w/o chanding the global state.
  // This operation is thread-safe as it only touches the dtls.
  bool onFirstLock(DeadlockDetectorTLS<BV> *dtls, uptr node) {
    if (!dtls->empty()) return false;
    if (dtls->getEpoch() && dtls->getEpoch() == nodeToEpoch(node)) {
      dtls->addLock(nodeToIndexUnchecked(node), nodeToEpoch(node));
      return true;
    }
    return false;
  }

  // Finds a path between the lock 'cur_node' (which is currently held in dtls)
  // and some other currently held lock, returns the length of the path
  // or 0 on failure.
  uptr findPathToHeldLock(DeadlockDetectorTLS<BV> *dtls, uptr cur_node,
                          uptr *path, uptr path_size) {
    tmp_bv_.copyFrom(dtls->getLocks(current_epoch_));
    uptr idx = nodeToIndex(cur_node);
    CHECK(tmp_bv_.clearBit(idx));
    uptr res = g_.findShortestPath(idx, tmp_bv_, path, path_size);
    for (uptr i = 0; i < res; i++)
      path[i] = indexToNode(path[i]);
    if (res)
      CHECK_EQ(path[0], cur_node);
    return res;
  }

  // Handle the unlock event.
  // This operation is thread-safe as it only touches the dtls.
  void onUnlock(DeadlockDetectorTLS<BV> *dtls, uptr node) {
    if (dtls->getEpoch() == nodeToEpoch(node))
      dtls->removeLock(nodeToIndexUnchecked(node));
  }

  bool isHeld(DeadlockDetectorTLS<BV> *dtls, uptr node) const {
    return dtls->getLocks(current_epoch_).getBit(nodeToIndex(node));
  }

  uptr testOnlyGetEpoch() const { return current_epoch_; }
  bool testOnlyHasEdge(uptr l1, uptr l2) {
    return g_.hasEdge(nodeToIndex(l1), nodeToIndex(l2));
  }
  // idx1 and idx2 are raw indices to g_, not lock IDs.
  bool testOnlyHasEdgeRaw(uptr idx1, uptr idx2) {
    return g_.hasEdge(idx1, idx2);
  }

  void Print() {
    for (uptr from = 0; from < size(); from++)
      for (uptr to = 0; to < size(); to++)
        if (g_.hasEdge(from, to))
          Printf("  %zx => %zx\n", from, to);
  }

 private:
  void check_idx(uptr idx) const { CHECK_LT(idx, size()); }

  void check_node(uptr node) const {
    CHECK_GE(node, size());
    CHECK_EQ(current_epoch_, nodeToEpoch(node));
  }

  uptr indexToNode(uptr idx) const {
    check_idx(idx);
    return idx + current_epoch_;
  }

  uptr nodeToIndexUnchecked(uptr node) const { return node % size(); }

  uptr nodeToIndex(uptr node) const {
    check_node(node);
    return nodeToIndexUnchecked(node);
  }

  uptr nodeToEpoch(uptr node) const { return node / size() * size(); }

  uptr getAvailableNode(uptr data) {
    uptr idx = available_nodes_.getAndClearFirstOne();
    data_[idx] = data;
    return indexToNode(idx);
  }

  uptr current_epoch_;
  BV available_nodes_;
  BV recycled_nodes_;
  BV tmp_bv_;
  BVGraph<BV> g_;
  uptr data_[BV::kSize];
};

} // namespace __sanitizer

#endif // SANITIZER_DEADLOCK_DETECTOR_H
