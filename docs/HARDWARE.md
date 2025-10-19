# 🔌 Guia de Hardware - WT32 IP Scanner

## Componentes Necessários

### 1. WT32-ETH01
- **Processador**: ESP32-WROOM-32
- **Ethernet**: LAN8720A PHY
- **Memória Flash**: 4MB
- **GPIO Disponíveis**: Vários pinos livres

**Onde comprar**:
- AliExpress
- Amazon
- Lojas especializadas em eletrônica

### 2. Display ILI9341 2.8"
- **Resolução**: 320x240 pixels
- **Interface**: SPI
- **Touchscreen**: XPT2046 (resistivo)
- **Tensão**: 3.3V/5V

**Características**:
- Controlador: ILI9341
- 262K cores
- Backlight LED ajustável

### 3. Fonte de Alimentação
- **Tensão**: 5V DC
- **Corrente**: Mínimo 2A (recomendado 3A)
- **Conector**: Micro USB ou P4 (depende do módulo)

## Esquemático de Conexões

```
┌─────────────────┐
│   WT32-ETH01    │
│                 │
│  GPIO33 ────────┼───► TFT_CS
│  GPIO15 ────────┼───► TFT_DC
│  GPIO32 ────────┼───► TFT_RST
│  GPIO14 ────────┼───► TFT_MOSI (SDA)
│  GPIO12 ────────┼───► TFT_SCLK (SCK)
│                 │
│  GPIO4  ────────┼───► TOUCH_CS
│  GPIO35 ────────┼───► TOUCH_IRQ
│                 │
│  3.3V   ────────┼───► VCC (Display)
│  GND    ────────┼───► GND (Display)
│                 │
│  [Ethernet RJ45]│
└─────────────────┘
```

## Pinagem Detalhada

### WT32-ETH01 → ILI9341 Display

| WT32-ETH01 | ILI9341 | Função |
|------------|---------|--------|
| GPIO 33 | CS | Chip Select (SPI) |
| GPIO 15 | DC/RS | Data/Command |
| GPIO 32 | RST/RESET | Reset |
| GPIO 14 | MOSI/SDI | Master Out Slave In |
| GPIO 12 | SCK | Serial Clock |
| 3.3V | VCC | Alimentação |
| GND | GND | Terra |
| (LED) | LED+ | Backlight + (opcional) |
| GND | LED- | Backlight - (opcional) |

### WT32-ETH01 → XPT2046 Touch

| WT32-ETH01 | XPT2046 | Função |
|------------|---------|--------|
| GPIO 4 | T_CS | Touch Chip Select |
| GPIO 35 | T_IRQ | Touch Interrupt |
| GPIO 14 | T_DIN | Touch Data In (compartilhado com display) |
| GPIO 12 | T_CLK | Touch Clock (compartilhado com display) |
| 3.3V | VCC | Alimentação |
| GND | GND | Terra |

**⚠️ Importante**: Touch compartilha MOSI e SCK com o display!

### Ethernet (Built-in no WT32-ETH01)

O WT32-ETH01 já possui Ethernet integrada. Basta conectar o cabo RJ45.

| Pino Interno | Função |
|--------------|--------|
| GPIO 23 | MDC |
| GPIO 18 | MDIO |
| GPIO 16 | PHY Power |
| GPIO 0 | Clock Input |

## Montagem Física

### Opção 1: Protoboard (Teste)

```
[WT32-ETH01] ─── Jumpers ─── [Display ILI9341]
      │
      └─────── Cabo RJ45 ────► [Switch/Router]
```

**Passos**:
1. Monte o WT32 na protoboard
2. Conecte os jumpers conforme pinagem acima
3. Conecte fonte 5V no WT32
4. Conecte cabo ethernet

### Opção 2: PCB Personalizada (Produção)

Considere desenvolver uma PCB que integre:
- Conector para WT32-ETH01
- Flat cable para display
- Regulador 3.3V dedicado
- Proteção ESD
- Botão de reset físico

## Considerações Importantes

### 1. Níveis de Tensão
- WT32-ETH01 opera em **3.3V** nos GPIO
- ILI9341 geralmente aceita 3.3V ou 5V
- **Verifique datasheet do seu display!**

### 2. Corrente
- Display com backlight consome ~100-150mA
- WT32-ETH01 consome ~200mA (picos maiores)
- **Total**: ~500mA mínimo, **2A recomendado**

### 3. Cabos
- Use cabos curtos (<20cm) para SPI
- Cabos longos causam interferência
- Para produção, use flat cables

### 4. Alimentação
```
Fonte 5V ──► Regulador 3.3V ──► WT32-ETH01
                    │
                    └──────────► Display (se necessário)
```

## Lista de Compras Sugerida

| Item | Quantidade | Preço Estimado (R$) |
|------|-----------|---------------------|
| WT32-ETH01 | 1 | 60-100 |
| Display ILI9341 2.8" | 1 | 40-70 |
| Fonte 5V 2A | 1 | 15-30 |
| Jumpers | 10-15 | 5-10 |
| Cabo Ethernet | 1 | 5-15 |
| **Total** | - | **~125-225** |

## Troubleshooting de Hardware

### Display branco/preto
- ✅ Verifique alimentação 3.3V/5V
- ✅ Verifique conexão CS, DC, RST
- ✅ Teste com exemplo básico da biblioteca

### Touch não responde
- ✅ Verifique T_CS e T_IRQ
- ✅ Confirme compartilhamento correto de MOSI/SCK
- ✅ Calibre coordenadas no código

### Ethernet não conecta
- ✅ Teste com cabo conhecido bom
- ✅ Verifique LED do RJ45 (deve piscar)
- ✅ Teste outro switch/porta

### Reset aleatório
- ✅ Fonte insuficiente - use 2A ou mais
- ✅ Cabos USB ruins
- ✅ Watchdog - ajuste timeout no código

## Próximos Passos

Após montar o hardware:
1. Teste cada componente individualmente
2. Rode exemplos básicos (Blink, Display Test, Touch Test)
3. Faça upload do código principal
4. Configure via Serial Monitor

---

📐 **Esquemático em PDF**: _(adicione aqui se criar)_

🔧 **PCB Gerber Files**: _(adicione aqui se desenvolver)_