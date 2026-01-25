#include "agent_manager.h"
#include "common.h"
#include "sampling.h"
#include <iostream>
#include <vector>
#include <string>

AgentManager::AgentManager() {
    llama_backend_init();
    mparams = llama_model_default_params();
    mparams.use_mmap = true; // Essential for 16GB RAM
    
    cparams = llama_context_default_params();
    cparams.n_ctx = 4096;
    cparams.n_threads = 4; // Default to 4 threads for CPU efficiency
}

AgentManager::~AgentManager() {
    for (auto& pair : agents) {
        llama_free(pair.second->ctx);
        llama_model_free(pair.second->model);
    }
    llama_backend_free();
}

bool AgentManager::register_agent(const std::string& name, const std::string& model_path, const std::vector<llama_token>& system_prompt) {
    std::cout << "[AgentManager] Loading agent: " << name << " from " << model_path << std::endl;
    
    llama_model* model = llama_model_load_from_file(model_path.c_str(), mparams);
    if (!model) {
        std::cerr << "[AgentManager] Failed to load model for: " << name << std::endl;
        return false;
    }

    llama_context* ctx = llama_init_from_model(model, cparams);
    if (!ctx) {
        std::cerr << "[AgentManager] Failed to create context for: " << name << std::endl;
        llama_model_free(model);
        return false;
    }

    auto instance = std::make_unique<AgentInstance>();
    instance->name = name;
    instance->model = model;
    instance->ctx = ctx;
    instance->system_prompt_tokens = system_prompt;

    agents[name] = std::move(instance);
    return true;
}

std::vector<llama_token> AgentManager::tokenize(const std::string& agent_name, const std::string& text, bool add_special) {
    auto it = agents.find(agent_name);
    if (it == agents.end()) return {};

    const llama_vocab* vocab = llama_model_get_vocab(it->second->model);
    // Note: common_tokenize is available in common.h/cpp
    return common_tokenize(it->second->ctx, text, add_special, true);
}


std::string AgentManager::run_inference(const std::string& agent_name, const std::string& user_input, int max_tokens) {
    auto it = agents.find(agent_name);
    if (it == agents.end()) return "Agent not found: " + agent_name;

    AgentInstance* agent = it->second.get();
    
    // 1. Tokenize user input
    std::vector<llama_token> tokens = tokenize(agent_name, user_input, true);
    
    // 2. RadixCache Prefix Match
    RadixCache::MatchResult match_result = radix_cache.match(tokens);
    int prefix_len = (int)match_result.length;
    llama_seq_id seq_id = match_result.seq_id;
    
    // 3. Prepare context
    if (seq_id != -1) {
        // Reuse KV cache
        // llama_kv_cache_seq_keep(agent->ctx, seq_id); // In current API, we might need a different approach
    } else {
        // llama_kv_cache_clear(agent->ctx); // Redundant for fresh context, and API changed
        // TODO: Find correct API for cache clearing in long-running sessions
        seq_id = 0; // Default sequence
    }

    // 4. Initial batch (only the non-cached part)
    std::vector<llama_token> pending_tokens(tokens.begin() + prefix_len, tokens.end());
    if (!pending_tokens.empty()) {
        llama_batch batch = llama_batch_get_one(pending_tokens.data(), pending_tokens.size());
        if (llama_decode(agent->ctx, batch) != 0) {
            return "[Error] llama_decode failed";
        }
    }

    // 5. Update RadixCache
    radix_cache.insert(tokens, seq_id);

    // 6. Sampling loop
    struct common_params_sampling sparams;
    struct common_sampler* smpl = common_sampler_init(agent->model, sparams);
    
    std::string result = "";
    llama_token last_token = -1;

    for (int i = 0; i < max_tokens; ++i) {
        last_token = common_sampler_sample(smpl, agent->ctx, -1);
        common_sampler_accept(smpl, last_token, true);

        if (llama_vocab_is_eog(llama_model_get_vocab(agent->model), last_token)) {
            break;
        }

        result += common_token_to_piece(agent->ctx, last_token);

        // Next batch
        llama_batch batch = llama_batch_get_one(&last_token, 1);
        if (llama_decode(agent->ctx, batch) != 0) {
            break;
        }
    }

    common_sampler_free(smpl);
    return result;
}
