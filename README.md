# 🌐 WT32-IP-Scanner

Scanner de rede com display touchscreen para WT32-ETH01 (Ethernet + ILI9341)

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-green.svg)
![Version](https://img.shields.io/badge/version-2.0-orange.svg)
![PlatformIO](https://img.shields.io/badge/PlatformIO-Ready-blue.svg)

## 📋 Descrição

Scanner de IP profissional para a placa **WT32-ETH01** com display touchscreen **ILI9341**. Detecta dispositivos ativos na rede local através de ping ICMP e conexões TCP, com interface visual intuitiva e controle via touch e comandos seriais.

## ✨ Características

- ✅ **Varredura inteligente de rede** com reconhecimento de máscara de sub-rede
- ✅ **Ping ICMP real** + fallback TCP para máxima detecção
- ✅ **Interface touchscreen** com botões visuais e feedback
- ✅ **Configuração flexível**: DHCP ou IP estático
- ✅ **Controle via Serial**: comandos completos para configuração remota
- ✅ **Auto-scan**: varredura automática a cada 60 segundos
- ✅ **Armazenamento persistente** de configurações (NVS)
- ✅ **Watchdog Timer** para operação contínua sem travamentos
- ✅ **Gerenciamento de energia** com modo economia após inatividade
- ✅ **100% em Português Brasil** 🇧🇷

## 🛠️ Hardware Necessário

| Componente | Especificação |
|------------|---------------|
| **Microcontrolador** | WT32-ETH01 (ESP32 + LAN8720) |
| **Display** | ILI9341 2.8" TFT 320x240 (SPI) |
| **Touchscreen** | XPT2046 (resistivo) |
| **Alimentação** | 5V 2A mínimo |

### Pinagem

#### Display ILI9341 (HSPI)
```
TFT_CS   -> GPIO 33
TFT_DC   -> GPIO 15
TFT_RST  -> GPIO 32
TFT_MOSI -> GPIO 14
TFT_SCLK -> GPIO 12
TFT_MISO -> (não usado)
```

#### Touchscreen XPT2046
```
TOUCH_CS  -> GPIO 4
TOUCH_IRQ -> GPIO 35
```

#### Ethernet (WT32-ETH01 - built-in)
```
PHY Addr: 1
MDC:  GPIO 23
MDIO: GPIO 18
Power: GPIO 16
Clock: GPIO0_IN
```

## 📦 Instalação

### 1. Instalar Visual Studio Code
- Baixe em: https://code.visualstudio.com/
- Instale seguindo as instruções para seu sistema operacional

### 2. Instalar PlatformIO Extension

1. Abra VS Code
2. Clique no ícone de **Extensions** (Ctrl+Shift+X)
3. Procure por: **PlatformIO IDE**
4. Clique em **Install**
5. Aguarde a instalação completa (~5-10 minutos)
6. Reinicie o VS Code quando solicitado

### 3. Clonar o Projeto

#### Opção A: Via Terminal Integrado

```bash
# Abra o terminal no VS Code (Ctrl+`)
cd ~/Documents/PlatformIO/Projects
git clone https://github.com/chicoviski/WT32-IP-Scanner.git
cd WT32-IP-Scanner
code .
```

#### Opção B: Via Interface do PlatformIO

1. Clique no ícone do **PlatformIO** na barra lateral
2. Clique em **Open** (ou **Clone Git Project**)
3. Cole: `https://github.com/chicoviski/WT32-IP-Scanner.git`
4. Escolha a pasta de destino

### 4. Instalar Dependências

As bibliotecas são instaladas automaticamente baseado no `platformio.ini`:

```ini
lib_deps = 
    adafruit/Adafruit GFX Library@^1.11.9
    adafruit/Adafruit ILI9341@^1.6.0
    paulstoffregen/XPT2046_Touchscreen@^1.4
    marian-craciunescu/ESP32Ping@^1.7
```

Para instalar manualmente (se necessário):
1. Abra **PIO Home** (ícone de casa no PlatformIO)
2. Vá em **Libraries**
3. Procure e instale cada biblioteca

### 5. Configurar e Fazer Upload

1. Conecte o WT32-ETH01 via USB-Serial
2. Coloque em **modo boot**:
   - Conecte **GPIO0** ao **GND**
   - Pressione **RESET** ou ligue o dispositivo
3. No VS Code:
   - Abra `platformio.ini`
   - Verifique a porta serial (ajuste `upload_port` se necessário)
4. Clique na **seta (→)** na barra inferior (Upload)
5. Aguarde compilação e upload
6. Quando concluído:
   - **Desconecte GPIO0 do GND**
   - Pressione **RESET**

### 6. Monitorar Serial

- Clique no **ícone de plug** na barra inferior (Serial Monitor)
- Ou use: `Ctrl+Alt+S`
- Velocidade: 115200 baud (já configurado no `platformio.ini`)

## 🚀 Como Usar

### Interface Touchscreen

#### Tela Principal
- **SCAN**: Inicia varredura manual da rede
- **CONFIG**: Abre menu de configuração
- **DHCP/STATIC**: Alterna entre DHCP e IP estático (requer reinício)
- **AUTO**: Ativa/desativa varredura automática a cada 60s

#### Menu de Configuração
- **SET IP**: Define endereço IP estático (via Serial)
- **SET MASK**: Define máscara de sub-rede (via Serial)
- **SET GW**: Define gateway (via Serial)
- **SET DNS**: Define servidor DNS (via Serial)
- **SAVE**: Salva configurações e reinicia
- **CANCEL**: Descarta alterações e volta

### Comandos Seriais (115200 baud)

Abra o Serial Monitor e digite:

```bash
help                        # Lista todos os comandos
ip 192.168.1.100           # Define IP estático
subnet 255.255.255.0       # Define máscara de sub-rede
gateway 192.168.1.1        # Define gateway
dns 8.8.8.8                # Define servidor DNS
scan                        # Inicia varredura manual
list                        # Lista dispositivos encontrados
```

### Exemplo de Configuração

```bash
# Configurar IP estático 192.168.1.50
ip 192.168.1.50
subnet 255.255.255.0
gateway 192.168.1.1
dns 8.8.8.8

# Tocar em CONFIG → SAVE no display
# O sistema reiniciará automaticamente
```

## 📊 Informações na Tela

- **Status**: Estado da conexão e operações
- **Modo de Rede**: DHCP ou Estático
- **Velocidade**: Link speed (10M/100M)
- **IP Local**: Endereço IP do dispositivo
- **Máscara**: Subnet mask configurada
- **Barra de Progresso**: Durante varredura (%)
- **Lista de Dispositivos**: IPs ativos encontrados
- **Contagem**: Total de dispositivos na rede

## 🔧 Configurações PlatformIO

### platformio.ini

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

; Bibliotecas
lib_deps = 
    adafruit/Adafruit GFX Library@^1.11.9
    adafruit/Adafruit ILI9341@^1.6.0
    paulstoffregen/XPT2046_Touchscreen@^1.4
    marian-craciunescu/ESP32Ping@^1.7

; Monitor Serial
monitor_speed = 115200
monitor_filters = 
    default
    time

; Upload
upload_speed = 115200
upload_port = COM3  ; Ajuste para sua porta (Linux: /dev/ttyUSB0, Mac: /dev/cu.usbserial)

; Build
build_flags = 
    -D CORE_DEBUG_LEVEL=0
    -D BOARD_HAS_PSRAM

; Particionamento
board_build.partitions = default.csv
```

### Comandos PlatformIO Úteis

```bash
# Compilar
pio run

# Upload
pio run --target upload

# Monitor Serial
pio device monitor

# Limpar build
pio run --target clean

# Upload + Monitor
pio run --target upload && pio device monitor

# Listar portas seriais
pio device list
```

### Atalhos no VS Code

| Atalho | Função |
|--------|--------|
| `Ctrl+Alt+B` | Build (Compilar) |
| `Ctrl+Alt+U` | Upload |
| `Ctrl+Alt+S` | Serial Monitor |
| `Ctrl+Alt+C` | Clean |
| `F7` | Build |
| `Shift+F7` | Clean |

## 🐛 Troubleshooting

### Display não funciona
- Verifique conexões SPI (MOSI, SCLK, CS, DC, RST)
- Teste com exemplo da biblioteca Adafruit_ILI9341

### Touch não responde
- Verifique pino CS e IRQ
- Use exemplo XPT2046_Touchscreen para testar
- Ajuste calibração (TS_MINX, etc.)

### Ethernet não conecta
- Verifique cabo de rede
- Teste DHCP primeiro antes de IP estático
- Verifique se o LED do ethernet pisca

### Erro de compilação: "library not found"
```bash
# Force reinstalar bibliotecas
pio lib install --force
```

### Erro de upload: "Failed to connect"
- Verifique GPIO0 conectado ao GND durante upload
- Teste outra porta USB
- Reduza upload_speed para 9600 no platformio.ini

### Watchdog Reset
```ini
; Adicione no platformio.ini
build_flags = 
    -D CONFIG_ESP_TASK_WDT_TIMEOUT_S=30
```

## 📚 Estrutura do Projeto

```
WT32-IP-Scanner/
├── platformio.ini                 # Configuração PlatformIO
├── README.md                      # Este arquivo
├── LICENSE                        # Licença MIT
├── .gitignore                     # Arquivos ignorados
├── src/
│   └── main.cpp                   # Código principal
├── include/
│   └── config.h                   # Configurações (opcional)
├── lib/                           # Bibliotecas locais (se houver)
├── test/                          # Testes unitários
└── docs/
    ├── INSTALACAO.md              # Guia de instalação PlatformIO
    ├── CONFIGURACAO.md            # Guia de configuração
    └── HARDWARE.md                # Esquemático e conexões
```

## 🔄 Changelog

### v2.0 (Atual - 2025-10-19)
- ✅ Ping ICMP real com ESP32Ping
- ✅ Varredura otimizada baseada em subnet
- ✅ Entrada serial completa com comandos
- ✅ Watchdog timer implementado
- ✅ Gerenciamento de energia
- ✅ Interface 100% em português
- ✅ Estrutura de Device melhorada
- ✅ Migração completa para PlatformIO

### v1.0 (Original)
- Scanner básico com ping TCP
- Interface touchscreen
- Configuração DHCP/Estático

## 🤝 Contribuindo

Contribuições são bem-vindas! Por favor:

1. Fork o projeto
2. Crie uma branch para sua feature (`git checkout -b feature/NovaFuncionalidade`)
3. Commit suas mudanças (`git commit -m 'Adiciona nova funcionalidade'`)
4. Push para a branch (`git push origin feature/NovaFuncionalidade`)
5. Abra um Pull Request

## 📜 Licença

Este projeto está sob a licença MIT. Veja o arquivo [LICENSE](LICENSE) para mais detalhes.

## 👤 Autor

**chicoviski**
- GitHub: [@chicoviski](https://github.com/chicoviski)

## 🙏 Agradecimentos

- Biblioteca Adafruit GFX
- Biblioteca ESP32Ping
- Comunidade ESP32 Brasil
- PlatformIO Team

## 📞 Suporte

- 🐛 **Issues**: [GitHub Issues](https://github.com/chicoviski/WT32-IP-Scanner/issues)
- 💬 **Discussões**: [GitHub Discussions](https://github.com/chicoviski/WT32-IP-Scanner/discussions)

---

⭐ Se este projeto foi útil, considere dar uma estrela!

🇧🇷 Feito com ❤️ no Brasil usando PlatformIO