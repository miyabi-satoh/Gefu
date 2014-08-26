Gefu
====

Gefu is an Experimental File Utility.

### TODO
  * オプション
    * ファンクションキー
    * キーバインドカスタマイズ
  * 簡易画像ビューア
  * 簡易アーカイバ
  * 簡易FTPクライアント
  * SNS連携(タイムライン表示とか) ← ファイラーの機能か？

#### 2014/08/26 Ver0.13 公開
  * 拡張子・サイズ・更新日時の列幅をフォントサイズに応じて調整するようにした。
  * 「最新バージョンをチェック」を「アップデートの確認」に文言変更。
    合わせて、ショートカットキーを「V」から「Shift+Z」に変更。
  * テキストビューアのコンテキストメニューに「選択範囲をクリップボードにコピー」を追加。
  * テキストビューアについても、メニューまたはキーボードで文字の大きさを変更できるようにした。
  * テキストビューアについても、カーソル移動系アクションを有効にした。
  * 不具合修正
    * 環境設定ダイアログで、「起動時のアップデート確認」の設定表示・変更ができていなかった。

#### 2014/08/26 Ver0.12 公開
  * コンテキストメニューを実装。
  * 拡張子を分離して表示するようにした。
  * ファイルビューの状態で文字の大きさを変更できるようにした。
  * GUIまわり(メニュー、ツールバー、ダイアログ)を調整。
  * テキストビューアの外観設定が反映されていなかったのを修正。

#### 2014/08/25 Ver0.11 公開
  * ファイル検索機能を実装。
  * 内部構造を大幅に変更。
    各ビューで処理していたアクションを、メインウィンドウで処理するように変更。
  * 起動時の位置とサイズが常にデフォルト値になっていたのを修正。
  * バージョン情報ダイアログのリンク先をSourceForgeに変更。

#### 2014/08/24 Ver0.10 公開
  * 環境設定ダイアログの変更
    * コントロールの活性・非活性切り替えを修正。
    * フォント選択をダイアログ使用に変更。
    * ファイルビューのサンプル表示を実際の形に変更。
    * テキストビューアの設定を追加。
  * 安直な文字コード判別を実装
    * BOMがあれば、それに従う。
    * 先頭1KB中に文字コードを示す文字列("shift_jis"等)があれば、その文字コードに変換する。
    * バイト単位で調べて推測(http://dobon.net/vb/dotnet/string/detectcode.html よりコードを拝借)

#### 2014/08/24 Ver0.09 公開
  * 左右矢印キーに機能割り当て
    * 左ペインで左矢印／右ペインで右矢印 → 親フォルダに移動
    * 左ペインで右矢印／右ペインで左矢印 → 反対側のペインにフォーカス
  * ファイル名をクリップボードにコピーを実装。
  * フォルダ名をクリップボードにコピーを実装。
  * 簡易テキストビューアをとりあえず実装。→ 次バージョンでオプション設定等
  * 不具合修正
    * Windows版で初回起動時、タイトルバーが画面外に出ていた。→ デフォルト位置・サイズ変更。

#### 2014/08/23 Ver0.08 公開
  * 「ターミナルで開く」を実装。
  * Shift+ダブルクリックで、ファイルを外部エディタで開くようにした。
  * Ctrl+ダブルクリックで、フォルダをターミナルで開くようにした。
  * 「色とフォント」にインポート・エクスポート機能を追加。
  * マスク表示(フィルタ)機能を追加。
  * 最新バージョンのチェック機能を追加。
  * 不具合修正
    * Windows版でドライブ直下のフォルダを開くと落ちるのを修正。

#### 2014/08/23 Ver0.07 公開
  * ダブルクリック時、ファイルは標準の関連付けで実行するようにした。
  * 設定項目追加。
  * 「外部エディタで開く」を実装。
  * 不具合修正
    * 初期表示時にカーソルがハイライトされない問題を修正。

#### 2014/08/22 Ver0.06 公開
  * 外部および内部のドラッグ＆ドロップ処理を実装。
  * 一部ダイアログのレイアウト、初期サイズを調整。
  * 不具合修正
    * 同一ファイルに対する強制上書きが考慮されていなかった。⇛ 先・元が同一の場合はスキップするようにした。

#### 2014/08/22 Ver0.05 公開
  * 外部アプリケーションからのドロップ処理を実装。

#### 2014/08/22 Ver0.04 公開
  * 不具合修正
    * ソート順でフォルダ位置が先頭以外の場合、".."もソートされていたのを修正。(ソート方法によらず、".."は必ず先頭にした)
    * 各ダイアログのフォーカス周りを修正。
    * is "an" Experimental ... とか、バージョン情報ダイアログの修正。
    * MacのDockにアイコンが正しく表示されていなかったのを修正。
    * フォルダ内容変更による再読込時、カーソル位置が初期化されていたのを修正。
  * 「最新の情報に更新」を追加。

#### 2014/08/21 Ver0.03 公開
  * 環境設定ダイアログおよびオプションを実装。
    * 終了時の確認ダイアログ。
    * 起動時のウィンドウ位置・サイズ。
    * 起動時に設定をリセット。
    * 色とフォントの設定。

#### 2014/08/20 Ver0.02 公開
  * QTableWidget -> QTableView + Modelへ変更し高速化。
  * フォルダ履歴機能を実装。
  * 行の高さを調整。
  * マーク時の背景色、前景色を変更。
  * ウィンドウの位置・サイズを保存するようにした。

#### 2014/08/18 Ver0.01 公開
  * 名前変更ダイアログ(単一)のレイアウトを修正。
  * すべて選択等で、カーソル位置が変わらないようにした。
  * マークしているフォルダ数、ファイル数、合計ファイルサイズの表示を追加。
  * ファイルサイズを小数点第一位まで表示するようにした。
  * システム属性ファイルの表示/非表示機能を追加。
  * 名前ソートで大文字小文字を区別しないようにした。
  * ソート方法の選択機能を追加。

#### 2014/08/18 Ver0.00 公開
  * 新規。
