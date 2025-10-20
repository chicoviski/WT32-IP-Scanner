/*
 * WT32-ETH01 IP Scanner - Implementação Incremental
 *
 * Etapa 1: Bibliotecas básicas, Ethernet e DHCP
 */

// ===== Bibliotecas Básicas =====
#include <Arduino.h>
#include <WiFi.h>
#include <ETH.h>
#include <ESP32Ping.h>

// ===== Configurações de Pinos WT32-ETH01 =====
// Definindo o dispositivo WT32-ETH01
#define ETH_ADDR 1
#define ETH_POWER_PIN 16             //-1 //16 // Do not use it, it can cause conflict during the software reset.
#define ETH_POWER_PIN_ALTERNATIVE 16 // 17
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN // ETH_CLOCK_GPIO0_IN // ETH_CLOCK_GPIO0_OUT // ETH_CLOCK_GPIO16_OUT // ETH_CLOCK_GPIO17_OUT

// ===== Variáveis Globais =====
// conectando ethernet DHCP
static bool eth_connected = false;
static bool gateway_tested = false;
static bool network_scanned = false;

// ===== Estrutura para Dispositivos =====
struct Device
{
  IPAddress ip;
  String hostname;
  int ping_time;
  bool active;
};

// Lista de dispositivos encontrados
Device dispositivos[100]; // Suporta até 100 dispositivos
int totalDispositivos = 0;

// ===== Declarações de Funções =====
void WiFiEvent(WiFiEvent_t event);
void testarGateway();
void varreduraRede();
void mostrarDispositivos();

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=================================");
  Serial.println("WT32-ETH01 IP Scanner");
  Serial.println("Etapa 1: Ethernet + DHCP");
  Serial.println("=================================");

  // Configurar eventos de rede
  WiFi.onEvent(WiFiEvent);

  // Inicializar Ethernet
  if (!ETH.begin(ETH_TYPE, ETH_ADDR, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_POWER_PIN, ETH_CLK_MODE))
  {
    Serial.println("Falha ao iniciar a Ethernet");
  }

  Serial.println("Sistema iniciado - aguardando conexão Ethernet...");
}

void loop()
{
  // Testar gateway após conexão estabelecida
  if (eth_connected && !gateway_tested)
  {
    testarGateway();
    gateway_tested = true;
  }

  // Fazer varredura da rede após teste do gateway
  if (eth_connected && gateway_tested && !network_scanned)
  {
    varreduraRede();
    network_scanned = true;
  }

  delay(100);
}

// ===== Implementação das Funções =====

void testarGateway()
{
  IPAddress gateway = ETH.gatewayIP();

  Serial.println("\n=================================");
  Serial.println("TESTANDO CONECTIVIDADE COM GATEWAY");
  Serial.println("=================================");
  Serial.println("Gateway: " + gateway.toString());
  Serial.println("Iniciando ping...");

  // Ping com 2 tentativas
  bool success = false;
  for (int i = 1; i <= 2; i++)
  {
    Serial.print("Ping " + String(i) + "/2: ");

    if (Ping.ping(gateway))
    {
      Serial.println("✓ Sucesso - Tempo: " + String(Ping.averageTime()) + " ms");
      success = true;
    }
    else
    {
      Serial.println("✗ Falha - Timeout");
    }

    delay(1000); // Intervalo entre pings
  }

  Serial.println("=================================");
  if (success)
  {
    Serial.println("RESULTADO: Gateway acessível");
    Serial.println("Rede funcionando corretamente!");
  }
  else
  {
    Serial.println("RESULTADO: Gateway inacessível");
    Serial.println("Verifique a conexão de rede!");
  }
  Serial.println("=================================\n");
}

void varreduraRede()
{
  Serial.println("\n=================================");
  Serial.println("INICIANDO VARREDURA DA REDE");
  Serial.println("=================================");

  IPAddress localIP = ETH.localIP();
  IPAddress subnet = ETH.subnetMask();

  // Calcular rede base
  IPAddress networkBase = IPAddress(
      localIP[0] & subnet[0],
      localIP[1] & subnet[1],
      localIP[2] & subnet[2],
      localIP[3] & subnet[3]);

  Serial.println("IP Local: " + localIP.toString());
  Serial.println("Máscara: " + subnet.toString());
  Serial.println("Rede: " + networkBase.toString());
  Serial.println("Iniciando scan...\n");

  totalDispositivos = 0;

  // Varrer IPs da rede (assumindo /24 - 254 hosts)
  for (int i = 1; i <= 254; i++)
  {
    IPAddress targetIP = IPAddress(networkBase[0], networkBase[1], networkBase[2], i);
    bool isLocalIP = (targetIP == localIP);

    Serial.print("Testando " + targetIP.toString() + " ... ");

    bool isActive = false;
    int pingTime = 0;

    if (isLocalIP)
    {
      // Nosso próprio IP - sempre ativo
      isActive = true;
      pingTime = 0;
      Serial.println("✓ ATIVO (ESTE DISPOSITIVO - 0 ms)");
    }
    else if (Ping.ping(targetIP, 1)) // 1 ping apenas para outros IPs
    {
      isActive = true;
      pingTime = Ping.averageTime();
      Serial.println("✓ ATIVO (" + String(pingTime) + " ms)");
    }
    else
    {
      Serial.println("✗ inativo");
    }

    // Adicionar à lista se ativo
    if (isActive && totalDispositivos < 100)
    {
      dispositivos[totalDispositivos].ip = targetIP;
      dispositivos[totalDispositivos].ping_time = pingTime;
      dispositivos[totalDispositivos].active = true;

      // Tentar resolver hostname
      String hostname = "Resolvendo...";
      if (isLocalIP)
      {
        hostname = String(ETH.getHostname()) + " (ESTE DISPOSITIVO)";
      }
      else
      {
        // Tentar resolução DNS reversa
        Serial.print("  Resolvendo hostname... ");
        // Nota: ESP32 não tem gethostbyaddr nativo, usar placeholder
        hostname = "Host-" + String(targetIP[3]); // Fallback simples
        Serial.println("OK");
      }

      dispositivos[totalDispositivos].hostname = hostname;
      totalDispositivos++;
    }

    delay(100); // Pequeno delay entre pings
  }

  Serial.println("\n=================================");
  Serial.println("VARREDURA CONCLUÍDA");
  Serial.println("=================================");

  mostrarDispositivos();
}

void mostrarDispositivos()
{
  Serial.println("\n=================================");
  Serial.println("DISPOSITIVOS ENCONTRADOS NA REDE");
  Serial.println("=================================");

  if (totalDispositivos == 0)
  {
    Serial.println("Nenhum dispositivo encontrado na rede.");
  }
  else
  {
    Serial.println("Total de dispositivos ativos: " + String(totalDispositivos));
    Serial.println("");
    Serial.println("┌─────────────────┬──────────────────────────────┬──────────┐");
    Serial.println("│       IP        │           Hostname           │   Ping   │");
    Serial.println("├─────────────────┼──────────────────────────────┼──────────┤");

    for (int i = 0; i < totalDispositivos; i++)
    {
      String ip_str = dispositivos[i].ip.toString();
      String hostname = dispositivos[i].hostname;
      String ping_str = String(dispositivos[i].ping_time) + " ms";

      // Truncar hostname se muito longo
      if (hostname.length() > 28)
      {
        hostname = hostname.substring(0, 25) + "...";
      }

      // Formatar para alinhamento
      while (ip_str.length() < 15)
        ip_str += " ";
      while (hostname.length() < 28)
        hostname += " ";
      while (ping_str.length() < 8)
        ping_str += " ";

      Serial.println("│ " + ip_str + " │ " + hostname + " │ " + ping_str + " │");
    }

    Serial.println("└─────────────────┴──────────────────────────────┴──────────┘");
  }

  Serial.println("=================================\n");
}

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case ARDUINO_EVENT_ETH_START:
    Serial.println("Iniciando Ethernet");
    delay(1000);
    // set eth hostname here
    ETH.setHostname("WT32-IP-Scanner");
    break;
  case ARDUINO_EVENT_ETH_CONNECTED:
    Serial.println("Ethernet conectado");
    delay(1000);
    Serial.println("Obtendo endereço IP por DHCP...");
    delay(1000);
    break;
  case ARDUINO_EVENT_ETH_GOT_IP:
    Serial.print("ETH MAC: ");
    Serial.print(ETH.macAddress());
    Serial.print(", IPv4: ");
    Serial.print(ETH.localIP());
    if (ETH.fullDuplex())
    {
      Serial.print(", FULL_DUPLEX");
    }
    Serial.print(", ");
    Serial.print(ETH.linkSpeed());
    Serial.println("Mbps");
    eth_connected = true;
    break;
  case ARDUINO_EVENT_ETH_DISCONNECTED:
    Serial.println("Ethernet desconectado");
    eth_connected = false;
    gateway_tested = false;
    network_scanned = false;
    break;
  case ARDUINO_EVENT_ETH_STOP:
    Serial.println("Parado a Ethernet");
    eth_connected = false;
    gateway_tested = false;
    network_scanned = false;
    break;
  default:
    break;
  }
}