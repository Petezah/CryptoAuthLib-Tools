package atca

import (
	"encoding/hex"
	"encoding/json"
	"fmt"
	"regexp"
)

const (
	KeyFilePrefix = "ATCA:"
)

func WriteHex(data []byte, numPerLine int) []byte {
	s := ""
	for i := 0; i < len(data); {
		for j := 0; j < numPerLine && i < len(data); j++ {
			comma := ""
			if i < len(data)-1 {
				comma = ", "
			}
			s += fmt.Sprintf("0x%02x%s", data[i], comma)
			i++
		}
		s += "\n"
	}
	return []byte(s)
}

func ReadHex(data []byte) []byte {
	var result []byte
	hexByteRegex := regexp.MustCompile(`[0-9a-fA-F]{2}`)
	for _, match := range hexByteRegex.FindAllString(string(data), -1) {
		b, _ := hex.DecodeString(match)
		result = append(result, b[0])
	}
	return result
}

func JSONStr(v interface{}) string {
	bb, _ := json.MarshalIndent(v, "", "  ")
	return string(bb)
}
