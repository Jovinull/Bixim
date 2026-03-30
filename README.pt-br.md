# Bixim: Sistema Embarcado de Pet Virtual Baseado em ESP32

[English](README.md) | [Português](README.pt-br.md)

## Visão Geral do Projeto
O projeto **Bixim** (nome provisório/código) é uma implementação completa (hardware e software) de um dispositivo portátil de entretenimento digital, inspirado na arquitetura clássica dos pets virtuais de 1996. 

Diferente de emuladores tradicionais que dependem de dumps de ROMs proprietárias, este projeto utiliza uma abordagem de **Emulação de Alto Nível (HLE) e Lógica Customizada**. Isso garante total independência de propriedade intelectual de terceiros, configurando uma obra 100% autoral que demonstra proficiência em engenharia reversa de conceitos, desenvolvimento de sistemas embarcados e design de hardware.

## Arquitetura de Software e Escolha Tecnológica

O núcleo do sistema foi desenvolvido inteiramente em **C/C++17**. A escolha desta linguagem justifica-se pelos seguintes fatores:

1. **Gerenciamento de Memória e Performance:** O desenvolvimento bare-metal ou baseado em RTOS no ESP32 exige controle rigoroso sobre a alocação de memória e ciclos de CPU. C/C++ oferece a granularidade necessária para garantir uma taxa de atualização de quadros (FPS) estável no display OLED, sem os gargalos de linguagens interpretadas.
2. **Eficiência Energética:** O código compilado nativamente minimiza o tempo ativo do processador, permitindo o uso de estratégias de *deep sleep* do ESP32 de forma mais eficiente, o que é crítico para um dispositivo alimentado por uma pequena bateria LiPo.
3. **Interação com Hardware:** A manipulação direta de registradores, interrupções (ISRs) para leitura de botões e controle de modulação por largura de pulso (PWM) para o buzzer são implementados com máxima eficiência através das bibliotecas nativas em C/C++.

### Arquitetura de Software: HAL e Game Loop

O projeto implementa uma arquitetura de engine moderna, focada em comportamento determinístico e portabilidade:

* **Hardware Abstraction Layer (HAL):** Interfaces principais (`IDisplay`, `ITimer`) permitem que o mesmo código de lógica rode sem modificações tanto no Windows (usando Raylib para simulação) quanto no ESP32 (usando drivers SSD1306 via I2C).
* **Arquitetura do Game Loop:** Utiliza um **Acumulador de Timestep Fixo** (Fixed Timestep Accumulator). Isso desacopla a lógica (rodando a 10 Hz fixos) da renderização (alvo de 30 FPS), garantindo velocidade de simulação consistente independente de variações na CPU ou taxa de atualização do display.
* **Lógica de Estado (Game Engine Customizada):** A engine interna implementa uma máquina de estados finitos (FSM) que simula o ciclo orgânico do sistema:
    * **Módulo de Vida:** Gerencia variáveis de decaimento temporal (Fome, Sono, Felicidade, Idade).
    * **Módulo de Renderização:** Uma classe **FrameBuffer** customizada gerencia a matriz de bits 128x64 monocromática, otimizada para transferências I2C.
    * **Módulo de Interrupção:** Lida com o debouncing dos botões físicos via hardware/software.

## Especificações de Hardware (Bill of Materials - BOM)

O projeto físico foi arquitetado para ser modular durante a fase de prototipagem (protoboard) e altamente miniaturizado para a versão de produção.

* **Microcontrolador: ESP32 (WROOM ou C3)**
    * Responsável pelo processamento central. A inclusão do ESP32 permite futuras atualizações Over-The-Air (OTA) e comunicação sem fio via Wi-Fi/Bluetooth (ESP-NOW).
* **Display: Módulo OLED 0.96" (Controlador SSD1306)**
    * Interface de vídeo monocromática de alto contraste e baixíssimo consumo energético. Comunicação realizada via protocolo I2C (SDA/SCL), ocupando apenas dois pinos de dados do microcontrolador.
* **Áudio: Buzzer Piezoelétrico Passivo**
    * Diferente de buzzers ativos, a versão passiva permite a síntese de ondas quadradas via PWM em diferentes frequências, possibilitando a recriação fiel de trilhas sonoras 8-bit e feedback sonoro dinâmico.
* **Interface de Entrada: Chaves Tácteis (Push Buttons 6x6x4.3mm)**
    * Três botões dispostos estrategicamente para navegação de menus (Selecionar, Confirmar, Cancelar), com resistores de pull-up internos ativados no microcontrolador.
* **Sistema de Alimentação e Gerenciamento de Bateria:**
    * **Bateria:** Célula LiPo de 3.7V (aprox. 300mAh - 500mAh).
    * **Módulo de Carga:** Placa TP4056 com interface USB-C para carregamento seguro da célula de lítio.
    * **Chave Deslizante (Slide Switch):** Controle de corte de energia a nível de hardware.

## Design Físico e Montagem

A evolução do projeto contempla a transição de um circuito de matriz de contatos para um produto final portátil. A carcaça foi projetada considerando a ergonomia e a restrição de espaço dos componentes eletrônicos.

A etapa de prototipagem física e manufatura da carcaça final pode ser realizada através de técnicas de fabricação digital, como impressão 3D (aproveitando os serviços de impressão e espaços makers disponíveis em Aracaju e região) ou através de técnicas de *retrofitting*, adaptando o circuito interno a carcaças de dispositivos clássicos obsoletos.

## Roadmap e Funcionalidades Futuras

- [x] Prototipagem da lógica de estados e renderização na matriz OLED (Timestep Fixo & HAL implementados).
- [x] Implementação da camada de entrada abstraída e debouncing (IInput HAL).
- [ ] Implementação de áudio (Buzzer PWM).
- [ ] Otimização de consumo energético (Sleep Modes do ESP32).
- [ ] Integração do hardware final na carcaça.
- [ ] **Multiplayer via ESP-NOW:** Implementação de comunicação ponto-a-ponto de baixa latência para interação entre dois dispositivos físicos próximos.
- [ ] **Integração Educacional:** Implementação de animações customizadas avançadas, incluindo um módulo experimental para o pet ensinar sinais básicos de Libras ao usuário.

## Configuração do Ambiente de Desenvolvimento (Getting Started)

Esta seção cobre a configuração completa para uma máquina Windows limpa. Siga cada passo na ordem indicada.

### Visão Geral dos Pré-requisitos

| Ferramenta | Finalidade |
|---|---|
| VS Code | IDE |
| Extensão PlatformIO IDE | Sistema de build, gerenciador de bibliotecas, uploader |
| MSYS2 + MinGW-w64 | Toolchain de compilador C++ para o build `native` (PC) |
| Raylib (via MSYS2) | Biblioteca gráfica para o build de debug no PC |

---

### Passo 1 — Instalar o VS Code

1. Acesse **code.visualstudio.com** e baixe o instalador **Windows x64**.
2. Execute o instalador. Durante a configuração, marque **"Add to PATH"** e **"Register Code as an editor for supported file types"**.
3. Verifique a instalação abrindo um novo terminal (`Win + R` -> `cmd`) e executando:
   ```
   code --version
   ```
   Deve exibir um número de versão (ex: `1.89.x`).

---

### Passo 2 — Instalar as Extensões do VS Code

Este repositório inclui um arquivo `.vscode/extensions.json` com todas as extensões necessárias listadas. Ao abrir a pasta do projeto no VS Code pela primeira vez, um popup aparecerá automaticamente perguntando se deseja instalar as extensões recomendadas. Clique em **"Install All"**.

Se o popup não aparecer, instale manualmente via `Ctrl + Shift + X`. As extensões necessárias e seus IDs exatos são:

| Extensão | Publisher | ID |
|---|---|---|
| PlatformIO IDE | PlatformIO | `platformio.platformio-ide` |
| C/C++ | Microsoft | `ms-vscode.cpptools` |
| C/C++ Extension Pack | Microsoft | `ms-vscode.cpptools-extension-pack` |
| C/C++ Themes | Microsoft | `ms-vscode.cpptools-themes` |
| CMake Tools | Microsoft | `ms-vscode.cmake-tools` |
| Makefile Tools | Microsoft | `ms-vscode.makefile-tools` |
| clangd | LLVM | `llvm-vs-code-extensions.vscode-clangd` |

> **AVISO — PlatformIO:** Existem extensões de terceiros com nomes similares publicadas por autores não oficiais. Instale **apenas** a extensão com ID `platformio.platformio-ide` (publisher: **PlatformIO**). Qualquer outra falhará em instalar o PlatformIO Core e o comando `pio` não estará disponível. Sempre verifique o ID da extensão antes de clicar em Install.

Após instalar o PlatformIO IDE, o VS Code será recarregado. Um ícone do PlatformIO (cabeça alienígena) deve aparecer na barra lateral esquerda. Clique nele e aguarde o carregamento da tela inicial.

> **Nota:** O PlatformIO gerencia seu próprio ambiente Python internamente. Não é necessário instalar o Python separadamente.

---

### Passo 2.5 — Adicionar o CLI do PlatformIO (`pio`) ao PATH do Sistema

A extensão do PlatformIO instala o executável `pio` em uma pasta local do usuário que **não** é adicionada ao PATH do sistema automaticamente. Sem este passo, executar `pio` no PowerShell ou Prompt de Comando falhará com "comando não reconhecido", mesmo que a extensão funcione dentro do VS Code.

**Verifique se o executável existe:**
```powershell
Test-Path "C:\Users\$env:USERNAME\.platformio\penv\Scripts\pio.exe"
```
Se retornar `True`, prossiga. Se retornar `False`, abra o VS Code, aguarde o PlatformIO terminar a configuração inicial (barra de progresso na barra de status inferior) e verifique novamente.

**Adicionar ao PATH do sistema:**
1. Abra **Iniciar** -> pesquise **"Editar as variáveis de ambiente do sistema"**.
2. Clique em **"Variáveis de Ambiente..."**.
3. Em **Variáveis do sistema**, selecione `Path` -> clique em **Editar** -> clique em **Novo**.
4. Adicione o caminho abaixo (substitua `felip` pelo seu nome de usuário Windows):
   ```
   C:\Users\felip\.platformio\penv\Scripts
   ```
5. Clique em **OK** em todos os diálogos.

**Abra uma nova janela do PowerShell** (a mudança de PATH só se aplica a terminais novos) e verifique:
```
pio --version
```
Saída esperada: `PlatformIO Core, version 6.x.x`

> **Nota:** Os comandos `pio` também podem ser executados pelo terminal integrado do VS Code (`Ctrl + ` `) sem esta alteração de PATH, pois o VS Code configura seu próprio ambiente de terminal. O passo de PATH só é necessário para executar `pio` em janelas standalone de PowerShell ou Prompt de Comando.

---

### Passo 3 — Instalar MSYS2 e a Toolchain MinGW-w64

O MSYS2 fornece o compilador GCC/G++ necessário para buildar e executar o ambiente `native` (PC).

1. Acesse **msys2.org** e baixe o instalador (`msys2-x86_64-YYYYMMDD.exe`).
2. Execute o instalador. Aceite o caminho de instalação padrão: `C:\msys64`.
3. Após a instalação, abra o shell **"MSYS2 MINGW64"** (não o MSYS2 MSYS).
4. Atualize o banco de dados de pacotes:
   ```bash
   pacman -Syu
   ```
   O terminal fechará automaticamente. Reabra o shell MINGW64 e execute:
   ```bash
   pacman -Su
   ```
5. Instale a toolchain GCC do MinGW-w64:
   ```bash
   pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-gdb make
   ```
6. Adicione o MinGW-w64 ao PATH do sistema Windows:
   - Abra **Iniciar** -> pesquise **"Editar as variáveis de ambiente do sistema"**.
   - Clique em **"Variáveis de Ambiente..."**.
   - Em **Variáveis do sistema**, selecione `Path` e clique em **Editar**.
   - Clique em **Novo** e adicione: `C:\msys64\mingw64\bin`
   - Clique em **OK** em todos os diálogos.
7. Verifique em uma **nova** janela do Prompt de Comando ou PowerShell:
   ```
   g++ --version
   ```
   Saída esperada: `g++.exe (Rev..., Built by MSYS2 project) 14.x.x`

---

### Passo 4 — Instalar o Raylib via MSYS2

O Raylib é a biblioteca gráfica usada pelo build `native` para simular o display OLED no PC.

1. Abra o shell **MSYS2 MINGW64**.
2. Execute:
   ```bash
   pacman -S mingw-w64-x86_64-raylib
   ```
3. Verifique a instalação:
   ```bash
   ls /mingw64/include/raylib.h
   ls /mingw64/lib/libraylib.a
   ```
   Ambos os arquivos devem existir. Se existirem, o Raylib está pronto. O `platformio.ini` deste repositório já está pré-configurado para encontrar esses caminhos em `C:/msys64/mingw64/`.

---

### Passo 5 — Clonar o Repositório e Abrir no VS Code

```bash
git clone https://github.com/jovinull/bixim.git
cd bixim
code .
```

O VS Code abrirá o projeto. O PlatformIO detectará o `platformio.ini` e indexará o projeto automaticamente (pode levar 1 a 2 minutos na primeira abertura).

---

### Passo 6 — Build e Execução: Ambiente Native (PC)

O ambiente `native` compila o projeto como um executável Windows padrão usando o Raylib para renderização. Use este alvo para todo o desenvolvimento de lógica e depuração antes de gravar no hardware.

**Via VS Code:**
- Clique no ícone do PlatformIO na barra lateral.
- Em **Project Tasks -> native**, clique em **Build** e depois em **Upload and Monitor** (que executa o binário).

**Via terminal (na raiz do projeto):**
```bash
# Apenas compilar
pio run -e native

# Compilar e executar o binário
pio run -e native -t exec
```

Uma janela intitulada **"Bixim - Native Debug Build"** deve abrir.

---

### Passo 7 — Build e Flash: Ambiente ESP32

1. Conecte sua placa ESP32 via USB.
2. Identifique a porta COM: abra o **Gerenciador de Dispositivos** (`Win + X` -> Gerenciador de Dispositivos) -> expanda **Portas (COM e LPT)** -> anote a porta (ex: `COM3`).
3. No VS Code, o PlatformIO detecta a porta automaticamente. Se não detectar, adicione `upload_port = COMX` à seção `[env:esp32dev]` no `platformio.ini`.

**Via VS Code:**
- Em **Project Tasks -> esp32dev**, clique em **Upload**.

**Via terminal:**
```bash
# Compilar e gravar
pio run -e esp32dev -t upload

# Abrir o monitor serial (115200 baud) após gravar
pio device monitor
```

Saída serial esperada:
```
[Bixim] Booting...
[Bixim] Display OK. Boot complete.
```

---

### Estrutura de Diretórios do Projeto

```
bixim/
├── platformio.ini      # Configuração do sistema de build (todos os ambientes)
├── src/
│   └── main.cpp        # Ponto de entrada unificado com guards #ifdef por plataforma
├── include/            # Arquivos de cabeçalho compartilhados (.h / .hpp)
├── lib/                # Bibliotecas locais (específicas do projeto, não do registry)
├── hardware/           # Esquemáticos, arquivos KiCad, BOM
├── docs/               # Marcos técnicos e documentação de arquitetura
├── README.md
└── README.pt-br.md
```

---

## Como Executar o Projeto

1. Clone o repositório: `git clone https://github.com/jovinull/bixim.git`
2. Siga a seção **Configuração do Ambiente de Desenvolvimento** acima.
3. Build para PC: `pio run -e native -t exec`
4. Build e flash para ESP32: `pio run -e esp32dev -t upload`
5. Siga o esquemático elétrico no diretório `/hardware` para a montagem dos periféricos.

## Licença

Este projeto é de código aberto e está licenciado sob a Licença MIT. Sinta-se à vontade para fazer um fork, estudar a arquitetura e contribuir com melhorias.
