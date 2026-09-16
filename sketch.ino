#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Update.h>

const char *FIRMWARE_VERSION = "1.0";
const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PASSWORD = "";
const char *MANIFEST_URL = "https://raw.githubusercontent.com/camposdigo/cp2-motiva-ota/main/ota/version.json";

const uint8_t LED_R = 25;
const uint8_t LED_G = 26;
const uint8_t LED_B = 27;

const unsigned long INTERVALO_SESSAO_MS = 48000UL;
const unsigned long INTERVALO_LEITURA_MS = 2000UL;
const uint8_t TOTAL_LEITURAS = 5;

int leituras[TOTAL_LEITURAS];
uint8_t indiceLeitura = 0;
uint32_t totalSessoesConcluidas = 0;
uint32_t ultimaSessaoComTentativaOTA = 0;

unsigned long inicioSessaoMs = 0;
unsigned long proximaLeituraMs = 0;
bool sessaoAtiva = false;

void definirLed(bool vermelho, bool verde, bool azul) {
  digitalWrite(LED_R, vermelho ? HIGH : LOW);
  digitalWrite(LED_G, verde ? HIGH : LOW);
  digitalWrite(LED_B, azul ? HIGH : LOW);
}

void configurarLed() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  definirLed(false, false, true);
}

void imprimirCabecalho() {
  Serial.println();
  Serial.println("========================================");
  Serial.println(" MONITORAMENTO DE VEGETACAO - FW 1.0");
  Serial.println("========================================");
  Serial.println("LED: AZUL | Estado: Firmware 1.0");
}

void iniciarSessao(unsigned long instanteInicio) {
  inicioSessaoMs = instanteInicio;
  proximaLeituraMs = instanteInicio;
  indiceLeitura = 0;
  sessaoAtiva = true;

  Serial.println();
  Serial.printf("--- Inicio da sessao %lu | t=%lu ms ---\n",
                static_cast<unsigned long>(totalSessoesConcluidas + 1),
                instanteInicio);
}

float calcularMedia() {
  long soma = 0;
  for (uint8_t i = 0; i < TOTAL_LEITURAS; i++) {
    soma += leituras[i];
  }
  return static_cast<float>(soma) / TOTAL_LEITURAS;
}

String extrairStringJson(const String &json, const String &chave) {
  String token = "\"" + chave + "\"";
  int posChave = json.indexOf(token);
  if (posChave < 0) return "";
  int posDoisPontos = json.indexOf(':', posChave + token.length());
  if (posDoisPontos < 0) return "";
  int aspasInicio = json.indexOf('"', posDoisPontos + 1);
  if (aspasInicio < 0) return "";
  int aspasFim = json.indexOf('"', aspasInicio + 1);
  if (aspasFim < 0) return "";
  return json.substring(aspasInicio + 1, aspasFim);
}

int valorVersao(const String &versao) {
  int principal = 0;
  int secundaria = 0;
  int patch = 0;
  int partes = sscanf(versao.c_str(), "%d.%d.%d", &principal, &secundaria, &patch);
  if (partes < 2) return -1;
  if (partes == 2) patch = 0;
  return principal * 10000 + secundaria * 100 + patch;
}

bool versaoMaisNova(const String &disponivel, const String &instalada) {
  int valorDisponivel = valorVersao(disponivel);
  int valorInstalada = valorVersao(instalada);
  return valorDisponivel >= 0 && valorInstalada >= 0 && valorDisponivel > valorInstalada;
}

bool conectarWifi() {
  if (WiFi.status() == WL_CONNECTED) return true;
  Serial.println("[Wi-Fi] Conectando a Wokwi-GUEST...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 12000UL) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERRO] Nao ha conexao Wi-Fi. OTA cancelada nesta tentativa.");
    return false;
  }
  Serial.print("[Wi-Fi] Conectado. IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

bool obterManifesto(String &conteudo) {
  WiFiClientSecure cliente;
  cliente.setInsecure();
  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(12000);
  if (!http.begin(cliente, MANIFEST_URL)) {
    Serial.println("[ERRO] Nao foi possivel iniciar a consulta do manifesto.");
    return false;
  }
  int codigoHttp = http.GET();
  if (codigoHttp != HTTP_CODE_OK) {
    Serial.printf("[ERRO] Manifesto nao pode ser acessado. HTTP %d\n", codigoHttp);
    http.end();
    return false;
  }
  conteudo = http.getString();
  http.end();
  return true;
}

bool baixarEAplicarFirmware(const String &urlFirmware) {
  Serial.println("[OTA] Baixando firmware_v2.bin...");
  WiFiClientSecure cliente;
  cliente.setInsecure();
  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(20000);
  http.useHTTP10(true);
  if (!http.begin(cliente, urlFirmware)) {
    Serial.println("[ERRO] Nao foi possivel iniciar o download do firmware.");
    return false;
  }
  int codigoHttp = http.GET();
  if (codigoHttp != HTTP_CODE_OK) {
    Serial.printf("[ERRO] Arquivo de firmware nao pode ser baixado. HTTP %d\n", codigoHttp);
    http.end();
    return false;
  }
  int tamanho = http.getSize();
  size_t tamanhoEsperado = tamanho > 0 ? static_cast<size_t>(tamanho) : UPDATE_SIZE_UNKNOWN;
  if (!Update.begin(tamanhoEsperado)) {
    Serial.printf("[ERRO] Update.begin falhou: %s\n", Update.errorString());
    http.end();
    return false;
  }
  WiFiClient *stream = http.getStreamPtr();
  size_t gravados = Update.writeStream(*stream);
  if (tamanho > 0 && gravados != static_cast<size_t>(tamanho)) {
    Serial.printf("[ERRO] Download/gravacao incompleta: %u de %d bytes.\n", static_cast<unsigned int>(gravados), tamanho);
    Update.abort();
    http.end();
    return false;
  }
  if (!Update.end()) {
    Serial.printf("[ERRO] Processo de atualizacao retornou erro: %s\n", Update.errorString());
    http.end();
    return false;
  }
  if (!Update.isFinished()) {
    Serial.println("[ERRO] OTA terminou sem gravar toda a imagem.");
    http.end();
    return false;
  }
  http.end();
  Serial.printf("[OTA] Firmware gravado com sucesso (%u bytes).\n", static_cast<unsigned int>(gravados));
  Serial.println("[OTA] Reiniciando ESP32 para executar a nova versao...");
  delay(1200);
  ESP.restart();
  return true;
}

void verificarAtualizacaoOTA() {
  Serial.println();
  Serial.println("========== VERIFICACAO OTA ==========");
  Serial.printf("Versao instalada: %s\n", FIRMWARE_VERSION);
  if (!conectarWifi()) {
    Serial.println("=====================================");
    return;
  }
  String manifesto;
  if (!obterManifesto(manifesto)) {
    Serial.println("=====================================");
    return;
  }
  String versaoDisponivel = extrairStringJson(manifesto, "version");
  String urlFirmware = extrairStringJson(manifesto, "url");
  if (versaoDisponivel.length() == 0 || urlFirmware.length() == 0) {
    Serial.println("[ERRO] Manifesto invalido: campos version/url ausentes.");
    Serial.println("=====================================");
    return;
  }
  Serial.printf("Versao disponivel: %s\n", versaoDisponivel.c_str());
  if (!versaoMaisNova(versaoDisponivel, FIRMWARE_VERSION)) {
    Serial.println("[OTA] A versao instalada ja e a mais recente.");
    Serial.println("=====================================");
    return;
  }
  Serial.println("[OTA] Nova versao encontrada.");
  Serial.printf("[OTA] URL: %s\n", urlFirmware.c_str());
  if (!baixarEAplicarFirmware(urlFirmware)) {
    Serial.println("[OTA] Atualizacao nao concluida. O FW 1.0 continuara em execucao.");
    Serial.println("=====================================");
  }
}

void finalizarSessao() {
  float media = calcularMedia();
  Serial.printf("Media da sessao: %.1f cm\n", media);
  totalSessoesConcluidas++;
  sessaoAtiva = false;
  unsigned long proximoInicio = inicioSessaoMs + INTERVALO_SESSAO_MS;
  unsigned long agora = millis();
  unsigned long restante = agora < proximoInicio ? proximoInicio - agora : 0;
  Serial.printf("Sessao %lu concluida.\n", static_cast<unsigned long>(totalSessoesConcluidas));
  Serial.println("Intervalo entre inicios de sessoes: 48 segundos.");
  Serial.printf("Proxima sessao em aproximadamente %.1f s.\n", restante / 1000.0f);
  if (totalSessoesConcluidas >= 3 && ultimaSessaoComTentativaOTA != totalSessoesConcluidas) {
    ultimaSessaoComTentativaOTA = totalSessoesConcluidas;
    verificarAtualizacaoOTA();
  }
}

void processarSessao() {
  if (!sessaoAtiva || indiceLeitura >= TOTAL_LEITURAS) return;
  unsigned long agora = millis();
  if (static_cast<long>(agora - proximaLeituraMs) < 0) return;
  int valor = random(10, 21);
  leituras[indiceLeitura] = valor;
  Serial.printf("Leitura %u: %d cm\n", indiceLeitura + 1, valor);
  indiceLeitura++;
  if (indiceLeitura < TOTAL_LEITURAS) {
    proximaLeituraMs = inicioSessaoMs + indiceLeitura * INTERVALO_LEITURA_MS;
  } else {
    finalizarSessao();
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  randomSeed(esp_random());
  configurarLed();
  imprimirCabecalho();
  iniciarSessao(millis());
}

void loop() {
  processarSessao();
  if (!sessaoAtiva) {
    unsigned long agora = millis();
    unsigned long proximoInicio = inicioSessaoMs + INTERVALO_SESSAO_MS;
    if (static_cast<long>(agora - proximoInicio) >= 0) iniciarSessao(proximoInicio);
  }
  delay(5);
}
