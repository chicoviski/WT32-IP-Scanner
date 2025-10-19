# 📦 Guia de Instalação Completo - PlatformIO

## Pré-requisitos

- ✅ Windows 10/11, macOS ou Linux
- ✅ Conexão com internet (estável)
- ✅ Cabo USB para programação (UART/TTL adapter)
- ✅ Hardware montado conforme [HARDWARE.md](HARDWARE.md)

## Passo 1: Instalar Visual Studio Code

### Windows
1. Baixe em: https://code.visualstudio.com/
2. Execute o instalador `.exe`
3. Marque "Add to PATH" durante instalação
4. Siga as instruções padrão

### macOS
1. Baixe o arquivo `.dmg`
2. Arraste VS Code para Applications
3. Abra e conceda permissões necessárias

### Linux (Ubuntu/Debian)
```bash
# Via Snap (recomendado)
sudo snap install code --classic

# Ou via APT
wget -qO- https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > packages.microsoft.gpg
sudo install -o root -g root -m 644 packages.microsoft.gpg /etc/apt/trusted.gpg.d/
sudo sh -c 'echo "deb [arch=amd64] https://packages.microsoft.com/repos/code stable main" > /etc/apt/sources.list.d/vscode.list'
sudo apt update
sudo apt install code
```

## Passo 2: Instalar PlatformIO Extension

### 2.1 Via VS Code Extensions

1. Abra VS Code
2. Pressione `Ctrl+Shift+X` (ou Cmd+Shift+X no Mac)
3. Na barra de pesquisa, digite: **PlatformIO IDE**
4. Encontre a extensão oficial (autor: PlatformIO)
5. Clique em **Install**
6. Aguarde instalação completa (5-10 minutos)
   - Baixa Python embutido
   - Instala toolchains
   - Configura ambiente
7. Reinicie VS Code quando solicitado

### 2.2 Verificar Instalação

1. Clique no ícone do **PlatformIO** na barra lateral (🏠)
2. Deve abrir o **PIO Home**
3. Se aparecer "PlatformIO IDE is ready!", está tudo OK!

## Passo 3: Clonar o Projeto

### Opção A: Via Git (Recomendado)

#### 3.1 Instalar Git (se não tiver)

**Windows**: https://git-scm.com/download/win

**Linux**:
```bash
sudo apt install git
```

**macOS**: (já vem instalado ou instala automaticamente)

#### 3.2 Clonar

```bash
# No terminal integrado do VS Code (Ctrl+`)
cd ~/Documents/PlatformIO/Projects  # Windows: cd %USERPROFILE%\Documents\PlatformIO\Projects
git clone https://github.com/chicoviski/WT32-IP-Scanner.git
cd WT32-IP-Scanner
code .  # Abre o projeto no VS Code
```

### Opção B: Via PlatformIO (Interface)

1. Abra **PIO Home** (ícone 🏠 na barra lateral)
2. Clique em **Open Project**
3. Ou: **Miscellaneous → Clone Git Project**
4. Cole: `https://github.com/chicoviski/WT32-IP-Scanner.git`
5. Escolha pasta de destino
6. Clique em **Clone**

### Opção C: Download Manual

1. Acesse: https://github.com/chicoviski/WT32-IP-Scanner
2. Clique em **Code → Download ZIP**
3. Extraia para `~/Documents/PlatformIO/Projects/`
4. No VS Code: **File → Open Folder** → selecione a pasta

## Passo 4: Instalar Dependências

### 4.1 Instalação Automática (Recomendado)

PlatformIO lê `platformio.ini` e instala automaticamente as bibliotecas:

1. Abra o projeto no VS Code
2. PlatformIO detecta `platformio.ini`
3. Na primeira compilação, baixa todas as libs automaticamente
4. Aguarde a mensagem: "Library Manager: Installation complete"

### 4.2 Instalação Manual (se necessário)

1. Clique no ícone **PlatformIO** na barra lateral
2. Vá em **PIO Home → Libraries**
3. Procure e instale:

| Biblioteca | ID/Nome | Versão |
|------------|---------|--------|
| Adafruit GFX Library | 13 | ^1.11.9 |
| Adafruit ILI9341 | 576 | ^1.6.0 |
| XPT2046_Touchscreen | 105 | ^1.4 |
| ESP32Ping | 1082 | ^1.7 |

### 4.3 Verificar Dependências

```bash
# No terminal do VS Code
pio pkg list

# Deve mostrar todas as 4 bibliotecas instaladas
```

## Passo 5: Configurar platformio.ini

Abra `platformio.ini` e ajuste a porta serial:

```ini
; Descomente e ajuste para sua porta:

; Windows (verifique no Gerenciador de Dispositivos):
upload_port = COM3

; Linux:
upload_port = /dev/ttyUSB0

; macOS:
upload_port = /dev/cu.usbserial-14410
```

### Como Descobrir a Porta

#### Windows
1. **Gerenciador de Dispositivos**
2. **Portas (COM & LPT)**
3. Procure "USB-SERIAL CH340" ou "CP210x"

#### Linux
```bash
ls /dev/ttyUSB*
# Ou
dmesg | grep tty
```

#### macOS
```bash
ls /dev/cu.*
```

#### Via PlatformIO
```bash
pio device list
```

## Passo 6: Preparar Hardware para Upload

### 6.1 Modo Boot

O WT32-ETH01 precisa entrar em modo boot para programação:

1. **Desligue** o WT32 (desconecte USB)
2. **Conecte GPIO0 ao GND** (use jumper/fio)
3. **Conecte USB** (liga automaticamente)
4. WT32 está agora em modo boot

**Nota**: Alguns adaptadores USB-Serial têm botão de BOOT, tornando esse passo desnecessário.

### 6.2 Verificar Conexão

```bash
pio device list

# Deve mostrar sua porta COM/tty
```

## Passo 7: Compilar e Fazer Upload

### Via Interface Gráfica

1. Na barra inferior do VS Code, clique:
   - **✓** (checkmark) = Compilar
   - **→** (arrow) = Upload
   - **🔌** (plug) = Monitor Serial
   - **🗑️** (trash) = Clean

2. Ou use o ícone **PlatformIO** na barra lateral:
   - **PROJECT TASKS → esp32dev → General → Build**
   - **PROJECT TASKS → esp32dev → General → Upload**

### Via Terminal

```bash
# Compilar
pio run

# Upload
pio run --target upload

# Upload + Monitor Serial (recomendado)
pio run --target upload --target monitor
```

### 7.1 Processo de Upload

1. Clique em **Upload (→)**
2. Aguarde compilação:
   ```
   Building .pio\build\esp32dev\firmware.bin
   Linking .pio\build\esp32dev\firmware.elf
   ```
3. Quando ver "Connecting.........", o upload começou
4. Aguarde até aparecer:
   ```
   Hard resetting via RTS pin...
   ========================= [SUCCESS] Took X.XX seconds
   ```

### 7.2 Executar

1. **Desconecte GPIO0 do GND**
2. Pressione o botão **RESET** no WT32
3. Ou desconecte e reconecte o USB
4. Abra **Serial Monitor** (ícone 🔌 na barra inferior)

## Passo 8: Verificar Funcionamento

### 8.1 Serial Monitor (115200 baud)

O Serial Monitor deve mostrar:

```
===== WT32-ETH01 Touch Scanner =====
Versao Melhorada v2.0
Digite 'help' para comandos disponiveis

===== Configuracao de Rede =====
Modo: DHCP
================================

Inicializando display...
Inicializando touch...
Display & Touch OK!
ETH Started
ETH Connected
ETH IPv4: 192.168.1.xxx
Setup completo - Toque para controlar!
```

### 8.2 Display

Deve mostrar:
- Header azul "WT32 IP SCANNER"
- Status: "Conectado - Toque para controlar"
- Informações de rede (IP, velocidade, máscara)
- 4 botões: SCAN, CONFIG, DHCP, AUTO

## Atalhos Úteis do PlatformIO

| Atalho | Windows/Linux | macOS | Função |
|--------|---------------|-------|--------|
| Build | `Ctrl+Alt+B` | `Cmd+Opt+B` | Compilar |
| Upload | `Ctrl+Alt+U` | `Cmd+Opt+U` | Upload |
| Serial Monitor | `Ctrl+Alt+S` | `Cmd+Opt+S` | Abrir Monitor |
| Clean | `Ctrl+Alt+C` | `Cmd+Opt+C` | Limpar Build |
| Terminal | `` Ctrl+` `` | `` Cmd+` `` | Terminal |

## Troubleshooting de Instalação

### ❌ PlatformIO não instala
**Solução**: 
- Verifique conexão com internet
- Desative antivírus temporariamente
- Instale manualmente o Python 3.9+ antes

### ❌ "Library not found" ao compilar
**Solução**:
```bash
# Force reinstalação
pio pkg install --force

# Ou instale uma por vez
pio pkg install "adafruit/Adafruit GFX Library@^1.11.9"
```

### ❌ "Failed to connect" durante upload
**Solução**: 
- Verifique GPIO0 conectado ao GND
- Teste outra porta USB
- Instale drivers CH340/CP2102:
  - Windows: https://www.wch-ic.com/downloads/CH341SER_EXE.html
  - macOS: https://github.com/adrianmihalko/ch340g-ch34g-ch34x-mac-os-x-driver

### ❌ Erro "espressif32 @ X.X.X is outdated"
**Solução**:
```bash
pio pkg update
```

### ❌ Upload muito lento
**Solução**: Altere no `platformio.ini`:
```ini
upload_speed = 921600  ; Mais rápido (pode não funcionar em todos os casos)
```

### ❌ Watchdog reset após upload
**Solução**: Adicione no `platformio.ini`:
```ini
build_flags = 
    -D CONFIG_ESP_TASK_WDT_TIMEOUT_S=30
```

### ❌ Serial Monitor não abre
**Solução**:
```bash
# Via terminal
pio device monitor -b 115200
```

## Comandos PlatformIO Essenciais

```bash
# Informações do projeto
pio project

# Listar ambientes disponíveis
pio run --list-targets

# Compilar ambiente específico
pio run -e esp32dev

# Upload para ambiente específico
pio run -e esp32dev --target upload

# Monitor serial com filtros
pio device monitor --filter time --filter default

# Atualizar plataformas e bibliotecas
pio pkg update

# Mostrar informações de memória
pio run --target size

# Gerar arquivo .hex
pio run --target buildprog
```

## Estrutura de Pastas PlatformIO

```
WT32-IP-Scanner/
├── .pio/                   # Build cache (ignorado pelo Git)
├── .vscode/                # Configurações do VS Code
├── include/                # Headers (.h)
├── lib/                    # Bibliotecas locais personalizadas
├── src/
│   └── main.cpp            # Código principal (renomeie .ino para .cpp)
├── test/                   # Testes unitários
├── platformio.ini          # Configuração do projeto
└── .gitignore              # Arquivos ignorados pelo Git
```

## Migrar de .ino para .cpp

Se seu código ainda é `.ino`, renomeie para `.cpp` e adicione no topo:

```cpp
#include <Arduino.h>

// Resto do código...
```

## Próximos Passos

Após instalação bem-sucedida:

1. ✅ Leia [CONFIGURACAO.md](CONFIGURACAO.md) para configurar rede
2. ✅ Teste comandos seriais (digite `help`)
3. ✅ Faça sua primeira varredura de rede
4. ✅ Explore os ambientes de debug e release

---

🎉 **Instalação PlatformIO concluída!** Aproveite seu scanner de IP!

💡 **Dica**: Use o ambiente `esp32dev-debug` para desenvolvimento e `esp32dev-release` para produção.