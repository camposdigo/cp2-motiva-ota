# Roteiro curto para demonstracao

## Antes de iniciar

- Repositorio GitHub publico.
- `ota/version.json` apontando para o binario real.
- `ota/firmware_v2.bin` presente e acessivel.
- Projeto Wokwi aberto com `sketch.ino` (FW 1.0).
- Serial Monitor visivel.

## Demonstracao

1. Inicie o Wokwi e mostre `FW 1.0` no Serial e o LED azul.
2. Mostre as 5 leituras de uma sessao e a media.
3. Aponte que as leituras ocorrem a cada 2 s e que o proximo inicio esta agendado em 48 s desde o inicio anterior.
4. Apos a terceira sessao, acompanhe no Serial a conexao Wi-Fi, leitura do manifesto e deteccao da versao 2.0.
5. Mostre a mensagem de download/gravacao e o reboot.
6. Depois do reboot, mostre `FW 2.0`.
7. Aguarde uma sessao do FW 2.0 e mostre ordem original, ordem crescente, media e mediana.
8. Explique a histerese e mostre o LED verde/vermelho conforme o estado.

## Explicacao curta sobre OTA

OTA permite atualizar dispositivos instalados em campo sem acesso fisico. O dispositivo consulta um manifesto remoto, identifica uma versao mais nova, baixa a imagem de firmware, grava em uma particao de atualizacao e reinicia executando o novo programa.

## Media, mediana e histerese

- **Media:** soma das cinco leituras dividida por cinco.
- **Mediana:** valor central das cinco leituras depois da ordenacao.
- **Histerese:** evita oscilacoes de NORMAL/ALERTA quando a medida fica perto do limite.
