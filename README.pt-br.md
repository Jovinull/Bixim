# Bixim: Sistema Embarcado de Pet Virtual Baseado em ESP32

[English](README.md) | [Português](README.pt-br.md)

## Visão Geral do Projeto
O projeto **Bixim** (nome provisório/código) é uma implementação completa (hardware e software) de um dispositivo portátil de entretenimento digital, inspirado na arquitetura clássica dos pets virtuais de 1996. 

Diferente de emuladores tradicionais que dependem de dumps de ROMs proprietárias, este projeto utiliza uma abordagem de **Emulação de Alto Nível (HLE) e Lógica Customizada**. Isso garante total independência de propriedade intelectual de terceiros, configurando uma obra 100% autoral que demonstra proficiência em engenharia reversa de conceitos, desenvolvimento de sistemas embarcados e design de hardware.

## Arquitetura de Software e Escolha Tecnológica

O núcleo do sistema foi desenvolvido inteiramente em **C/C++**. A escolha desta linguagem justifica-se pelos seguintes fatores:

1.  **Gerenciamento de Memória e Performance:** O desenvolvimento bare-metal ou baseado em RTOS no ESP32 exige controle rigoroso sobre a alocação de memória e ciclos de CPU. C/C++ oferece a granularidade necessária para garantir uma taxa de atualização de quadros (FPS) estável no display OLED, sem os gargalos de linguagens interpretadas.
2.  **Eficiência Energética:** O código compilado nativamente minimiza o tempo ativo do processador, permitindo o uso de estratégias de *deep sleep* do ESP32 de forma mais eficiente, o que é crítico para um dispositivo alimentado por uma pequena bateria LiPo.
3.  **Interação com Hardware:** A manipulação direta de registradores, interrupções (ISRs) para leitura de botões e controle de modulação por largura de pulso (PWM) para o buzzer são implementados com máxima eficiência através das bibliotecas nativas em C/C++.

### Lógica de Estado (Game Engine Customizada)
A engine interna não emula o processador Epson S1C63 original. Em vez disso, ela implementa uma máquina de estados finitos (FSM) que simula o ciclo orgânico do sistema:
* **Módulo de Vida:** Gerencia variáveis de decaimento temporal (Fome, Sono, Felicidade, Idade).
* **Módulo de Renderização:** Transforma matrizes de bits em sinais I2C para atualização do display OLED.
* **Módulo de Interrupção:** Lida com o debouncing dos botões físicos via hardware/software para garantir respostas precisas do usuário.

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

- [ ] Prototipagem da lógica de estados e renderização na matriz OLED.
- [ ] Implementação do sistema de debouncing e controle PWM de áudio.
- [ ] Otimização de consumo energético (Sleep Modes do ESP32).
- [ ] Integração do hardware final na carcaça.
- [ ] **Multiplayer via ESP-NOW:** Implementação de comunicação ponto-a-ponto de baixa latência para interação entre dois dispositivos físicos próximos.
- [ ] **Integração Educacional:** Implementação de animações customizadas avançadas, incluindo um módulo experimental para o pet ensinar sinais básicos de Libras ao usuário.

## Como Executar o Projeto

1. Clone o repositório: `git clone https://github.com/jovinull/bixim.git`
2. Configure o ambiente de desenvolvimento: Recomenda-se o uso do VS Code em conjunto com a extensão PlatformIO.
3. Instale as dependências listadas no arquivo `platformio.ini` (ex: bibliotecas Adafruit SSD1306 e GFX).
4. Compile e faça o upload para o seu módulo ESP32.
5. Siga o esquemático elétrico detalhado no diretório `/hardware` para a montagem dos periféricos.

## Licença

Este projeto é de código aberto e está licenciado sob a Licença MIT. Sinta-se à vontade para fazer um fork, estudar a arquitetura e contribuir com melhorias.
