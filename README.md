[English](/README_en.md)

auto_rec_mini
===============

最小機能のTV録画ミドルウェア。  
小規模、軽量、簡易、コマンド操作。  
  
MPEG-2TSやARIBの勉強のため。  
開発中のため環境によっては動作が安定しない可能性があります。


![demo](https://github.com/ysan/auto_rec_mini/blob/master/etc/demo.gif)


Features
------------ 
* 選局
  * 1チューナー、地デジのみとなります。(チューナー：[`KTV-FSUSB2/V3`](http://www.keian.co.jp/products/ktv-fsusb2v3/#spec-table))
* 地デジ チャンネルスキャン
* 録画 及び 録画予約
  * 予約は50件まで。
  * イベント追従。
  * 予約時間が被っている場合は先に録画していたものが終わり次第、次の録画を行います。
  * EPG取得後に指定したキーワードを検索し録画予約を行います。
* EPG
  * 定時EPG取得を行います。
* コマンドラインインターフェース (CLI)
  * `netcat`や`telnet`等で`command server`に接続して、各機能にアクセスするコマンドの実行等を行います。


How to use
------------
### System requirements ###

#### Tuner ####
対象のチューナーは [`KTV-FSUSB2/V3`](http://www.keian.co.jp/products/ktv-fsusb2v3/#spec-table) です。 (S/N: K1212 以降)  
1チューナーで地デジのみとなります。  
（他のチューナーにも対応してきたい。）

#### Platforms ####
一般的なLinuxであれば問題なく動作すると思います。(`Ubuntu`, `Fedora`, `Raspbian`で確認済。)  
  
メインで使用しているのは `Raspbian` Raspberry pi model B ですが、  
パケットロスが起こり、ブロックノイズ、画飛びが起きやすいです。  
~~また選局開始時に電力が足りないせいかtsが取れないケースがありました。~~
  
B-CASカードは別途USB接続のICカードリーダを用意して使用しています。

#### Dependencies ####
下記のライブラリに依存します。  
適宜インストールをお願いします。  
  
*libpcsclite*

	$ sudo apt-get install pcscd libpcsclite-dev libccid pcsc-tools

*libarib25*

	$ sudo apt-get install cmake
	$ git clone https://github.com/stz2012/libarib25
	$ cd libarib25
	$ mkdir build
	$ cd build
	$ cmake ..
	$ make
	$ sudo make install

### Build and install ###
作業ディレクトリにインストールする場合。

	$ mkdir -p ~/work/data
	$ git clone https://github.com/ysan/auto_rec_mini
	$ cd auto_rec_mini
	$ make
	$ make INSTALLDIR=~/work install
	$ cp -p ./data/settings.json ~/work/data
	$ cd ~/work
	$ tree
	.
	├── bin
	│   └── auto_rec_mini
	├── data
	│   └── settings.json
	└── lib
	   	└── auto_rec_mini
	        ├── libchannelmanager.so
	        ├── libcommandserver.so
	        ├── libcommon.so
	        ├── libdsmccparser.so
	        ├── libeventschedulemanager.so
	        ├── libeventsearch.so
	        ├── libit9175.so
	        ├── libparser.so
	        ├── libpsisimanager.so
	        ├── libpsisiparser.so
	        ├── librecmanager.so
	        ├── libthreadmgr.so
	        ├── libthreadmgrpp.so
	        ├── libtunercontrol.so
	        └── libtunethread.so
	
	4 directories, 17 files

### settings.json ###

`settings.json`は設定ファイルです。  
`auto_rec_mini` プロセスの起動時に読み込みます。  
環境に合わせて設定してください。

| item | description |
|:-----|:------------|
| `is_syslog_output` | ログを`syslog`に出力するかどうかを切り替えます。 |
| `command_server_port` | `command server` の待受ポートです。 |
| `channels_json_path` | チャンネルスキャン結果の書き込み/読み込み先パスです。 |
| `rec_reserves_json_path` | 録画予約リストの書き込み/読み込み先パスです。 |
| `rec_results_json_path` | 録画結果リストの書き込み/読み込み先パスです。 |
| `rec_ts_path` | 録画ストリームの保存先パスです。(.m2ts) |
| `dummy_tuner_ts_path` | unused |
| `event_schedule_cache_is_enable` | EPGを有効にするスイッチ。 |
| `event_schedule_cache_start_interval_day` | EPG取得の間隔日。 |
| `event_schedule_cache_start_hour` | EPG取得の開始時間。(何時) |
| `event_schedule_cache_start_min` | EPG取得の開始時間。 (何分) |
| `event_schedule_cache_timeout_min` | EPG取得タイムアウト時間。(分) |
| `event_schedule_cache_histories_json_path` | EPG取得履歴の書き込み/読み込み先パスです。 |
| `event_name_keywords_json_path` | `event name` 検索のキーワードリストの読み込み先パスです。 |
| `extended_event_keywords_json_path` | `extended event` 検索のキーワードリストの読み込み先パスです。 |

#### is_syslog_output ####
`syslog facirity` を `user` に設定することで、ログを `/var/log/user.log` に出力できます。  
以下 `/etc/rsyslog.d/50-default.conf` を編集する必要があります。(`ubuntu16.04`の場合)

	9c9
	< *.*;auth,authpriv.none        -/var/log/syslog
	---
	> *.*;auth,authpriv.none,user.none      -/var/log/syslog
	15c15
	< #user.*               -/var/log/user.log
	---
	> user.*                -/var/log/user.log

#### event_name_keywords_json_path ####

`event_name_keywords_json_path` に下記形式の json を作成することにより    
EPG取得後に番組名にキーワードが含まれる番組を検索して録画予約を入れます。  
キーワードの数に制限はありません。

	{
	    "m_event_name_keywords": [
	        "ＸＸＸニュース",
	        "ＸＸＸスポーツ",
	        "ＸＸＸ報道"
	    ]
	}

#### extended_event_keywords_json_path ####

`extended_event_keywords_json_path` に下記形式の json を作成することにより  
EPG取得後に番組詳細情報にキーワードが含まれる番組を検索して録画予約を入れます。  
キーワードの数に制限はありません。

	{
	    "m_extended_event_keywords": [
	        "ＸＸＸワールドカップ",
	        "ＸＸＸオリンピック",
	        "ＸＸＸ野球"
	    ]
	}

### How to run ###

`Build and install`でインストールした作業ディレクトリで実行する場合。  
（※実行するにはrootが必要です。）

	$ cd ~/work
	$ export LD_LIBRARY_PATH=./lib/auto_rec_mini
	$ sudo ./bin/auto_rec_mini ./data/settings.json &

以上で `auto_rec_mini` は起動して、チューナーを使用する準備ができた状態になります。  
`command server` はポート20001で接続を待ち受けています。  
最初に下記コマンドからチャンネルスキャンの実施お願いします。  

### How to use CLI ###

#### Connect to command server ####
端末から`netcat` や `telnet` で接続できます。  
`localhost` でも外部アドレスからでも接続可能です。  
また `command server` はシングルクライアントとなります。  

	$ nc -C localhost 20001
	
	###  command line  begin. ###
	
	  ------ root tables ------
	  s                    -- system commands               
	  tc                   -- tuner control                 
	  pm                   -- psisi manager                 
	  rec                  -- rec manager                   
	  cm                   -- channel manager               
	  em                   -- event schedule manager        
	  es                   -- event search                  
	
	/ >
	/ >

#### channel scan ####

	/ > cm
	
	  ------ channel manager ------
	  scan                 -- channel scan                  
	  tr                   -- tune by remote_control_key_id (usage: tr {remote_control_key_id} )
	  d                    -- dump channels
	
	/channel manager > 
	/channel manager > scan

以上でチャンネルスキャンが開始します。


Future tasks
------------
* 現状は録画、EPG等のチューナーを使う機能間の排他ができてない。その実装を対応する。
* データ放送のBMLの取得を行う。 (dsmccの実装)
* libarib25で行っているPMT周りの実装の勉強、と実装を取り込みたい。
* 複数チューナー、BS/CS対応したい。


Component diagram
------------
![component diagram](https://github.com/ysan/auto_rec_mini/blob/master/etc/component_diagram.png)


Others
------------
設定値などの静的データの読み込み/書き込みは [`cereal`](https://github.com/USCiLab/cereal) のjsonシリアライザを使用しています。  
(現状DBは使用していません。)


パーサー周りは以下のレポジトリ様を参考にさせていただいています:
* [`libarib25`](https://github.com/stz2012/libarib25)
* [`epgdump`](https://github.com/Piro77/epgdump)
* [`ariblib`](https://github.com/youzaka/ariblib)
* [`eit_txtout_mod`](https://github.com/arairait/eit_txtout_mod)
 
流用させていただいているもの:
* aribstr of [`epgdump`](https://github.com/Piro77/epgdump)
* [`recfsusb2i`](https://github.com/jeeb/recfsusb2i) (tuner<->USB control)

LICENSE
------------
流用しているコードがGPLとなっていますので、それに従っています。

