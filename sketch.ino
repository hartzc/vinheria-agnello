#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <EEPROM.h>

// Flag para alternar entre simulação de apresentação e leitura real
#define MODO_DEMONSTRACAO 1 

// Definições de pinos e sensores
#define DHTPIN 2       
#define DHTTYPE DHT22  // Utilizando DHT22 no simulador (alterar para DHT11 no hardware físico)
#define LDR_PIN A0     
#define LED_GREEN 3    
#define LED_RED 4      
#define BUZZER 5       

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;

// Variáveis para cálculo de média móvel
unsigned long tempoAnterior = 0;
float somaTemp = 0, somaUmid = 0, somaLuz = 0;
int leituras = 0;

// Variáveis para armazenar o estado atual das médias
float mediaTemp = 0, mediaUmid = 0, mediaLuz = 0;
bool temMedia = false;

// Controle de temporização e interface do Display
unsigned long tempoDisplay = 0;
int telaAtual = 0; 
int cenarioAtual = 0; // Utilizado apenas no modo de demonstração

// Ponteiro para gravação sequencial na memória EEPROM
int addrEEPROM = 0; 

void setup() {
  Serial.begin(9600);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  
  // Inicializa os atuadores em estado inativo
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  noTone(BUZZER);
  
  dht.begin();
  lcd.init();
  lcd.backlight();
  
  // Inicialização e verificação do módulo RTC
  if (!rtc.begin()) {
    Serial.println("Erro: RTC nao encontrado!");
  }
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (MODO_DEMONSTRACAO) {
    Serial.println("--- MODO DEMONSTRACAO ATIVADO ---");
  }

  // Animação inicial com o logo do projeto no display LCD
  lcd.setCursor(0, 0); 
  lcd.print("Vinheria Agnello"); 
  for(int i = 0; i < 16; i++) {
    lcd.setCursor(i, 1);
    lcd.print(">");
    delay(100); 
  }
  
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("A analisar...");
}

void loop() {
  float t, h;
  int luz;

  // 1. LEITURA DOS DADOS (Cenários de apresentação ou sensores físicos)
  if (MODO_DEMONSTRACAO == 1) {
    // Mock de dados para simular os diferentes estados de alerta na apresentação
    if (cenarioAtual == 0) {
      t = 14.0; h = 60.0; luz = 30; // Estado normal
    } else if (cenarioAtual == 1) {
      t = 22.0; h = 60.0; luz = 30; // Alerta crítico de temperatura
    } else {
      t = 14.0; h = 40.0; luz = 85; // Alerta crítico de umidade e luminosidade
    }
  } else {
    // Leitura real dos hardwares
    t = dht.readTemperature();
    h = dht.readHumidity();
    int valorLDR = analogRead(LDR_PIN);
    luz = map(valorLDR, 0, 1023, 0, 100); 
  }
  
  // Tratamento do valor de luz para garantir escala percentual de 0 a 100
  luz = constrain(luz, 0, 100);

  // Acumula os valores válidos para calcular a média da janela de tempo
  if (!isnan(t) && !isnan(h)) {
    somaTemp += t;
    somaUmid += h;
    somaLuz += luz;
    leituras++;
  }

  // 2. PROCESSAMENTO E LÓGICA DE ALERTAS (Janela móvel de 10 segundos)
  if (millis() - tempoAnterior >= 10000) {
    tempoAnterior = millis();
    
    if (leituras > 0) {
      mediaTemp = somaTemp / leituras;
      mediaUmid = somaUmid / leituras;
      mediaLuz = somaLuz / leituras;
      temMedia = true;
      
      // Zera os acumuladores para o próximo ciclo de medição
      somaTemp = 0; somaUmid = 0; somaLuz = 0; leituras = 0;

      // Validação das regras de negócio (limites ideais de conservação do vinho)
      bool alerta = false;
      if (mediaTemp > 18.0 || mediaTemp < 10.0 || mediaUmid > 70.0 || mediaUmid < 50.0 || mediaLuz > 50.0) {
        alerta = true;
      }

      if (alerta) {
        // Aciona os atuadores de emergência sonoros e visuais
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
        tone(BUZZER, 1000); 
        
        // Registra o evento de anomalia na memória não volátil (EEPROM) com timestamp do RTC
        DateTime now = rtc.now();
        EEPROM.write(addrEEPROM, now.hour());
        EEPROM.write(addrEEPROM + 1, now.minute());
        EEPROM.write(addrEEPROM + 2, 1);
        
        addrEEPROM += 3;
        if (addrEEPROM >= EEPROM.length()) addrEEPROM = 0; // Previne buffer overflow na EEPROM
        
        Serial.print("[ALERTA CRITICO] Log salvo. UTC - "); 
        Serial.print(now.hour()); Serial.print(":"); Serial.println(now.minute());
      } else {
        // Mantém/retorna ao estado operacional normal
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, HIGH);
        noTone(BUZZER);
      }

      // Força a atualização do display para refletir instantaneamente o estado atual
      tempoDisplay = millis() - 3000; 
      telaAtual = 0; 

      // Incrementa o cenário de testes para o próximo ciclo de demonstração
      if (MODO_DEMONSTRACAO == 1) {
        cenarioAtual++;
        if (cenarioAtual > 2) cenarioAtual = 0; 
      }
    }
  }

  // 3. INTERFACE DE USUÁRIO (Alternância do carrossel de telas no LCD a cada 3 segundos)
  if (temMedia && (millis() - tempoDisplay >= 3000)) {
    tempoDisplay = millis();
    lcd.clear();
    lcd.setCursor(0, 0);
    
    if (telaAtual == 0) {
      // Renderização do sensor de Temperatura
      if (mediaTemp < 10.0)      lcd.print("Temp. BAIXA");
      else if (mediaTemp > 18.0) lcd.print("Temp. ALTA");
      else                       lcd.print("Temperatura OK");
      
      lcd.setCursor(0, 1);
      lcd.print("Temp. = "); lcd.print(mediaTemp, 1); lcd.print("C");
      telaAtual = 1;
      
    } else if (telaAtual == 1) {
      // Renderização do sensor de Umidade
      if (mediaUmid < 50.0)      lcd.print("Umidade BAIXA");
      else if (mediaUmid > 70.0) lcd.print("Umidade ALTA");
      else                       lcd.print("Umidade OK");
      
      lcd.setCursor(0, 1);
      lcd.print("Umidade = "); lcd.print(mediaUmid, 0); lcd.print("%");
      telaAtual = 2;
      
    } else if (telaAtual == 2) {
      // Renderização do sensor LDR
      if (mediaLuz > 50.0) {
        lcd.print("Ambiente muito");
        lcd.setCursor(0, 1);
        lcd.print("CLARO");
      } else {
        lcd.print("Ambiente a meia");
        lcd.setCursor(0, 1);
        lcd.print("luz");
      }
      telaAtual = 0; // Reinicia o carrossel
    }
  }
  
  delay(50);
}