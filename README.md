# TURIPlibrariesForArduino
[![Build Status](https://travis-ci.org/turippj/TURIPlibrariesForArduino.svg?branch=master)](https://travis-ci.org/turippj/TURIPlibrariesForArduino)

TURIPをArduino上で利用するためのライブラリです。

TURIPにはServerとClientが存在します。
組み込み分野ではServerのことをSlaveと、ClientのことをMasterと呼ぶ場合がありますが、TURIPでは先述の表記に統一しています。
ライブラリは、ServerとClientそれぞれ、2層構造になっています。
例えば、Serverを構築するには、TURIPserverライブラリと、TURIPserver[インターフェイス名]ライブラリの両方を必要とします。
