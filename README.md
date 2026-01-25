# Agent.cpp: Tiny-MoA를 위한 고성능 CPU 전용 추론 엔진

<div align="center">

[🇺🇸 English Version](./README_EN.md)

</div>

**Agent.cpp**는 **[Tiny-MoA](https://github.com/gyunggyung/Tiny-MoA)** 프로젝트를 위해 특별히 설계된 **CPU 전용 고성능 C++ 추론 엔진**입니다.

유명한 [llama.cpp](https://github.com/ggerganov/llama.cpp)를 기반(Clone)으로 개발되었으나, 단순한 복제가 아닙니다. **다중 에이전트(Mixture of Agents, MoA)** 환경에서 필수적인 반복적 추론과 문맥 전환 속도를 극대화하기 위해 핵심 아키텍처를 재설계했습니다.

![License](https://img.shields.io/badge/license-MIT-blue.svg) ![C++](https://img.shields.io/badge/language-C++17-orange.svg) ![Optimization](https://img.shields.io/badge/Optimization-CPU%20AVX2-green)

---

## ⚡ 왜 Agent.cpp인가? (vs llama.cpp)

`llama.cpp`는 훌륭한 범용 엔진이지만, MoA와 같이 여러 모델이 대화하며 수십 번의 **Context Switching**이 일어나는 환경에서는 비효율적입니다. `Agent.cpp`는 이 병목을 해결하기 위해 태어났습니다.

| 기능 | 표준 llama.cpp | Agent.cpp (Tiny-MoA Engine) |
| :--- | :--- | :--- |
| **타겟 용도** | 범용 LLM 추론 | **Tiny-MoA 전용 다중 에이전트 오케스트레이션** |
| **캐시 관리** | 선형적 캐시 (단일 대화 최적화) | **RadixCache (트리 기반, 다중 분기 대화 즉시 복원)** |
| **메모리 기술** | 표준 KV Cache | **PagedAttention & RadixAttention (메모리 파편화 0%)** |
| **오케스트레이션** | 외부 Python 스크립트 필요 | **C++ 네이티브 내장 (Brain/Specialist 자동 라우팅)** |
| **성능 (TTFT)** | 매 턴마다 프롬프트 재연산 | **캐시 히트 시 즉시 생성 (약 1.8배~수십 배 가속)** |

## 🚀 핵심 기술 구현

이 엔진은 단순한 래퍼(Wrapper)가 아닙니다. 최신 LLM 서빙 기술을 C++로 직접 구현하여 탑재했습니다.

1.  **RadixAttention (트리 기반 캐동)**: 시스템 프롬프트(System Prompt)와 퓨샷(Few-shot) 예제들이 여러 에이전트 간에 공유될 때, 이를 트리 구조로 관리하여 중복 연산을 완벽하게 제거합니다.
2.  **PagedAttention**: 운영체제의 페이징 기법을 차용하여 KV Cache 메모리를 페이지 단위로 관리, 조각모음 없이도 100%에 가까운 메모리 효율을 달성했습니다.
3.  **CPU 최적화**: GPU가 없는 환경에서도 Tiny-MoA(1B~3B 모델)가 실시간에 준하는 속도로 동작하도록 AVX2/AVX-512 명령어를 적극 활용합니다.

## 🛠️ 빌드 및 실행

### 필수 조건
*   Windows (Visual Studio 2022), CMake 3.20+
*   참고: `Tiny-MoA` 모델 파일 (.gguf)

### 빌드 (Windows PowerShell)
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release --target agent
```

### 실행 (벤치마크 모드)
`llama.cpp`와 달리 복잡한 설정 없이 `-m` (모델)과 `-p` (프롬프트)만 주면 내부적으로 최적화된 MoA 파이프라인이 동작합니다.
```powershell
./build/bin/Release/agent.exe -m path/to/LFM2.5-1.2B.gguf -p "Human: 양자 역학을 50단어로 설명해줘. Assistant:"
```

## 🗺️ 향후 로드맵 (Available Next)

현재 `Agent.cpp`는 Tiny-MoA의 핵심 코어를 담당하고 있습니다. 앞으로 다음과 같은 기능들이 추가될 예정입니다.

- [ ] **Python 바인딩 (pybind11)**: C++ 코어를 Python에서 라이브러리처럼 import하여 사용할 수 있도록 지원.
- [ ] **Tools & Function Calling**: 검색(Search), 계산기 등 외부 도구를 C++ 레벨에서 직접 호출하여 오버헤드 최소화.
- [ ] **멀티모달 (Vision) 지원**: MoA 파이프라인에 이미지 인식을 담당하는 'Vision Specialist' 통합.
- [ ] **경량화 (Quantization) 파이프라인**: 런타임에 모델 양자화 레벨을 조절하여 저사양 하드웨어 대응력 강화.

---
**Agent.cpp Project**
기여를 원하시거나 문의사항이 있으시면 Issue를 등록해주세요.
