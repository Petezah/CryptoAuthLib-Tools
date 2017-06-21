atca-tool-go
============

A tool to read and write ATCA config data

This tool derives from a chunk of code in the "mos" tool found [here](https://github.com/cesanta/mongoose-os/tree/master/mos).  I extracted it to help validate configuration data using a known working parser.

## Building

All vendor packages have been stripped from this implementation.  You should just be able to build this tool with:

```
$ go build
```

You may additionally need to do this prior to building:

```
$ go get
```
