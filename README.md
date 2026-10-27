# vinheria-agenello

Este projeto consiste num sistema de automação e monitorização inteligente para adegas de vinho, desenvolvido com um Arduino Uno como controlador central. O sistema recolhe dados digitais do sensor de temperatura e humidade DHT e dados analógicos do sensor de luminosidade, estabelecendo comunicação com o display LCD e o módulo de relógio RTC através de I2C. O sistema executa uma análise inicial durante 10 segundos antes de iniciar o ciclo contínuo de funcionamento.

O projeto avalia em simultâneo a temperatura, a humidade e a luminosidade. O display LCD exibe uma interface dinâmica que alterna os parâmetros automaticamente a cada 3 segundos, apresentando os estados OK, ALTA ou BAIXA acompanhados do valor exato medido. Para a sinalização, o sistema aciona um LED verde durante o estado normal e um LED vermelho em conjunto com um buzzer quando os parâmetros não são ideais. Adicionalmente, o sistema regista a hora exata da ocorrência de qualquer anomalia e grava a informação na memória EEPROM para preservar os dados em caso de falha de energia.

Como diferenciais técnicos, o sistema garante que, ao atingir um limite crítico, o alerta visual e sonoro ocorre no exato milissegundo em que a informação correspondente aparece no ecrã, prevenindo confusões ou falsos alarmes. A gravação de logs utiliza um mecanismo circular: quando a memória atinge a capacidade máxima, o sistema retorna ao início de forma segura e sobrescreve os registos mais antigos, tudo isto sem travar ou corromper a execução do Arduino.

Criadores do projeto:
Caio Medeiros Machado /
Arthur Deocleciano de Araujo /
Guilherme Stock de Carvalho / 
Anauã Paulo Campolina Vieitas Lima /
Bruno Smoktunowicz Baptista
