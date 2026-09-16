#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>

// Display LCD (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(12, 14, 27, 25, 33, 32);

// Pinos
const int PIN_BUZZER = 18;
const int PIN_LED_VERMELHO = 4;
const int PIN_LED_VERDE = 2;
const int PIN_BOTAO = 5; // GPIO 5

// Dados da Rede Wi-Fi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configurações da API do Supabase
const char* supabaseUrl = "https://dyerfgakougflmiarogl.supabase.co/rest/v1/estacoes?id=eq.ESP32_01&select=*";
const char* supabaseApiKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImR5ZXJmZ2Frb3VnZmxtaWFyb2dsIiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODc2Mzk2MTQsImV4cCI6MjEwMzIxNTYxNH0.4N5pABfNlfN3xx6ipiHvLLVgpeUEjza6WdxWpaFkbuw";

// Estado do Sistema
bool pagamentoConfirmado = false;
bool liberadoPeloBotao = false;
bool recargaConcluida = false;

// Cronômetro
unsigned long tempoTotalSegundos = 0;
unsigned long tempoRestanteSegundos = 0;
unsigned long ultimoSegundo = 0;
unsigned long ultimaChecagemSupabase = 0;

// Leitura do Botão
bool estadoAnteriorBotao = HIGH;

// Efeitos Sonoros
void tocarSomAprovado() {
  tone(PIN_BUZZER, 1500, 150); delay(200);
  tone(PIN_BUZZER, 2000, 300); delay(300);
  noTone(PIN_BUZZER);
}

void tocarSomConcluido() {
  tone(PIN_BUZZER, 1000, 200); delay(250);
  tone(PIN_BUZZER, 1500, 200); delay(250);
  tone(PIN_BUZZER, 2000, 400); delay(400);
  noTone(PIN_BUZZER);
}

void checarStatusSupabase() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(supabaseUrl);
    http.addHeader("apikey", supabaseApiKey);
    http.addHeader("Authorization", String("Bearer ") + supabaseApiKey);

    int httpCode = http.GET();

    if (httpCode == 200) {
      String payload = http.getString();
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error && doc.size() > 0) {
        const char* status = doc[0]["pagamento_status"];

        if (String(status) == "aprovado") {
          float energiakWh = doc[0]["energia_solicitada_kwh"] | 22.0;
          float tempoHoras = energiakWh / 22.0;

          tempoTotalSegundos = (unsigned long)(tempoHoras * 3600);
          tempoRestanteSegundos = tempoTotalSegundos;

          pagamentoConfirmado = true;
          tocarSomAprovado();
          
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Pgt Confirmado!");
          lcd.setCursor(0, 1);
          lcd.print("Aperte o Botao");
        }
      }
    }
    http.end();
  }
}

void checarBotao() {
  bool leitura = digitalRead(PIN_BOTAO);

  // Detecta quando o botão é pressionado (LOW por causa do INPUT_PULLUP)
  if (leitura == LOW && estadoAnteriorBotao == HIGH) {
    delay(50); // Debounce para evitar ruído mecânico
    if (digitalRead(PIN_BOTAO) == LOW) {
      liberadoPeloBotao = true;

      digitalWrite(PIN_LED_VERMELHO, LOW);
      digitalWrite(PIN_LED_VERDE, HIGH);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Acesso Liberado");
      lcd.setCursor(0, 1);
      lcd.print("Iniciando Carga");
      tocarSomAprovado();
      delay(2000);

      lcd.clear();
      ultimoSegundo = millis();
    }
  }
  estadoAnteriorBotao = leitura;
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_VERMELHO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_BOTAO, INPUT_PULLUP);

  digitalWrite(PIN_LED_VERMELHO, HIGH);
  digitalWrite(PIN_LED_VERDE, LOW);

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("   GoodWe Smart   ");
  lcd.setCursor(0, 1);
  lcd.print("Conectando WiFi");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aguardando Pgt..");
  lcd.setCursor(0, 1);
  lcd.print("Status: Pendente");
}

void loop() {
  // Etapa 1: Aguarda aprovação do pagamento no Supabase
  if (!pagamentoConfirmado) {
    if (millis() - ultimaChecagemSupabase >= 3000) {
      ultimaChecagemSupabase = millis();
      checarStatusSupabase();
    }
  }
  // Etapa 2: Pagamento Aprovado -> Aguarda 1 clique no Botão
  else if (pagamentoConfirmado && !liberadoPeloBotao) {
    checarBotao();
  }
  // Etapa 3: Botão Pressionado -> Inicia contagem regressiva de 60 min
  else if (liberadoPeloBotao && !recargaConcluida) {
    if (millis() - ultimoSegundo >= 1000) {
      ultimoSegundo = millis();

      if (tempoRestanteSegundos > 0) {
        tempoRestanteSegundos--;

        int minutos = tempoRestanteSegundos / 60;
        int segundos = tempoRestanteSegundos % 60;

        lcd.setCursor(0, 0);
        lcd.print("Carregando EV...");

        lcd.setCursor(0, 1);
        lcd.print("Tempo: ");
        if (minutos < 10) lcd.print("0");
        lcd.print(minutos);
        lcd.print("m:");
        if (segundos < 10) lcd.print("0");
        lcd.print(segundos);
        lcd.print("s   ");

      } else {
        recargaConcluida = true;
        digitalWrite(PIN_LED_VERDE, LOW);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" Recarga 100% ");
        lcd.setCursor(0, 1);
        lcd.print("GoodWe Agradece!");

        tocarSomConcluido();
      }
    }
  }
}
