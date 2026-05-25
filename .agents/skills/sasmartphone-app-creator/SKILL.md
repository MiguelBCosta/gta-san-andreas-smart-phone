---
name: sasmartphone-app-creator
description: Cria novos aplicativos (PhoneApp) no projeto SaSmartPhone, gerenciando layouts, ícones FontAwesome 5, renderização com ImGui e registro no sandbox e no GTA, garantindo desacoplamento rígido via Providers e fidelidade ao estilo visual do iPhone/iOS.
---

# Criador de Aplicativos SaSmartPhone

Este skill ensina o agente de IA a criar, programar e registrar de forma correta e padronizada novos aplicativos para o ecossistema **SaSmartPhone**, seguindo uma separação rígida de plataformas e o padrão visual premium do iPhone/iOS.

## Quando usar esta skill
- Sempre que for solicitado para criar um novo aplicativo ou tela de aplicativo para o celular.
- Quando for preciso adicionar novos recursos visuais (ImGui) ou comportamentos ao celular.
- Para entender as dependências e o fluxo de registro/compilação do smartphone.

---

## 🏗️ Diretrizes de Arquitetura (Isolamento Absoluto de Plataforma)

Para manter o núcleo do celular (`source/core/`) limpo e portátil, siga rigorosamente as seguintes regras de desacoplamento:

### 1. Proibição de Dependências do Jogo no Core
- **NUNCA** inclua headers do `plugin-sdk`, APIs do GTA:SA ou APIs do sistema operacional Windows diretamente na pasta `source/core/` ou em qualquer `PhoneApp`.
- O Sandbox **deve rodar de forma 100% independente**, sem qualquer dependência ou inicialização relacionada ao GTA:SA.

### 2. Proibição de Condicionais de Compilação (`#ifdef` / `if`)
- **NUNCA** use diretivas de pré-processador como `#ifdef GTA_SA`, `#ifdef SANDBOX` ou ifs dinâmicos como `if (isGta)` no código do `core` para desviar comportamentos de plataforma.
- Toda lógica que varia entre o jogo e o simulador standalone deve ser abstraída usando o **Padrão Provider (Provider Pattern)**.

### 3. Como Implementar o Padrão Provider
Sempre que precisar de informações externas (ex: coordenadas do mapa, nível de procurado, dinheiro do jogador, clima, hora, contatos):

1. **Defina a Interface Abstrata** na pasta `source/core/providers/`:
   ```cpp
   // source/core/providers/IPlayerInfoProvider.h
   #pragma once
   
   class IPlayerInfoProvider {
   public:
       virtual ~IPlayerInfoProvider() = default;
       virtual int GetPlayerMoney() = 0;
       virtual float GetPlayerHealth() = 0;
   };
   ```

2. **Implemente no Simulador Sandbox (`source/sandbox/providers/`)**:
   - Forneça dados simulados (mockados), estáticos ou configuráveis através de sliders de controle para testes rápidos na janela standalone.
   ```cpp
   // source/sandbox/providers/SandboxPlayerInfoProvider.h
   #pragma once
   #include "../../core/providers/IPlayerInfoProvider.h"
   
   class SandboxPlayerInfoProvider : public IPlayerInfoProvider {
   public:
       int GetPlayerMoney() override { return 999999; } // Mocked value
       float GetPlayerHealth() override { return 100.0f; }
   };
   ```

3. **Implemente no GTA:SA (`source/gta/providers/`)**:
   - Utilize de preferência métodos do **`plugin-sdk`** para buscar informações nativas do jogo de forma limpa.
   ```cpp
   // source/gta/providers/GtaPlayerInfoProvider.h
   #pragma once
   #include "../../core/providers/IPlayerInfoProvider.h"
   #include <plugin.h>
   #include <game_sa/CWorld.h>
   #include <game_sa/FindPlayerPed.h>
   
   class GtaPlayerInfoProvider : public IPlayerInfoProvider {
   public:
       int GetPlayerMoney() override {
           // Usando o plugin-sdk para ler a memória do jogo de forma nativa
           return FindPlayerPed()->m_nMoney;
       }
       float GetPlayerHealth() override {
           return FindPlayerPed()->m_fHealth;
       }
   };
   ```

4. **Instancie e Registre o Provedor**:
   - No `SandboxMain.cpp` e no `Main.cpp`, instancie as respectivas classes e passe-as para o celular ou aplicativo que irá consumi-las.

---

## 🎨 Diretrizes Estéticas e de Interface (Padrão iPhone/iOS)

Todos os aplicativos devem ter uma interface limpa, moderna e premium, mimetizando o estilo oficial do iOS da Apple:

### 1. Paleta de Cores e Temas
- **Tema Escuro Padrão (iOS Dark Mode):** 
  - Fundo principal dos aplicativos: Preto profundo ou cinza muito escuro (`ImVec4(0.08f, 0.08f, 0.09f, 1.0f)` ou `ImVec4(0.11f, 0.11f, 0.12f, 1.0f)`).
  - Cartões e containers de agrupamento: Cinza escuro translúcido (`ImVec4(0.18f, 0.18f, 0.20f, 0.8f)`).
- **Cor de Destaque Oficial (iOS Accent Blue):**
  - Use o azul clássico do iOS para links, botões importantes, setas de retorno e botões de ação: `ImVec4(0.039f, 0.518f, 1.0f, 1.0f)` (também expresso como `#0A84FF`).

### 2. Layout, Formatos e Arredondamentos
- **Cantos Arredondados:** 
  - Sempre aplique cantos arredondados nos cartões de conteúdo e containers (use `rounding = 12.0f` a `16.0f`).
  - Botões secundários e controles de formulário devem usar `rounding = 8.0f` a `10.0f`.
- **Large Titles (Títulos Grandes do iOS):**
  - No cabeçalho da página ou início da rolagem, utilize fontes maiores em negrito para os títulos das seções principais, seguindo o padrão de design limpo e tipográfico da Apple.

### 3. Elementos de Interface e Controles do Sistema
- **Botão Voltar Nativo:** O telefone fornece um botão voltar na barra superior do cabeçalho que executa `closeApp()`. Ele usa a cor clássica azul do iOS com a label `< Voltar` (com ícone `ICON_FA_CHEVRON_LEFT`).
- **Home Indicator (Barra Inferior):** Respeite o espaço da barra horizontal fina que fica centralizada na parte inferior da tela, que serve para retornar à tela inicial. Evite desenhar botões interativos diretamente sobre ela.
- **Lista de Categoria / Agrupamento (List Groups):** Use listas agrupadas no estilo iOS (retângulos cinzas com cantos arredondados contendo linhas com divisores finos semi-transparentes de `ImVec4(1.0f, 1.0f, 1.0f, 0.08f)`).
- **Ícones:** Use exclusivamente ícones do FontAwesome 5 através das macros `ICON_FA_*`.

---

## 🛠️ Como Criar um Novo Aplicativo (`PhoneApp`)

Os novos aplicativos devem ser criados em arquivos `.h` (e opcionalmente `.cpp`) dentro da pasta `source/core/apps/`. Eles devem herdar de `PhoneApp` (`source/core/PhoneApp.h`) e implementar seus métodos virtuais.

#### Template Padrão de Aplicativo:
```cpp
#pragma once
#include "../PhoneApp.h"
#include <IconsFontAwesome5.h>

class NomeDoSeuApp : public PhoneApp {
public:
    NomeDoSeuApp() {
        id = "nome_do_app";
        icon = ICON_FA_GAMEPAD; // Usar macros FontAwesome 5
        name = "Nome Do App";
        color = ImVec4(0.12f, 0.56f, 1.00f, 1.0f); // Cor premium (evite cores puras básicas)
        dock = false; // Se true, fica no dock inferior (max 4)
    }

    void onOpen() override {
        // Inicialização quando o app abre
    }

    void onClose() override {
        // Limpeza quando o app fecha
    }

    void onDraw() override {
        // Renderização usando Dear ImGui
        ImGui::Text("Minha Interface");
    }
};
```

---

## 🔄 Fluxo de Trabalho de Registro

Depois de escrever a classe do app, registre-o nos dois pontos de entrada:

1. **Simulador Sandbox Desktop (`source/sandbox/SandboxMain.cpp`)**:
   - Incluir o header: `#include "../core/apps/NomeDoSeuApp.h"`
   - Declarar uma instância estática: `static NomeDoSeuApp seuAppInstance;`
   - Chamar `phone.registerApp(&seuAppInstance);` no setup do celular.

2. **GTA ASI Plugin (`source/gta/Main.cpp`)**:
   - Incluir o header: `#include "../core/apps/NomeDoSeuApp.h"`
   - Declarar a instância estática do app.
   - Chamar `phone.registerApp(&seuAppInstance);` no setup do celular.
