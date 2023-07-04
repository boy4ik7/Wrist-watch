// Wrist watch v1.0 MultiLanguage
// Страница проекта: https://github.com/boy4ik7/Wrist-watch
// Упраление часами: Удержание кнопки вверх - назад, удержание кнопки вниз - вперед
// Вверх на главном экране включит будильник (можно включить из меню)
// Ввниз на главном экране откроет меню
// Настройки эмулятора
// Кнопки управления на клавиатуре: Стрелка вверх - верхняя кнопка, Вних - нижняя
// Первый переключатель отвечает за подключение зарядки, второй за окончание зарядки
#include <Wire.h>
#include "DS1307.h"
#include <GyverOLED.h>
//#include <EncButton.h>
#include <EncButton2.h>
#include <TimerMs.h>
#include "buildTime.h"
#include <EEPROM.h>
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;
//GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;
DS1307 clock;
// подключаем кнопки на пины D2 и D3
EncButton2<EB_BTN> btn_up(INPUT, 2);
EncButton2<EB_BTN> btn_down(INPUT, 3);
TimerMs tmr_charging(300, 1, 1);
TimerMs tmr_screen(5000, 1, 1);
TimerMs tmr_menu(10000, 0, 1);
TimerMs tmr_stopwatch(120000, 0, 1);
TimerMs tmr_alarm(1000, 1, 1);
#define chargerPin 4 // пин подключения з\у
#define chargerPin_done 5 // пин окончания заряда
#define vibrationPin 6 // пин вибромотора
#define batteryPin A1 // пин измерения заряда аккумулятора
#define INIT_ADDR 1023 // номер резервной ячейки
#define INIT_KEY 50 // ключ первого запуска. 0-254, на выбор

uint8_t charging_animation = 0, alarm_hour = 24, alarm_minute = 60, language = 1;
int battery;
bool charging, charging_done, battery_setting = false, screen_status = true, alarm_status = false;

// иконка будильника
const uint8_t alarm_7x8[] PROGMEM = {
0x60, 0x7C, 0x7E, 0xFF, 0x7E, 0x7C, 0x60
};
// иконка батареи
const uint8_t bat25_7x12[] PROGMEM = {
0xFE, 0x02, 0x03, 0x03, 0x03, 0x02, 0xFE, 0x0F, 0x08, 0x0A,
0x0A, 0x0A, 0x08, 0x0F
};
const uint8_t bat50_7x12[] PROGMEM = {
0xFE, 0x02, 0x83, 0x83, 0x83, 0x02, 0xFE, 0x0F, 0x08, 0x0A,
0x0A, 0x0A, 0x08, 0x0F
};
const uint8_t bat75_7x12[] PROGMEM = {
0xFE, 0x02, 0xA3, 0xA3, 0xA3, 0x02, 0xFE, 0x0F, 0x08, 0x0A,
0x0A, 0x0A, 0x08, 0x0F
};
const uint8_t bat100_7x12[] PROGMEM = {
0xFE, 0x02, 0xAB, 0xAB, 0xAB, 0x02, 0xFE, 0x0F, 0x08, 0x0A,
0x0A, 0x0A, 0x08, 0x0F
};
const uint8_t bat_done_7x12[] PROGMEM = {
0xFE, 0x02, 0xFB, 0xFB, 0xFB, 0x02, 0xFE, 0x0F, 0x08, 0x0B,
0x0B, 0x0B, 0x08, 0x0F
};
//qrcode сайта проекта
const uint8_t qr_code_33x33[] PROGMEM = {
0xFF, 0xFF, 0x03, 0xFB, 0x8B, 0x8B, 0x8B, 0xFB, 0x03, 0xFF,
0xD3, 0x8F, 0x13, 0x7F, 0xB7, 0x07, 0x87, 0x9F, 0x9B, 0xD7,
0xF7, 0x0B, 0x5F, 0xFF, 0x03, 0xFB, 0x8B, 0x8B, 0x8B, 0xFB,
0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xB2, 0xD6, 0x82, 0x52, 0x26,
0x22, 0xAA, 0x5B, 0xFC, 0x7B, 0x04, 0x2F, 0x82, 0xE3, 0x3C,
0x1F, 0x22, 0x31, 0x10, 0x75, 0xE6, 0xCF, 0x52, 0xE6, 0xD6,
0xBA, 0x2E, 0x6A, 0x52, 0xFF, 0xFF, 0xFF, 0xFF, 0x93, 0xCE,
0xA1, 0xFC, 0xA0, 0x92, 0xAA, 0xA9, 0x57, 0xA9, 0x17, 0x3D,
0x3A, 0x8E, 0x81, 0xE1, 0x55, 0xC6, 0x9E, 0xF8, 0x28, 0xBD,
0x8C, 0x85, 0x09, 0x33, 0x06, 0x70, 0x7E, 0xFF, 0xFF, 0xFF,
0xFF, 0x80, 0xBE, 0xA2, 0xA2, 0xA2, 0xBE, 0x80, 0xFF, 0xA4,
0xEC, 0x80, 0xA0, 0xD2, 0xBD, 0x9C, 0x91, 0xF0, 0xE3, 0x94,
0xB6, 0x88, 0x93, 0xBA, 0xC3, 0xF0, 0xD4, 0xEB, 0x9E, 0xE7,
0xFF, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x01, 0x01, 0x01, 0x01, 0x01
};

void setup() {
    //Serial.begin(9600);
    clock.begin();
    // установка времени и даты компиляции с помощью библиотеки buildTime
    clock.fillByYMD(BUILD_YEAR, BUILD_MONTH, BUILD_DAY);
    clock.fillByHMS(BUILD_HOUR, BUILD_MIN, BUILD_SEC);
    // установка времени и даты вручную, удалите " /* */"
    /*
    clock.fillByYMD(2023, 7, 1); // год, месяц, дата
    clock.fillByHMS(15, 28, 30); // часы, минуты, секунды
    */
    //clock.fillDayOfWeek(6); // день недели
    // 1 - MON - Понедельник
    // 2 - TUE - Вторник
    // 3 - WED - Среда
    // 4 - THU - Четверг
    // 5 - FRI - Пяница
    // 6 - SAT - Суббота
    // 7 - SUN - Воскресенье
    clock.setTime(); // запись времени
    oled.init(); // инициализация дисплея
    pinMode(chargerPin, INPUT);
    pinMode(chargerPin_done, INPUT);
    pinMode(vibrationPin, OUTPUT);
    pinMode(batteryPin, INPUT);
    tmr_charging.setPeriodMode();
    tmr_screen.setTimerMode();
    tmr_menu.setTimerMode();
    tmr_stopwatch.setTimerMode();
    tmr_alarm.setPeriodMode();
    if (EEPROM.read(INIT_ADDR) != INIT_KEY) { // первый запуск
        EEPROM.write(INIT_ADDR, INIT_KEY); // записали ключ
        EEPROM.put(0, language);
        EEPROM.put(1, battery_setting);
        EEPROM.put(3, alarm_status);
        EEPROM.put(4, alarm_hour);
        EEPROM.put(5, alarm_minute);
    }
    EEPROM.get(0, language); // прочитали
    EEPROM.get(1, battery_setting);
    EEPROM.get(3, alarm_status);
    EEPROM.get(4, alarm_hour);
    EEPROM.get(5, alarm_minute);
}

void loop() {
    if ((tmr_screen.tick()) && (screen_status == true) ) {
        screen_status = false;
        oled.clear();
        oled.update();
        oled.setPower(screen_status);
    }
    btn_up.tick();
    btn_down.tick();
    // клик по любой из кнопке - включает экран
    if (((btn_up.click()) || (btn_down.click())) && (screen_status == false)) {
        screen_status = true;
        oled.setPower(screen_status);
        tmr_screen.start();
    }
    if (screen_status == true) {
        battery_check();
        main_screen();
    }
    // удержание верхней кнопки включает будильник
    if ((btn_up.held()) && (screen_status == true)) {
        if (alarm_status == true) {
            alarm_status = false;
            oled.clear(120 ,1, 127, 8);
        } else {
            if ((alarm_hour == 24) && (alarm_minute == 60)) {
                oled.clear();
                oled.setCursor(0, 3);
                oled.setScale(1);
                oled.autoPrintln(true);
                if (language == 1) {
                    oled.print("Set the alarm time from the menu");
                } else if (language == 2) {
                    oled.print("Встановiть час будильника з меню");
                } else {
                    oled.print("Установите время будильника из меню");
                }
                oled.autoPrintln(false);
                oled.update();
                delay(3000);
                oled.clear();
                oled.update();
            } else {
                alarm_status = true;
            }
            tmr_screen.start();
        }
    }
    // удержание нижней кнопки открывает меню
    if ((btn_down.held()) && (screen_status == true)) {
        menu();
    }
    if (alarm_status == true) {
        alarm_check();
    }
}

void main_screen() {
    clock.getTime();
    //oled.clear(); // очистка дисплея
    oled.setScale(3); // масштаб текста (1..4)
    //oled.home(); // курсор в 0,0
    oled.setCursor(16, 2);
    if (clock.hour < 10) {
        oled.print("0");
    }
    oled.print(clock.hour, DEC);
    oled.print(":");
    if (clock.minute < 10) {
        oled.print("0");
        }
    oled.print(clock.minute, DEC);
    //oled.print(":");
    //oled.print(clock.second, DEC);
    oled.setCursor(32, 5);
    oled.setScale(1);
    oled.print(clock.dayOfMonth, DEC);
    oled.print(".");
    oled.print(clock.month, DEC);
    oled.print("  ");
    //oled.print(".");
    //oled.print(clock.year + 2000, DEC);
    oled.setCursor(75, 5);
    //oled.setScale(1);
    //Serial.println(clock.dayOfWeek);
    // Незнаю почему, но либа спешит на один день по дням недели, исправляю костылем
    uint8_t dayOfWeek = clock.dayOfWeek - 1;
    if (dayOfWeek == 0) dayOfWeek = 7;
    switch (dayOfWeek) { // День недели
        case 1:
            if (language == 1) {
                oled.print("MON");
            } else {
                oled.print("ПНД");
            }
            break;
        case 2:
            if (language == 1) {
                oled.print("TUE");
            } else {
                oled.print("BTP"); 
            }
            break;
        case 3:
            if (language == 1) {
                oled.print("WED");
            } else {
                oled.print("СРД");
            }
            break;
        case 4:
            if (language == 1) {
                oled.print("THU");
            } else {
                oled.print("ЧТВ");
            }
            break;
        case 5:
            if (language == 1) {
                oled.print("FRI");
            } else {
                oled.print("ПТН");
            }
            break;
        case 6:
            if (language == 1) {
                oled.print("SAT");
            } else {
                oled.print("СБТ");
            }
            break;
        case 7:
            if (language == 1) {
                oled.print("SUN");
            } else if (language == 2) {
                oled.print("НДЛ");
            } else {
                oled.print("BCK");
            }
            break;
    }
    if (alarm_status == true) {
        //oled.setCursorXY(1, 1);
        oled.drawBitmap(120, 1, alarm_7x8, 7, 8, BITMAP_NORMAL, BUF_ADD);  
    }
    charging = digitalRead(chargerPin);
    charging_done = digitalRead(chargerPin_done);
    //Serial.print("Зарядка идет - ");
    //Serial.println(charging);
    //Serial.print("Зарядка закончилась - ");
    //Serial.println(charging_done);
    if ((charging == true) && (charging_done == false)) { // Идет зарядка
        if ((tmr_charging.tick())) {
            switch (charging_animation) {
                case 0:
                    //oled.clear(99, 1, 103, 12);
                    oled.drawBitmap(1, 1, bat25_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);
                    //oled.update(100, 1, 126, 11); 
                    charging_animation += 1;
                    break;
                case 1:
                    oled.drawBitmap(1, 1, bat50_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);
                    //oled.update(100, 1, 126, 11);
                    charging_animation += 1;
                    break;
                case 2:
                    oled.drawBitmap(1, 1, bat75_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);
                    //oled.update(100, 1, 126, 11);
                    charging_animation += 1;
                    break;
                    case 3:
                    oled.drawBitmap(1, 1, bat100_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);
                    //oled.update(100, 1, 126, 11);
                    charging_animation = 0;
                    break;
                } 
              
        }
    } 
    if ((charging == true) && (charging_done == true)) { // Зарядка закончилась
        oled.drawBitmap(1, 1, bat_done_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);
    } 
    if ((charging == false) && (charging_done == false)) { // Не заряжается
        if (charging_animation > 0) {
            charging_animation = 0;
        }
        //oled.clear(97, 1, 103, 12);
        if (battery > 75) {
            oled.drawBitmap(1, 1, bat100_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);  
        } else if (battery > 50) {
            oled.drawBitmap(1, 1, bat75_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD); 
        } else if (battery > 25) {
            oled.drawBitmap(1, 1, bat50_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);
        } else if (battery >= 0) {
            oled.drawBitmap(1, 1, bat25_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);
        }
    }
    if (battery_setting == true) {
        oled.setCursorXY(10, 3);
        oled.setScale(1);
        oled.print(battery);
        oled.print("%   ");
    }
    oled.update();
    //t = t+1;
    //Serial.println(t);
}

void menu() {
    tmr_menu.start();
    oled.clear();
    uint8_t menu_choice = 1;
    bool back = false; // костыль, который появился из-за появившегося бага в пункте будильника, почему-то стало игнорировать таймер выхода, либо не полностью выходить из циклов
    while (true) {
        btn_up.tick();
        btn_down.tick();
        //Serial.print("таймер - ");
        //Serial.println(tmr_menu.tick());
        if ((tmr_menu.tick()) || (back == true)) {
            oled.clear();
            oled.update();
            break;
        }
        if (btn_up.click()) {
            tmr_menu.start();
            menu_choice -= 1;
            oled.clear();
            if (menu_choice < 1 ) {
                menu_choice = 5;
            }
        }
        if (btn_down.click()) {
            tmr_menu.start();
            menu_choice += 1;
            oled.clear();
            if (menu_choice > 5) {
                menu_choice = 1;
            }
        }
        if (btn_up.held()) {
            oled.clear();
            tmr_screen.start();
            break;
        }
        switch (menu_choice) {
            case 1: // Фонарик
                oled.setCursor(55, 0);
                oled.setScale(1);
                if (language == 1) {
                    oled.print("Menu");
                    oled.setCursor(0, 1);
                    oled.setScale(2);
                    oled.print("Flashlight");
                    oled.setScale(1);
                    oled.setCursor(0, 3);
                    oled.print("Stopwatch");
                    oled.setCursor(0, 4);
                    oled.print("Timer");
                    oled.setCursor(0, 5);
                    oled.print("Alarm");
                    oled.setCursor(0, 6);
                    oled.print("Settings");
                } else if (language == 2) {
                    oled.print("Меню");
                    oled.setCursor(0, 1);
                    oled.setScale(2);
                    oled.print("Лiхтарик");
                    oled.setScale(1);
                    oled.setCursor(0, 3);
                    oled.print("Секундомiр");
                    oled.setCursor(0, 4);
                    oled.print("Таймер");
                    oled.setCursor(0, 5);
                    oled.print("Будильник");
                    oled.setCursor(0, 6);
                    oled.print("Налаштування");
                } else {
                    oled.print("Меню");
                    oled.setCursor(0, 1);
                    oled.setScale(2);
                    oled.print("Фонарик");
                    oled.setScale(1);
                    oled.setCursor(0, 3);
                    oled.print("Секундомер");
                    oled.setCursor(0, 4);
                    oled.print("Таймер");
                    oled.setCursor(0, 5);
                    oled.print("Будильник");
                    oled.setCursor(0, 6);
                    oled.print("Настройки");
                }
                if (btn_down.held()) {
                    oled.clear();
                    bool light = true;
                    long done = 1800000; // сек до отключения
                    while (true) {
                        btn_up.tick();
                        btn_down.tick();
                        if ((btn_up.click()) || (btn_down.click()) || (btn_up.held()) || (btn_down.held()) || (done == 0)) {
                            oled.clear();
                            tmr_menu.start();
                            break;
                        }
                        if (tmr_alarm.tick()) {
                            done -= 1;
                        }
                        if (light == true) {
                            oled.rect(0, 0, 128, 64);
                            light = false;
                            oled.update();
                        }
                    }
                }
                oled.update();
                break;
            case 2: // Секундомер
                if (language == 1) {
                    oled.setCursor(55, 0);
                    oled.setScale(1);
                    oled.print("Menu");
                    oled.setCursor(0, 1);
                    oled.print("Flashlight");
                    oled.setCursor(0, 2);
                    oled.setScale(2);
                    oled.print("Stopwatch");
                    oled.setScale(1);
                    oled.setCursor(0, 4);
                    oled.print("Timer");
                    oled.setCursor(0, 5);
                    oled.print("Alarm");
                    oled.setCursor(0, 6);
                    oled.print("Settings");
                } else if (language == 2) {
                    oled.setCursor(55, 0);
                    oled.setScale(1);
                    oled.print("Меню");
                    oled.setCursor(0, 1);
                    oled.print("Лiхтарик");
                    oled.setCursor(0, 2);
                    oled.setScale(2);
                    oled.print("Секундомiр");
                    oled.setScale(1);
                    oled.setCursor(0, 4);
                    oled.print("Таймер");
                    oled.setCursor(0, 5);
                    oled.print("Будильник");
                    oled.setCursor(0, 6);
                    oled.print("Налаштування");
                } else {
                    oled.setCursor(55, 0);
                    oled.setScale(1);
                    oled.print("Меню");
                    oled.setCursor(0, 1);
                    oled.print("Фонарик");
                    oled.setCursor(0, 2);
                    oled.setScale(2);
                    oled.print("Секундомер");
                    oled.setScale(1);
                    oled.setCursor(0, 4);
                    oled.print("Таймер");
                    oled.setCursor(0, 5);
                    oled.print("Будильник");
                    oled.setCursor(0, 6);
                    oled.print("Настройки");
                }
                if (btn_down.held()) {
                    oled.clear();
                    bool stopwatch_status = false;
                    int hour = 0, minute = 0, second = 0, second_real, difference; 
                    oled.setScale(3);
                    tmr_stopwatch.start();
                    while (true) {
                        btn_up.tick();
                        btn_down.tick();
                        if ((tmr_stopwatch.tick()) && (stopwatch_status == false)) {
                            break;
                        }
                        if ((btn_up.click()) || (btn_down.click())) {
                            tmr_stopwatch.start();
                            if (stopwatch_status == false) {
                                stopwatch_status = true;
                                clock.getTime();
                                second_real = clock.second;
                            } else {
                                stopwatch_status = false;
                            }
                        }
                        if (btn_up.held()) {
                            tmr_menu.start();
                            oled.clear();
                            break;
                        }
                        if (btn_down.held()) {
                            if (stopwatch_status == true) {
                                stopwatch_status = false;
                                tmr_stopwatch.start();
                            }
                            hour = 0;
                            minute = 0;
                            second = 0;
                            oled.clear();
                        }
                        if (stopwatch_status == true) {
                            clock.getTime();
                            difference = second_real - clock.second;
                            if (difference !=  0) {
                                second += 1;
                            }
                            second_real = clock.second;
                            if (second > 59) {
                                minute += 1;
                                second = 0;
                                oled.clear();
                            }
                            if (minute > 59) {
                                hour += 1;
                                minute = 0;
                                oled.clear();
                            }
                            if (hour > 9) {
                                oled.clear();
                                break;
                            }
                        }
                        oled.setCursor(1, 2);
                        oled.print(hour, DEC);
                        oled.print(":");
                        oled.print(minute, DEC);
                        oled.print(":");
                        oled.print(second, DEC);
                        oled.update();
                    }
                }
                oled.update();
                break;
            case 3: // Таймер
                if (language == 1) {
                    oled.setCursor(55, 0);
                    oled.setScale(1);
                    oled.print("Menu");
                    oled.setCursor(0, 1);
                    oled.print("Flashlight");
                    oled.setCursor(0, 2);
                    oled.print("Stopwatch");
                    oled.setCursor(0, 3);
                    oled.setScale(2);
                    oled.print("Timer");
                    oled.setScale(1);
                    oled.setCursor(0, 5);
                    oled.print("Alarm");
                    oled.setCursor(0, 6);
                    oled.print("Settings");
                } else if (language == 2) {
                    oled.setCursor(55, 0);
                    oled.setScale(1);
                    oled.print("Меню");
                    oled.setCursor(0, 1);
                    oled.print("Лiхтарик");
                    oled.setCursor(0, 2);
                    oled.print("Секундомiр");
                    oled.setCursor(0, 3);
                    oled.setScale(2);
                    oled.print("Таймер");
                    oled.setScale(1);
                    oled.setCursor(0, 5);
                    oled.print("Будильник");
                    oled.setCursor(0, 6);
                    oled.print("Налаштування");
                } else {
                    oled.setCursor(55, 0);
                    oled.setScale(1);
                    oled.print("Меню");
                    oled.setCursor(0, 1);
                    oled.print("Фонарик");
                    oled.setCursor(0, 2);
                    oled.print("Секундомер");
                    oled.setCursor(0, 3);
                    oled.setScale(2);
                    oled.print("Таймер");
                    oled.setScale(1);
                    oled.setCursor(0, 5);
                    oled.print("Будильник");
                    oled.setCursor(0, 6);
                    oled.print("Настройки");
                }
                if (btn_down.held()) {
                    oled.clear();
                    uint8_t choice = 1;
                    int minute = 0, second = 0, second_real, difference, minute_, second_; 
                    tmr_stopwatch.start();
                    while (true) {
                        btn_up.tick();
                        btn_down.tick();
                        oled.setScale(3);
                        if (tmr_stopwatch.tick()) {
                            break;
                        }
                        if (btn_up.held()) {
                            tmr_menu.start();
                            oled.clear();
                            break;
                        }
                        if (btn_down.held()) {
                            tmr_stopwatch.tick();
                            oled.clear();
                            choice += 1;
                        }
                        if (choice > 3) {
                            choice = 1;
                        }
                        if (choice < 1) {
                            choice = 3;
                        }
                        switch (choice) {
                            case 1: // Минуты
                                if (btn_up.click()) {
                                    tmr_stopwatch.tick();
                                    minute += 1;
                                }
                                if (btn_down.click()) {
                                    tmr_stopwatch.tick();
                                    minute -= 1;
                                }
                                if (minute > 60) {
                                    minute = 0;
                                }
                                if (minute < 0) {
                                    minute = 60;
                                }
                                oled.setCursor(1, 5);
                                oled.print("--");
                                break;
                            case 2: // Секунды
                                if (btn_up.click()) {
                                    tmr_stopwatch.tick();
                                    second += 1;
                                }
                                if (btn_down.click()) {
                                    tmr_stopwatch.tick();
                                    second -= 1;
                                }
                                if (second > 59) {
                                    second = 0;
                                }
                                if (second < 0) {
                                    second = 59;
                                }
                                oled.setCursor(1, 5);
                                oled.print("   --");
                                break;
                            case 3: // Старт
                                if (((minute > 0) || (second > 0)) && ((btn_up.click()) || (btn_down.click()))) {
                                    minute_ = minute;
                                    if (minute == 0) {
                                        second_ = second + 1;
                                    } else {
                                        second_ = second;
                                    }
                                    uint8_t timer_done = 30; // сек до завершения
                                    while (true) {
                                        btn_up.tick();
                                        btn_down.tick();
                                        if ((btn_up.held()) || (btn_down.held()) || (timer_done == 0)) {
                                            digitalWrite(vibrationPin, LOW);
                                            second_real = 0;
                                            tmr_stopwatch.tick();
                                            oled.clear();
                                            break;
                                        }
                                        clock.getTime();
                                        difference = second_real - clock.second;
                                        if (difference !=  0) {
                                            second_ -= 1;
                                        }
                                        second_real = clock.second;
                                        if (second_ < 0) {
                                            minute_ -= 1;
                                            if (minute_ < 0) {
                                                second_ = 0;
                                            } else {
                                                second_ = 59;
                                            }
                                        }
                                        if (minute_ < 0) {
                                            if (tmr_alarm.tick()) {
                                                digitalWrite(vibrationPin, HIGH);
                                                timer_done -= 1;
                                            } else {
                                                digitalWrite(vibrationPin, LOW);
                                            }
                                        }
                                        oled.setScale(3); 
                                        oled.setCursor(1, 2);
                                        if (minute_ < 10) {
                                            oled.print("0");
                                        }
                                        if (minute_ < 0) {
                                            oled.print("0");
                                        } else {
                                            oled.print(minute_, DEC);
                                        }
                                        oled.print(":");
                                        if (second_ < 10) {
                                            oled.print("0");
                                        }
                                        oled.print(second_, DEC);
                                        oled.setScale(1);
                                        oled.setCursor(88, 3);
                                        if (language == 1) {
                                            oled.print(" Stop ");
                                        } else {
                                            oled.print(" Стоп ");
                                        }
                                        oled.setCursor(88, 4);
                                        oled.print(" ---- ");
                                        oled.update();
                                    }
                                }
                                oled.setScale(1);
                                oled.setCursor(88, 4);
                                oled.print(" -----");
                                oled.setScale(3);
                                break;
                        }
                        oled.setCursor(1, 2);
                        if (minute < 10) {
                            oled.print("0");
                        }
                        oled.print(minute, DEC);
                        oled.print(":");
                        if (second < 10) {
                            oled.print("0");
                        }
                        oled.print(second, DEC);
                        oled.setScale(1);
                        oled.setCursor(88, 3);
                        if (language == 1) {
                            oled.print(" Start");
                        } else {
                            oled.print(" Старт");
                        }
                        oled.update();
                    }
                }
                oled.update();
                break;
            case 4: // Будильник
                if (language == 1) {
                    oled.setCursor(55, 0);
                    oled.setScale(1);
                    oled.print("Menu");
                    oled.setCursor(0, 1);
                    oled.print("Flashlight");
                    oled.setCursor(0, 2);
                    oled.print("Stopwatch");
                    oled.setCursor(0, 3);
                    oled.print("Timer");
                    oled.setCursor(0, 4);
                    oled.setScale(2);
                    oled.print("Alarm");
                    oled.setCursor(0, 6);
                    oled.setScale(1);
                    oled.print("Settings");
                } else if (language == 2) {
                    oled.setCursor(55, 0);
                    oled.setScale(1);
                    oled.print("Меню");
                    oled.setCursor(0, 1);
                    oled.print("Лiхтарик");
                    oled.setCursor(0, 2);
                    oled.print("Секундомiр");
                    oled.setCursor(0, 3);
                    oled.print("Таймер");
                    oled.setCursor(0, 4);
                    oled.setScale(2);
                    oled.print("Будильник");
                    oled.setCursor(0, 6);
                    oled.setScale(1);
                    oled.print("Налаштування");
                } else {
                    oled.setCursor(55, 0);
                    oled.setScale(1);
                    oled.print("Меню");
                    oled.setCursor(0, 1);
                    oled.print("Фонарик");
                    oled.setCursor(0, 2);
                    oled.print("Секундомер");
                    oled.setCursor(0, 3);
                    oled.print("Таймер");
                    oled.setCursor(0, 4);
                    oled.setScale(2);
                    oled.print("Будильник");
                    oled.setCursor(0, 6);
                    oled.setScale(1);
                    oled.print("Настройки");
                }
                if (btn_down.held()) {
                    tmr_menu.start();
                    oled.clear();
                    oled.setScale(3);
                    oled.setCursor(16, 2);
                    int alarm_hour_, alarm_minute_;
                    bool choice = true;
                    if ((alarm_hour > 23) || (alarm_minute > 59)) {
                        clock.getTime();
                        alarm_hour_ = clock.hour;
                        alarm_minute_ = clock.minute;
                    } else {
                        alarm_hour_ = alarm_hour;
                        alarm_minute_ = alarm_minute;
                    }
                    while (true) {
                        if ((tmr_menu.tick()) || (back == true)) {
                            oled.clear();
                            oled.update();
                            back = true;
                            //tmr_menu.force();
                            break;
                        }
                        btn_up.tick();
                        btn_down.tick();
                        if ((btn_up.click()) || (btn_down.click())) {
                            tmr_menu.start();
                            if (choice == true) {
                                choice = false;
                            } else {
                                choice = true;
                            }
                            oled.clear();
                        }
                        if (btn_up.held()) {
                            oled.clear();
                            tmr_menu.start();
                            break;
                        }
                        oled.setCursor(3, 0);
                        oled.setScale(1);
                        if (language == 1) {
                            oled.print("Setting the alarm clock");
                        } else {
                            oled.print("Установка будильника");
                        }
                        switch (choice) {
                            case true: // статус будильника
                                if (btn_down.held()) {
                                    if (alarm_status == true) {
                                        alarm_status = false;
                                    } else {
                                        alarm_status = true;
                                    }
                                    EEPROM.put(3, alarm_status);
                                    tmr_menu.start();
                                }
                                oled.setScale(2);
                                oled.setCursor(0, 1);
                                if (alarm_status == true) {
                                    if (language == 1) {
                                        oled.print("ON  ");
                                    } else {
                                        oled.print("ВКЛ ");
                                    }
                                } else {
                                    if (language == 1) {
                                        oled.print("OFF ");
                                    } else if (language == 2) {
                                        oled.print("ВИКЛ");
                                    } else {
                                        oled.print("ВЫКЛ");
                                    }
                                }
                                oled.setScale(1);
                                oled.setCursor(0, 3);
                                if (alarm_hour_ < 10) {
                                    oled.print("0");
                                }
                                oled.print(alarm_hour_);
                                oled.print(":");
                                if (alarm_minute_ < 10) {
                                    oled.print("0");
                                }
                                oled.print(alarm_minute_);
                                break;
                            case false: // установка будильника
                                oled.setScale(1);
                                oled.setCursor(0, 1);
                                if (alarm_status == true) {
                                    if (language == 1) {
                                        oled.print("ON  ");
                                    } else {
                                        oled.print("ВКЛ ");
                                    }
                                } else {
                                    if (language == 1) {
                                        oled.print("OFF ");
                                    } else if (language == 2) {
                                        oled.print("ВИКЛ");
                                    } else {
                                        oled.print("ВЫКЛ");
                                    }
                                }
                                oled.setScale(2);
                                oled.setCursor(0, 2);
                                if (alarm_hour_ < 10) {
                                    oled.print("0");
                                }
                                oled.print(alarm_hour_);
                                oled.print(":");
                                if (alarm_minute_ < 10) {
                                    oled.print("0");
                                }
                                oled.print(alarm_minute_);
                                if (btn_down.held()) {
                                    tmr_menu.start();
                                    oled.clear();
                                    oled.setCursor(3, 0);
                                    oled.setScale(1);
                                    oled.print("Установка будильника");
                                    oled.setScale(1);
                                    oled.setCursor(0, 1);
                                    if (alarm_status == true) {
                                        if (language == 1) {
                                            oled.print("ON  ");
                                        } else {
                                            oled.print("ВКЛ ");
                                        }
                                    } else {
                                        if (language == 1) {
                                            oled.print("OFF ");
                                        } else if (language == 2) {
                                            oled.print("ВИКЛ");
                                        } else {
                                            oled.print("ВЫКЛ");
                                        }
                                    }
                                    bool choice_ = true;
                                    while (true) {
                                        if (tmr_menu.tick()) {
                                            oled.clear();
                                            oled.update();
                                            back = true;
                                            break;
                                        }
                                        btn_up.tick();
                                        btn_down.tick();
                                        if (btn_up.held()) {
                                            oled.clear();
                                            alarm_hour = alarm_hour_;
                                            alarm_minute = alarm_minute_;
                                            EEPROM.put(4, alarm_hour);
                                            EEPROM.put(5, alarm_minute);
                                            tmr_menu.start();
                                            break;
                                        }
                                        if (btn_down.held()) {
                                            if (choice_ == true) {
                                                choice_ = false;
                                            } else {
                                                choice_ = true;
                                            }
                                            tmr_menu.start();
                                        }
                                        oled.setScale(2);
                                        oled.setCursor(30, 2);
                                        if (alarm_hour_ < 10) {
                                            oled.print("0");
                                        }
                                        oled.print(alarm_hour_);
                                        oled.print(":");
                                        if (alarm_minute_ < 10) {
                                            oled.print("0");
                                        }
                                        oled.print(alarm_minute_);
                                        oled.setScale(2);
                                        oled.setCursor(30, 4);
                                        switch (choice_) {
                                            case true:
                                                if (btn_up.click()) {
                                                    tmr_menu.start();
                                                    alarm_hour_ += 1;
                                                }
                                                if (btn_down.click()) {
                                                    tmr_menu.start();
                                                    alarm_hour_ -= 1;
                                                }
                                                if (alarm_hour_ > 23) {
                                                    alarm_hour_ = 0;
                                                }
                                                if (alarm_hour_ < 0) {
                                                    alarm_hour_ = 23;
                                                }
                                                oled.print("--   ");
                                                break;
                                            case false:
                                                if (btn_up.click()) {
                                                    tmr_menu.start();
                                                    alarm_minute_ += 1;
                                                }
                                                if (btn_down.click()) {
                                                    tmr_menu.start();
                                                    alarm_minute_ -= 1;
                                                }
                                                if (alarm_minute_ > 59) {
                                                    alarm_minute_ = 0;
                                                }
                                                if (alarm_minute_ < 0) {
                                                    alarm_minute_ = 59;
                                                }
                                                oled.print("   --");
                                                break;
                                        }
                                        oled.update();
                                    }
                                }
                                break;
                        }
                        oled.update();
                    }
                }
                oled.update();
                break;
            case 5: // Настройки
                if (language == 1) {
                    oled.setCursor(55, 0);
                    oled.setScale(1);
                    oled.print("Menu");
                    oled.setCursor(0, 1);
                    oled.print("Flashlight");
                    oled.setCursor(0, 2);
                    oled.print("Stopwatch");
                    oled.setCursor(0, 3);
                    oled.print("Timer");
                    oled.setCursor(0, 4);
                    oled.print("Alarm");
                    oled.setCursor(0, 5);
                    oled.setScale(2);
                    oled.print("Settings");
                    oled.setScale(1);
                } else if (language == 2) {
                    oled.setCursor(55, 0);
                    oled.setScale(1);
                    oled.print("Меню");
                    oled.setCursor(0, 1);
                    oled.print("Лiхтарик");
                    oled.setCursor(0, 2);
                    oled.print("Секундомiр");
                    oled.setCursor(0, 3);
                    oled.print("Таймер");
                    oled.setCursor(0, 4);
                    oled.print("Будильник");
                    oled.setCursor(0, 5);
                    oled.setScale(2);
                    oled.print("Налаштування");
                    oled.setScale(1);
                } else {
                    oled.setCursor(55, 0);
                    oled.setScale(1);
                    oled.print("Меню");
                    oled.setCursor(0, 1);
                    oled.print("Фонарик");
                    oled.setCursor(0, 2);
                    oled.print("Секундомер");
                    oled.setCursor(0, 3);
                    oled.print("Таймер");
                    oled.setCursor(0, 4);
                    oled.print("Будильник");
                    oled.setCursor(0, 5);
                    oled.setScale(2);
                    oled.print("Настройки");
                    oled.setScale(1);
                }
                if (btn_down.held()) {
                    oled.clear();
                    tmr_screen.start();
                    uint8_t choice = 1;
                    while (true) {
                        if ((tmr_menu.tick()) || (back == true)) {
                            oled.clear();
                            oled.update();
                            back = true;
                            break;
                        }
                        btn_up.tick();
                        btn_down.tick();
                        if (btn_up.click()) {
                            tmr_menu.start();
                            choice -= 1;
                            oled.clear();
                            if (choice < 1 ) {
                                choice = 4;
                            }
                        }
                        if (btn_down.click()) {
                            tmr_menu.start();
                            choice += 1;
                            oled.clear();
                            if (choice > 4 ) {
                                choice = 1;
                            }
                        }
                        if (btn_up.held()) {
                            oled.clear();
                            tmr_menu.start();
                            break;
                        }
                        switch (choice) {
                            case 1: // Язык
                                if (language == 1) {
                                    oled.setCursor(40, 0);
                                    oled.setScale(1);
                                    oled.print("Settings");
                                    oled.setCursor(0, 1);
                                    oled.setScale(2);
                                    oled.print("Language");
                                    oled.setCursor(0, 3);
                                    oled.setScale(1);
                                    oled.print("Data/time");
                                    oled.setCursor(0, 4);
                                    oled.print("Battery");
                                    oled.setCursor(0, 5);
                                    oled.print("Information");
                                } else if (language == 2) {
                                    oled.setCursor(40, 0);
                                    oled.setScale(1);
                                    oled.print("Налаштування");
                                    oled.setCursor(0, 1);
                                    oled.setScale(2);
                                    oled.print("Мова");
                                    oled.setCursor(0, 3);
                                    oled.setScale(1);
                                    oled.print("Дата/час");
                                    oled.setCursor(0, 4);
                                    oled.print("Батарея");
                                    oled.setCursor(0, 5);
                                    oled.print("Iнформацiя");
                                } else {
                                    oled.setCursor(40, 0);
                                    oled.setScale(1);
                                    oled.print("Настройки");
                                    oled.setCursor(0, 1);
                                    oled.setScale(2);
                                    oled.print("Язык");
                                    oled.setCursor(0, 3);
                                    oled.setScale(1);
                                    oled.print("Дата/время");
                                    oled.setCursor(0, 4);
                                    oled.print("Батарея");
                                    oled.setCursor(0, 5);
                                    oled.print("Информация");
                                }
                                if (btn_down.held()) {
                                    oled.clear();
                                    tmr_screen.start();
                                    uint8_t choice_ = 1;
                                    while (true) {
                                        if ((tmr_menu.tick()) || (back == true)) {
                                            oled.clear();
                                            oled.update();
                                            back = true;
                                            break;
                                        }
                                        btn_up.tick();
                                        btn_down.tick();
                                        if (btn_up.click()) {
                                            tmr_menu.start();
                                            oled.clear();
                                            choice_ -= 1;
                                            if (choice_ < 1 ) {
                                                choice_ = 3;
                                            }
                                        }
                                        if (btn_down.click()) {
                                            tmr_menu.start();
                                            oled.clear();
                                            choice_ += 1;
                                            if (choice_ > 3 ) {
                                                choice_ = 1;
                                            }
                                        }
                                        if (btn_up.held()) {
                                            oled.clear();
                                            tmr_menu.start();
                                            break;
                                        }
                                        oled.setCursor(40, 0);
                                        oled.setScale(1);
                                        if (language == 1) {
                                            oled.print("Language");
                                        } else if (language == 2) {
                                            oled.print("Мова");
                                        } else {
                                            oled.print("Язык");
                                        }
                                        oled.setCursor(0, 1);
                                        switch (choice_) {
                                            case 1: // English
                                                oled.setCursor(0, 1);
                                                oled.setScale(2);
                                                oled.print("English");
                                                oled.setCursor(0, 3);
                                                oled.setScale(1);
                                                oled.print("Ukrainian");
                                                oled.setCursor(0, 4);
                                                oled.print("Russian");
                                                if (btn_down.held()) {
                                                    oled.clear();
                                                    tmr_menu.start();
                                                    language = choice_;
                                                    EEPROM.put(0, language);
                                                }
                                                break;
                                            case 2: // Українська
                                                oled.setCursor(0, 1);
                                                oled.setScale(1);
                                                oled.print("English");
                                                oled.setCursor(0, 2);
                                                oled.setScale(2);
                                                oled.print("Ukrainian");
                                                oled.setCursor(0, 4);
                                                oled.setScale(1);
                                                oled.print("Russian");
                                                if (btn_down.held()) {
                                                    oled.clear();
                                                    tmr_menu.start();
                                                    language = choice_;
                                                    EEPROM.put(0, language);
                                                }
                                                break;
                                            case 3: // Русский
                                                oled.setCursor(0, 1);
                                                oled.setScale(1);
                                                oled.print("English");
                                                oled.setCursor(0, 2);
                                                oled.print("Ukrainian");
                                                oled.setCursor(0, 3);
                                                oled.setScale(2);
                                                oled.print("Russian");
                                                if (btn_down.held()) {
                                                    oled.clear();
                                                    tmr_menu.start();
                                                    language = choice_;
                                                    EEPROM.put(0, language);
                                                }
                                                break;
                                        }
                                        oled.update();
                                    }
                                }
                                break;
                            case 2: // Дата/время
                                if (language == 1) {
                                    oled.setCursor(40, 0);
                                    oled.setScale(1);
                                    oled.print("Settings");
                                    oled.setCursor(0, 1);
                                    oled.print("Language");
                                    oled.setCursor(0, 2);
                                    oled.setScale(2);
                                    oled.print("Data/time");
                                    oled.setCursor(0, 4);
                                    oled.setScale(1);
                                    oled.print("Battery");
                                    oled.setCursor(0, 5);
                                    oled.print("Information");
                                } else if (language == 2) {
                                    oled.setCursor(40, 0);
                                    oled.setScale(1);
                                    oled.print("Налаштування");
                                    oled.setCursor(0, 1);
                                    oled.print("Мова");
                                    oled.setCursor(0, 2);
                                    oled.setScale(2);
                                    oled.print("Дата/час");
                                    oled.setCursor(0, 4);
                                    oled.setScale(1);
                                    oled.print("Батарея");
                                    oled.setCursor(0, 5);
                                    oled.print("Iнформацiя");
                                } else {
                                    oled.setCursor(40, 0);
                                    oled.setScale(1);
                                    oled.print("Настройки");
                                    oled.setCursor(0, 1);
                                    oled.print("Язык");
                                    oled.setCursor(0, 2);
                                    oled.setScale(2);
                                    oled.print("Дата/время");
                                    oled.setCursor(0, 4);
                                    oled.setScale(1);
                                    oled.print("Батарея");
                                    oled.setCursor(0, 5);
                                    oled.print("Информация");  
                                }
                                if (btn_down.held()) {
                                    oled.clear();
                                    tmr_menu.start();
                                    uint8_t choice_ = 1;
                                    int day, month, year, hour, minute;
                                    clock.getTime();
                                    day = clock.dayOfMonth;
                                    month = clock.month;
                                    year = clock.year + 2000;
                                    hour = clock.hour;
                                    minute = clock.minute;
                                    while (true) {
                                        if ((tmr_menu.tick()) || (back == true)) {
                                            oled.clear();
                                            oled.update();
                                            back = true;
                                            break;
                                        }
                                        btn_up.tick();
                                        btn_down.tick();
                                        if (btn_up.click()) {
                                            tmr_menu.start();
                                            oled.clear();
                                            choice_ -= 1;
                                            if (choice_ < 1 ) {
                                                choice_ = 3;
                                            }
                                        }
                                        if (btn_down.click()) {
                                            tmr_menu.start();
                                            oled.clear();
                                            choice_ += 1;
                                            if (choice_ > 3 ) {
                                                choice_ = 1;
                                            }
                                        }
                                        if (btn_up.held()) {
                                            oled.clear();
                                            tmr_menu.start();
                                            break;
                                        }
                                        oled.setCursor(40, 0);
                                        oled.setScale(1);
                                        if (language == 1) {
                                            oled.print("Data/time");
                                        } else if (language == 2) {
                                            oled.print("Дата/час");
                                        } else {
                                            oled.print("Дата/время");
                                        }
                                        oled.setCursor(0, 1);
                                        switch (choice_) {
                                            case 1: // Дата
                                                oled.setScale(2);
                                                oled.print(day);
                                                oled.print(".");
                                                oled.print(month);
                                                oled.print(".");
                                                oled.print(year);
                                                oled.setCursor(0, 3);
                                                oled.setScale(1);
                                                if (hour < 10) {
                                                    oled.print("0");
                                                }
                                                oled.print(hour);
                                                oled.print(":");
                                                if (minute < 10) {
                                                    oled.print("0");
                                                    }
                                                oled.print(minute);
                                                oled.setCursor(0, 4);
                                                if (language == 1) {
                                                    oled.print("Install");
                                                } else if (language == 2) {
                                                    oled.print("Встановити");
                                                } else {
                                                    oled.print("Установить");
                                                }
                                                if (btn_down.held()) {
                                                    oled.clear();
                                                    tmr_menu.start();
                                                    uint8_t choice__ = 1;
                                                    while (true) {
                                                        if ((tmr_menu.tick()) || (back == true)) {
                                                            oled.clear();
                                                            oled.update();
                                                            back = true;
                                                            break;
                                                        }
                                                        btn_up.tick();
                                                        btn_down.tick();
                                                        if (btn_up.held()) {
                                                            oled.clear();
                                                            tmr_menu.start();
                                                            break;
                                                        }
                                                        if (btn_down.held()) {
                                                            //oled.clear();
                                                            tmr_menu.start();
                                                            choice__ += 1;
                                                            if (choice__ > 3) {
                                                                choice__ = 1;
                                                            }
                                                            if (choice__ < 1) {
                                                                choice__ = 3;
                                                            }
                                                        }
                                                        oled.setCursor(40, 0);
                                                        oled.setScale(1);
                                                        if (language == 1) {
                                                            oled.print("Data");
                                                        } else {
                                                            oled.print("Дата");
                                                        }
                                                        oled.setCursor(0, 1);
                                                        oled.setScale(2);
                                                        if (day < 10) {
                                                            oled.print(" ");
                                                        }
                                                        oled.print(day);
                                                        oled.print(".");
                                                        if (month < 10) {
                                                            oled.print(" ");
                                                        }
                                                        oled.print(month);
                                                        oled.print(".");
                                                        if (year < 1000) {
                                                            oled.print(" ");
                                                        } else if (year < 100) {
                                                            oled.print("  ");
                                                        } else if (year < 10) {
                                                            oled.print("   ");
                                                        }
                                                        oled.print(year);
                                                        oled.print(" ");
                                                        oled.setCursor(0, 3);
                                                        switch (choice__) {
                                                            case 1: // Дата
                                                                if (day < 10) {
                                                                    oled.print(" -        ");
                                                                } else {
                                                                    oled.print("--        ");
                                                                }
                                                                if (btn_up.click()) {
                                                                    tmr_menu.start();
                                                                    day += 1;
                                                                    if (day > 31 ) {
                                                                        day = 1;
                                                                    }
                                                                }
                                                                if (btn_down.click()) {
                                                                    tmr_menu.start();
                                                                    day -= 1;
                                                                    if (day < 1 ) {
                                                                        day = 31;
                                                                    }
                                                                }
                                                                break;
                                                            case 2: // Месяц
                                                                if (month < 10) {
                                                                    oled.print("    -     ");
                                                                } else {
                                                                    oled.print("   --     ");
                                                                }
                                                                if (btn_up.click()) {
                                                                    tmr_menu.start();
                                                                    month += 1;
                                                                    if (month > 12 ) {
                                                                        month = 1;
                                                                    }
                                                                }
                                                                if (btn_down.click()) {
                                                                    tmr_menu.start();
                                                                    month -= 1;
                                                                    if (month < 1 ) {
                                                                        month = 12;
                                                                    }
                                                                }
                                                                break;
                                                            case 3: // Год
                                                                if (year > 1000) {
                                                                    oled.print("      ----");
                                                                } else if (year > 100) {
                                                                    oled.print("       ---");
                                                                } else if (year > 10) {
                                                                    oled.print("        --");
                                                                } else {
                                                                    oled.print("         -");
                                                                }
                                                                if (btn_up.click()) {
                                                                    tmr_menu.start();
                                                                    year += 1;
                                                                    if (year > 9999 ) {
                                                                        year = 1;
                                                                    }
                                                                }
                                                                if (btn_down.click()) {
                                                                    tmr_menu.start();
                                                                    year -= 1;
                                                                    if (year < 1 ) {
                                                                        year = 2023;
                                                                    }
                                                                }
                                                                break;
                                                        }
                                                        oled.update();
                                                    }
                                                }
                                                break;
                                            case 2: // Время
                                                oled.print(day);
                                                oled.print(".");
                                                oled.print(month);
                                                oled.print(".");
                                                oled.print(year);
                                                oled.setCursor(0, 2);
                                                oled.setScale(2);
                                                if (hour < 10) {
                                                    oled.print("0");
                                                }
                                                oled.print(hour);
                                                oled.print(":");
                                                if (minute < 10) {
                                                    oled.print("0");
                                                    }
                                                oled.print(minute);
                                                oled.setScale(1);
                                                oled.setCursor(0, 4);
                                                if (language == 1) {
                                                    oled.print("Install");
                                                } else if (language == 2) {
                                                    oled.print("Встановити");
                                                } else {
                                                    oled.print("Установить");
                                                }
                                                if (btn_down.held()) {
                                                    oled.clear();
                                                    tmr_menu.start();
                                                    bool choice__ = true;
                                                    while (true) {
                                                        if ((tmr_menu.tick()) || (back == true)) {
                                                            oled.clear();
                                                            oled.update();
                                                            back = true;
                                                            break;
                                                        }
                                                        btn_up.tick();
                                                        btn_down.tick();
                                                        if (btn_up.held()) {
                                                            oled.clear();
                                                            tmr_menu.start();
                                                            break;
                                                        }
                                                        if (btn_down.held()) {
                                                            //oled.clear();
                                                            tmr_menu.start();
                                                            if (choice__ == true) {
                                                                choice__ = false;
                                                            } else {
                                                                choice__ = true;
                                                            }
                                                        }
                                                        oled.setCursor(40, 0);
                                                        oled.setScale(1);
                                                        if (language == 1) {
                                                            oled.print("Time");
                                                        } else if (language == 2) {
                                                            oled.print("Час");
                                                        } else {
                                                            oled.print("Время");
                                                        }
                                                        oled.setCursor(0, 1);
                                                        oled.setScale(2);
                                                        if (hour < 10) {
                                                            oled.print("0");
                                                        }
                                                        oled.print(hour);
                                                        oled.print(":");
                                                        if (minute < 10) {
                                                            oled.print("0");
                                                        }
                                                        oled.print(minute);
                                                        oled.setCursor(0, 3);
                                                        switch (choice__) {
                                                            case true: // Часы
                                                                oled.print("--   ");
                                                                if (btn_up.click()) {
                                                                    tmr_menu.start();
                                                                    hour += 1;
                                                                    if (hour > 23) {
                                                                        hour = 0;
                                                                    }
                                                                }
                                                                if (btn_down.click()) {
                                                                    tmr_menu.start();
                                                                    hour -= 1;
                                                                    if (hour < 0 ) {
                                                                        hour = 23;
                                                                    }
                                                                }
                                                                break;
                                                            case false: // Минуты
                                                                oled.print("   --");
                                                                if (btn_up.click()) {
                                                                    tmr_menu.start();
                                                                    minute += 1;
                                                                    if (minute > 59 ) {
                                                                        minute = 0;
                                                                    }
                                                                }
                                                                if (btn_down.click()) {
                                                                    tmr_menu.start();
                                                                    minute -= 1;
                                                                    if (minute < 0 ) {
                                                                        minute = 59;
                                                                    }
                                                                }
                                                                break;
                                                        }
                                                        oled.update();
                                                    }
                                                }
                                                break;
                                            case 3: // Установить
                                                oled.print(day);
                                                oled.print(".");
                                                oled.print(month);
                                                oled.print(".");
                                                oled.print(year);
                                                oled.setCursor(0, 2);
                                                if (hour < 10) {
                                                    oled.print("0");
                                                }
                                                oled.print(hour);
                                                oled.print(":");
                                                if (minute < 10) {
                                                    oled.print("0");
                                                }
                                                oled.print(minute);
                                                oled.setScale(2);
                                                oled.setCursor(0, 3);
                                                if (language == 1) {
                                                    oled.print("Install");
                                                } else if (language == 2) {
                                                    oled.print("Встановити");
                                                } else {
                                                    oled.print("Установить");
                                                }
                                                if (btn_down.held()) {
                                                    //oled.clear();
                                                    tmr_menu.start();
                                                    // устанавливаем дату
                                                    clock.fillByYMD(year, month, day); // год, месяц, дата
                                                    clock.fillByHMS(hour, minute, 30); // часы, минуты, секунды
                                                    clock.setTime();
                                                    oled.setCursor(0, 3);
                                                    if (language == 1) {
                                                        oled.print("Ready     ");
                                                    } else {
                                                        oled.print("Готово    ");
                                                    }
                                                    oled.update();
                                                    delay(300);
                                                }
                                                break;
                                        }   
                                        oled.update();  
                                    }
                                }
                                break;
                            case 3: // Батарея
                                if (language == 1) {
                                    oled.setCursor(40, 0);
                                    oled.setScale(1);
                                    oled.print("Settings");
                                    oled.setCursor(0, 1);
                                    oled.print("Language");
                                    oled.setCursor(0, 2);
                                    oled.print("Data/time");
                                    oled.setCursor(0, 3);
                                    oled.setScale(2);
                                    oled.print("Battery");
                                    oled.setScale(1);
                                    oled.setCursor(0, 5);
                                    oled.print("Information");
                                } else if (language == 2) {
                                    oled.setCursor(40, 0);
                                    oled.setScale(1);
                                    oled.print("Налаштування");
                                    oled.setCursor(0, 1);
                                    oled.print("Мова");
                                    oled.setCursor(0, 2);
                                    oled.print("Дата/час");
                                    oled.setCursor(0, 3);
                                    oled.setScale(2);
                                    oled.print("Батарея");
                                    oled.setScale(1);
                                    oled.setCursor(0, 5);
                                    oled.print("Iнформацiя");
                                } else {
                                    oled.setCursor(40, 0);
                                    oled.setScale(1);
                                    oled.print("Настройки");
                                    oled.setCursor(0, 1);
                                    oled.print("Язык");
                                    oled.setCursor(0, 2);
                                    oled.print("Дата/время");
                                    oled.setCursor(0, 3);
                                    oled.setScale(2);
                                    oled.print("Батарея");
                                    oled.setScale(1);
                                    oled.setCursor(0, 5);
                                    oled.print("Информация");
                                }
                                if (btn_down.held()) {
                                    oled.clear();
                                    tmr_menu.start();
                                    bool choice_ = true;
                                    while (true) {
                                        if ((tmr_menu.tick()) || (back == true)) {
                                            oled.clear();
                                            oled.update();
                                            back = true;
                                            break;
                                        }
                                        btn_up.tick();
                                        btn_down.tick();
                                        if (btn_up.held()) {
                                            oled.clear();
                                            tmr_menu.start();
                                            break;
                                        }
                                        oled.setScale(1);
                                        switch (battery_setting) {
                                            case true:
                                                if (language == 1) {
                                                    oled.setCursor(40, 0);
                                                    oled.print("Battery");
                                                    oled.setCursor(0, 1);
                                                    oled.print("Percentage - ON ");
                                                } else if (language == 2) {
                                                    oled.setCursor(40, 0);
                                                    oled.print("Батарея");
                                                    oled.setCursor(0, 1);
                                                    oled.print("Проценти - ВКЛ ");
                                                } else {
                                                    oled.setCursor(40, 0);
                                                    oled.print("Батарея");
                                                    oled.setCursor(0, 1);
                                                    oled.print("Проценты - ВКЛ ");
                                                }
                                                if (btn_down.held()) {
                                                    battery_setting = false;
                                                    EEPROM.put(1, battery_setting);
                                                    tmr_menu.start();
                                                }
                                                /*
                                                oled.setCursor(0, 4);
                                                oled.print(battery_setting);
                                                */
                                                break;
                                            case false:
                                                if (language == 1) {
                                                    oled.setCursor(40, 0);
                                                    oled.print("Battery");
                                                    oled.setCursor(0, 1);
                                                    oled.print("Percentage - OFF");
                                                } else if (language == 2) {
                                                    oled.setCursor(40, 0);
                                                    oled.print("Батарея");
                                                    oled.setCursor(0, 1);
                                                    oled.print("Проценти - ВИКЛ");
                                                } else {
                                                    oled.setCursor(40, 0);
                                                    oled.print("Батарея");
                                                    oled.setCursor(0, 1);
                                                    oled.print("Проценты - ВЫКЛ");
                                                }
                                                if (btn_down.held()) {
                                                    battery_setting = true;
                                                    EEPROM.put(1, battery_setting);
                                                    tmr_menu.start();
                                                }
                                                /*
                                                oled.setScale(1);
                                                oled.setCursor(0, 4);
                                                oled.print(battery_setting);
                                                */
                                                break;
                                        }
                                        oled.update();
                                    }
                                }
                                break;
                            case 4: // Информация
                                if (language == 1) {
                                    oled.setCursor(40, 0);
                                    oled.setScale(1);
                                    oled.print("Settings");
                                    oled.setCursor(0, 1);
                                    oled.print("Language");
                                    oled.setCursor(0, 2);
                                    oled.print("Data/time");
                                    oled.setCursor(0, 3);
                                    oled.print("Battery");
                                    oled.setCursor(0, 4);
                                    oled.setScale(2);
                                    oled.print("Information");
                                } else if (language == 2) {
                                    oled.setCursor(40, 0);
                                    oled.setScale(1);
                                    oled.print("Налаштування");
                                    oled.setCursor(0, 1);
                                    oled.print("Мова");
                                    oled.setCursor(0, 2);
                                    oled.print("Дата/час");
                                    oled.setCursor(0, 3);
                                    oled.print("Батарея");
                                    oled.setCursor(0, 4);
                                    oled.setScale(2);
                                    oled.print("Iнформацiя");
                                } else {
                                    oled.setCursor(40, 0);
                                    oled.setScale(1);
                                    oled.print("Настройки");
                                    oled.setCursor(0, 1);
                                    oled.print("Язык");
                                    oled.setCursor(0, 2);
                                    oled.print("Дата/время");
                                    oled.setCursor(0, 3);
                                    oled.print("Батарея");
                                    oled.setCursor(0, 4);
                                    oled.setScale(2);
                                    oled.print("Информация");
                                } 
                                if (btn_down.held()) {
                                    oled.clear();
                                    tmr_menu.start();
                                    bool screen = false;
                                    uint8_t tmr = 0;
                                    while (true) {
                                        if ((tmr_menu.tick()) || (back == true)) {
                                            tmr += 1;
                                            if (tmr > 3) {
                                                oled.clear();
                                                oled.update();
                                                back = true;
                                                break;
                                            } else {
                                                tmr_menu.start();
                                            }
                                        }
                                        btn_up.tick();
                                        btn_down.tick();
                                        if ((btn_up.held()) || (btn_up.click()) || (btn_down.held()) || (btn_up.click())) {
                                            oled.clear();
                                            tmr_menu.start();
                                            break;
                                        }
                                        if (screen == false) {
                                            screen = true;
                                            oled.drawBitmap(11, 15, qr_code_33x33, 33, 33, BITMAP_NORMAL, BUF_ADD);
                                            oled.setCursor(40, 0);
                                            oled.setScale(1);
                                            if (language == 1) {
                                                oled.print("Information");
                                                oled.setCursorXY(51, 18);
                                                oled.print("Version: ");
                                            } else if (language == 2) {
                                                oled.print("Iнформацiя");
                                                oled.setCursorXY(51, 18);
                                                oled.print("Версiя: ");
                                            } else {
                                                oled.print("Информация");
                                                oled.setCursorXY(51, 18);
                                                oled.print("Версия: ");
                                            }
                                            oled.print("1.0");
                                            oled.update();
                                        }
                                    }
                                }
                                break;
                        }
                        oled.update();
                    }
                }
                oled.update();
                break;
        }
    }
}

void alarm_check() {
    clock.getTime();
    if ((clock.hour == alarm_hour) && (clock.minute == alarm_minute)) {
        oled.setPower(true);
        int alarm_done = 30;
        bool vibro = true;
        while (true) {
            btn_up.tick();
            btn_down.tick();
            if ((btn_up.click()) || (btn_down.click()) || (btn_up.held()) || (btn_down.held()) || (alarm_done == 0)) {
                alarm_status = false;
                oled.clear();
                screen_status = true;
                tmr_screen.start();
                digitalWrite(vibrationPin, LOW);
                break;
            }
            if (tmr_alarm.tick()) {
                if (vibro == true) {
                    oled.drawBitmap(120, 1, alarm_7x8, 7, 8, BITMAP_NORMAL, BUF_ADD);
                    digitalWrite(vibrationPin, HIGH);
                    alarm_done -= 1;
                    vibro = false;
                } else {
                    oled.clear(120 ,1, 127, 8);
                    digitalWrite(vibrationPin, LOW);
                    vibro = true;
                }
            }
            oled.setScale(3);
            oled.setCursor(16, 2);
            if (clock.hour < 10) {
                oled.print("0");
            }
            oled.print(clock.hour, DEC);
            oled.print(":");
            if (clock.minute < 10) {
                oled.print("0");
            }
            oled.print(clock.minute, DEC);
            oled.setCursor(32, 5);
            oled.setScale(1);
            oled.print(clock.dayOfMonth, DEC);
            oled.print(".");
            oled.print(clock.month, DEC);
            oled.print("  ");
            oled.setCursor(75, 5);
            // Дублируем костыль
            uint8_t dayOfWeek = clock.dayOfWeek - 1;
            if (dayOfWeek == 0) dayOfWeek = 7;
            switch (dayOfWeek) { // День недели
                case MON:
                    if (language == 1) {
                        oled.print("MON");
                    } else {
                        oled.print("ПНД");
                    }
                    break;
                case TUE:
                    if (language == 1) {
                        oled.print("TUE");
                    } else {
                        oled.print("BTP"); 
                    }
                    break;
                case WED:
                    if (language == 1) {
                        oled.print("WED");
                    } else {
                        oled.print("СРД");
                    }
                    break;
                case THU:
                    if (language == 1) {
                        oled.print("THU");
                    } else {
                        oled.print("ЧТВ");
                    }
                    break;
                case FRI:
                    if (language == 1) {
                        oled.print("FRI");
                    } else {
                        oled.print("ПТН");
                    }
                    break;
                case SAT:
                    if (language == 1) {
                        oled.print("SAT");
                    } else {
                        oled.print("СБТ");
                    }
                    break;
                case SUN:
                    if (language == 1) {
                        oled.print("SUN");
                    } else if (language == 2) {
                        oled.print("НДЛ");
                    } else {
                        oled.print("BCK");
                    }
                    break;
            }
            charging = digitalRead(chargerPin);
            charging_done = digitalRead(chargerPin_done);
            //Serial.print("Зарядка идет - ");
            //Serial.println(charging);
            //Serial.print("Зарядка закончилась - ");
            //Serial.println(charging_done);
            if ((charging == true) && (charging_done == false)) { // Идет зарядка
                if ((tmr_charging.tick())) {
                    switch (charging_animation) {
                        case 0:
                            //oled.clear(99, 1, 103, 12);
                            oled.drawBitmap(1, 1, bat25_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);
                            //oled.update(100, 1, 126, 11); 
                            charging_animation += 1;
                            break;
                        case 1:
                            oled.drawBitmap(1, 1, bat50_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);
                            //oled.update(100, 1, 126, 11);
                            charging_animation += 1;
                            break;
                        case 2:
                            oled.drawBitmap(1, 1, bat75_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);
                            //oled.update(100, 1, 126, 11);
                            charging_animation += 1;
                            break;
                            case 3:
                            oled.drawBitmap(1, 1, bat100_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);
                            //oled.update(100, 1, 126, 11);
                            charging_animation = 0;
                            break;
                        } 
                    
                }
            } 
            if ((charging == true) && (charging_done == true)) { // Зарядка закончилась
                oled.drawBitmap(1, 1, bat_done_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);
            } 
            if ((charging == false) && (charging_done == false)) { // Не заряжается
                if (charging_animation > 0) {
                    charging_animation = 0;
                }
                //oled.clear(97, 1, 103, 12);
                if (battery > 75) {
                    oled.drawBitmap(1, 1, bat100_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);  
                } else if (battery > 50) {
                    oled.drawBitmap(1, 1, bat75_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD); 
                } else if (battery > 25) {
                    oled.drawBitmap(1, 1, bat50_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);
                } else if (battery >= 0) {
                    oled.drawBitmap(1, 1, bat25_7x12, 7, 12, BITMAP_NORMAL, BUF_ADD);
                }
            }
            if (battery_setting == true) {
                oled.setCursorXY(10, 3);
                oled.setScale(1);
                oled.print(battery);
                oled.print("%   ");
            }
            oled.update();
        }
    }
}

void battery_check() {
    if (tmr_alarm.tick()) {
        battery = analogRead(batteryPin);
        //510 = 2.5v, 593 = 2.9v, 614 = 3.0v, 859 = 4.2v Arduino, подтяжка 10 кОм к земле
        battery = map(battery, 593, 859, 0, 100);
        //battery = map(battery, 0, 1023, 0, 100);
    }
    //tmr_alarm.start();
}
