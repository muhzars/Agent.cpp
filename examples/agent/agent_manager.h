#pragma once

#include "llama.h"
#include "radix_cache.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

struct AgentInstance {
    std::string name;
    llama_model* model;
    llama_context* ctx;
    std::vector<llama_token> system_prompt_tokens;
    // Each agent can have its own sampler or sharing one
};

class AgentManager {
public:
    AgentManager();
    ~AgentManager();

    // Register a new agent model
    bool register_agent(const std::string& name, const std::string& model_path, const std::vector<llama_token>& system_prompt = {});

    // Run inference with a specific agent
    std::string run_inference(const std::string& agent_name, const std::string& user_input, int max_tokens = 512);

    // Tokenize text for a specific agent's vocab
    std::vector<llama_token> tokenize(const std::string& agent_name, const std::string& text, bool add_special = true);

private:
    std::map<std::string, std::unique_ptr<AgentInstance>> agents;
    RadixCache radix_cache;
    
    // Shared parameters
    llama_model_params mparams;
    llama_context_params cparams;
};
