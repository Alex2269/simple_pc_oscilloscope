// gui_control_panel.c

#include "raylib.h"
#include "main.h" // для OscData, MAX_CHANNELS
#include "gui_control_panel.h"
#include "button.h"
#include "guicheckbox.h"
#include "sliders.h"
// #include "sliders_ex.h"
#include "slider_widget.h"
#include "knob_gui.h"

#include <stddef.h>
#include <stdio.h>

extern int fontSize;       // Зовнішня змінна розміру шрифту
extern int LineSpacing;    // Відступ між рядками тексту
extern Font font;          // Зовнішній шрифт для малювання тексту

#include "psf_font.h"      // Заголовок із парсером PSF-шрифту
// #include "GlyphCache.h"
extern PSF_Font font12;    // Глобальна змінна шрифту PSF
extern PSF_Font font18;
extern PSF_Font font24;
extern int spacing;        // Відступ між символами, той самий, що передається у DrawPSFText

extern int points_to_display; // число точок для відображення

// #include "gui_radiobutton.h"
#include "gui_radiobutton_row.h"

#include "arrow_button.h"
HoldState holdUp = {0};
HoldState holdDown = {0};

// Статичний масив для стану відкриття radiobutton для кожного каналу
static bool radiobuttonOpen[MAX_CHANNELS] = { false };
// static const char *radiobuttonItems[] = { "Rising", "Falling", "Auto" };
static const char *radiobuttonItems[] = { "R", "F", "A" };


void send_command(OscData *data, char* command, size_t buffer_size, int number);
// void write_usb_device(OscData *data, unsigned char* str);
void write_usb_device(OscData *data, unsigned char* str, size_t len);

// Функція відображає панель керування параметрами осцилографа
// Масив кольорів каналів
static Color channel_colors[MAX_CHANNELS] = { YELLOW, GREEN, RED, BLUE };

void gui_control_panel(OscData *oscData, int screenWidth, int screenHeight) {
    // Позиції і розміри панелі
    int panelX = screenWidth - 345;
    int panelY = 10;
    int panelWidth = 340;
    int panelHeight = screenHeight - 20;

    DrawRectangle(panelX, panelY, panelWidth, panelHeight, Fade(DARKGRAY, 0.9f));
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, GRAY);

    // Заголовок панелі
    // DrawText("Control Panel", panelX + 10, panelY + 10, 20, WHITE);
    // DrawPSFText(font12, panelX + 20, panelY + 10, "Панель Керування", 1, WHITE);

    // Кнопки вибору активного каналу з кольорами
    for (int i = 0; i < MAX_CHANNELS; i++) {
        Rectangle btnRect = { panelX + 20 + i * 80, panelY + 20, 60, 30 };
        Color btnColor = (oscData->active_channel == i) ? channel_colors[i] : Fade(channel_colors[i], 0.5f);

        if (Gui_Button(btnRect, font24, TextFormat("CH%d", i + 1), btnColor, GRAY, DARKGRAY, (Color){0,0,0,0})) {
            oscData->active_channel = i;
        }
    }

    // розміри слайдерів
    int W_size = 200;
    int H_size = 30;
    // Відступи для слайдерів
    int sliderX = panelX + 20;
    int sliderY = panelY + 90;
    int spacingY = 140;

    int ch = oscData->active_channel;
    Color activeColor = channel_colors[ch];

    sliderY += 40;

    int knob_radius = 45;
    // Масштабування по вертикалі
    Gui_Knob_Channel(0, sliderX + 65, sliderY,
                     TextFormat("Масштабування CH%d\nпо вертикалі", (int)oscData->active_channel + 1),
                     NULL/*TextFormat("%0.1f", oscData->channels[ch].scale_y)*/,
                     knob_radius,
                     &oscData->channels[ch].scale_y, 0.1f, 2.0f, true, activeColor);

    // Зміщення по вертикалі
    Gui_Knob_Channel(1, sliderX + 234, sliderY,
                     "Vertical\noffset",
                     NULL/*TextFormat("%d", (int)oscData->channels[ch].offset_y)*/,
                     knob_radius,
                     &oscData->channels[ch].offset_y, -200.0f, 1200.0f, true, activeColor);

    sliderY += spacingY;

    // Рівень тригера
    Gui_Knob_Channel(2, sliderX + 65, sliderY,
                     "Trigger\nlevel",
                     NULL/*TextFormat("%0.1f", oscData->channels[ch].trigger_level)*/,
                     knob_radius,
                     &oscData->channels[ch].trigger_level, 0.0f, 1.0f, true, activeColor);

    // Рівень гістерезису
    Gui_Knob_Channel(3, sliderX + 234, sliderY,
                     "Trigger\nhysteresis",
                     NULL/*TextFormat("%0.1f", oscData->channels[ch].trigger_hysteresis_px)*/,
                     knob_radius,
                     &oscData->channels[ch].trigger_hysteresis_px, -1.0f, 1.0f, true, activeColor);

    sliderY += spacingY;


    // Регулятор частоти оновлення інтерфейсу (мс)
    Gui_Knob_Channel(4, sliderX + 65, sliderY,
                     "Refresh\nRate (ms)",
                     NULL/* TextFormat("%d", (int)oscData->refresh_rate_ms)*/,
                     knob_radius,
                     &oscData->refresh_rate_ms, 5.0f, 55.0f, true, WHITE);

    // **Пояснення:**
    // - Мета отримати інкремент слайдеру 5
    // - `GuiSlider` повертає значення з плаваючою точкою.
    // - Ми додаємо 2.5 для коректного округлення при діленні на 5.
    // - Приводимо до int для цілочисельного ділення і множимо назад на 5 — отримуємо крок 5.
    // - Таким чином користувач бачить і змінює значення слайдера з кроком 5.
    oscData->refresh_rate_ms = ((int)(oscData->refresh_rate_ms + 2.5f) / 5) * 5;

    static float old_refresh_rate_ms;
    char command[24] = "Rate:";  // достатній розмір буфера

    if (old_refresh_rate_ms != oscData->refresh_rate_ms)
    {
        old_refresh_rate_ms = oscData->refresh_rate_ms;
        send_command(oscData, command, sizeof(command), (int)oscData->refresh_rate_ms);
    }

    // Регулятор горизонтального зміщення тригера
    Gui_Knob_Channel(5, sliderX + 234, sliderY,
                     "Trigger\noffset X",
                     NULL/*TextFormat("%d", (int)oscData->trigger_offset_x)*/,
                     knob_radius,
                     &oscData->trigger_offset_x, 0, 1000, true, WHITE);

    sliderY += spacingY / 2 + 15;

    Gui_CheckBox((Rectangle){sliderX, sliderY, 30, 30},
                &oscData->channels[ch].trigger_active,
                font24 ,"Activate\ntrigger", "Trigger", activeColor);

    // радіокнопоки тригера
    Rectangle radioBounds = { sliderX + 200, sliderY, 3 * 30 + 2 * 5, 30 }; // 3 кнопки 30px + 2 проміжки по 5px

    Color colorTriggerMode; // Встановлюємо колір якщо тригер активований
    if (oscData->channels[ch].trigger_active)
        colorTriggerMode = activeColor;
    else
        colorTriggerMode = LIGHTGRAY;

    int newSelection = Gui_RadioButtons_Row(radioBounds, font24, radiobuttonItems, 3,
                                        oscData->channels[ch].trigger_edge,
                                        colorTriggerMode, 30, 5);

    if (newSelection != oscData->channels[ch].trigger_edge) {
        oscData->channels[ch].trigger_edge = newSelection;

        // Відправка команди на пристрій
        char cmd[32] = "TriggerEdge:";
        send_command(oscData, cmd, sizeof(cmd), newSelection);
    }

    sliderY += spacingY /2 - 25;

    // Перемикач тестового сигналу
    Gui_CheckBox((Rectangle){sliderX, sliderY, 30, 30},
                &oscData->test_signal,
                font24 ,"Тестовий\nсигнал", NULL, DARKGRAY);

    static bool test_signal;
    char command2[24] = "Test signal:";  // достатній розмір буфера

    if (test_signal != oscData->test_signal)
    {
        test_signal = oscData->test_signal;
        send_command(oscData, command2, sizeof(command2), (int)oscData->test_signal);
    }

    // sliderY += spacingY / 2;

    // Перемикач перемотки сигналу
    Gui_CheckBox((Rectangle){sliderX + 40, sliderY, 30, 30},
                &oscData->movement_signal,
                font24 ,"Прокручування\nсигналу\nMovement", NULL, DARKGRAY);

    // sliderY += spacingY / 2;

    // Перемикач реверсу напрямку малювання сигналу
    Gui_CheckBox((Rectangle){sliderX+80, sliderY, 30, 30},
                &oscData->reverse_signal,
                font24 ,"Реверс\nсигналу", NULL, DARKGRAY);

    static bool last_dynamic_mode = false;
    Gui_CheckBox((Rectangle){sliderX+120, sliderY, 30, 30},
                &oscData->dynamic_buffer_mode,
                font24 ,"Динамічний\nбуфер\npoints_to_display",
                /*TextFormat(": %d",points_to_display)*/NULL, DARKGRAY);

    if (oscData->dynamic_buffer_mode != last_dynamic_mode) {
        int target_history_size = oscData->dynamic_buffer_mode ? points_to_display : 10000;
        setup_channel_buffers(oscData, target_history_size);
        last_dynamic_mode = oscData->dynamic_buffer_mode;
    }

     // --- кнопки регулювання кількості точок сберігання в буфері ---
        Rectangle btnUp = {sliderX+175, sliderY, 20, 20};
        Rectangle btnDown = {sliderX+280, sliderY, 20, 20};

        if (Gui_ArrowButton(btnUp, font18, ARROW_UP, true, &points_to_display, 50, 100, 10000, /*"Збільшити"*/NULL, /*"Вгору"*/NULL, BLUE, &holdUp)) {
            // Обробка натискання кнопки "вгору"
        }
        if (Gui_ArrowButton(btnDown, font18, ARROW_DOWN, true, &points_to_display, 50, 100, 10000, /*"Зменшити"*/NULL, /*"Вниз"*/NULL, RED, &holdDown)) {
            // Обробка натискання кнопки "вниз"
        }

        static int last_buffer_size = 0;
        if (oscData->dynamic_buffer_mode && points_to_display != last_buffer_size) {
            setup_channel_buffers(oscData, points_to_display);
            last_buffer_size = points_to_display;
        }

        // --- виводим кількості точок в буфері ---
        // Визначаємо текст значення
        char valueText[32];
        sprintf(valueText, "%d", points_to_display);
        int charCount = utf8_strlen(valueText);
        int textWidth = charCount * (font18.width + spacing) - spacing;

        // Прямокутник фону між кнопками
        Rectangle valueRect = {
            btnUp.x + btnUp.width + 5,
            btnUp.y,
            btnDown.x - (btnUp.x + btnUp.width) - 10,
            btnUp.height
        };

        // Малюємо фон і рамку
        DrawRectangleRec(valueRect, LIGHTGRAY);
        DrawRectangleLines(valueRect.x, valueRect.y, valueRect.width, valueRect.height, DARKGRAY);

        // Малюємо значення по центру
        float textX = valueRect.x + (valueRect.width - textWidth) / 2.0f;
        float textY = valueRect.y + (valueRect.height - font18.height) / 2.0f;
        DrawPSFText(font18, (int)textX, (int)textY, valueText, spacing, BLACK);
    // DrawPSFText(font24, sliderX + 20, sliderY + 50, TextFormat("points_to_display: %d",points_to_display), 1, WHITE);

    // --- початок блоку регулювання розміру буфера колесом миші ---

    Vector2 mousePos = GetMousePosition(); // Позиція миші
    bool mouseOver = CheckCollisionPointRec(mousePos, valueRect); // Чи курсор над прямокутником значення

    /* Якщо миша знаходиться над прямокутником значення,
       тоді можемо регулювати ширину буфера зберігання
       колесом миші */
    if (mouseOver) {
        // Проста зміна масштабу колесом миші
        int wheelMove = GetMouseWheelMove();
        points_to_display += wheelMove * 100; // Змінюємо крок масштабу за бажанням
        if (points_to_display < 100) points_to_display = 100;
        if (points_to_display > 10000) points_to_display = 10000;
    }

    // --- кінець блоку регулювання розміру буфера колесом миші ---

    // Автоматичне підключення (тимчасово вимкнено)
    /* Gui_CheckBox((Rectangle){sliderX, sliderY, 30, 30}, font24 , "Auto Connect", &oscData->auto_connect);
    sliderY += spacingY; */

    Gui_Slider((Rectangle){25, screenHeight-15, 600, 10},
               font24, NULL/*"Sl"*/, NULL/*"sl"*/,
               &oscData->trigger_offset_x,
               0.0f, 1000.0f, 0, WHITE);
    //
    // int sliderWidth = 10;
    // int sliderHeight = 550;
    // Rectangle sliderBounds = { sliderX - 25, 20, sliderWidth, sliderHeight };
    // Gui_Slider(sliderBounds, font18,
    //         NULL/*TextFormat("Масштабування CH%d\nпо вертикалі", ch + 1)*/,
    //         NULL,
    //         &oscData->channels[ch].offset_y, 550.0f, 0.0f, true, activeColor);


    int sliderWidth = 10;
    int sliderHeight = 560;
    Rectangle sliderBounds = { sliderX - 35, 20, sliderWidth, sliderHeight };
    // Gui_SliderEx(0, sliderBounds, font18, NULL, NULL, &oscData->channels[0].offset_y, 700.0f, 0.0f, true, YELLOW);
    // Gui_SliderEx(1, sliderBounds, font18, NULL, NULL, &oscData->channels[1].offset_y, 700.0f, 0.0f, true, GREEN);
    // Gui_SliderEx(2, sliderBounds, font18, NULL, NULL, &oscData->channels[2].offset_y, 700.0f, 0.0f, true, RED);
    // Gui_SliderEx(3, sliderBounds, font18, NULL, NULL, &oscData->channels[3].offset_y, 700.0f, 0.0f, true, SKYBLUE);

    RegisterSlider(0, sliderBounds, &oscData->channels[0].offset_y, 700.0f, 0.0f, true, YELLOW, NULL, NULL);
    RegisterSlider(1, sliderBounds, &oscData->channels[1].offset_y, 700.0f, 0.0f, true, GREEN, NULL, NULL);
    RegisterSlider(2, sliderBounds, &oscData->channels[2].offset_y, 700.0f, 0.0f, true, RED, NULL, NULL);
    RegisterSlider(3, sliderBounds, &oscData->channels[3].offset_y, 700.0f, 0.0f, true, SKYBLUE, NULL, NULL);

    // Централізована функція, яка обробляє взаємодію і малює всі слайдери
    UpdateSlidersAndDraw(font18, 2);

    // Аналогічно інші слайдери для offset_y, trigger_level і т.д.
}

void send_command(OscData *data, char* command, size_t buffer_size, int number)
{
    //snprintf(command, command_size, "Rate: %d", number);
    size_t len = strlen(command);

    // Перед викликом snprintf добре перевірити,
    // що buffer_size > len щоб уникнути переповнення:
    if (buffer_size > len) {
        snprintf(command + len, buffer_size - len, "%d", number);
    } else {
        // Обробка помилки: недостатньо місця в буфері
        fprintf(stderr, "Buffer overflow prevented in send_command\n");
        return;
    }

    if (buffer_size > len) {
        command[len] = '\0';
    }

    /*
    // Якщо потрібно, щоб між текстом і числом був пробіл,
    // двокрапка або інший символ, додай його перед snprintf:
    if (buffer_size > len) {
        command[len] = ':';  // або ':', або інший символ
        command[len + 1] = ' ';  // або ':', або інший символ
        command[len + 2] = '\0';
        len += 2;
    }
    */

    snprintf(command + len, buffer_size - len, "%d\n", number);
    // printf("command: %s\n", command);
    // printf("number: %d\n", number);


    len = strlen(command);
    write_usb_device(data, command, len);
}

void write_usb_device(OscData *data, unsigned char* str, size_t len)
{
    // RS232_cputs(data->comport_number, str); // не працює ???
    // size_t len = strlen(str);
    RS232_SendBuf(data->comport_number, str, len);
    // printf("%s\n", str);

    // Код перевірки парсингу
    int new_rate;
    if (strncmp(str, "Rate:", 5) == 0)
    {
        new_rate = atoi(str + 5);
        // set_adc_sampling_rate(new_rate);
    }

    /*
    if (strncmp(str, "Rate:", 5) == 0)
    {
        new_rate = atoi(str + 6);
        // set_adc_sampling_rate(new_rate);
        // printf("new_rate %d \n", new_rate);
    }*/

    /*
    int parse;
    if (strncmp(str, "Test signal:", 12) == 0)
    {
        parse = atoi(str + 12);
        printf("parse %d\n", parse);
    }
    */

    printf("%s", str);
    // printf("new_rate: %d\n\n", new_rate);
}

