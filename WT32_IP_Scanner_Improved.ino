#include <ETH.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <Preferences.h>
#include <ESP32Ping.h>
#include "esp_task_wdt.h"

// ===== Configuração dos Pinos do Display =====
#define TFT_CS    33
#define TFT_DC    15
#define TFT_RST   32
#define TFT_MOSI  14
#define TFT_SCLK  12
#define TFT_MISO  -1

// ===== Configuração do Touch Screen =====
#define TOUCH_CS  4
#define TOUCH_IRQ 35

// Usar HSPI (não conflita com Ethernet)
SPIClass hspi(HSPI);
Adafruit_ILI9341 tft = Adafruit_ILI9341(&hspi, TFT_DC, TFT_CS, TFT_RST);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// ===== Calibração do Touch =====
#define TS_MINX 300
#define TS_MINY 300
#define TS_MAXX 3800
#define TS_MAXY 3800

// ===== Configuração de Rede =====
Preferences preferences;

struct NetworkConfig {
  bool useDHCP;
  IPAddress staticIP;
  IPAddress subnet;
  IPAddress gateway;
  IPAddress dns;
};

NetworkConfig netConfig;

// ===== Estrutura Melhorada de Dispositivos =====
struct Device {
  IPAddress ip;
  String mac;
  unsigned long lastSeen;
  bool active;
};

// ===== Variáveis Globais =====
static bool eth_connected = false;
static bool scanning = false;
static IPAddress localIP;
static IPAddress subnet;
static Device deviceList[50];
static int deviceCount = 0;

// Variáveis do Touch
unsigned long lastTouchTime = 0;
unsigned long lastActivity = 0;
bool inConfigMenu = false;
bool displayDimmed = false;

// ===== Estruturas de Botões =====
struct Button {
  int x, y, w, h;
  String label;
  uint16_t color;
  bool enabled;
};

// Botões da tela principal
Button btnScan = {10, 195, 70, 35, "SCAN", ILI9341_GREEN, true};
Button btnConfig = {90, 195, 70, 35, "CONFIG", ILI9341_BLUE, true};
Button btnToggleDHCP = {170, 195, 70, 35, "", ILI9341_ORANGE, true};
Button btnAutoScan = {250, 195, 60, 35, "AUTO", ILI9341_PURPLE, true};

// Botões do menu de configuração
Button btnIPConfig = {10, 80, 145, 35, "SET IP", ILI9341_CYAN, true};
Button btnSubnetConfig = {165, 80, 145, 35, "SET MASK", ILI9341_CYAN, true};
Button btnGatewayConfig = {10, 125, 145, 35, "SET GW", ILI9341_CYAN, true};
Button btnDNSConfig = {165, 125, 145, 35, "SET DNS", ILI9341_CYAN, true};
Button btnSaveConfig = {80, 175, 80, 35, "SAVE", ILI9341_GREEN, true};
Button btnCancelConfig = {170, 175, 80, 35, "CANCEL", ILI9341_RED, true};

// Auto-scan
bool autoScanEnabled = true;

// ===== Funções de Desenho =====
void drawButton(Button btn) {
  uint16_t color = btn.enabled ? btn.color : ILI9341_DARKGREY;
  
  tft.fillRoundRect(btn.x, btn.y, btn.w, btn.h, 5, color);
  tft.drawRoundRect(btn.x, btn.y, btn.w, btn.h, 5, ILI9341_WHITE);
  
  // Centralizar texto
  int16_t x1, y1;
  uint16_t w, h;
  tft.setTextSize(1);
  tft.getTextBounds(btn.label, 0, 0, &x1, &y1, &w, &h);
  
  int textX = btn.x + (btn.w - w) / 2;
  int textY = btn.y + (btn.h - h) / 2 + 2;
  
  tft.setCursor(textX, textY);
  tft.setTextColor(ILI9341_WHITE);
  tft.println(btn.label);
}

void drawHeader() {
  tft.fillRect(0, 0, 320, 30, ILI9341_NAVY);
  tft.setCursor(5, 7);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("WT32 IP SCANNER");
}

void drawStatus(String text, uint16_t color) {
  tft.fillRect(0, 35, 320, 18, ILI9341_BLACK);
  tft.setCursor(5, 37);
  tft.setTextColor(color);
  tft.setTextSize(1);
  tft.println(text);
}

void drawNetworkInfo() {
  tft.fillRect(0, 58, 320, 40, ILI9341_BLACK);
  
  tft.setCursor(5, 60);
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(1);
  tft.print("Modo: ");
  tft.setTextColor(ILI9341_WHITE);
  tft.print(netConfig.useDHCP ? "DHCP" : "Estatico");
  
  tft.setCursor(180, 60);
  tft.setTextColor(ILI9341_CYAN);
  tft.print("Vel: ");
  tft.setTextColor(ILI9341_WHITE);
  tft.print(ETH.linkSpeed());
  tft.println("M");
  
  tft.setCursor(5, 73);
  tft.setTextColor(ILI9341_CYAN);
  tft.print("IP: ");
  tft.setTextColor(ILI9341_WHITE);
  tft.println(localIP.toString());
  
  tft.setCursor(5, 86);
  tft.setTextColor(ILI9341_CYAN);
  tft.print("Mascara: ");
  tft.setTextColor(ILI9341_WHITE);
  tft.println(subnet.toString());
}

void drawMainButtons() {
  // Atualizar label do botão DHCP
  btnToggleDHCP.label = netConfig.useDHCP ? "DHCP" : "STATIC";
  
  // Atualizar cor do botão AUTO
  btnAutoScan.color = autoScanEnabled ? ILI9341_PURPLE : ILI9341_DARKGREY;
  
  drawButton(btnScan);
  drawButton(btnConfig);
  drawButton(btnToggleDHCP);
  drawButton(btnAutoScan);
}

void drawScanProgress(int current, int total) {
  int barWidth = 300;
  int barHeight = 20;
  int barX = 10;
  int barY = 103;
  
  tft.fillRect(barX, barY, barWidth, barHeight, ILI9341_BLACK);
  tft.drawRect(barX, barY, barWidth, barHeight, ILI9341_WHITE);
  
  int progress = (current * (barWidth - 4)) / total;
  tft.fillRect(barX + 2, barY + 2, progress, barHeight - 4, ILI9341_GREEN);
  
  tft.fillRect(0, 128, 320, 15, ILI9341_BLACK);
  tft.setCursor(10, 130);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(1);
  tft.print("Varrendo: ");
  tft.print(current);
  tft.print("/");
  tft.print(total);
  tft.print(" (");
  tft.print((current * 100) / total);
  tft.print("%)");
}

void drawDevicesList() {
  tft.fillRect(0, 148, 320, 42, ILI9341_BLACK);
  
  tft.setCursor(5, 150);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(1);
  tft.print("Dispositivos: ");
  tft.setTextColor(ILI9341_WHITE);
  tft.println(deviceCount);
  
  tft.drawLine(0, 163, 320, 163, ILI9341_DARKGREY);
  
  int yPos = 168;
  int maxDisplay = min(2, deviceCount);
  
  for (int i = 0; i < maxDisplay; i++) {
    tft.setCursor(5, yPos);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    
    String displayText = deviceList[i].ip.toString();
    if (deviceList[i].ip == localIP) {
      displayText += " (ESTE)";
    }
    
    tft.println(displayText);
    yPos += 11;
  }
  
  if (deviceCount > 2) {
    tft.setCursor(180, 168);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print("+ ");
    tft.print(deviceCount - 2);
    tft.println(" mais");
  }
}

// ===== Funções de Configuração de Rede =====
void loadNetworkConfig() {
  preferences.begin("network", false);
  
  netConfig.useDHCP = preferences.getBool("useDHCP", true);
  
  uint32_t ip = preferences.getUInt("staticIP", 0xC0A80164);
  netConfig.staticIP = IPAddress(ip);
  
  uint32_t sub = preferences.getUInt("subnet", 0xFFFFFF00);
  netConfig.subnet = IPAddress(sub);
  
  uint32_t gw = preferences.getUInt("gateway", 0xC0A80101);
  netConfig.gateway = IPAddress(gw);
  
  uint32_t dns = preferences.getUInt("dns", 0x08080808);
  netConfig.dns = IPAddress(dns);
  
  preferences.end();
  
  Serial.println("\n===== Configuracao de Rede =====");
  Serial.print("Modo: ");
  Serial.println(netConfig.useDHCP ? "DHCP" : "IP Estatico");
  if (!netConfig.useDHCP) {
    Serial.print("IP: "); Serial.println(netConfig.staticIP);
    Serial.print("Mascara: "); Serial.println(netConfig.subnet);
    Serial.print("Gateway: "); Serial.println(netConfig.gateway);
    Serial.print("DNS: "); Serial.println(netConfig.dns);
  }
  Serial.println("================================\n");
}

void saveNetworkConfig() {
  preferences.begin("network", false);
  
  preferences.putBool("useDHCP", netConfig.useDHCP);
  preferences.putUInt("staticIP", (uint32_t)netConfig.staticIP);
  preferences.putUInt("subnet", (uint32_t)netConfig.subnet);
  preferences.putUInt("gateway", (uint32_t)netConfig.gateway);
  preferences.putUInt("dns", (uint32_t)netConfig.dns);
  
  preferences.end();
  
  Serial.println(">>> Configuracao salva!");
}

// ===== NOVA FUNÇÃO: Entrada Serial =====
void handleSerialInput() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toLowerCase();
    
    if (cmd.startsWith("ip ")) {
      IPAddress newIP;
      if (newIP.fromString(cmd.substring(3))) {
        netConfig.staticIP = newIP;
        Serial.println("✓ IP atualizado para: " + newIP.toString());
        if (inConfigMenu) {
          showConfigMenu(); // Atualizar display
        }
      } else {
        Serial.println("✗ IP invalido! Use: ip 192.168.1.100");
      }
    }
    else if (cmd.startsWith("subnet ")) {
      IPAddress newSubnet;
      if (newSubnet.fromString(cmd.substring(7))) {
        netConfig.subnet = newSubnet;
        Serial.println("✓ Mascara atualizada para: " + newSubnet.toString());
        if (inConfigMenu) {
          showConfigMenu();
        }
      } else {
        Serial.println("✗ Mascara invalida! Use: subnet 255.255.255.0");
      }
    }
    else if (cmd.startsWith("gateway ")) {
      IPAddress newGateway;
      if (newGateway.fromString(cmd.substring(8))) {
        netConfig.gateway = newGateway;
        Serial.println("✓ Gateway atualizado para: " + newGateway.toString());
        if (inConfigMenu) {
          showConfigMenu();
        }
      } else {
        Serial.println("✗ Gateway invalido! Use: gateway 192.168.1.1");
      }
    }
    else if (cmd.startsWith("dns ")) {
      IPAddress newDNS;
      if (newDNS.fromString(cmd.substring(4))) {
        netConfig.dns = newDNS;
        Serial.println("✓ DNS atualizado para: " + newDNS.toString());
        if (inConfigMenu) {
          showConfigMenu();
        }
      } else {
        Serial.println("✗ DNS invalido! Use: dns 8.8.8.8");
      }
    }
    else if (cmd == "help" || cmd == "ajuda") {
      Serial.println("\n===== COMANDOS DISPONIVEIS =====");
      Serial.println("ip 192.168.1.100       - Define IP estatico");
      Serial.println("subnet 255.255.255.0   - Define mascara de rede");
      Serial.println("gateway 192.168.1.1    - Define gateway");
      Serial.println("dns 8.8.8.8            - Define servidor DNS");
      Serial.println("scan                   - Inicia varredura manual");
      Serial.println("list                   - Lista dispositivos encontrados");
      Serial.println("help                   - Mostra esta mensagem");
      Serial.println("================================\n");
    }
    else if (cmd == "scan") {
      if (eth_connected && !scanning) {
        Serial.println(">>> Iniciando varredura via serial...");
        scanNetwork();
      } else {
        Serial.println("✗ Nao e possivel varrer - verifique a conexao");
      }
    }
    else if (cmd == "list" || cmd == "listar") {
      Serial.println("\n===== DISPOSITIVOS ENCONTRADOS =====");
      if (deviceCount == 0) {
        Serial.println("Nenhum dispositivo encontrado ainda.");
        Serial.println("Execute 'scan' para iniciar varredura.");
      } else {
        for (int i = 0; i < deviceCount; i++) {
          Serial.print(i + 1);
          Serial.print(". ");
          Serial.print(deviceList[i].ip.toString());
          if (deviceList[i].ip == localIP) {
            Serial.print(" (ESTE DISPOSITIVO)");
          }
          Serial.println();
        }
      }
      Serial.println("====================================\n");
    }
    else if (cmd != "") {
      Serial.println("✗ Comando desconhecido. Digite 'help' para ajuda.");
    }
  }
}

// ===== Menu de Configuração =====
void showConfigMenu() {
  inConfigMenu = true;
  tft.fillScreen(ILI9341_BLACK);
  
  // Header
  drawHeader();
  drawStatus("Configuracao de Rede", ILI9341_CYAN);
  
  int yPos = 60;
  tft.setTextSize(1);
  
  // Mostrar configuração atual
  tft.setCursor(5, yPos);
  tft.setTextColor(ILI9341_YELLOW);
  tft.print("Modo: ");
  tft.setTextColor(ILI9341_WHITE);
  tft.println(netConfig.useDHCP ? "DHCP Ativado" : "IP Estatico");
  
  if (!netConfig.useDHCP) {
    yPos += 13;
    tft.setCursor(5, yPos);
    tft.setTextColor(ILI9341_CYAN);
    tft.print("IP: ");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(netConfig.staticIP.toString());
    
    tft.setCursor(180, yPos);
    tft.setTextColor(ILI9341_CYAN);
    tft.print("Masc: ");
    tft.setTextColor(ILI9341_WHITE);
    tft.println(netConfig.subnet.toString());
    
    yPos += 13;
    tft.setCursor(5, yPos);
    tft.setTextColor(ILI9341_CYAN);
    tft.print("GW: ");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(netConfig.gateway.toString());
    
    tft.setCursor(180, yPos);
    tft.setTextColor(ILI9341_CYAN);
    tft.print("DNS: ");
    tft.setTextColor(ILI9341_WHITE);
    tft.println(netConfig.dns.toString());
  }
  
  // Desenhar botões de configuração
  if (!netConfig.useDHCP) {
    drawButton(btnIPConfig);
    drawButton(btnSubnetConfig);
    drawButton(btnGatewayConfig);
    drawButton(btnDNSConfig);
  } else {
    // Mostrar aviso que configuração estática está desabilitada
    tft.setCursor(10, 100);
    tft.setTextColor(ILI9341_YELLOW);
    tft.println("Ative o modo IP Estatico para");
    tft.setCursor(10, 113);
    tft.println("configurar parametros de rede");
  }
  
  drawButton(btnSaveConfig);
  drawButton(btnCancelConfig);
  
  Serial.println(">>> MENU CONFIG aberto");
  Serial.println(">>> Digite 'help' para ver comandos disponiveis");
}

void showIPInputDialog(String title, IPAddress currentIP, int configType) {
  tft.fillScreen(ILI9341_BLACK);
  drawHeader();
  
  tft.setCursor(10, 40);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.println(title);
  
  tft.setCursor(10, 70);
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(1);
  tft.print("Atual: ");
  tft.setTextColor(ILI9341_WHITE);
  tft.println(currentIP.toString());
  
  tft.setCursor(10, 95);
  tft.setTextColor(ILI9341_GREEN);
  tft.println("Use o Monitor Serial:");
  tft.setCursor(10, 110);
  tft.setTextColor(ILI9341_WHITE);
  
  switch(configType) {
    case 0: 
      tft.println("Exemplo: ip 192.168.1.100");
      Serial.println("\n>>> Digite: ip 192.168.1.100");
      break;
    case 1: 
      tft.println("Exemplo: subnet 255.255.255.0");
      Serial.println("\n>>> Digite: subnet 255.255.255.0");
      break;
    case 2: 
      tft.println("Exemplo: gateway 192.168.1.1");
      Serial.println("\n>>> Digite: gateway 192.168.1.1");
      break;
    case 3: 
      tft.println("Exemplo: dns 8.8.8.8");
      Serial.println("\n>>> Digite: dns 8.8.8.8");
      break;
  }
  
  // Botão para voltar
  Button btnBack = {100, 190, 120, 35, "VOLTAR", ILI9341_RED, true};
  drawButton(btnBack);
  
  Serial.println(">>> Digite o comando e pressione Enter");
  Serial.println(">>> Ou toque em VOLTAR para cancelar");
}

// ===== Funções de Touch =====
bool isTouchInButton(int tx, int ty, Button btn) {
  return (tx >= btn.x && tx <= (btn.x + btn.w) && 
          ty >= btn.y && ty <= (btn.y + btn.h) && 
          btn.enabled);
}

void handleMainScreenTouch(int tx, int ty) {
  lastActivity = millis(); // Resetar timer de inatividade
  
  // Botão SCAN
  if (isTouchInButton(tx, ty, btnScan)) {
    if (eth_connected && !scanning) {
      Serial.println(">>> SCAN tocado");
      
      tft.fillRoundRect(btnScan.x, btnScan.y, btnScan.w, btnScan.h, 5, ILI9341_DARKGREEN);
      delay(150);
      drawButton(btnScan);
      
      scanNetwork();
    } else {
      drawStatus("Impossivel varrer - verifique rede", ILI9341_RED);
      delay(1000);
      drawStatus("Conectado - Toque para controlar", ILI9341_GREEN);
    }
  }
  // Botão CONFIG
  else if (isTouchInButton(tx, ty, btnConfig)) {
    Serial.println(">>> CONFIG tocado");
    
    tft.fillRoundRect(btnConfig.x, btnConfig.y, btnConfig.w, btnConfig.h, 5, ILI9341_NAVY);
    delay(150);
    
    showConfigMenu();
  }
  // Botão Toggle DHCP
  else if (isTouchInButton(tx, ty, btnToggleDHCP)) {
    Serial.println(">>> Alternancia DHCP tocada");
    
    tft.fillRoundRect(btnToggleDHCP.x, btnToggleDHCP.y, btnToggleDHCP.w, btnToggleDHCP.h, 5, ILI9341_DARKCYAN);
    delay(150);
    
    netConfig.useDHCP = !netConfig.useDHCP;
    saveNetworkConfig();
    
    drawStatus("Reiniciando para aplicar...", ILI9341_YELLOW);
    delay(1500);
    
    ESP.restart();
  }
  // Botão AUTO SCAN
  else if (isTouchInButton(tx, ty, btnAutoScan)) {
    Serial.println(">>> AUTO SCAN alternado");
    
    autoScanEnabled = !autoScanEnabled;
    
    drawButton(btnAutoScan);
    
    if (autoScanEnabled) {
      drawStatus("Auto-scan: ATIVADO", ILI9341_GREEN);
    } else {
      drawStatus("Auto-scan: DESATIVADO", ILI9341_ORANGE);
    }
    
    delay(1000);
    drawStatus("Conectado - Toque para controlar", ILI9341_GREEN);
  }
}

void handleConfigMenuTouch(int tx, int ty) {
  lastActivity = millis(); // Resetar timer de inatividade
  
  // Botão SAVE
  if (isTouchInButton(tx, ty, btnSaveConfig)) {
    Serial.println(">>> SAVE tocado");
    
    tft.fillRoundRect(btnSaveConfig.x, btnSaveConfig.y, btnSaveConfig.w, btnSaveConfig.h, 5, ILI9341_DARKGREEN);
    delay(150);
    
    saveNetworkConfig();
    drawStatus("Salvo! Reiniciando...", ILI9341_GREEN);
    delay(1500);
    ESP.restart();
  }
  // Botão CANCEL
  else if (isTouchInButton(tx, ty, btnCancelConfig)) {
    Serial.println(">>> CANCEL tocado");
    
    tft.fillRoundRect(btnCancelConfig.x, btnCancelConfig.y, btnCancelConfig.w, btnCancelConfig.h, 5, ILI9341_DARKRED);
    delay(150);
    
    loadNetworkConfig();
    inConfigMenu = false;
    
    tft.fillScreen(ILI9341_BLACK);
    drawHeader();
    drawStatus("Configuracao cancelada", ILI9341_YELLOW);
    
    if (eth_connected) {
      delay(1000);
      drawStatus("Conectado - Toque para controlar", ILI9341_GREEN);
      drawNetworkInfo();
      drawDevicesList();
      drawMainButtons();
    }
  }
  // Botões de configuração de IP (apenas se estático)
  else if (!netConfig.useDHCP) {
    if (isTouchInButton(tx, ty, btnIPConfig)) {
      showIPInputDialog("Definir IP", netConfig.staticIP, 0);
    }
    else if (isTouchInButton(tx, ty, btnSubnetConfig)) {
      showIPInputDialog("Definir Mascara", netConfig.subnet, 1);
    }
    else if (isTouchInButton(tx, ty, btnGatewayConfig)) {
      showIPInputDialog("Definir Gateway", netConfig.gateway, 2);
    }
    else if (isTouchInButton(tx, ty, btnDNSConfig)) {
      showIPInputDialog("Definir DNS", netConfig.dns, 3);
    }
  }
}

void handleTouch() {
  if (!touch.touched()) return;
  
  // Debounce
  if (millis() - lastTouchTime < 250) return;
  lastTouchTime = millis();
  
  TS_Point p = touch.getPoint();
  
  // Mapear coordenadas
  int tx = map(p.x, TS_MINX, TS_MAXX, 0, 320);
  int ty = map(p.y, TS_MINY, TS_MAXY, 0, 240);
  
  // Ajustar para rotação landscape
  int temp = tx;
  tx = 320 - ty;
  ty = temp;
  
  Serial.print("Toque: X=");
  Serial.print(tx);
  Serial.print(" Y=");
  Serial.println(ty);
  
  // Processar toque conforme a tela atual
  if (inConfigMenu) {
    handleConfigMenuTouch(tx, ty);
  } else {
    handleMainScreenTouch(tx, ty);
  }
}

// ===== Funções de Rede =====
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Iniciado");
      drawStatus("Ethernet Iniciando...", ILI9341_YELLOW);
      break;
      
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Conectado");
      
      if (!netConfig.useDHCP) {
        Serial.println("Aplicando IP estatico...");
        ETH.config(netConfig.staticIP, netConfig.gateway, netConfig.subnet, netConfig.dns);
      }
      break;
      
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH IPv4: ");
      Serial.println(ETH.localIP());
      
      eth_connected = true;
      localIP = ETH.localIP();
      subnet = ETH.subnetMask();
      
      drawStatus("Conectado - Toque para controlar", ILI9341_GREEN);
      drawNetworkInfo();
      drawMainButtons();
      break;
      
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Desconectado");
      eth_connected = false;
      drawStatus("Rede Desconectada!", ILI9341_RED);
      break;
      
    default:
      break;
  }
}

// ===== MELHORADO: Ping com ICMP =====
bool pingIP(IPAddress ip, int timeout = 1000) {
  // Usar ping ICMP real
  bool result = Ping.ping(ip, 1); // 1 tentativa
  
  if (result) {
    return true;
  }
  
  // Fallback: tentar conexão TCP em portas comuns
  WiFiClient client;
  int commonPorts[] = {80, 22, 23, 443, 445};
  
  for (int i = 0; i < 5; i++) {
    if (client.connect(ip, commonPorts[i], 300)) {
      client.stop();
      return true;
    }
  }
  
  return false;
}

// ===== MELHORADO: Varredura com Subnet Awareness =====
void scanNetwork() {
  if (!eth_connected) {
    drawStatus("Sem conexao de rede!", ILI9341_RED);
    return;
  }
  
  scanning = true;
  deviceCount = 0;
  
  // Limpar lista de dispositivos
  for (int i = 0; i < 50; i++) {
    deviceList[i].ip = IPAddress(0, 0, 0, 0);
    deviceList[i].mac = "";
    deviceList[i].lastSeen = 0;
    deviceList[i].active = false;
  }
  
  drawStatus("Varrendo rede...", ILI9341_YELLOW);
  Serial.println("\n===== Varredura de Rede Iniciada =====");
  
  // Calcular alcance baseado na máscara de sub-rede
  uint32_t mask = (uint32_t)subnet;
  uint32_t network = (uint32_t)localIP & mask;
  uint32_t broadcast = network | ~mask;
  
  // Extrair primeiro e último host
  uint8_t firstHost = ((network >> 0) & 0xFF) + 1;
  uint8_t lastHost = ((broadcast >> 0) & 0xFF) - 1;
  
  // Se a máscara for /24, varrer apenas o último octeto
  IPAddress baseIP = localIP;
  baseIP[3] = 0;
  
  int totalHosts = lastHost - firstHost + 1;
  int currentHost = 0;
  
  Serial.print("Alcance de varredura: ");
  Serial.print(baseIP[0]); Serial.print(".");
  Serial.print(baseIP[1]); Serial.print(".");
  Serial.print(baseIP[2]); Serial.print(".");
  Serial.print(firstHost);
  Serial.print(" - ");
  Serial.print(baseIP[0]); Serial.print(".");
  Serial.print(baseIP[1]); Serial.print(".");
  Serial.print(baseIP[2]); Serial.print(".");
  Serial.println(lastHost);
  
  for (int i = firstHost; i <= lastHost; i++) {
    IPAddress targetIP = baseIP;
    targetIP[3] = i;
    
    currentHost++;
    drawScanProgress(currentHost, totalHosts);
    
    // Adicionar este dispositivo automaticamente
    if (targetIP == localIP) {
      if (deviceCount < 50) {
        deviceList[deviceCount].ip = targetIP;
        deviceList[deviceCount].mac = ETH.macAddress();
        deviceList[deviceCount].lastSeen = millis();
        deviceList[deviceCount].active = true;
        deviceCount++;
      }
      continue;
    }
    
    // Alimentar watchdog
    esp_task_wdt_reset();
    
    if (pingIP(targetIP, 400)) {
      Serial.print("Encontrado: ");
      Serial.println(targetIP);
      
      if (deviceCount < 50) {
        deviceList[deviceCount].ip = targetIP;
        deviceList[deviceCount].lastSeen = millis();
        deviceList[deviceCount].active = true;
        deviceCount++;
        drawDevicesList();
      }
    }
    
    delay(10);
  }
  
  scanning = false;
  
  Serial.println("===== Varredura Completa =====");
  Serial.print("Dispositivos encontrados: ");
  Serial.println(deviceCount);
  
  drawStatus("Completo - " + String(deviceCount) + " dispositivos", ILI9341_GREEN);
  drawDevicesList();
  drawMainButtons();
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n===== WT32-ETH01 Touch Scanner =====");
  Serial.println("Versao Melhorada v2.0");
  Serial.println("Digite 'help' para comandos disponiveis\n");
  
  // Configurar Watchdog Timer
  esp_task_wdt_init(30, false); // 30 segundos, sem panic
  
  // Carregar configuração
  loadNetworkConfig();
  
  // Inicializar Display
  Serial.println("Inicializando display...");
  hspi.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.begin(40000000);
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  
  // Inicializar Touch
  Serial.println("Inicializando touch...");
  touch.begin(hspi);
  touch.setRotation(1);
  
  Serial.println("Display & Touch OK!");
  
  drawHeader();
  drawStatus("Inicializando rede...", ILI9341_YELLOW);
  
  // Inicializar Ethernet
  WiFi.onEvent(WiFiEvent);
  ETH.begin(1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN);
  
  lastActivity = millis();
  
  Serial.println("Setup completo - Toque para controlar!");
}

// ===== Loop =====
void loop() {
  static unsigned long lastAutoScan = 0;
  
  // Processar entrada serial
  handleSerialInput();
  
  // Processar touch
  if (!scanning) {
    handleTouch();
  }
  
  // Alimentar watchdog durante varredura
  if (scanning) {
    esp_task_wdt_reset();
  }
  
  // Auto-scan a cada 60 segundos (se habilitado)
  if (autoScanEnabled && eth_connected && !scanning && !inConfigMenu) {
    if (millis() - lastAutoScan > 60000 || lastAutoScan == 0) {
      lastAutoScan = millis();
      Serial.println("\n>>> Auto-scan acionado");
      scanNetwork();
    }
  }
  
  // Gerenciamento de energia: dimmer após 5 minutos de inatividade
  if (!displayDimmed && (millis() - lastActivity > 300000)) {
    Serial.println(">>> Display em modo economia de energia");
    // Aqui você pode implementar dimming do backlight se tiver controle de hardware
    displayDimmed = true;
  }
  
  // Restaurar display ao detectar atividade
  if (displayDimmed && touch.touched()) {
    displayDimmed = false;
    lastActivity = millis();
    Serial.println(">>> Display restaurado");
  }
  
  delay(50);
}