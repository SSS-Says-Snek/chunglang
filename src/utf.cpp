#include "chung/utf.hpp"

#include <cstdint>

// def u32tostring(c):
// 	code_points = []
// 	if c <= 0x7F:
// 		code_points.append(c)
// 	elif c <= 0x07FF:
// 		code_points.append(0b11000000 | (c >> 6))
// 		code_points.append(0b10000000 | (c & 0b111111))
// 	elif c <= 0xFFFF:
// 		code_points.append(0b11100000 | (c >> 12))
// 		code_points.append(0b10000000 | ((c >> 6) & 0b111111))
// 		code_points.append(0b10000000 | (c & 0b111111))
// 	elif c <= 0x10FFFF:
// 		code_points.append(0b11110000 | (c >> 18))
// 		code_points.append(0b10000000 | ((c >> 12) & 0b111111))
// 		code_points.append(0b10000000 | ((c >> 6) & 0b111111))
// 		code_points.append(0b10000000 | (c & 0b111111))
// 	return code_points

// https://en.wikipedia.org/wiki/UTF-8#Encoding
std::string u32tostring(const std::u32string& str) {
    const char mask = 0b111111;
    std::string out;

    for (char32_t u32char: str) {
        if (u32char <= 0x7F) {
            out += u32char;
        } else if (u32char <= 0x7FF) {
            out += 0b11000000 | (u32char >> 6);
            out += 0b10000000 | (u32char & mask);
        } else if (u32char <= 0xFFFF) {
            out += 0b11100000 | (u32char >> 12);
            out += 0b10000000 | ((u32char >> 6) & mask);
            out += 0b10000000 | (u32char & mask);
        } else if (u32char <= 0x10FFFF) {
            out += 0b11110000 | (u32char >> 18);
            out += 0b10000000 | ((u32char >> 12) & mask);
            out += 0b10000000 | ((u32char >> 6) & mask);
            out += 0b10000000 | (u32char & mask);
        }
    }

    return out;
}

// def stringtou32(code_points):
// 	c = 0
// 	if code_points[0] & 0b11110000 >= 0b11110000:
// 		c = (code_points[0] & 0b1111) << 18
// 		c |= (code_points[1] & 0b111111) << 12
// 		c |= (code_points[2] & 0b111111) << 6
// 		c |= (code_points[3] & 0b111111)
// 	elif code_points[0] & 0b11100000 >= 0b11100000:
// 		c = (code_points[0] & 0b11111) << 12
// 		c |= (code_points[1] & 0b111111) << 6
// 		c |= (code_points[2] & 0b111111)
// 	elif code_points[0] & 0b11000000 >= 0b11000000:
// 		c = (code_points[0] & 0b111111) << 6
// 		c |= (code_points[1] & 0b111111)
// 	else:
// 		c = code_points[0]
// 	return c

std::u32string stringtou32(const std::string& str) {
    std::u32string out;

    size_t i = 0;
    uint32_t u32char = 0;
    while (i < str.size()) {
        uint8_t start = str[i];

        if ((start & 0b11110000) >= 0b11110000) {
            u32char = (str[i++] & 0b1111) << 18;
            u32char |= (str[i++] & 0b111111) << 12;
            u32char |= (str[i++] & 0b111111) << 6;
            u32char |= (str[i] & 0b111111);
        } else if ((start & 0b11100000) >= 0b11100000) {
            u32char = (str[i++] & 0b11111) << 12;
            u32char |= (str[i++] & 0b111111) << 6;
            u32char |= (str[i] & 0b111111);
        } else if ((start & 0b11000000) >= 0b11000000) {
            u32char = (str[i++] & 0b111111) << 6;
            u32char |= (str[i] & 0b111111);
        } else {
            u32char = start;
        }

        out += u32char;
        i++;
    }

    return out;
}