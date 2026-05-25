---
name: cpp-clean-architecture
description: Especialista em refatoração, limpeza de código (clean up) e revisão de arquitetura para projetos C++ de modificações de GTA San Andreas, focando em legibilidade, organização estrutural e segurança de hooks.
---

# Especialista em Code Clean Up & Arquitetura C++ (GTA:SA Mod)

Este skill ensina o agente de IA a revisar, refatorar e organizar projetos C++ que atuam como mods/plugins ASI para o **GTA San Andreas**, garantindo código de alta performance, legibilidade excelente e arquitetura desacoplada.

## Quando usar esta skill
- Para revisar a organização de pastas e cabeçalhos do projeto.
- Ao refatorar métodos, funções, assinaturas e nomes de variáveis para aumentar a legibilidade.
- Para verificar problemas estruturais, de gerenciamento de memória ou acoplamento excessivo.
- Para garantir práticas seguras de desenvolvimento de mods GTA:SA (segurança de threads, gta memory patches e hooks).

---

## 🏗️ Diretrizes de Arquitetura e Organização do Projeto

### 1. Separação de Responsabilidades (Folder Structure)
- **`core/` (Lógica Pura):** Deve conter apenas lógica do telefone, sem referências ao Direct3D, MinHook, Kiero ou `plugin-sdk`.
- **`gta/` (Lógica do Jogo):** Contém ganchos de renderização (Kiero), hooks de funções do GTA (MinHook) e implementações de provedores que acessam a memória do jogo.
- **`sandbox/` (Lógica Standalone):** Contém mocks, janelas auxiliares de depuração e implementações mockadas de provedores.

### 2. Padrões de Código e Legibilidade (C++ Clean Code)
- **Nomes Significativos e Padronizados:**
  - Classes e Structs: `CamelCase` (ex: `CalculatorApp`).
  - Métodos e Funções: `camelCase` ou `PascalCase` dependendo do estilo atual do arquivo (ex: `onDraw`, `registerApp`). Mantenha a consistência local.
  - Variáveis membro privativas: Usar prefixo `m_` (ex: `m_isOpen`).
- **Funções Pequenas e Focadas:**
  - Cada método/função deve ter uma única responsabilidade (Single Responsibility Principle - SRP).
  - Evite funções com mais de 50-60 linhas. Se crescer muito, extraia subfunções auxiliares privadas.
- **Constantes em vez de Números Mágicos:**
  - Substitua valores numéricos soltos no código por `const`, `constexpr` ou `enums` bem explicados.
- **Modern C++:**
  - Use `nullptr` em vez de `NULL` ou `0`.
  - Use `auto` onde o tipo for óbvio e redundante.
  - Utilize RAII (`std::unique_ptr`, `std::lock_guard`) para gerenciar ciclos de vida de memória e travas.

---

## 🚗 Diretrizes Específicas para Modding do GTA:SA (ASI Plugins)

Desenvolver plugins `.asi` em C++ exige cuidados extras com estabilidade e compatibilidade:

### 1. Segurança de Threads (Thread Safety)
- O loop principal do GTA:SA roda em uma única thread principal (Main Thread). 
- A renderização do ImGui e chamadas ao Direct3D ocorrem dentro da thread de desenho (EndScene/Present hooks). 
- **Regra de Ouro:** Não faça leitura/escrita concorrente em estruturas do GTA:SA fora da thread principal do jogo ou da thread de renderização sem a devida sincronização (mutex), para evitar travamentos inexplicáveis (Crash to Desktop - CTD).

### 2. Uso do `plugin-sdk`
- Evite redeclarar structs ou variáveis internas do jogo que já estejam mapeadas no `plugin-sdk`.
- Sempre consulte classes do `plugin-sdk` (como `CWorld`, `CPed`, `CVehicle`, `CTheZones`) para ler/escrever dados do jogo de forma nativa e performática.

### 3. Gerenciamento de Hooks (MinHook & Kiero)
- **Clean Unload:** Quando o plugin for descarregado (seja ao fechar o jogo ou ao usar um loader de plugins), certifique-se de desabilitar e liberar todos os hooks criados com o MinHook.
- Evite hooks excessivamente intrusivos em funções críticas executadas a cada frame da CPU (como loops de física do jogo) a menos que estritamente necessário. Prefira hooks orientados a eventos ou eventos expostos pelo `plugin-sdk`.

### 4. Patches de Memória Seguros
- Ao alterar bytes na memória do GTA:SA para aplicar patches, use as funções auxiliares seguras do `plugin-sdk` (como `plugin::patch::Write`, `plugin::patch::SetPointer`) que cuidam da proteção de escrita de memória (`VirtualProtect`) de forma limpa.

---

## 🔍 Processo de Revisão (Workflow de Clean Up)

Sempre que este agente for solicitado para revisar um arquivo ou pasta, ele deve seguir estes passos:
1. **Análise de Dependências:** Garantir que o arquivo não está importando pacotes desnecessários ou violando o encapsulamento de plataforma.
2. **Avaliação de Complexidade:** Identificar funções muito longas ou com múltiplos níveis de indentação aninhada e propor simplificações.
3. **Checagem de Padrões:** Verificar se os nomes de métodos, parâmetros e variáveis seguem as convenções adotadas no projeto.
4. **Verificação de Performance/Vazamentos:** Garantir que recursos alocados dinamicamente estão sendo liberados corretamente (RAII) e que não há cópias de strings desnecessárias (preferir `const std::string&`).
