# 音響オブジェクト(拡張編集フィルタプラグイン)
- [ダウンロードはこちら](../../releases/)
- ReverberationObject.eefをpluginsフォルダに配置してください

- このプラグインを読み込むためにはpatch.aul r43_ss_58以降が必要です https://scrapbox.io/nazosauna/patch.aul

## 役割
- セリフやBGMなどの音声オブジェクトに音声ディレイなどを付与した場合に、元のファイルの長さに従ってぶつ切りになり、後ろにあるはずのディレイ音のみを鳴らすことが出来ないため、音響オブジェクトを置くことでそれを実現する

![image](https://github.com/nazonoSAUNA/ReverberationObject.eef/assets/99536641/43de558e-7bf3-4117-ab01-23c7067e98cb)

## パラメータ
- 前レイヤー：0の場合は左に隣り合っている音声に対してのみ有効

0の例：
![image](https://github.com/nazonoSAUNA/ReverberationObject.eef/assets/99536641/46add200-59f1-48a7-93d8-2bd0239e1397)

-2の例：
![image](https://github.com/nazonoSAUNA/ReverberationObject.eef/assets/99536641/592efca7-e82b-41f8-aff9-6c52521f7419)


- 対象数：前レイヤーがｰ1以下の場合のみ有効。音響を読み込む対象数。増やすと負荷が大きくなります

例として-2に設定されている場合、次の画像の赤線の位置では1と2のオブジェクトの音響を読み込むことになる

![image](https://github.com/nazonoSAUNA/ReverberationObject.eef/assets/99536641/160c4218-954b-4d54-8fb0-4d5d98f782a3)

※音響オブジェクトに掛かる**フィルタ効果や標準再生**(音量・左右トラック)**は全て無効**です。

# 開発者向け
- aviutl_exedit_sdk：ほぼhttps://github.com/nazonoSAUNA/aviutl_exedit_sdk/tree/command を使用しています
