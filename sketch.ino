#include <WiFi.h>
#include <HTTPClient.h>

#define led_verde 2 // Pino utilizado para controle do led verde
#define led_vermelho 40 // Pino utilizado para controle do led vermelho
#define led_amarelo 9 // Pino utilizado para controle do led amarelo

const int pinoBotao = 18;  // Pino do botão
int estadoBotao = 0;  // Variável de estado do botão
bool botaoFoiPressionado = false;

const int pinoLDR = 4;  // Pino do LDR
int limiteLDR = 600;

//Intervalos de duração de piscagem dos leds
int intervaloAmareloNoite = 1000;
int ultimaPiscadaAmareloNoite = 0;
bool estadoAmarelo = false;
int estadoConvencional = 1;
int ultimaTrocaConvencional = 0;
int intervaloVerde = 3000;
int intervaloAmarelo = 2000;
int intervaloVermelho = 5000;
//Intervalo de debounce;
int intervaloDebounce = 50;
int ultimoApertoBotao = 0;
//Intervalo de apertar o botao no vermelho e contagem de botão
int contagemBotao = 0;

void setup() {

  // Configuração inicial dos pinos para controle dos leds como OUTPUTs (saídas) do ESP32
  pinMode(led_verde, OUTPUT);
  pinMode(led_vermelho, OUTPUT);
  pinMode(led_amarelo, OUTPUT);

  // Inicialização das entradas
  pinMode(pinoBotao, INPUT); // Inicializa o pino do botão como input
  pinMode(pinoLDR, INPUT);

  digitalWrite(led_amarelo, LOW);
  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);

  Serial.begin(9600); // Configuração para debug por interface serial entre ESP e computador com baud rate de 9600

  WiFi.begin("Wokwi-GUEST", ""); // Conexão à rede WiFi aberta com SSID Wokwi-GUEST.

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(WiFi.status());
  Serial.println("Conectado ao WiFi com sucesso!"); // Considerando que saiu do loop acima, o ESP32 agora está conectado ao WiFi (outra opção é colocar este comando dentro do if abaixo)

}
void modoNoturno() {
  if (ultimaTrocaConvencional != 0) {
    ultimaTrocaConvencional = 0;
  }
  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);
  if (millis() - ultimaPiscadaAmareloNoite >= intervaloAmareloNoite) {
    ultimaPiscadaAmareloNoite = millis();
    if (estadoAmarelo) {
      digitalWrite(led_amarelo, LOW);
      estadoAmarelo = false;
    } else {
      digitalWrite(led_amarelo, HIGH);
      estadoAmarelo = true;
    }
  }
}
void fazerRequisicao() {
  HTTPClient http;

  String serverPath = "http://www.google.com.br/"; // Endpoint da requisição HTTP

  http.begin(serverPath.c_str());

  int httpResponseCode = http.GET(); // Código do Resultado da Requisição HTTP

  if (httpResponseCode > 0) {
    Serial.print("Código Resposta HTTP: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("Erro: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}
void modoConvencional() {
  switch (estadoConvencional) {
    //Caso 1: Farol vermelho
    case 1:
      estadoBotao = digitalRead(pinoBotao);
      if (estadoBotao == HIGH && millis() - intervaloDebounce >= ultimoApertoBotao) {
        Serial.println("Botão pressionado!");
        botaoFoiPressionado = true;
        ultimoApertoBotao = millis();
        contagemBotao++;
      }
      if (contagemBotao >= 3) {
        fazerRequisicao();
        contagemBotao = 0;
      }
      if (botaoFoiPressionado) {
        if (millis() - ultimoApertoBotao >= 1000) {
          digitalWrite(led_verde, HIGH);
          digitalWrite(led_vermelho, LOW);
          ultimaTrocaConvencional = millis();
          estadoConvencional = 2;
          botaoFoiPressionado = false;
          contagemBotao = 0;
        }
        break;
      }
      if (millis() - ultimaTrocaConvencional >= intervaloVermelho || ultimaTrocaConvencional == 0) {
        digitalWrite(led_verde, HIGH);
        digitalWrite(led_vermelho, LOW);
        digitalWrite(led_amarelo, LOW);
        ultimaTrocaConvencional = millis();
        estadoConvencional = 2;
      }
      break;
    //Caso 2: Farol Verde
    case 2:
      if (millis() - ultimaTrocaConvencional >= intervaloVerde) {
        digitalWrite(led_verde, LOW);
        digitalWrite(led_amarelo, HIGH);
        ultimaTrocaConvencional = millis();
        estadoConvencional = 3;
      }
      break;
    //Caso 3: Farol Amarelo
    case 3:
      if (millis() - ultimaTrocaConvencional >= intervaloAmarelo) {
        digitalWrite(led_vermelho, HIGH);
        digitalWrite(led_amarelo, LOW);
        ultimaTrocaConvencional = millis();
        estadoConvencional = 1;
      }
      break;

  }
}
void loop() {
  // Verifica estado do botão
  int ldrstatus = analogRead(pinoLDR);
  if (ldrstatus >= limiteLDR) {
    modoNoturno();

  } else {
    modoConvencional();
  }
}
