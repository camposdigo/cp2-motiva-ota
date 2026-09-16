#include <Arduino.h>

const char *FIRMWARE_VERSION = "2.0";

const uint8_t LED_R = 25;
const uint8_t LED_G = 26;
const uint8_t LED_B = 27;

const unsigned long INTERVALO_SESSAO_MS = 48000UL;
const unsigned long INTERVALO_LEITURA_MS = 2000UL;
const uint8_t TOTAL_LEITURAS = 5;

const bool MODO_TESTE_HISTERESE = false;

const int CENARIOS_TESTE[][TOTAL_LEITURAS] = {
  {13, 14, 14, 15, 16},
  {16, 17, 18, 12, 20},
  {14, 15, 15, 15, 16}
};
const uint8_t TOTAL_CENARIOS_TESTE = sizeof(CENARIOS_TESTE) / sizeof(CENARIOS_TESTE[0]);

int leituras[TOTAL_LEITURAS];
uint8_t indiceLeitura = 0;
uint32_t totalSessoesConcluidas = 0;

unsigned long inicioSessaoMs = 0;
unsigned long proximaLeituraMs = 0;
bool sessaoAtiva = false;

enum EstadoVegetacao {
  NORMAL,
  ALERTA
};

EstadoVegetacao estadoAtual = NORMAL;

void definirLed(bool vermelho, bool verde, bool azul) {
  digitalWrite(LED_R, vermelho ? HIGH : LOW);
  digitalWrite(LED_G, verde ? HIGH : LOW);
  digitalWrite(LED_B, azul ? HIGH : LOW);
}

void atualizarLedDoEstado() {
  if (estadoAtual == ALERTA) {
    definirLed(true, false, false);
  } else {
    definirLed(false, true, false);
  }
}

void configurarLed() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  atualizarLedDoEstado();
}

const char *nomeEstado(EstadoVegetacao estado) {
  return estado == ALERTA ? "ALERTA" : "NORMAL";
}

void imprimirCabecalho() {
  Serial.println();
  Serial.println("========================================");
  Serial.println(" MONITORAMENTO DE VEGETACAO - FW 2.0");
  Serial.println("========================================");
  Serial.println("Novidades: ordenacao, mediana e histerese.");
  Serial.printf("Estado inicial: %s | LED: VERDE\n", nomeEstado(estadoAtual));

  if (MODO_TESTE_HISTERESE) {
    Serial.println("ATENCAO: MODO_TESTE_HISTERESE ATIVO.");
  }
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

int gerarLeitura(uint8_t indice) {
  if (!MODO_TESTE_HISTERESE) {
    return random(10, 21);
  }

  uint8_t cenario = totalSessoesConcluidas % TOTAL_CENARIOS_TESTE;
  return CENARIOS_TESTE[cenario][indice];
}

float calcularMedia() {
  long soma = 0;
  for (uint8_t i = 0; i < TOTAL_LEITURAS; i++) {
    soma += leituras[i];
  }
  return static_cast<float>(soma) / TOTAL_LEITURAS;
}

void copiarVetor(const int origem[], int destino[], uint8_t tamanho) {
  for (uint8_t i = 0; i < tamanho; i++) {
    destino[i] = origem[i];
  }
}

void ordenarCrescente(int valores[], uint8_t tamanho) {
  for (uint8_t i = 0; i < tamanho - 1; i++) {
    for (uint8_t j = 0; j < tamanho - 1 - i; j++) {
      if (valores[j] > valores[j + 1]) {
        int temporario = valores[j];
        valores[j] = valores[j + 1];
        valores[j + 1] = temporario;
      }
    }
  }
}

void imprimirVetor(const char *rotulo, const int valores[], uint8_t tamanho) {
  Serial.print(rotulo);
  Serial.print(": ");
  for (uint8_t i = 0; i < tamanho; i++) {
    Serial.print(valores[i]);
    if (i < tamanho - 1) {
      Serial.print(" ");
    }
  }
  Serial.println();
}

void aplicarHisterese(int mediana) {
  EstadoVegetacao estadoAnterior = estadoAtual;

  if (mediana >= 16) {
    estadoAtual = ALERTA;
    Serial.println("Histerese: mediana >= 16 cm -> entrar em ALERTA.");
  } else if (mediana <= 14) {
    estadoAtual = NORMAL;
    Serial.println("Histerese: mediana <= 14 cm -> entrar/retornar a NORMAL.");
  } else {
    Serial.println("Histerese: 14 < mediana < 16 cm -> manter estado anterior.");
  }

  atualizarLedDoEstado();
  Serial.printf("Estado anterior: %s | Estado atual: %s\n",
                nomeEstado(estadoAnterior), nomeEstado(estadoAtual));
  Serial.printf("LED: %s\n", estadoAtual == ALERTA ? "VERMELHO" : "VERDE");
}

void finalizarSessao() {
  int ordenadas[TOTAL_LEITURAS];
  copiarVetor(leituras, ordenadas, TOTAL_LEITURAS);
  ordenarCrescente(ordenadas, TOTAL_LEITURAS);

  float media = calcularMedia();
  int mediana = ordenadas[2];

  imprimirVetor("Ordem original", leituras, TOTAL_LEITURAS);
  imprimirVetor("Ordem crescente", ordenadas, TOTAL_LEITURAS);
  Serial.printf("Media da sessao: %.1f cm\n", media);
  Serial.printf("Mediana da sessao: %d cm\n", mediana);

  aplicarHisterese(mediana);

  totalSessoesConcluidas++;
  sessaoAtiva = false;

  unsigned long proximoInicio = inicioSessaoMs + INTERVALO_SESSAO_MS;
  unsigned long agora = millis();
  unsigned long restante = agora < proximoInicio ? proximoInicio - agora : 0;

  Serial.printf("Sessao %lu concluida.\n", static_cast<unsigned long>(totalSessoesConcluidas));
  Serial.println("Intervalo entre inicios de sessoes: 48 segundos.");
  Serial.printf("Proxima sessao em aproximadamente %.1f s.\n", restante / 1000.0f);
}

void processarSessao() {
  if (!sessaoAtiva || indiceLeitura >= TOTAL_LEITURAS) {
    return;
  }

  unsigned long agora = millis();
  if (static_cast<long>(agora - proximaLeituraMs) < 0) {
    return;
  }

  int valor = gerarLeitura(indiceLeitura);
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

    if (static_cast<long>(agora - proximoInicio) >= 0) {
      iniciarSessao(proximoInicio);
    }
  }

  delay(5);
}
