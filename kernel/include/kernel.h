// kernel/include/kernel.h
#pragma once

// Системные переменные и типы данных
#include <variable.h>

// Стандартная библиотека и математика
#include <lib.h>
#include <math.h>
#include <time.h>

// Драйверы ядра
#include <drivers/idt.h>
#include <drivers/screen.h>
#include <drivers/font.h>
#include <drivers/keyboard.h>
#include <drivers/window.h>
#include <drivers/start_menu.h>

// Память и диски
#include <memory/disk.h>
#include <memory/fs.h>
