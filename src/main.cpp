#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include "esp_sntp.h"

String pedirParaApiCLima(String codigo);
const char *nomeRedeWiFi = "SENAI EDUCACIONAL";
const char *senhaRedeWiFi = "info@134";
const String enderecoAPI = "https://wttr.in/Sao+Caetano+do+Sul?m&lang=pt&format=%t";
LiquidCrystal_I2C lcd(0x27, 20, 4);
//*------------------SERVIDORES NTP----------------
const char *NTP1 = "a.st1.ntp.br";
const char *NTP2 = "pool.ntp.org";
const char *FUSO = "<-03>3";
const char *DIAS[] = {"domingo", "segunda", "terça", "quarta", "quinta", "sexta", "sabado"};

int posicaoClima = 0;
unsigned long tempoAnterior = 0;

void setup()
{
    lcd.init();
    lcd.backlight();
    Serial.begin(115200);
    WiFi.begin(nomeRedeWiFi, senhaRedeWiFi);

    while (WiFi.status() != WL_CONNECTED)
        ;
    {
        Serial.print("#");
        delay(500);
    }
    Serial.print("Conectado com sucesso:");
    Serial.printf("Endereço IP: %s", WiFi.localIP().toString());
    Serial.printf("Endereço MAC: %s", WiFi.macAddress().c_str());
    //*-----------------------------------------------------------------
    //*configuraçao de fuso e servidores NTP e inicia a sincronizaçao
    sntp_set_sync_interval(60 * 60 * 1000); //*intervalo de atualizaçao da hora na internet padrao 24*60*60*1000

    configTzTime(FUSO, NTP1, NTP2);

    struct tm agora;
    Serial.print("\n\rSincronizando hora\n\r");
    while (!getLocalTime(&agora)) //*o parametro é o endereço da estrutura tipo tm
    {
        Serial.print(".");
        delay(500);
    }
    Serial.print("\n\rSincronizaçao concluida");
    //*------------------------------------------------------------------
}

void loop()
{
    struct tm agora;
    if (getLocalTime(&agora))
    {
        Serial.printf("Hoje é %s, %02d:%02d:%02d\n\r",
                      DIAS[agora.tm_wday],
                      agora.tm_hour,
                      agora.tm_min,
                      agora.tm_sec);
        Serial.printf("timestamp: %d", time(nullptr));
    }
    delay(1000);

    String clima;
    int temperatura;
    int sensacaoTermica;
    clima = pedirParaApiCLima("%C");
    temperatura = pedirParaApiCLima("%d").toInt();
    sensacaoTermica = pedirParaApiCLima("%f").toInt();
    if (millis() - tempoAnterior >= 300)
    {
        tempoAnterior = millis();

        lcd.setCursor(0, 0);

        for (int i = 0; i < 20; i++)
        {
            if (posicaoClima + i < clima.length())
            {
                lcd.print(clima[posicaoClima + i]);
            }
            else
            {
                lcd.print(" ");
            }
        }

        posicaoClima++;

        if (posicaoClima >= clima.length())
        {
            posicaoClima = 0;
        }
    }
    lcd.setCursor(0, 1);
    lcd.printf("Temp.Atual: %dC", temperatura);
    lcd.setCursor(0, 2);
    lcd.printf("e sens.term: %dC", sensacaoTermica);
}

String pedirParaApiCLima(String codigo)
{
    HTTPClient http;
    http.begin(enderecoAPI + codigo);

    int codigoHTTP = http.GET(); //* Retorna o codigo http ex: 200 -> OK  ou 404 -> pagina nao encontrada

    String resposta;

    if (codigoHTTP == 200)
    {
        resposta = http.getString(); //* retorna o conteudo html da pagina
    }

    else
    {
        lcd.printf("Deu erro. Cod: %d", codigoHTTP);
    }

    http.end();
    return resposta;
}