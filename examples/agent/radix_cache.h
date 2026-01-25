#pragma once

#include "llama.h"
#include <vector>
#include <map>
#include <memory>
#include <mutex>

class RadixCache {
public:
    struct Node {
        std::vector<llama_token> tokens;
        llama_seq_id seq_id = -1;
        std::map<llama_token, std::unique_ptr<Node>> children;
        long last_access_time;

        Node() : last_access_time(0) {}
    };

    RadixCache();
    ~RadixCache();

    // Find the longest prefix in the cache
    // Returns {seq_id, matched_length}
    struct MatchResult {
        llama_seq_id seq_id;
        size_t length;
    };
    MatchResult match(const std::vector<llama_token>& tokens);

    // Insert or update a prefix in the cache
    void insert(const std::vector<llama_token>& tokens, llama_seq_id seq_id);

    // Evict least recently used nodes (TODO)
    void evict(size_t target_size);

private:
    std::unique_ptr<Node> root;
    std::mutex mutex;
};
