# GPS_LoRa_tracker

first prototype version.

Sender (On-board) <br>
Wemons D1 mini (ESP8266EX) <br>
GPS module <br>
LoRa E220 <br>

Receiver (Field) <br>
Wemos mini (ESP32) for use Wifi and/or BLE <br>
Lora E220 <br>
OLED SSD1106  <br>

OLED SSD1306 

「フォントのサイズ」と「表示開始位置」の関係
「フォントのサイズ」に「1」を指定した場合、1つの文字の高さは「8」になります。
ある文字の真下に、重なることなく別の文字を表示する場合、setCursor()の第2引数には「現在のカーソル位置」に「8」以上の値を加えた値を設定する必要があります。同様に、1つの文字の幅は「6」になります。

fontsize(1)		幅6  x  高さ8			21 char
fontsize(2)		幅12 x 高さ16			10 char

