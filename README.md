#### 编译环境：vscode + esp-idf拓展

#### 使用8bit模式，需短接屏幕R2、R4（把R3那个0Ω电阻挪到R2）

### 参考接线（可在vscode下部小齿轮中配置）：

Compilation environment: vscode + esp-idf extension

Using 8bit mode, you need to short-connect the screen R2 and R4 (move the 0Ω resistance of R3 to R2).

Reference wiring (configurable in the lower pinion of vscode):

| 屏幕 | ESP32 |
| ---- | ---- |
|  D0  |  23  |
|  D1  |  22  |
|  D2  |  21  |
|  D3  |  19  |
|  D4  |  18  |
|  D5  |  5   |
|  D6  |  17  |
|  D7  |  16  |
|  WR  |  2   |
|  RS  |  4   |
|  RST |  15  |
|  CS  |  GND |
|  RD  |  3V3 |
|  BL  |  3V3 |
|  VCC |  3V3 |
|  GND |  GND |
| MISO |  12  |
| MOSI |  13  |
| T_CS |  27  |
|  CLK |  14  |
|  PEN |  26  |
