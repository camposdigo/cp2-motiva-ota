# S2-CP02 - Projeto Motiva | Atualizacao Remota de Firmware (OTA)

**Disciplina:** IoT / Sistemas Embarcados  
**Professor:** Marcelo Fernando Morgantini  
**Integrante:** Rodrigo Campos Cordeiro - RM566386  
**Demais integrantes/RMs:** [PREENCHER]  
**Data de entrega:** 21/09/2026

## 1. Links publicos

- Wokwi: [PREENCHER]
- Repositorio GitHub: https://github.com/camposdigo/cp2-motiva-ota
- Manifesto `version.json`: https://raw.githubusercontent.com/camposdigo/cp2-motiva-ota/main/ota/version.json
- Binario `firmware_v2.bin`: https://raw.githubusercontent.com/camposdigo/cp2-motiva-ota/main/ota/firmware_v2.bin

## 2. Arquitetura

O ESP32 inicia executando o Firmware 1.0. O programa realiza sessoes de cinco leituras pseudoaleatorias de altura de vegetacao, com intervalo de dois segundos entre leituras e quarenta e oito segundos entre o inicio de sessoes consecutivas. Depois de pelo menos tres sessoes, o dispositivo conecta a rede Wokwi-GUEST e consulta o manifesto remoto `version.json`. Ao identificar a versao 2.0, baixa `firmware_v2.bin`, grava a atualizacao por OTA e reinicia.

A versao 2.0 mantem o monitoramento e acrescenta ordenacao, mediana e histerese. O LED RGB indica azul no Firmware 1.0, verde no Firmware 2.0 em estado NORMAL e vermelho no Firmware 2.0 em estado ALERTA.

## 3. Firmware 1.0

O Firmware 1.0 implementa as cinco leituras, vetor, media, temporizacao baseada em `millis()`, LED azul, Wi-Fi, consulta do manifesto, comparacao de versao, download e gravacao OTA, alem das mensagens de erro exigidas.

## 4. Firmware 2.0

O Firmware 2.0 mantem a rotina de monitoramento e acrescenta copia/ordenacao das leituras, calculo de mediana e histerese:

- Mediana >= 16 cm: ALERTA.
- 14 cm < mediana < 16 cm: mantem o estado anterior.
- Mediana <= 14 cm: NORMAL.

## 5. Evidencias dos testes

| Teste | Resultado | Evidencia |
|---|---|---|
| FW 1.0 | 5 leituras, media e LED azul | [INSERIR CAPTURA] |
| Temporizacao | nova sessao em 48 s a partir do inicio anterior | [INSERIR CAPTURA] |
| Manifesto 2.0 | atualizacao detectada apos 3 ciclos | [INSERIR CAPTURA] |
| OTA | reboot no FW 2.0 | [INSERIR CAPTURA] |
| FW 2.0 | media, ordenacao e mediana | [INSERIR CAPTURA] |
| Mediana >= 16 | ALERTA / vermelho | [INSERIR CAPTURA] |
| 14 < mediana < 16 | estado anterior mantido | [INSERIR CAPTURA] |
| Mediana <= 14 | NORMAL / verde | [INSERIR CAPTURA] |

## 6. Tratamento de erros

O Firmware 1.0 informa no Serial Monitor falhas de Wi-Fi, impossibilidade de acessar o manifesto, versao ja atual, falha no download do binario e erro durante a gravacao OTA.

## 7. Instrucoes de execucao

1. Confirmar a execucao do GitHub Actions e a criacao de `ota/firmware_v2.bin`.
2. Confirmar que `ota/version.json` e o binario abrem por URL publica direta.
3. Abrir o projeto no Wokwi usando `sketch.ino` e `diagram.json`.
4. Iniciar a simulacao e acompanhar as tres sessoes do Firmware 1.0.
5. Aguardar a verificacao OTA, download, gravacao e reboot.
6. Registrar as evidencias da versao 2.0 e dos estados de histerese.
