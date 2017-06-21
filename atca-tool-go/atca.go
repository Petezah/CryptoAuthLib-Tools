package main

import (
	"encoding/json"
	"flag"
	"io/ioutil"
	"os"
	"strings"

	"./atca"

	"github.com/cesanta/errors"
	yaml "gopkg.in/yaml.v2"
)

var (
	filename = flag.String("filename", "", "Filename to read or write")
	format   = flag.String("format", "", "File format: json, yaml or hex")
)

func getFormat(f, fn string) string {
	f = strings.ToLower(f)
	if f == "" {
		fn := strings.ToLower(fn)
		if strings.HasSuffix(fn, ".yaml") || strings.HasSuffix(fn, ".yml") {
			f = "yaml"
		} else if strings.HasSuffix(strings.ToLower(fn), ".json") {
			f = "json"
		} else {
			f = "hex"
		}
	}
	return f
}

func atcaGetConfig(confData []byte) error {
	fn := *filename
	format := *format
	f := getFormat(format, fn)

	cfg, err := atca.ParseBinaryConfig(confData)

	var s []byte
	if f == "json" || f == "yaml" {
		if f == "json" {
			s, _ = json.MarshalIndent(cfg, "", "  ")
		} else {
			s, _ = yaml.Marshal(cfg)
		}
	} else if f == "hex" {
		s = atca.WriteHex(confData, 4)
	} else {
		return errors.Errorf("%s: format not specified and could not be guessed", fn)
	}

	if fn != "" {
		err = ioutil.WriteFile(fn, s, 0644)
		if err != nil {
			return errors.Trace(err)
		}
	} else {
		os.Stdout.Write(s)
	}

	return nil
}

func atcaSetConfig(confData []byte) error {
	fn := *filename
	format := *format
	data, err := ioutil.ReadFile(fn)
	if err != nil {
		return errors.Trace(err)
	}

	f := getFormat(format, fn)

	if f == "yaml" || f == "json" {
		var c atca.Config
		if f == "yaml" {
			err = yaml.Unmarshal(data, &c)
		} else {
			err = json.Unmarshal(data, &c)
		}
		if err != nil {
			return errors.Annotatef(err, "failed to decode %s as %s", fn, f)
		}

		confData, err = atca.WriteBinaryConfig(&c)
		if err != nil {
			return errors.Annotatef(err, "encode %s", fn)
		}
	} else if f == "hex" {
		confData = atca.ReadHex(data)
	} else {
		return errors.Errorf("%s: format not specified and could not be guessed", fn)
	}

	if len(confData) != atca.ConfigSize {
		return errors.Errorf("%s: expected %d bytes, got %d", fn, atca.ConfigSize, len(confData))
	}

	return nil
}

var (
	testConfigGolden = [128]byte{
		0x01, 0x23, 0x52, 0xaa,
		0x00, 0x00, 0x50, 0x00,
		0xd1, 0xbb, 0xf3, 0x78,
		0xee, 0xc0, 0x01, 0x00,
		0xc0, 0x00, 0x55, 0x00,
		0x87, 0x64, 0x87, 0x64,
		0x87, 0x64, 0x87, 0x64,
		0x80, 0x0f, 0x8f, 0x8f,
		0x9f, 0x8f, 0x82, 0x20,
		0xc4, 0x44, 0xc4, 0x44,
		0x0f, 0x0f, 0x0f, 0x0f,
		0x0f, 0x0f, 0x0f, 0x0f,
		0x0f, 0x0f, 0x0f, 0x0f,
		0xff, 0xff, 0xff, 0xff,
		0x00, 0x00, 0x00, 0x00,
		0xff, 0xff, 0xff, 0xff,
		0x00, 0x00, 0x00, 0x00,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff,
		0x00, 0x00, 0x00, 0x00,
		0xff, 0xff, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x33, 0x00, 0x33, 0x00,
		0x33, 0x00, 0x33, 0x00,
		0x3c, 0x00, 0x1c, 0x00,
		0x1c, 0x00, 0x33, 0x00,
		0x1c, 0x00, 0x1c, 0x00,
		0x3c, 0x00, 0x3c, 0x00,
		0x3c, 0x00, 0x3c, 0x00,
		0x1c, 0x00, 0x3c, 0x00,
	}
)

func main() {
	flag.Parse()

	config := testConfigGolden
	atcaGetConfig(config[:])
}
