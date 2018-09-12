#include <WioLTEforArduino.h>
#include <stdio.h>

#define DIALPIN  (WIOLTE_A4)
#define APN               "iijmio.jp"
#define USERNAME          "mio@iij"
#define PASSWORD          "iij"

#define BASEURL       "http://52.197.122.211/dices"

#define INTERVAL          (500) //待機時間
#define TERM 10 //ダイヤルの値がが同じ場合の待機期間
#define WAITMODE 5 //待機モードまでのPOSTの再試行回数
#define DIALMIN 1 //ダイヤルの最小値
#define DIALMAX 6 //ダイヤルの最大値

WioLTE Wio;

//初期起動時の処理（基本的に変更はしない）---------------------------------------------------
void setup() {
  delay(200);

  SerialUSB.println("");
  SerialUSB.println("--- START ---------------------------------------------------");

  SerialUSB.println("初期化中");
  Wio.Init();

  SerialUSB.println("###電源供給開始");
  Wio.PowerSupplyLTE(true);
  Wio.PowerSupplyGrove(true);
  delay(1000);

  SerialUSB.println("###電源投入");
  if (!Wio.TurnOnOrReset()) {
    SerialUSB.println("### エラー! ###");
    return;
  }

  SerialUSB.println("###ダイヤルピンをアクティブ");
  pinMode(DIALPIN, INPUT_ANALOG);

  SerialUSB.println("###\""APN"\"へ接続中...");
  delay(5000);
  if (!Wio.Activate(APN, USERNAME, PASSWORD)) {
    SerialUSB.println("### エラー!モバイルネットワーク回線への接続に失敗しました。APN設定を再度確認して、やり直して下さい。 ###");
    return;
  }
}

int before_dials = 0; //一つ前のダイヤルの値
int timecounter = 0; //値が同じ場合で再度送信するまでのカウント
int waitcounter = 0; //待機までのカウント
int dials; //ダイヤルの値

//ループ処理---------------------------------------------------
void loop() {
  SerialUSB.println(analogRead(DIALPIN));
  dials = map(analogRead(DIALPIN), 1, 4085, DIALMIN, DIALMAX); //ダイヤル（可変抵抗器）の値を指定した段階に変換

  SerialUSB.println(millis() / 1000);

  if (dials != before_dials || timecounter > TERM ) { //ダイヤルの値が同じ場合はすぐにPOSTをしないで、一定期間後に指定回数だけ再度POSTをする
    
    sending(); //POST関連の関数
    
    if (dials == before_dials && timecounter == 0) { //ダイヤルをずっと回していないと、待機モードに入るまでのカウントダウンを行う
      SerialUSB.print("残り試行回数: "); SerialUSB.println(WAITMODE - waitcounter);
      if (WAITMODE - waitcounter == 0) {
        SerialUSB.println("待機モードに入ります。再開するにはダイヤルをまわして下さい。");
      }
      waitcounter += 1;
    } else if (dials != before_dials) { //ダイヤルを回した時に待機モードから脱ける
      waitcounter = 0;
    }
  }
  if (waitcounter > WAITMODE) { //待機モード
    timecounter = 0;
    SerialUSB.println("待機中");
  }
  before_dials = dials;
  timecounter += 1;
  delay(INTERVAL); //指定した間隔でインターバル
}

//POST関連の関数---------------------------------------------------
void sending() {
  timecounter = 0; //インターバルを解除してPOSTの開始
  char num[20]; //SIMの電話番号を一時保存するための配列
  int result = Wio.GetPhoneNumber(num, 20); //電話番号を取得
  char data[100]; //POSTするデータを保存するための配列
  int status; //POSTした時のレスポンス値を保存するための変数

  SerialUSB.println("### POST中");
  sprintf(data, "{\"dice_id\":\"%s\",\"angle\":\"%d\"}", num, dials); //data配列にPOSTするデータ（電話番号、ダイヤル値）を格納
  SerialUSB.print("Post:");
  SerialUSB.print(data);
  SerialUSB.println("");
  if (!Wio.HttpPost(BASEURL, data, & status)) { //POSTを送信
    SerialUSB.println("### エラー! ###"); //以下POST失敗時の処理
    goto err;
  }
  SerialUSB.print("Status:");
  SerialUSB.println(status);

err:
  SerialUSB.println("### インターバル中");
}

