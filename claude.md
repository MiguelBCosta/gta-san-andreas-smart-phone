# Guia de Desenvolvimento & Instruções de IA (claude.md)

Este documento serve como referência de arquitetura, padrões de projeto e diretrizes para desenvolvedores e assistentes de Inteligência Artificial (como Antigravity e Claude) que trabalham no projeto **SaSmartPhone**.

---

## 📱 Visão Geral do Projeto

O **SaSmartPhone** é um simulador de smartphone de última geração integrado como um plugin `.asi` para o jogo **Grand Theft Auto: San Andreas (GTA:SA)**. Ele é escrito em **C++** e utiliza a biblioteca **Dear ImGui** para renderização da interface e **FontAwesome 5** para ícones vetoriais.

Para permitir o desenvolvimento rápido de aplicativos e estilização sem precisar abrir o jogo a todo momento, o projeto é dividido em duas partes principais:
1. **ASI Plugin (GTA:SA):** Injeta hooks no jogo usando as bibliotecas Kiero e MinHook, integrando-se aos sistemas nativos do GTA:SA.
2. **Sandbox Simulator:** Um aplicativo desktop standalone (`.exe`) que renderiza o celular em uma janela dedicada, útil para desenvolvimento rápido.

---

## 📂 Estrutura de Diretórios

* **`source/core/`**: Código principal do celular e lógica independente de plataforma.
  * [Phone.h](file:///e:/projects/utility-phone/SaSmartPhone/source/core/Phone.h) / `Phone.cpp`: Controlador do estado do telefone, barra de status, animações, dock e desenho da interface principal.
  * [PhoneApp.h](file:///e:/projects/utility-phone/SaSmartPhone/source/core/PhoneApp.h): Classe base abstrata para todos os aplicativos do celular.
  * `source/core/apps/`: Implementação de cada aplicativo (ex: [WeatherApp.h](file:///e:/projects/utility-phone/SaSmartPhone/source/core/apps/WeatherApp.h), `CalculatorApp`, `ClockApp`, `NotesApp`).
  * `source/core/providers/`: Interfaces abstratas dos Provedores (Pattern Provider).
* **`source/gta/`**: Lógica específica do GTA:SA.
  * `Main.cpp`: Ponto de entrada do plugin ASI e inicialização dos hooks de renderização.
  * `source/gta/providers/`: Implementações dos provedores integrados com a API do GTA:SA (`plugin-sdk`).
* **`source/sandbox/`**: Lógica do simulador de desenvolvimento.
  * `SandboxMain.cpp`: Ponto de entrada da versão standalone.
  * `source/sandbox/providers/`: Implementações mockadas de provedores para rodar fora do jogo.
* **`vendor/`**: Dependências externas (ImGui, FontAwesome, Kiero, MinHook).

---

## 🏗️ Padrões de Arquitetura e Design

### 1. Desacoplamento via Provedores (Provider Pattern)
Para manter o núcleo do celular e seus aplicativos rodando de forma idêntica tanto no GTA:SA quanto no Sandbox, **nenhum código de lógica do jogo ou do sistema operacional deve ser chamado diretamente** no `core`. 

Em vez disso, utilizamos provedores abstratos. Por exemplo:
* `IClockProvider` ([IClockProvider.h](file:///e:/projects/utility-phone/SaSmartPhone/source/core/providers/IClockProvider.h)): Abstrai o relógio do sistema.
  * No GTA, retorna o horário in-game.
  * No Sandbox, retorna o horário real do computador.
* `IWeatherProvider` ([IWeatherProvider.h](file:///e:/projects/utility-phone/SaSmartPhone/source/core/providers/IWeatherProvider.h)): Abstrai o clima.
  * No GTA, obtém o clima atual e intensidade da chuva do jogo.
  * No Sandbox, retorna valores controlados por sliders de teste.

> [!IMPORTANT]  
> **Regra de Ouro:** Ao adicionar qualquer funcionalidade que dependa de leitura ou escrita no estado do jogo (ex: GPS, nível de procurado, dinheiro, contatos), **crie uma interface em `providers/`**, implemente-a separadamente em `gta/providers/` e `sandbox/providers/`, e registre-a na inicialização.

### 2. Layout do Smartphone (Constantes de Interface)
O layout do celular tenta replicar os valores de posicionamento e proporção do projeto irmão em Lua (`MoonPhone`). Essas constantes estão definidas em [Phone.h](file:///e:/projects/utility-phone/SaSmartPhone/source/core/Phone.h):
* **Largura (PH_W):** `320.0f`
* **Altura (PH_H):** `620.0f`
* **Margem da Tela (BEZEL):** `12.0f`
* **Arredondamento da Tela (SCR_R):** `26.0f`
* **Tamanho de Ícones:** `58.0f` (Home Grid) e `52.0f` (Dock)

### 3. Criando um Novo Aplicativo (`PhoneApp`)
Para criar um aplicativo, herde de `PhoneApp` e implemente os métodos virtuais:

```cpp
#pragma once
#include "../PhoneApp.h"
#include <IconsFontAwesome5.h>

class MinhaApp : public PhoneApp {
public:
    MinhaApp() {
        id = "minha_app";
        icon = ICON_FA_GAMEPAD; // Macro do FontAwesome
        name = "Meu Jogo";
        color = ImVec4(0.8f, 0.1f, 0.4f, 1.0f); // Cor do ícone na home
        dock = false; // Define se fica no dock inferior
    }

    void onOpen() override {
        // Inicialização opcional quando abre o app
    }

    void onClose() override {
        // Limpeza opcional quando fecha o app
    }

    void onDraw() override {
        // Renderização principal do app usando Dear ImGui
        ImGui::Text("Olá Mundo!");
    }
};
```

---

## 🛠️ Compilação e Configurações no Visual Studio

Ao abrir o arquivo [SaSmartPhone.sln](file:///e:/projects/utility-phone/SaSmartPhone/SaSmartPhone.sln) no Visual Studio, você tem duas configurações importantes de projeto:
* **SaSmartPhone (Plugin ASI):** Compila o arquivo `.asi`.
  * *Configurações:* `Debug GTA-SA` ou `Release GTA-SA`.
* **SaSmartPhone_Sandbox (Desktop App):** Compila o `.exe` standalone.
  * *Configurações:* `Debug Sandbox` ou `Release Sandbox`.

---

## 📝 Regras e Boas Práticas para IAs

* **Não commitar configurações locais:** Arquivos `.user` e `imgui.ini` estão no `.gitignore`. Nunca tente adicioná-los de volta.
* **Ajustes de UI:** O smartphone usa estilo iOS (cantos arredondados, fontes modernas, transparências, gradientes sutis). Use `ImDrawList` quando precisar desenhar elementos estilizados complexos (como cards com gradientes ou layouts circulares).
* **Botão Voltar nativo:** O telefone possui um botão voltar na barra superior quando um aplicativo está ativo. No entanto, se um app precisar de submenus ou abas de navegação, desenhe setas de retorno estilizadas usando FontAwesome (ex: `ICON_FA_ARROW_LEFT`) em vez de botões de texto simples.
* **Fontes & Ícones:** Ícones são sempre baseados em FontAwesome 5. Importe `<IconsFontAwesome5.h>` e use as constantes `ICON_FA_*`.
