#include <XBee.h>

#define DIALPIN A3
#define INTERVAL 500 //待機時間
#define TERM 15 //ダイヤルの値がが同じ場合の待機期間
#define WAITMODE 5 //待機モードまでのPOSTの再試行回数
#define DEFMIN 0 //ダイヤルの初期最小値
#define DEFMAX 1023 //ダイヤルの初期最大値
#define DIALMIN 1 //ダイヤルの最小値
#define DIALMAX 6 //ダイヤルの最大値
#define XBEE_SLEEP_PIN 2

XBee xbee = XBee();

uint8_t payload[8]; //XBee送信用にデータを変換したものを保存するための配列
//親機へ送信
XBeeAddress64 addr64 = XBeeAddress64(0, 0);
//XBeeへのリクエストを作成
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));

//初期起動時の処理（基本的に変更はしない）---------------------------------------------------
void setup() {
  Serial.begin(9600);
  xbee.setSerial(Serial);
  pinMode(DIALPIN, INPUT);
  pinMode(XBEE_SLEEP_PIN, OUTPUT);
  digitalWrite(XBEE_SLEEP_PIN, HIGH); //XBEEスリープ解除
}

int before_dials = 0; //一つ前のダイヤルの値
int timecounter = 0; //値が同じ場合で再度送信するまでのカウント
int waitcounter = 0; //待機までのカウント
int dials;

void loop() {
  //  Serial.println(analogRead(DIALPIN)); //始めにダイヤルの初期値の最小・最大値を調べて、ぞれぞれをDEFMIN、DEFMAXの値に入れる
  dials = map(analogRead(DIALPIN), DEFMIN, DEFMAX, DIALMIN, DIALMAX); //ダイヤル（可変抵抗器）の値を指定した段階に変換

  if (dials != before_dials || timecounter > TERM) { //ダイヤルの値が同じ場合はすぐにPOSTをしないで、一定期間後に指定回数だけ再度POSTをする

    sending(); //XBee送信関連の関数

    if (dials == before_dials && timecounter == 0) { //ダイヤルをずっと回していないと、待機モードに入るまでのカウントダウンを行う
      Serial.print("残り試行回数: "); Serial.println(WAITMODE - waitcounter);
      if (WAITMODE - waitcounter == 0) {
        digitalWrite(XBEE_SLEEP_PIN, LOW); //XBEEスリープ
        Serial.println("待機モードに入ります。再開するにはダイヤルをまわして下さい。");
      }
      waitcounter += 1;
    } else if (dials != before_dials) {
      waitcounter = 0;
    }
  }

  if (waitcounter > WAITMODE) { //待機モード
    while (dials == before_dials) { //ダイヤルを回すまで待機モード
      dials = map(analogRead(DIALPIN), DEFMIN, DEFMAX, DIALMIN, DIALMAX); //ダイヤル（可変抵抗器）の値を指定した段階に変換
      Serial.println("待機中");
      delay(INTERVAL);
    }
    waitcounter = 0;
    Serial.println("送信を再開します。");
    digitalWrite(XBEE_SLEEP_PIN, HIGH); //XBEEスリープ解除
    delay(50);//起動待ち
  }

  before_dials = dials;
  timecounter += 1;
  delay(INTERVAL);
}

//XBee送信関連の関数---------------------------------------------------
void sending() {
  timecounter = 0; //インターバルを解除してPOSTの開始
  Serial.println(dials);
  set_float_to_payload(dials, 0); //XBee送信用にデータを変換する関数
  xbee.send(zbTx);
}

//XBee送信用にデータを変換する関数---------------------------------------------------
void set_float_to_payload(float value, int index) {
  uint8_t *value_array;
  value_array = reinterpret_cast<uint8_t*>(&value);
  for (int i = 0; i < sizeof(value); i++) {
    payload[i + index] = value_array[i];
  }
}
