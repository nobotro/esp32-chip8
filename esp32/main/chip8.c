/*
 * chip8.c
 *
 *  Created on: Jul 16, 2018
 *      Author: nobotro
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include <stdio.h>
#include "freertos/event_groups.h"
#include "lwip/sockets.h"

uint8_t ram[4096]; //ram for chip8
uint16_t rom_load_addres = 0x200; //chip-8 program start from that adress
uint8_t v_registers[16]; // v registers
uint16_t i_register = 0; // memory register,stores memory adress,last 12 bit used
uint16_t delay_register = 0;
uint16_t sound_register = 0; // When these registers are non-zero, they are automatically decremented at a rate of 60Hz
uint16_t pc_register = 0; // stores currently executing adress
uint16_t stack[16]; /* used to store the address that the interpreter shoud return to when finished with a subroutine.
 Chip-8 allows for up to 16 levels of nested subroutines.(16 nested returns)*/
uint16_t *stack_pointer = stack; //store index of stack
bool draw_flag = false;
uint8_t keyboard[16];
uint8_t display[2048];
uint16_t opcode = 0;
uint8_t rom_index = 0;

uint8_t fonts[] = { 0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70,
		0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90,
		0xF0, 0x10, 0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80, 0xF0, 0x90,
		0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40, 0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0,
		0x90, 0xF0, 0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0,
		0x90, 0xE0, 0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0,
		0xF0, 0x80, 0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80 };

//chip8 picture
uint8_t intro[] = { 0, 224, 162, 72, 96, 0, 97, 30, 98, 0, 210, 2, 210, 18, 114,
		8, 50, 64, 18, 10, 96, 0, 97, 62, 98, 2, 162, 74, 208, 46, 209, 46, 114,
		14, 208, 46, 209, 46, 162, 88, 96, 11, 97, 8, 208, 31, 112, 10, 162,
		103, 208, 31, 112, 10, 162, 118, 208, 31, 112, 3, 162, 133, 208, 31,
		112, 10, 162, 148, 208, 31, 18, 70, 255, 255, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192, 192, 255, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128, 128, 255, 129, 129, 129, 129,
		129, 129, 129, 255, 129, 129, 129, 129, 129, 129, 129, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 255, 129,
		129, 129, 129, 129, 129, 255, 128, 128, 128, 128, 128, 128, 128, 255,
		129, 129, 129, 129, 129, 129, 255, 129, 129, 129, 129, 129, 129, 255,
		255 };

uint8_t brix[] = { 110, 5, 101, 0, 107, 6, 106, 0, 163, 12, 218, 177, 122, 4,
		58, 64, 18, 8, 123, 2, 59, 18, 18, 6, 108, 32, 109, 31, 163, 16, 220,
		209, 34, 246, 96, 0, 97, 0, 163, 18, 208, 17, 112, 8, 163, 14, 208, 17,
		96, 64, 240, 21, 240, 7, 48, 0, 18, 52, 198, 15, 103, 30, 104, 1, 105,
		255, 163, 14, 214, 113, 163, 16, 220, 209, 96, 4, 224, 161, 124, 254,
		96, 6, 224, 161, 124, 2, 96, 63, 140, 2, 220, 209, 163, 14, 214, 113,
		134, 132, 135, 148, 96, 63, 134, 2, 97, 31, 135, 18, 71, 31, 18, 172,
		70, 0, 104, 1, 70, 63, 104, 255, 71, 0, 105, 1, 214, 113, 63, 1, 18,
		170, 71, 31, 18, 170, 96, 5, 128, 117, 63, 0, 18, 170, 96, 1, 240, 24,
		128, 96, 97, 252, 128, 18, 163, 12, 208, 113, 96, 254, 137, 3, 34, 246,
		117, 1, 34, 246, 69, 96, 18, 222, 18, 70, 105, 255, 128, 96, 128, 197,
		63, 1, 18, 202, 97, 2, 128, 21, 63, 1, 18, 224, 128, 21, 63, 1, 18, 238,
		128, 21, 63, 1, 18, 232, 96, 32, 240, 24, 163, 14, 126, 255, 128, 224,
		128, 4, 97, 0, 208, 17, 62, 0, 18, 48, 18, 222, 120, 255, 72, 254, 104,
		255, 18, 238, 120, 1, 72, 2, 104, 1, 96, 4, 240, 24, 105, 255, 18, 112,
		163, 20, 245, 51, 242, 101, 241, 41, 99, 55, 100, 0, 211, 69, 115, 5,
		242, 41, 211, 69, 0, 238, 224, 0, 128, 0, 252, 0, 170, 0, 0, 0, 0, 0 };

uint8_t filter[] = { 0, 224, 110, 0, 109, 7, 107, 29, 108, 28, 34, 142, 34, 158,
		34, 120, 34, 110, 96, 1, 240, 21, 240, 7, 48, 0, 18, 22, 34, 128, 73,
		31, 34, 52, 96, 4, 224, 161, 34, 94, 112, 2, 224, 161, 34, 102, 70, 1,
		34, 72, 18, 18, 34, 138, 96, 2, 240, 24, 34, 158, 125, 255, 34, 158, 77,
		0, 18, 92, 34, 110, 0, 238, 96, 5, 240, 24, 34, 158, 126, 1, 34, 158,
		78, 99, 18, 92, 34, 138, 34, 110, 0, 238, 18, 92, 34, 120, 123, 253, 34,
		120, 0, 238, 34, 120, 123, 3, 34, 120, 0, 238, 105, 7, 200, 61, 120, 2,
		216, 145, 0, 238, 162, 188, 219, 193, 162, 189, 0, 238, 34, 138, 121, 1,
		34, 138, 134, 240, 0, 238, 216, 145, 0, 238, 99, 6, 100, 0, 162, 190,
		212, 49, 116, 8, 68, 64, 0, 238, 18, 148, 162, 192, 99, 55, 100, 0, 254,
		51, 242, 101, 241, 41, 211, 69, 115, 5, 242, 41, 211, 69, 99, 0, 253,
		41, 211, 69, 162, 189, 0, 238, 252, 128, 255, 117, 0, 0, 0, 0, 0, 0 };

uint8_t pongai[] = { 106, 2, 107, 12, 108, 63, 109, 12, 162, 234, 218, 182, 220,
		214, 110, 0, 34, 212, 102, 3, 104, 2, 96, 96, 240, 21, 240, 7, 48, 0,
		18, 26, 199, 23, 119, 8, 105, 255, 162, 240, 214, 113, 162, 234, 218,
		182, 220, 214, 96, 1, 224, 161, 123, 254, 96, 4, 224, 161, 123, 2, 96,
		31, 139, 2, 218, 182, 141, 112, 192, 10, 125, 254, 64, 0, 125, 2, 96, 0,
		96, 31, 141, 2, 220, 214, 162, 240, 214, 113, 134, 132, 135, 148, 96,
		63, 134, 2, 97, 31, 135, 18, 70, 2, 18, 120, 70, 63, 18, 130, 71, 31,
		105, 255, 71, 0, 105, 1, 214, 113, 18, 42, 104, 2, 99, 1, 128, 112, 128,
		181, 18, 138, 104, 254, 99, 10, 128, 112, 128, 213, 63, 1, 18, 162, 97,
		2, 128, 21, 63, 1, 18, 186, 128, 21, 63, 1, 18, 200, 128, 21, 63, 1, 18,
		194, 96, 32, 240, 24, 34, 212, 142, 52, 34, 212, 102, 62, 51, 1, 102, 3,
		104, 254, 51, 1, 104, 2, 18, 22, 121, 255, 73, 254, 105, 255, 18, 200,
		121, 1, 73, 2, 105, 1, 96, 4, 240, 24, 118, 1, 70, 64, 118, 254, 18,
		108, 162, 242, 254, 51, 242, 101, 241, 41, 100, 20, 101, 0, 212, 85,
		116, 21, 242, 41, 212, 85, 0, 238, 128, 128, 128, 128, 128, 128, 128, 0,
		0, 0, 0, 0 };

uint8_t pong2[] = { 34, 252, 107, 12, 108, 63, 109, 12, 162, 234, 218, 182, 220,
		214, 110, 0, 34, 212, 102, 3, 104, 2, 96, 96, 240, 21, 240, 7, 48, 0,
		18, 26, 199, 23, 119, 8, 105, 255, 162, 240, 214, 113, 162, 234, 218,
		182, 220, 214, 96, 1, 224, 161, 123, 254, 96, 4, 224, 161, 123, 2, 96,
		31, 139, 2, 218, 182, 96, 12, 224, 161, 125, 254, 96, 13, 224, 161, 125,
		2, 96, 31, 141, 2, 220, 214, 162, 240, 214, 113, 134, 132, 135, 148, 96,
		63, 134, 2, 97, 31, 135, 18, 70, 0, 18, 120, 70, 63, 18, 130, 71, 31,
		105, 255, 71, 0, 105, 1, 214, 113, 18, 42, 104, 2, 99, 1, 128, 112, 128,
		181, 18, 138, 104, 254, 99, 10, 128, 112, 128, 213, 63, 1, 18, 162, 97,
		2, 128, 21, 63, 1, 18, 186, 128, 21, 63, 1, 18, 200, 128, 21, 63, 1, 18,
		194, 96, 32, 240, 24, 34, 212, 142, 52, 34, 212, 102, 62, 51, 1, 102, 3,
		104, 254, 51, 1, 104, 2, 18, 22, 121, 255, 73, 254, 105, 255, 18, 200,
		121, 1, 73, 2, 105, 1, 96, 4, 240, 24, 118, 1, 70, 64, 118, 254, 18,
		108, 162, 242, 254, 51, 242, 101, 241, 41, 100, 20, 101, 2, 212, 85,
		116, 21, 242, 41, 212, 85, 0, 238, 128, 128, 128, 128, 128, 128, 128, 0,
		0, 0, 0, 0, 192, 192, 192, 0, 255, 0, 107, 32, 108, 0, 162, 246, 219,
		196, 124, 4, 60, 32, 19, 2, 106, 0, 107, 0, 108, 31, 162, 250, 218, 177,
		218, 193, 122, 8, 58, 64, 19, 18, 162, 246, 106, 0, 107, 32, 219, 161,
		0, 238 };

uint8_t invaders[] = { 18, 37, 83, 80, 65, 67, 69, 32, 73, 78, 86, 65, 68, 69,
		82, 83, 32, 118, 48, 46, 57, 32, 66, 121, 32, 68, 97, 118, 105, 100, 32,
		87, 73, 78, 84, 69, 82, 96, 0, 97, 0, 98, 8, 163, 211, 208, 24, 113, 8,
		242, 30, 49, 32, 18, 45, 112, 8, 97, 0, 48, 64, 18, 45, 105, 5, 108, 21,
		110, 0, 35, 135, 96, 10, 240, 21, 240, 7, 48, 0, 18, 75, 35, 135, 126,
		1, 18, 69, 102, 0, 104, 28, 105, 0, 106, 4, 107, 10, 108, 4, 109, 60,
		110, 15, 0, 224, 35, 107, 35, 71, 253, 21, 96, 4, 224, 158, 18, 125, 35,
		107, 56, 0, 120, 255, 35, 107, 96, 6, 224, 158, 18, 139, 35, 107, 56,
		57, 120, 1, 35, 107, 54, 0, 18, 159, 96, 5, 224, 158, 18, 233, 102, 1,
		101, 27, 132, 128, 163, 207, 212, 81, 163, 207, 212, 81, 117, 255, 53,
		255, 18, 173, 102, 0, 18, 233, 212, 81, 63, 1, 18, 233, 212, 81, 102, 0,
		131, 64, 115, 3, 131, 181, 98, 248, 131, 34, 98, 8, 51, 0, 18, 201, 35,
		115, 130, 6, 67, 8, 18, 211, 51, 16, 18, 213, 35, 115, 130, 6, 51, 24,
		18, 221, 35, 115, 130, 6, 67, 32, 18, 231, 51, 40, 18, 233, 35, 115, 62,
		0, 19, 7, 121, 6, 73, 24, 105, 0, 106, 4, 107, 10, 108, 4, 125, 244,
		110, 15, 0, 224, 35, 71, 35, 107, 253, 21, 18, 111, 247, 7, 55, 0, 18,
		111, 253, 21, 35, 71, 139, 164, 59, 18, 19, 27, 124, 2, 106, 252, 59, 2,
		19, 35, 124, 2, 106, 4, 35, 71, 60, 24, 18, 111, 0, 224, 164, 211, 96,
		20, 97, 8, 98, 15, 208, 31, 112, 8, 242, 30, 48, 44, 19, 51, 240, 10, 0,
		224, 166, 244, 254, 101, 18, 37, 163, 183, 249, 30, 97, 8, 35, 95, 129,
		6, 35, 95, 129, 6, 35, 95, 129, 6, 35, 95, 123, 208, 0, 238, 128, 224,
		128, 18, 48, 0, 219, 198, 123, 12, 0, 238, 163, 207, 96, 28, 216, 4, 0,
		238, 35, 71, 142, 35, 35, 71, 96, 5, 240, 24, 240, 21, 240, 7, 48, 0,
		19, 127, 0, 238, 106, 0, 141, 224, 107, 4, 233, 161, 18, 87, 166, 2,
		253, 30, 240, 101, 48, 255, 19, 165, 106, 0, 107, 4, 109, 1, 110, 1, 19,
		141, 165, 0, 240, 30, 219, 198, 123, 8, 125, 1, 122, 1, 58, 7, 19, 141,
		0, 238, 60, 126, 255, 255, 153, 153, 126, 255, 255, 36, 36, 231, 126,
		255, 60, 60, 126, 219, 129, 66, 60, 126, 255, 219, 16, 56, 124, 254, 0,
		0, 127, 0, 63, 0, 127, 0, 0, 0, 1, 1, 1, 3, 3, 3, 3, 0, 0, 63, 32, 32,
		32, 32, 32, 32, 32, 32, 63, 8, 8, 255, 0, 0, 254, 0, 252, 0, 254, 0, 0,
		0, 126, 66, 66, 98, 98, 98, 98, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255,
		0, 0, 255, 0, 125, 0, 65, 125, 5, 125, 125, 0, 0, 194, 194, 198, 68,
		108, 40, 56, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 247,
		16, 20, 247, 247, 4, 4, 0, 0, 124, 68, 254, 194, 194, 194, 194, 0, 0,
		255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 239, 32, 40, 232, 232,
		47, 47, 0, 0, 249, 133, 197, 197, 197, 197, 249, 0, 0, 255, 0, 0, 0, 0,
		0, 0, 0, 0, 255, 0, 0, 255, 0, 190, 0, 32, 48, 32, 190, 190, 0, 0, 247,
		4, 231, 133, 133, 132, 244, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0,
		0, 255, 0, 0, 127, 0, 63, 0, 127, 0, 0, 0, 239, 40, 239, 0, 224, 96,
		111, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 254, 0,
		252, 0, 254, 0, 0, 0, 192, 0, 192, 192, 192, 192, 192, 0, 0, 252, 4, 4,
		4, 4, 4, 4, 4, 4, 252, 16, 16, 255, 249, 129, 185, 139, 154, 154, 250,
		0, 250, 138, 154, 154, 155, 153, 248, 230, 37, 37, 244, 52, 52, 52, 0,
		23, 20, 52, 55, 54, 38, 199, 223, 80, 80, 92, 216, 216, 223, 0, 223, 17,
		31, 18, 27, 25, 217, 124, 68, 254, 134, 134, 134, 252, 132, 254, 130,
		130, 254, 254, 128, 192, 192, 192, 254, 252, 130, 194, 194, 194, 252,
		254, 128, 248, 192, 192, 254, 254, 128, 240, 192, 192, 192, 254, 128,
		190, 134, 134, 254, 134, 134, 254, 134, 134, 134, 16, 16, 16, 16, 16,
		16, 24, 24, 24, 72, 72, 120, 156, 144, 176, 192, 176, 156, 128, 128,
		192, 192, 192, 254, 238, 146, 146, 134, 134, 134, 254, 130, 134, 134,
		134, 134, 124, 130, 134, 134, 134, 124, 254, 130, 254, 192, 192, 192,
		124, 130, 194, 202, 196, 122, 254, 134, 254, 144, 156, 132, 254, 192,
		254, 2, 2, 254, 254, 16, 48, 48, 48, 48, 130, 130, 194, 194, 194, 254,
		130, 130, 130, 238, 56, 16, 134, 134, 150, 146, 146, 238, 130, 68, 56,
		56, 68, 130, 130, 130, 254, 48, 48, 48, 254, 2, 30, 240, 128, 254, 0, 0,
		0, 0, 6, 6, 0, 0, 0, 96, 96, 192, 0, 0, 0, 0, 0, 0, 24, 24, 24, 24, 0,
		24, 124, 198, 12, 24, 0, 24, 0, 0, 254, 254, 0, 0, 254, 130, 134, 134,
		134, 254, 8, 8, 8, 24, 24, 24, 254, 2, 254, 192, 192, 254, 254, 2, 30,
		6, 6, 254, 132, 196, 196, 254, 4, 4, 254, 128, 254, 6, 6, 254, 192, 192,
		192, 254, 130, 254, 254, 2, 2, 6, 6, 6, 124, 68, 254, 134, 134, 254,
		254, 130, 254, 6, 6, 6, 68, 254, 68, 68, 254, 68, 168, 168, 168, 168,
		168, 168, 168, 108, 90, 0, 12, 24, 168, 48, 78, 126, 0, 18, 24, 102,
		108, 168, 90, 102, 84, 36, 102, 0, 72, 72, 24, 18, 168, 6, 144, 168, 18,
		0, 126, 48, 18, 168, 132, 48, 78, 114, 24, 102, 168, 168, 168, 168, 168,
		168, 144, 84, 120, 168, 72, 120, 108, 114, 168, 18, 24, 108, 114, 102,
		84, 144, 168, 114, 42, 24, 168, 48, 78, 126, 0, 18, 24, 102, 108, 168,
		114, 84, 168, 90, 102, 24, 126, 24, 78, 114, 168, 114, 42, 24, 48, 102,
		168, 48, 78, 126, 0, 108, 48, 84, 78, 156, 168, 168, 168, 168, 168, 168,
		168, 72, 84, 126, 24, 168, 144, 84, 120, 102, 168, 108, 42, 48, 90, 168,
		132, 48, 114, 42, 168, 216, 168, 0, 78, 18, 168, 228, 162, 168, 0, 78,
		18, 168, 108, 42, 84, 84, 114, 168, 132, 48, 114, 42, 168, 222, 156,
		168, 114, 42, 24, 168, 12, 84, 72, 90, 120, 114, 24, 102, 168, 114, 24,
		66, 66, 108, 168, 114, 42, 0, 114, 168, 114, 42, 24, 168, 48, 78, 126,
		0, 18, 24, 102, 108, 168, 48, 78, 12, 102, 24, 0, 108, 24, 168, 114, 42,
		24, 48, 102, 168, 30, 84, 102, 12, 24, 156, 168, 36, 84, 84, 18, 168,
		66, 120, 12, 60, 168, 174, 168, 168, 168, 168, 168, 168, 168, 255, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

uint8_t tetris[] = { 162, 180, 35, 230, 34, 182, 112, 1, 208, 17, 48, 37, 18, 6,
		113, 255, 208, 17, 96, 26, 208, 17, 96, 37, 49, 0, 18, 14, 196, 112, 68,
		112, 18, 28, 195, 3, 96, 30, 97, 3, 34, 92, 245, 21, 208, 20, 63, 1, 18,
		60, 208, 20, 113, 255, 208, 20, 35, 64, 18, 28, 231, 161, 34, 114, 232,
		161, 34, 132, 233, 161, 34, 150, 226, 158, 18, 80, 102, 0, 246, 21, 246,
		7, 54, 0, 18, 60, 208, 20, 113, 1, 18, 42, 162, 196, 244, 30, 102, 0,
		67, 1, 102, 4, 67, 2, 102, 8, 67, 3, 102, 12, 246, 30, 0, 238, 208, 20,
		112, 255, 35, 52, 63, 1, 0, 238, 208, 20, 112, 1, 35, 52, 0, 238, 208,
		20, 112, 1, 35, 52, 63, 1, 0, 238, 208, 20, 112, 255, 35, 52, 0, 238,
		208, 20, 115, 1, 67, 4, 99, 0, 34, 92, 35, 52, 63, 1, 0, 238, 208, 20,
		115, 255, 67, 255, 99, 3, 34, 92, 35, 52, 0, 238, 128, 0, 103, 5, 104,
		6, 105, 4, 97, 31, 101, 16, 98, 7, 0, 238, 64, 224, 0, 0, 64, 192, 64,
		0, 0, 224, 64, 0, 64, 96, 64, 0, 64, 64, 96, 0, 32, 224, 0, 0, 192, 64,
		64, 0, 0, 224, 128, 0, 64, 64, 192, 0, 0, 224, 32, 0, 96, 64, 64, 0,
		128, 224, 0, 0, 64, 192, 128, 0, 192, 96, 0, 0, 64, 192, 128, 0, 192,
		96, 0, 0, 128, 192, 64, 0, 0, 96, 192, 0, 128, 192, 64, 0, 0, 96, 192,
		0, 192, 192, 0, 0, 192, 192, 0, 0, 192, 192, 0, 0, 192, 192, 0, 0, 64,
		64, 64, 64, 0, 240, 0, 0, 64, 64, 64, 64, 0, 240, 0, 0, 208, 20, 102,
		53, 118, 255, 54, 0, 19, 56, 0, 238, 162, 180, 140, 16, 60, 30, 124, 1,
		60, 30, 124, 1, 60, 30, 124, 1, 35, 94, 75, 10, 35, 114, 145, 192, 0,
		238, 113, 1, 19, 80, 96, 27, 107, 0, 208, 17, 63, 0, 123, 1, 208, 17,
		112, 1, 48, 37, 19, 98, 0, 238, 96, 27, 208, 17, 112, 1, 48, 37, 19,
		116, 142, 16, 141, 224, 126, 255, 96, 27, 107, 0, 208, 225, 63, 0, 19,
		144, 208, 225, 19, 148, 208, 209, 123, 1, 112, 1, 48, 37, 19, 134, 75,
		0, 19, 166, 125, 255, 126, 255, 61, 1, 19, 130, 35, 192, 63, 1, 35, 192,
		122, 1, 35, 192, 128, 160, 109, 7, 128, 210, 64, 4, 117, 254, 69, 2,
		101, 4, 0, 238, 167, 0, 242, 85, 168, 4, 250, 51, 242, 101, 240, 41,
		109, 50, 110, 0, 221, 229, 125, 5, 241, 41, 221, 229, 125, 5, 242, 41,
		221, 229, 167, 0, 242, 101, 162, 180, 0, 238, 106, 0, 96, 25, 0, 238,
		55, 35 };

uint8_t* roms[] = { intro, brix, filter, pongai, pong2, invaders, tetris };
int sizes[] = { sizeof(intro), sizeof(brix), sizeof(filter), sizeof(pongai),
		sizeof(pong2), sizeof(invaders), sizeof(tetris) };
bool active = true;
/*chip-8 keyboard layout
 # 1, 2, 3, 0xC,
 # 4, 5, 6, 0xD,
 # 7, 8, 9, 0xE,
 # 0xA, 0, 0xB, 0xF
 */

void init_fonts() {
	for (int i = 0; i < 0x50; i++) {
		ram[i] = fonts[i];
	}
}

void load_rom(uint8_t rom[], int size) {

	for (int i = 0; i < size; i++) {
		ram[i + rom_load_addres] = rom[i];

	}

}

void del_sound_timer() {
	if (delay_register) {
		delay_register -= 1;
	}

}
void _00() {

// 00E0 - CLS Clear the display.
	bzero(display, sizeof(display));
	draw_flag = true;
}

void _0E() {
	/*
	 00EE - RET
	 Return from a subroutine.
	 The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.'''
	 */
	pc_register = *(stack_pointer);
	stack_pointer--;
}

void _1() {
	/* 1nnn - JP addr
	 jump to location nnn.
	 The interpreter sets the program counter to nnn.
	 */

	pc_register = opcode & 0x0FFF;

}

void _2() {
	/*2nnn - CALL addr
	 Call subroutine at nnn.
	 The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.
	 */

//    self.stack.append(self.pc_register)
	stack_pointer++;
	*(stack_pointer) = pc_register;
	pc_register = opcode & 0x0FFF;

}

void _3() {
	/*3xkk - SE Vx, byte
	 Skip next instruction if Vx = kk.
	 The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
	 */

	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t kk = opcode & 0x00FF;

	if (v_registers[x] == kk)
		pc_register += 2;

}

void _4() {

	/*4xkk - SNE Vx, byte
	 Skip next instruction if Vx != kk.
	 The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.*/

	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t kk = opcode & 0x00FF;
	if (v_registers[x] != kk)
		pc_register += 2;
}

void _5() {
	/*5xy0 - SE Vx, Vy
	 Skip next instruction if Vx = Vy.
	 The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
	 */
	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t y = (opcode & 0x00F0) >> 4;
	if (v_registers[x] == v_registers[y])
		pc_register += 2;

}

void _6() {
	/*6xkk - LD Vx, byte
	 Set Vx = kk.
	 The interpreter puts the value kk into register Vx.*/

	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t kk = opcode & 0x00FF;
	v_registers[x] = kk;
	v_registers[x] &= 0xff;
}

void _7() {
	/*
	 7xkk - ADD Vx, byte
	 Set Vx = Vx + kk.
	 Adds the value kk to the value of register Vx, then stores the result in Vx.
	 */
	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t kk = opcode & 0x00FF;
	v_registers[x] = v_registers[x] + kk;
	v_registers[x] &= 0xff;

}

void _80() {
	/*8xy0 - LD Vx, Vy
	 Set Vx = Vy.
	 Stores the value of register Vy in register Vx.*/

	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t y = (opcode & 0x00F0) >> 4;
	v_registers[x] = v_registers[y];
	v_registers[x] &= 0xff;

}

void _81() {
	/*8xy1 - OR Vx, Vy
	 Set Vx = Vx OR Vy.
	 Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx.
	 A bitwise OR compares the corrseponding bits from two values, and if either bit is 1, then the same bit in the result is also 1. Otherwise, it is 0.
	 */

	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t y = (opcode & 0x00F0) >> 4;
	v_registers[x] = (v_registers[x] | v_registers[y]);
	v_registers[x] &= 0xff;

}

void _82() {
	/*8xy2 - AND Vx, Vy
	 Set Vx = Vx AND Vy.
	 Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
	 A bitwise AND compares the corrseponding bits from two values, and if both bits are 1, then the same bit in the result is also 1. Otherwise, it is 0.
	 */

	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t y = (opcode & 0x00F0) >> 4;
	v_registers[x] = (v_registers[x] & v_registers[y]);
	v_registers[x] &= 0xff;
}

void _83() {
	/*8xy3 - XOR Vx, Vy
	 Set Vx = Vx XOR Vy.
	 Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
	 An exclusive OR compares the corrseponding bits from two values, and if the bits are not both the same,
	 then the corresponding bit in the result is set to 1. Otherwise, it is 0.
	 */

	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t y = (opcode & 0x00F0) >> 4;
	v_registers[x] = (v_registers[x] ^ v_registers[y]);
	v_registers[x] &= 0xff;

}

void _84() {
	/*8xy4 - ADD Vx, Vy
	 Set Vx = Vx + Vy, set VF = carry.
	 The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,)
	 VF is set to 1, otherwise 0. Only the lowest 8 bits of the result are kept, and stored in Vx.
	 */

	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t y = (opcode & 0x00F0) >> 4;
	if (v_registers[x] + v_registers[y] > 255) {
		v_registers[15] = 1;
	} else {
		v_registers[15] = 0;
	}

	v_registers[x] = (v_registers[x] + v_registers[y]);
	v_registers[x] &= 0xff;
}

void _85() {

	/*8xy5 - SUB Vx, Vy
	 Set Vx = Vx - Vy, set VF = NOT borrow.
	 If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
	 */

	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t y = (opcode & 0x00F0) >> 4;

	if (v_registers[x] > v_registers[y]) {
		v_registers[15] = 1;
	} else {
		v_registers[15] = 0;
	}

	v_registers[x] = (v_registers[x] - v_registers[y]);
	v_registers[x] &= 0xff;
}

void _86() {
	/*8xy6 - SHR Vx {, Vy} Set Vx = Vx SHR 1.
	 If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
	 */

	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t y = (opcode & 0x00F0) >> 4;
	v_registers[15] = v_registers[x] & 1;

	v_registers[x] = v_registers[y] >> 1;
	v_registers[x] &= 0xff;

}

void _87() {
	/*8xy7 - SUBN Vx, Vy
	 Set Vx = Vy - Vx, set VF = NOT borrow.
	 If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
	 */

	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t y = (opcode & 0x00F0) >> 4;

	if (v_registers[y] > v_registers[x]) {
		v_registers[15] = 1;
	} else {
		v_registers[15] = 0;
	}

	v_registers[x] = v_registers[y] - v_registers[x];
	v_registers[x] &= 0xff;

}

void _8E() {
	/*8xyE - SHL Vx {, Vy}
	 Set Vx = Vx SHL 1.
	 If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
	 */

	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t y = (opcode & 0x00F0) >> 4;

	v_registers[15] = v_registers[x] >> 7;

	v_registers[x] = (v_registers[y] << 1);
	v_registers[x] &= 0xff;

}

void _9() {
	/*9xy0 - SNE Vx, Vy
	 Skip next instruction if Vx != Vy.
	 The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.*/

	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t y = (opcode & 0x00F0) >> 4;

	if (x != y) {
		pc_register += 2;
	}

}

void _A() {
	/*Annn - LD I, addr
	 Set I = nnn.
	 The value of register I is set to nnn.*/

	uint16_t nnn = opcode & 0x0FFF;
	i_register = nnn;

}

void _B() {
	/*Bnnn - JP V0, addr
	 Jump to location nnn + V0.
	 The program counter is set to nnn plus the value of V0.*/

	uint16_t nnn = opcode & 0x0FFF;
	pc_register = nnn + v_registers[0];
}

void _C() {
	/*Cxkk - RND Vx, byte
	 Set Vx = random byte AND kk.
	 The interpreter generates a random number from 0 to 255,
	 which is then ANDed with the value kk.
	 The results are stored in Vx. See instruction 8xy2 for more information on AND.*/

	uint16_t x = (opcode & 0x0F00) >> 8;
	uint16_t kk = opcode & 0x00FF;
	uint8_t randn = rand() % 255;

	v_registers[x] = kk & randn;
	v_registers[x] &= 0xff;
}

void _D() {
	/*Dxyn - DRW Vx, Vy, nibble
	 Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.

	 The interpreter reads n bytes from memory, starting at the address stored in I.
	 These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
	 Sprites are XORed onto the existing screen.
	 If this causes any pixels to be erased, VF is set to 1, otherwise it is set to 0.
	 If the sprite is positioned so part of it is outside the coordinates of the display, it wraps around to the opposite side of the screen.
	 See instruction 8xy3 for more information on XOR, and section 2.4, Display, for more information on the Chip-8 screen and sprites.'''*/

	int vx = v_registers[(opcode & 0x0F00) >> 8];
	int vy = v_registers[(opcode & 0x00F0) >> 4];
	int n = opcode & 0x000F;
	v_registers[0xF] = 0;
	for (int y = 0; y < n; y++) {
		uint8_t pixel = ram[i_register + y];
		for (uint8_t x = 0; x < 8; x++) {
			if ((pixel & (0x80 >> x)) && x + vx + ((y + vy) * 64) < 2048) {

				if (display[x + vx + ((y + vy) * 64)]) {
					v_registers[0xF] = 1;
				}
				display[x + vx + ((y + vy) * 64)] ^= 1;

			}

		}

	}

	draw_flag = true;

}

void _EE() {
	/*'''

	 Ex9E - SKP Vx
	 Skip next instruction if key with the value of Vx is pressed.
	 Checks the keyboard, and if the key corresponding to the value of Vx is
	 currently in the down position, PC is increased by 2.'''*/

	uint16_t x = (opcode & 0x0F00) >> 8;

	if (keyboard[v_registers[x] & 0xf] == 1) {
		pc_register += 2;
	}

}

void _E1() {
	/*'''ExA1 - SKNP Vx
	 Skip next instruction if key with the value of Vx is not pressed.
	 Checks the keyboard, and
	 if the key corresponding to the value of Vx is currently in the up position,
	 PC is increased by 2.
	 '''*/

	uint16_t x = (opcode & 0x0F00) >> 8;

	if (keyboard[v_registers[x] & 0xf] == 0) {

		pc_register += 2;
	}

}

void _F07() {
	/*'''
	 Fx07 - LD Vx, DT
	 Set Vx = delay timer value.
	 The value of DT is placed into Vx'''*/

	uint16_t x = (opcode & 0x0F00) >> 8;

	v_registers[x] = delay_register;
}

//not need

void _F0A() {

	//not implemented
	/*
	 '''Fx0A - LD Vx, K
	 Wait for a key press, store the value of the key in Vx.
	 All execution stops until a key is pressed, then the value of that key is stored in Vx.'''

	 x = (self.opcode & 0x0F00) >> 8
	 cur_state=self.keyboard
	 while True:
	 for i in self.keyboard:
	 if i!=0 and not i in cur_state:
	 self.v_registers[x]=i
	 break
	 */

}

void _F15() {
	/*'''
	 Set delay timer = Vx.

	 DT is set equal to the value of Vx.
	 '''
	 */

	uint16_t x = (opcode & 0x0F00) >> 8;
	delay_register = v_registers[x];

}

void _F18() {

	/*'''
	 Set sound timer = Vx.

	 ST is set equal to the value of Vx.

	 '''*/

	uint16_t x = (opcode & 0x0F00) >> 8;
	sound_register = v_registers[x];
}

void _F1E() {
	/*'''
	 Set I = I + Vx.

	 The values of I and Vx are added, and the results are stored in I.'''*/

	uint16_t x = (opcode & 0x0F00) >> 8;
	i_register = i_register + v_registers[x];

}

void _F29() {

	/*'''

	 Set I = location of sprite for digit Vx.

	 The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx.
	 See section 2.4, Display, for more information on the Chip-8 hexadecimal font.

	 '''*/
	uint16_t x = (opcode & 0x0F00) >> 8;

	i_register = v_registers[x] * 5;

}

void _F33() {

	/*'''Store BCD representation of Vx in memory locations I, I+1, and I+2.

	 The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I
	 , the tens digit at location I+1, and the ones digit at location I+2.
	 '''*/

	uint16_t x = v_registers[(opcode & 0x0F00) >> 8];

	uint16_t hundred = x / 100;
	uint16_t ten = (x - (hundred * 100)) / 10;
	uint16_t one = (x - (hundred * 100) - (ten * 10));
	ram[i_register] = hundred;
	ram[i_register + 1] = ten;
	ram[i_register + 2] = one;

}

void _F55() {

	/*'''
	 Store registers V0 through Vx in memory starting at location I.

	 The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.

	 '''*/

	uint16_t x = (opcode & 0x0F00) >> 8;
	for (int i = 0; i <= x; i++) {
		ram[i_register + i] = v_registers[i];
	}

}

void _F65() {

	/* '''

	 Read registers V0 through Vx from memory starting at location I.

	 The interpreter reads values from memory starting at location I into registers V0 through Vx.
	 '''*/

	uint16_t x = (opcode & 0x0F00) >> 8;
	for (int i = 0; i <= x; i++) {
		v_registers[i] = ram[i_register + i];
	}

}

struct {
	char* name;
	void (*func)(void);
} funcs[] = { { "00", _00 }, { "0E", _0E }, { "1", _1 }, { "2", _2 },
		{ "3", _3 }, { "4", _4 }, { "5", _5 }, { "6", _6 }, { "7", _7 }, { "80",
				_80 }, { "81", _81 }, { "82", _82 }, { "83", _83 },
		{ "84", _84 }, { "85", _85 }, { "86", _86 }, { "87", _87 },
		{ "8E", _8E }, { "9", _9 }, { "A", _A }, { "B", _B }, { "C", _C }, {
				"D", _D }, { "EE", _EE }, { "E1", _E1 }, { "F07", _F07 }, {
				"F0A", _F0A }, { "F15", _F15 }, { "F18", _F18 },
		{ "F1E", _F1E }, { "F29", _F29 }, { "F33", _F33 }, { "F55", _F55 }, {
				"F65", _F65 }

};

int call_function(const char *name) {
	int i;

	for (i = 0; i < (sizeof(funcs) / sizeof(funcs[0])); i++) {
		if (!strcmp(funcs[i].name, strupr(name)) && funcs[i].func) {

			funcs[i].func();
			return 0;
		}
	}

	return -1;
}

void execop(uint16_t opcode) {

//	printf("\n%x",opcode);
	uint8_t f = (opcode & 0xF000) >> 12;
	if (f == 8 || f == 0 || f == 14) {

		uint8_t l = opcode & 0x000F;

		char hex[3];

		sprintf(hex, "%x%x", f, l);
//		printf(hex);
//		printf("\n");
		call_function(hex);
//		eval('self._'+hex(f)[2:].upper() + hex(l)[2:].upper() + '()')
	} else if (f == 15) {

		uint8_t l = opcode & 0x000F;
		uint8_t ll = (opcode & 0x00F0) >> 4;
		char hex[4];
		sprintf(hex, "%x%x%x", f, ll, l);
//		printf(hex);
//		printf("\n");
		call_function(hex);
//		eval('self._'+hex(f)[2:].upper() + hex(ll)[2:].upper() + hex(l)[2:].upper() + '()')
	} else {
		char hex[2];
		sprintf(hex, "%x", f);
//		printf(hex);
//		printf("\n");
		call_function(hex);
//		eval('self._'+hex(f)[2:].upper() + '()')
	}

}

void run_cycle() {
	init_fonts();
	load_rom(roms[rom_index], sizes[rom_index]);
	pc_register = rom_load_addres;

	while (true) {
		if (active) {
			del_sound_timer();
			opcode = ram[pc_register] << 8 | ram[pc_register + 1];
			pc_register = (pc_register + 2);
			execop(opcode);
			vTaskDelay(1 / portTICK_PERIOD_MS);
		} else {
			bzero(ram, sizeof(ram));
			bzero(v_registers, sizeof(v_registers));
			bzero(stack, sizeof(stack));
			bzero(keyboard, sizeof(keyboard));
			bzero(display, sizeof(display));
			*stack_pointer = stack;
			i_register = 0;
			delay_register = 0;
			sound_register = 0;
			pc_register = 0;
			opcode = 0;
			pc_register = rom_load_addres;

			init_fonts();

			bool draw_flag = false;

			load_rom(roms[rom_index], sizes[rom_index]);
			vTaskDelay(500 / portTICK_PERIOD_MS);
			active = true;

		}

	}

}
