#include "radix_cache.h"
#include <chrono>
#include <algorithm>

RadixCache::RadixCache() {
    root = std::make_unique<Node>();
}

RadixCache::~RadixCache() {}

static long get_now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

RadixCache::MatchResult RadixCache::match(const std::vector<llama_token>& tokens) {
    std::lock_guard<std::mutex> lock(mutex);
    Node* current = root.get();
    size_t matched_len = 0;
    llama_seq_id best_seq_id = -1;
    size_t best_len = 0;

    for (size_t i = 0; i < tokens.size(); ++i) {
        auto it = current->children.find(tokens[i]);
        if (it == current->children.end()) {
            break;
        }
        current = it->second.get();
        current->last_access_time = get_now();
        matched_len++;
        
        if (current->seq_id != -1) {
            best_seq_id = current->seq_id;
            best_len = matched_len;
        }
    }

    return {best_seq_id, best_len};
}

void RadixCache::insert(const std::vector<llama_token>& tokens, llama_seq_id seq_id) {
    std::lock_guard<std::mutex> lock(mutex);
    Node* current = root.get();
    long now = get_now();

    for (size_t i = 0; i < tokens.size(); ++i) {
        auto& child = current->children[tokens[i]];
        if (!child) {
            child = std::make_unique<Node>();
            child->tokens = {tokens.begin(), tokens.begin() + i + 1};
        }
        current = child.get();
        current->last_access_time = now;
    }
    current->seq_id = seq_id;
}

void RadixCache::evict(size_t target_size) {
    // Basic LRU eviction to be implemented if needed
    // For now, assume KV cache management is handled by llama.cpp
}
