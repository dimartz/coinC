# coinC

> 💰 Cryptocurrency price ticker CLI.

Check cryptocurrencies' prices, changes on your console.
Best CLI tool for those who are both **Crypto investors** and **Engineers**.

All data comes from [coincap](https://coincap.io/) APIs.

## Install

The following is step-by-step instruction.

```
$ git clone https://github.com/dimartz/coinC.git
$ cd coinC
$ make
$ make clean install
```

## Uninstall

In the previously `coinC` folder:
```
$ make uninstall
```

## Usage

To check the top 10 cryptocurrencies ranked by their market cap, simply enter
```
$ coinc
```

![screenshot](https://user-images.githubusercontent.com/98893034/159277641-a53f7599-93cd-4e0f-a110-6f266711ab43.png)

## Improvements

### Faster

https://user-images.githubusercontent.com/98893034/159282406-3049b567-f344-4ece-a622-38b0c81df1fe.mp4

### No error

`coinC` doesn't return an error on first failed call to coincap.

https://user-images.githubusercontent.com/98893034/159284738-d57504be-5c2b-49d7-ade9-ebf8016c996a.mp4

## Options

### Find specific coin(s)

You can use the `-f` (or `--find`) with coin symbol to search cryptocurrencies. You can add symbols seperated by comma.

```
$ coinc -f btc      // search coins included keyword btc
$ coinc -f btc,eth  // search coins included keyword btc or eth
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

## License

MIT
