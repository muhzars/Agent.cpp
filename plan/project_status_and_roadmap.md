# Project Status & Roadmap: Agent.cpp MoA Implementation

## 1. Accomplishments (What has been done)

### Core Engine Development
- **`AgentManager` Class**: Implemented the core orchestration logic for the Mixture of Agents (MoA) system.
    - **Dynamic Agent Loading**: Capable of loading multiple GGUF models (Brain, Specialist).
    - **MoA Pipeline**: Implemented the `Brain -> Specialist -> Aggregate` workflow.
    - **JSON Routing**: Brain agent generates JSON to route queries to appropriate specialists.
- **`RadixCache` Implementation**:
    - Developed a custom `RadixCache` class inspired by SGLang/PagedAttention.
    - Implemented `insert` and `match` methods for efficient prefix matching and KV cache reuse.
    - Solved "undeclared identifier" compilation errors by correcting header dependencies.

### Build System & Configuration
- **CMake Configuration**:
    - Updated `examples/CMakeLists.txt` to include the new `agent` target.
    - Linked necessary libraries (`common`, `llama`, `cpp-httplib`).
- **Build Scripts**: Created `build_agent.ps1` to automate the build process (though currently debugging execution).

### Benchmarking Preparation
- **Model Identified**: `LFM2.5-1.2B-Instruct-GGUF` identified in Hugging Face cache for testing.
- **Comparison Target**: `llama-cli.exe` located standard `llama.cpp` build.

---

## 2. Current Problems (Blockers)

### Critical: Missing `agent.exe` Binary
- **Symptom**: The build process reports "success" or runs without error, but the `agent.exe` file is not found in `build/examples/agent/Release/` or other expected locations.
- **Root Causes (Diagnosed & Suspected)**:
    1.  **CMake execution failure**: The `cmake --build` command might be failing silently or outputting to an unexpected location due to environment variable issues in PowerShell (failed to find `cmake` initially).
    2.  **Linker Errors**: It's possible the compilation succeeds (creating `.obj` files) but linking fails due to library mismatches, preventing `.exe` creation.
    3.  **Path Visibility**: Standard PowerShell does not inherit Visual Studio Build Tools paths (`vcvarsall.bat`), leading to "Command not found" for `cmake` or `cl.exe`.

---

## 3. Plan & Solutions (How to fix)

### Immediate Fix: Robust Rebuild Strategy
1.  **Use Absolute Paths**: Explicitly invoke CMake using its full path:
    `"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"`
2.  **Verify Target**: Run `cmake --build . --target agent --config Release --verbose` to see exact linker output.
3.  **Check Build Artifacts**: Inspect the `build/examples/agent/agent.dir/Release` folder for `.obj` files to verify compilation happened.

### Runtime Verification
1.  **Run Benchmark**: Once `agent.exe` is found, run:
    ```powershell
    .\agent.exe -m <model_path> -p "Test prompt" --benchmark
    ```
2.  **Compare**: Run `llama-cli.exe` with the same parameters and compare "eval time" (TPS) and "prompt eval time" (TTFT).

---

## 4. Google Search & Research Insights

### Build Issues in Windows
- **Search Query**: "llama.cpp windows build cmake outputs missing executables"
- **Findings**:
    - **CMake Command Sequence**: Must ensure `cmake -B build` is followed by `cmake --build build --config Release`. Missing `--config Release` is a common error.
    - **Visual Studio Tools**: PowerShell sessions often lack the `PATH` setup provided by "Developer Command Prompt". Running from a standard shell risks "Command not found" or using wrong tool versions.
    - **Action**: We will use the full path to `cmake.exe` or execute from a strictly configured environment.

### Radix Attention Implementation
- **Search Query**: "llama.cpp kv cache reuse radix attention implementation"
- **Findings**:
    - **Standard behavior**: `llama.cpp` uses basic "slot-based" caching and prefix matching. It does *not* natively implement a full Radix Tree for complex shared prefix management like SGLang.
    - **Validation**: This confirms that our custom `RadixCache` implementation is a **value-add feature** and not a duplicate of existing logic using `agent.cpp` is justified for this architecture.

---

## 5. Next Steps Roadmap

1.  **[High Priority] Rebuild & Locate `agent.exe`**:
    - Run absolute path build command.
    - Verify file existence with `dir /s`.
2.  **Benchmark Execution**:
    - Run `agent.exe` (Proposed MoA).
    - Run `llama-cli.exe` (Baseline).
    - Record TTFT and TPS.
3.  **Report Generation**:
    - Quantify the speedup (if any) or overhead.
    - Document the successful MoA call flow (Brain -> Specialist).
