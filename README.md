## coinC

> 💰 Cryptocurrency price ticker CLI.

Check cryptocurrencies' prices, changes on your console.
Best CLI tool for those who are both **Crypto investors** and **Engineers**.

All data comes from [coincap](https://coincap.io/) APIs.

## Quick Update
* As coinmarketcap API is only free with API keys, we moved to use coincap API and disabled some features.

## Install

It's simple to compile `coinC` on your local computer.  
The following is step-by-step instruction.

```
$ git clone https://github.com/dimartz/coinc.git
$ cd coinc
$ make
$ make clean install
```

## Usage

To check the top 10 cryptocurrencies ranked by their market cap, simply enter
```
$ coinc
```

## Options

### Find specific coin(s)

You can use the `-f` (or `--find`) with coin symbol to search cryptocurrencies. You can add symbols seperated by comma.

```
$ coinc -f btc // search coins included keyword btc
$ coinc -f btc,eth // search coins included keyword btc or eth
```

### Find top coin(s)

You can use the `-t` (or `--top`) with the index to find the top n cryptocurrencies ranked by their market cap.

```
$ coinc -t 50 // find top 50
```

### Show option menu

You can use the `-h` (or `--help`) to find all valid options of coinC

```
$ coinc -h
```

## Screenshot

<img src="https://raw.githubusercontent.com/bichenkk/coinmon/master/screenshot.png">

## License

MIT
