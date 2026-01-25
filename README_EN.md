# Agent.cpp: High-Performance On-Device CPU Inference Engine for Tiny-MoA

<div align="center">

[🇰🇷 한국어](./README.md)

</div>

**Agent.cpp** is a **high-performance CPU-only C++ inference engine** specifically designed for the **[Tiny-MoA](https://github.com/gyunggyung/Tiny-MoA)** project.

While it is based on (cloned from) the famous [llama.cpp](https://github.com/ggerganov/llama.cpp), it is not a simple copy. We have redesigned the core architecture to maximize performance for **Mixture of Agents (MoA)** environments, where iterative inference and frequent context switching are critical.

![License](https://img.shields.io/badge/license-MIT-blue.svg) ![C++](https://img.shields.io/badge/language-C++17-orange.svg) ![Optimization](https://img.shields.io/badge/Optimization-CPU%20AVX2-green)

---

## ⚡ Why Agent.cpp? (vs llama.cpp)

`llama.cpp` is an excellent general-purpose engine, but it can be inefficient in MoA environments where multiple models converse and **Context Switching** occurs dozens of times. `Agent.cpp` was born to solve this bottleneck.

| Feature | Standard llama.cpp | Agent.cpp (Tiny-MoA Engine) |
| :--- | :--- | :--- |
| **Target Use** | General LLM Inference | **Tiny-MoA Multi-Agent Orchestration** |
| **Cache Management** | Linear Cache (Optimized for single chat) | **RadixCache (Tree-based, Instant restore for multi-branch chats)** |
| **Memory Tech** | Standard KV Cache | **PagedAttention & RadixAttention (0% Memory Fragmentation)** |
| **Orchestration** | External Python script required | **Native C++ Built-in (Brain/Specialist Auto-Routing)** |
| **Performance (TTFT)** | Prompt re-computation every turn | **Instant Generation on Cache Hit (~1.8x to 10x Faster)** |

## 🚀 Key Technologies

This engine is not just a wrapper. We have implemented state-of-the-art LLM serving technologies directly in C++.

1.  **RadixAttention (Tree-based Caching)**: When System Prompts and Few-shot examples are shared across multiple agents, they are managed in a tree structure, completely eliminating redundant computations.
2.  **PagedAttention**: Borrowing paging techniques from OS, KV Cache memory is managed in pages, achieving nearly 100% memory efficiency without defragmentation.
3.  **CPU Optimization**: actively utilizes AVX2/AVX-512 instructions to ensure Tiny-MoA (1B~3B models) runs near real-time even without a GPU.

## 🛠️ Build & Run

### Prerequisites
*   Windows (Visual Studio 2022), CMake 3.20+
*   Reference: `Tiny-MoA` model file (.gguf)

### Build (Windows PowerShell)
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release --target agent
```

### Run (Benchmark Mode)
Unlike `llama.cpp`, you don't need complex configurations. Just provide `-m` (model) and `-p` (prompt), and the optimized MoA pipeline runs internally.
```powershell
./build/bin/Release/agent.exe -m path/to/LFM2.5-1.2B.gguf -p "Human: Explain quantum mechanics in 50 words. Assistant:"
```

## 🗺️ Roadmap (Available Next)

Currently, `Agent.cpp` serves as the core kernel for Tiny-MoA. The following features are planned:

- [ ] **Python Bindings (pybind11)**: Support for importing the C++ core as a library in Python.
- [ ] **Tools & Function Calling**: Native C++ implementation of external tools (Search, Calculator) to minimize overhead.
- [ ] **Multimodal (Vision) Support**: Integration of 'Vision Specialist' for image recognition in the MoA pipeline.
- [ ] **Runtime Quantization**: Adaptive quantization levels for better support on low-end hardware.

---
**Agent.cpp Project**
If you wish to contribute or have questions, please open an Issue.
