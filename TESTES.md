# Testes obrigatorios

| # | Condicao | Como executar | Resultado esperado | Evidencia |
|---|---|---|---|---|
| 1 | Firmware 1.0 | Iniciar a simulacao | 5 leituras, media e LED azul | Captura Serial + circuito |
| 2 | Sessao completa | Observar timestamps | Nova sessao inicia 48 s apos o inicio da anterior | Captura Serial |
| 3 | Manifesto 2.0 | Deixar completar 3 sessoes | ESP32 identifica versao 2.0 disponivel | Captura Serial |
| 4 | OTA executada | Aguardar download e reboot | ESP32 reinicia no Firmware 2.0 | Captura Serial antes/depois |
| 5 | Firmware 2.0 | Completar uma sessao | Media, ordem original/crescente e mediana corretas | Captura Serial |
| 6 | Mediana >= 16 | Usar modo de teste ou aguardar caso aleatorio | ALERTA + LED vermelho | Captura Serial + LED |
| 7 | 14 < mediana < 16 | Gerar mediana 15 depois de um estado conhecido | Estado anterior mantido | Captura Serial |
| 8 | Mediana <= 14 | Usar modo de teste ou aguardar caso aleatorio | NORMAL + LED verde | Captura Serial + LED |

## Testes de erro minimos

- Desabilitar/alterar Wi-Fi: deve surgir mensagem clara de erro de conexao.
- Usar URL de manifesto invalida: deve informar erro HTTP/acesso ao manifesto.
- Alterar temporariamente `version.json` para `1.0`: deve informar que a versao instalada ja e a mais recente.
- Usar URL invalida para o `.bin`: deve informar falha de download.
- Uma imagem OTA invalida deve ser rejeitada pela API `Update` e o FW 1.0 deve continuar executando.
