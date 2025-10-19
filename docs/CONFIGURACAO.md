# ⚙️ Guia de Configuração

## Configuração de Rede

O scanner suporta dois modos de configuração de rede:

### 1. DHCP (Padrão)
- IP atribuído automaticamente pelo roteador
- Plug-and-play
- Recomendado para uso doméstico

### 2. IP Estático
- IP fixo configurado manualmente
- Ideal para ambientes corporativos
- Necessário para acesso remoto consistente

## Alternar entre DHCP e IP Estático

### Via Touchscreen
1. Toque no botão **DHCP** (ou **STATIC**)
2. Confirme no prompt
3. Sistema reiniciará automaticamente
4. Nova configuração será aplicada

### Via Serial
```bash
# Não há comando direto - use o touchscreen
# Ou modifique o código e re-faça upload
```

## Configurar IP Estático

### Método 1: Via Serial Monitor (Recomendado)

1. Abra Serial Monitor (115200 baud)
2. Toque em **CONFIG** no display
3. Toque em **SET IP** (se em modo estático)
4. Digite no Serial Monitor:

```bash
ip 192.168.1.50
subnet 255.255.255.0
gateway 192.168.1.1
dns 8.8.8.8
```

5. Toque em **SAVE** no display
6. Sistema reinicia com nova configuração

### Método 2: Via Código

Edite antes do upload:

```cpp
void loadNetworkConfig() {
  preferences.begin("network", false);
  
  netConfig.useDHCP = false;  // Altere para false
  
  // Defina seus valores aqui
  netConfig.staticIP = IPAddress(192, 168, 1, 50);
  netConfig.subnet = IPAddress(255, 255, 255, 0);
  netConfig.gateway = IPAddress(192, 168, 1, 1);
  netConfig.dns = IPAddress(8, 8, 8, 8);
  
  preferences.end();
}
```

## Exemplos de Configuração

### Rede Doméstica Típica
```bash
ip 192.168.1.50
subnet 255.255.255.0
gateway 192.168.1.1
dns 8.8.8.8
```

### Rede Corporativa
```bash
ip 10.0.10.50
subnet 255.255.255.0
gateway 10.0.10.1
dns 10.0.10.10
```

### Rede com Subnet Diferente
```bash
ip 192.168.0.100
subnet 255.255.255.0
gateway 192.168.0.1
dns 1.1.1.1          # Cloudflare DNS
```

## Comandos Seriais Disponíveis

Abra Serial Monitor (115200 baud) e use:

| Comando | Descrição | Exemplo |
|---------|-----------|---------|
| `help` | Mostra lista de comandos | `help` |
| `ip <endereço>` | Define IP estático | `ip 192.168.1.100` |
| `subnet <máscara>` | Define subnet mask | `subnet 255.255.255.0` |
| `gateway <endereço>` | Define gateway | `gateway 192.168.1.1` |
| `dns <endereço>` | Define servidor DNS | `dns 8.8.8.8` |
| `scan` | Inicia varredura manual | `scan` |
| `list` | Lista dispositivos encontrados | `list` |

## Auto-Scan

### Ativar/Desativar

- **Via Touchscreen**: Toque no botão **AUTO**
  - 🟣 **Roxo**: Auto-scan ATIVADO
  - ⚫ **Cinza**: Auto-scan DESATIVADO

- **Funcionamento**: Quando ativado, faz varredura automática a cada 60 segundos

### Modificar Intervalo

Edite no código:

```cpp
if (millis() - lastAutoScan > 60000)  // 60 segundos
```

Altere `60000` para o valor desejado em milissegundos:
- 30s = `30000`
- 2min = `120000`
- 5min = `300000`

## Calibração do Touchscreen

Se os toques não estão precisos:

### 1. Encontrar Valores Ideais

Execute este código de teste:

```cpp
void loop() {
  if (touch.touched()) {
    TS_Point p = touch.getPoint();
    Serial.print("X: "); Serial.print(p.x);
    Serial.print(" Y: "); Serial.println(p.y);
  }
}
```

### 2. Tocar nos Cantos

- **Canto superior esquerdo**: anote X e Y
- **Canto inferior direito**: anote X e Y

### 3. Atualizar Calibração

```cpp
#define TS_MINX 300   // X do canto superior esquerdo
#define TS_MINY 300   // Y do canto superior esquerdo
#define TS_MAXX 3800  // X do canto inferior direito
#define TS_MAXY 3800  // Y do canto inferior direito
```

### 4. Re-fazer Upload

## Configurações de Varredura

### Timeout de Ping

Controla quanto tempo esperar resposta de cada IP:

```cpp
bool result = Ping.ping(ip, 1);  // 1 tentativa
// ...
if (client.connect(ip, commonPorts[i], 300)) {  // 300ms timeout
```

**Valores sugeridos**:
- Rede rápida/local: 200-300ms
- Rede lenta/WiFi: 500-1000ms
- Rede com problemas: 1500-2000ms

### Portas TCP para Fallback

Se ping ICMP falhar, tenta conexão TCP nessas portas:

```cpp
int commonPorts[] = {80, 22, 23, 443, 445};
```

**Personalize** conforme seus dispositivos:
- Adicione: `8080` (web alternativa)
- Adicione: `3389` (RDP - Windows)
- Adicione: `5900` (VNC)

## Reset para Configuração Padrão

### Método 1: Via Código

Adicione temporariamente no `setup()`:

```cpp
void setup() {
  preferences.begin("network", false);
  preferences.clear();  // Limpa todas as configurações
  preferences.end();
  
  // Resto do código...
}
```

Faça upload, deixe rodar uma vez, depois remova essas linhas.

### Método 2: Via Serial (futuro)

Planeje adicionar um comando:
```cpp
else if (cmd == "reset") {
  preferences.begin("network", false);
  preferences.clear();
  preferences.end();
  ESP.restart();
}
```

## Troubleshooting de Configuração

### ❌ IP Estático não funciona
- ✅ Verifique se IP está na mesma rede do gateway
- ✅ Confirme que IP não está em uso
- ✅ Teste gateway com `ping 192.168.1.1` do PC

### ❌ Não conecta após mudar configuração
- ✅ Volte para DHCP temporariamente
- ✅ Verifique cabo ethernet
- ✅ Confirme que switch/roteador está ligado

### ❌ Configuração não salva
- ✅ Certifique-se de tocar em **SAVE**
- ✅ Aguarde reinício automático
- ✅ Verifique logs no Serial Monitor

### ❌ Touch não responde no menu CONFIG
- ✅ Recalibre touchscreen
- ✅ Verifique conexões físicas
- ✅ Use comandos seriais como alternativa

## Backup de Configuração

As configurações ficam salvas na NVS (Non-Volatile Storage) do ESP32.

Para fazer backup manual:
1. Anote as configurações atuais do Serial Monitor
2. Ou leia via código:

```cpp
void printConfig() {
  preferences.begin("network", true);
  Serial.println("IP: " + preferences.getUInt("staticIP"));
  // ... outros campos
  preferences.end();
}
```

## Configuração para Produção

Para deployar múltiplos dispositivos:

1. Configure um WT32 como "master"
2. Salve configuração ideal
3. Escreva script para flash em lote
4. Ou use PlatformIO com `extra_scripts`

---

📝 **Dica**: Sempre teste configurações no Serial Monitor antes de fazer upload definitivo!