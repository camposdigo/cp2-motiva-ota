# CP2 - Projeto Motiva | Atualizacao Remota de Firmware (OTA)

Solucao para o Check Point de IoT com ESP32 no Wokwi. O equipamento inicia no **Firmware 1.0**, executa sessoes de monitoramento de vegetacao e, apos pelo menos tres sessoes, consulta um manifesto remoto para atualizar por OTA para o **Firmware 2.0**.

## O que o projeto atende

### Firmware 1.0
- Identificacao clara da versao no Serial Monitor.
- 5 leituras pseudoaleatorias entre 10 e 20 cm.
- Uma leitura a cada 2 segundos.
- Armazenamento em vetor e exibicao individual.
- Media aritmetica das 5 leituras.
- Nova sessao a cada 48 segundos contados do inicio da sessao anterior.
- LED RGB azul.
- Consulta OTA depois de pelo menos 3 sessoes.
- Tratamento de falha de Wi-Fi, manifesto, versao, download e gravacao.

### Firmware 2.0
- Mantem 5 leituras, intervalo, vetor, media e sessoes de 48 s.
- Exibe ordem original e crescente.
- Ordena uma copia do vetor no proprio firmware.
- Calcula mediana pelo terceiro elemento do vetor ordenado.
- Histerese baseada na mediana:
  - mediana >= 16 cm: ALERTA;
  - 14 < mediana < 16 cm: mantem o estado anterior;
  - mediana <= 14 cm: NORMAL.
- LED verde em NORMAL e vermelho em ALERTA.

## Estrutura

```text
.
|-- sketch.ino                     # FW 1.0 para abrir diretamente no Wokwi
|-- diagram.json                   # ESP32 + LED RGB
|-- firmware_v1/
|   `-- firmware_v1.ino
|-- firmware_v2/
|   `-- firmware_v2.ino
|-- ota/
|   |-- version.json
|   `-- firmware_v2.bin            # gerado automaticamente pelo GitHub Actions
|-- tools/
|   `-- configurar_repo.py
|-- .github/workflows/
|   `-- build-firmware.yml
|-- TESTES.md
|-- ROTEIRO_DEMO.md
`-- CHECKLIST_ENTREGA.md
```

## URLs OTA deste repositorio

- Manifesto: `https://raw.githubusercontent.com/camposdigo/cp2-motiva-ota/main/ota/version.json`
- Firmware 2.0: `https://raw.githubusercontent.com/camposdigo/cp2-motiva-ota/main/ota/firmware_v2.bin`

## Gerar o firmware_v2.bin

O workflow `.github/workflows/build-firmware.yml` compila os dois firmwares e publica `ota/firmware_v2.bin` automaticamente na branch `main`.

No GitHub, abra **Actions > Compilar firmware ESP32** e confirme que o workflow terminou com sucesso. Se necessario, use **Run workflow**.

## Abrir no Wokwi

A raiz do repositorio contem `sketch.ino` e `diagram.json`, portanto ela representa a simulacao inicial do **Firmware 1.0**.

### Ligacao do LED RGB
- Vermelho: GPIO 25
- Verde: GPIO 26
- Azul: GPIO 27
- LED RGB configurado como catodo comum

## Fluxo esperado da demonstracao

1. ESP32 inicia no FW 1.0 e LED fica azul.
2. Sessao 1: leituras em t=0, 2, 4, 6 e 8 s.
3. Sessao 2 comeca 48 s depois do inicio da sessao 1.
4. Sessao 3 comeca 48 s depois do inicio da sessao 2.
5. Depois da terceira sessao, o ESP32 conecta em `Wokwi-GUEST`.
6. O ESP32 acessa `ota/version.json`.
7. Compara 1.0 com 2.0.
8. Baixa `ota/firmware_v2.bin`.
9. Grava a atualizacao e executa `ESP.restart()`.
10. Apos o reboot aparece `MONITORAMENTO DE VEGETACAO - FW 2.0`.
11. O FW 2.0 exibe media, ordem original, ordem crescente, mediana e estado de histerese.

## Teste controlado da histerese

No arquivo `firmware_v2/firmware_v2.ino` existe:

```cpp
const bool MODO_TESTE_HISTERESE = false;
```

Mantenha `false` no binario usado no OTA. Para produzir evidencias dos tres casos de histerese, voce pode temporariamente mudar para `true` e compilar uma versao de teste.

## Observacao sobre HTTPS

No Wokwi, o cliente HTTPS usa `setInsecure()` para simplificar o laboratorio. Isso atende a demonstracao academica, mas nao deve ser tratado como implementacao de seguranca para producao.

## Entrega

Capture as evidencias dos testes, gere o link publico do Wokwi e mantenha Wokwi/GitHub ativos pelo periodo solicitado pelo professor.
