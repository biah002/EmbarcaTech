🛡️ EmbarcaTech Security IoT - RP2040

Sistema de monitoramento de segurança inteligente com telemetria em nuvem e interface interativa, utilizando a Raspberry Pi Pico W.



📋 Sobre o Projeto

Este projeto foi desenvolvido como parte do programa EmbarcaTech. 
Ele transforma uma Raspberry Pi Pico W num sistema de segurança capaz de detetar intrusões, monitorizar a temperatura e enviar dados para a plataforma ThingSpeak.



🚀 Funcionalidades

Monitoramento: Deteta movimentos via Joystick (Eixos X/Y).

Alarme: Feedback sonoro (Buzzer) e visual (LED RGB) em caso de intrusão.

IoT: Envio automático do contador de alertas para a nuvem via HTTP.

Interface: Menu de seleção e status do sistema num display OLED SSD1306.

Entretenimento: Inclui um mini-game interativo para demonstração de performance.



📁 Estrutura do Repositório

src/: Código-fonte (.c) e ficheiros de hardware (.pio).

include/: Ficheiros de cabeçalho (.h) e configurações de rede.

CMakeLists.txt: Configuração de compilação do projeto.



🛠️ Requisitos e Compilação

Ter o Pico SDK v2.2.0 configurado no ambiente.

(Opcional) Criar uma conta no ThingSpeak e obter uma API Key.

Compilar o projeto

Carregar o ficheiro Projeto_Finale.uf2 na placa.



⚖️ Licença e Créditos

Este projeto foi desenvolvido por Ana Beatriz Rocha como projeto final.

Licença: MIT

Ferramentas: Pico SDK, LWIP, SSD1306 Library.
