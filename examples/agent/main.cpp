#include "agent_manager.h"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " -m <brain_model> [--reasoner <path>] [--tool <path>] [-p <prompt>]" << std::endl;
        return 1;
    }

    AgentManager manager;
    std::string brain_path;
    std::string reasoner_path;
    std::string tool_path;
    std::string prompt;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-m") {
            if (++i < argc) brain_path = argv[i];
        } else if (arg == "--reasoner") {
            if (++i < argc) reasoner_path = argv[i];
        } else if (arg == "--tool") {
            if (++i < argc) tool_path = argv[i];
        } else if (arg == "-p") {
            if (++i < argc) prompt = argv[i];
        }
    }

    if (brain_path.empty()) {
        std::cerr << "Error: Brain model path (-m) is required." << std::endl;
        return 1;
    }

    if (!manager.register_agent("Brain", brain_path)) {
        std::cerr << "Failed to load Brain model: " << brain_path << std::endl;
        return 1;
    }

    if (!reasoner_path.empty()) manager.register_agent("Reasoner", reasoner_path);
    if (!tool_path.empty()) manager.register_agent("Tool", tool_path);

    std::cout << "[agent.cpp] All agents registered. Ready for MoA inference." << std::endl;

    if (!prompt.empty()) {
        std::cout << "\n[Benchmarking] Run 1 (Cold): " << prompt << std::endl;
        
        auto run_moa = [&](const std::string& q) {
            // 1. Brain Routing
            std::string brain_raw = manager.run_inference("Brain", q, 128);
            std::string route = "DIRECT";
            if (brain_raw.find("REASONER") != std::string::npos) route = "REASONER";
            else if (brain_raw.find("TOOL") != std::string::npos) route = "TOOL";

            // 2. Execute based on route
            std::string specialist_output;
            if (route == "REASONER" && !reasoner_path.empty()) specialist_output = manager.run_inference("Reasoner", q, 256);
            else if (route == "TOOL" && !tool_path.empty()) specialist_output = manager.run_inference("Tool", q, 256);

            // 3. Integration
            if (!specialist_output.empty()) {
                 std::string integration_prompt = "User: " + q + "\nResult: " + specialist_output + "\nFinal answer:";
                 return manager.run_inference("Brain", integration_prompt, 256);
            }
            return brain_raw;
        };

        std::cout << "Result 1: " << run_moa(prompt) << std::endl;

        std::cout << "\n[Benchmarking] Run 2 (Warm - Prefix Cached): " << prompt << std::endl;
        std::cout << "Result 2: " << run_moa(prompt) << std::endl;
        
        return 0;
    }

    while (true) {
        std::cout << "\nUser > ";
        std::string input;
        if (!std::getline(std::cin, input) || input == "exit") break;
        if (input.empty()) continue;

        // 1. Brain Routing
        std::cout << "[Orchestrator] Brain is routing..." << std::endl;
        std::string brain_raw = manager.run_inference("Brain", input, 128);
        
        // Mock JSON parsing for the route (in reality uses nlohmann/json)
        std::string route = "DIRECT";
        if (brain_raw.find("REASONER") != std::string::npos) {
            route = "REASONER";
        } else if (brain_raw.find("TOOL") != std::string::npos) {
            route = "TOOL";
        }

        // 2. Execute based on route
        std::string specialist_output;
        if (route == "REASONER" && !reasoner_path.empty()) {
            std::cout << "[Orchestrator] Using REASONER Specialist..." << std::endl;
            specialist_output = manager.run_inference("Reasoner", input, 512);
        } else if (route == "TOOL" && !tool_path.empty()) {
            std::cout << "[Orchestrator] Using TOOL Specialist..." << std::endl;
            specialist_output = manager.run_inference("Tool", input, 256);
        }

        // 3. Brain Integration (Final answer)
        std::string final_response;
        if (!specialist_output.empty()) {
            std::cout << "[Orchestrator] Brain is integrating response..." << std::endl;
            std::string integration_prompt = "User asked: " + input + "\nSpecialist output: " + specialist_output + "\nFinal answer:";
            final_response = manager.run_inference("Brain", integration_prompt, 512);
        } else {
            final_response = brain_raw;
        }

        // 4. Final Output
        std::cout << "\nAgent > " << final_response << std::endl;
    }

    return 0;
}
