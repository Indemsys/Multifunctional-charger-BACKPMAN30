#ifndef CAN_BUS_DEFS_H
  #define CAN_BUS_DEFS_H

  #define   REQUEST_STATE_TO_ALL              0x00000001  // Команда всем передать свое состояние



  #define   LAND_STATE_PACKET                 0x00000030  // Ответ контроллера остановки. Младшие 4-е бита показывают номер контроллера
  #define   LAND_CMD_PACKET                   0x00000040  // Команда контроллеру остановки. Младшие 4-е бита показывают номер контроллера
  #define   LEDS_STATE_TO_ALL                 0x00000050  // Команда всем контроллерам остановок установить состояние своих LED индикаторов на кнопках
  #define   LAND_DALLAS_KEY                   0x00000060  // Ответ контроллера остановки c информацией о принятом коде Dallas ключа . Младшие 4-е бита показывают номер контроллера


  #define   BCKPC_STATE_PACKET                0x00000060  // Ответ контроллера резервного питания.
  #define   BCKPC_CMD_PACKET                  0x00000070  // Команда контроллеру резервного питания.


  #define   ALL_ENTER_TO_WRK_MOD              0x00001000  // Команда всем перейти в рабочий режим


  #define   TO_PLATF_CONTROL_DATA             0x00002000  // Пакет с управляющей информацией платформе
  #define   TO_PLATF_PLAY_DATA                0x00002001  // Команда проигрывателю информационных сообщений
  #define   TO_PLATF_LIGHTING_DATA            0x00002002  // Установка интенсивности подсветки платформы в процентах
  #define   TO_PLATF_QUAD_ENC_REF_POINT       0x00002003  // Пакет с данными для установки референсной точки квадратурного энкодера
  #define   TO_PLATF_LIN_ENC_REF_POINT        0x00002004  // Пакет с данными для установки референсной точки линейного энкодера
  #define   FROM_PLATF_STATE_DATA             0x00003001  // Ответ платформы со словом состояния
  #define   FROM_PLATF_QUAD_ENC_DATA          0x00003002  // Пакет с данными о скорости и позиции с квадратурного энкодера на гайке
  #define   FROM_PLATF_LIN_ENC_DATA           0x00003003  // Пакет с данными сенсора позиции


  #define   DMC01_ONBUS_MSG                   0x1A00FFFF  // Асинхронная посылка из платы управления автоматическими дверями
  #define   DMC01_REQ                         0x1A01FFFF  // Запрос данных
  #define   DMC01_ANS                         0x1A02FFFF  // Ответ с данными

//******************************************************************************************************************************************************
// Идентификаторы для работы с платой LED дисплея
// Номер платы записывается в биты 20...23 идентификатора
//
// Дисплей предсталяет собой матрицу 8 на 8 двухцветных пикселей (зеленых и красных)
// Зеленые и красные символы имеют собственный экранный буфер и изображения на них выводятсяне зависимо.
// Можно задать независимо красны и зеленый символы.
// Символы могут быть анимированными. Анимация или отсутствие анимации также независимо для красных и зеленых пиксел
//
// В дисплее существует N предустановленных символов. Они хранаяться в ОЗУ и изображение каждого из них можно изменить.
//
//******************************************************************************************************************************************************
  #define PDISPLx_ONBUS_MSG                     0x1E00FFFF  // Асинхронная посылка из платы LED дисплея
  #define PDISPLx_REQ                           0x1E02FFFF  // Посылка к плате с запросом данных или командой
  #define PDISPLx_ANS                           0x1E03FFFF  // Посылка из платы в ответ на запрос
  #define PDISPLx_UPGRADE_RX_ID                 0x1E04FFFF  // Посылка из платы во время программирования
  #define PDISPLx_UPGRADE_TX_ID                 0x1E05FFFF  // Посылка к плате во время программирования
  #define PDISPLx_SET_RED_SYMB                  0x1E06FFFF  // Посылка к плате 8-и байт красного символа
  #define PDISPLx_SET_GREEN_SYMB                0x1E07FFFF  // Посылка к плате 8-и байт зеленого символа

// Идентификатором PDISPLx_REQ вызываются следующие команды (передаются в байте 0 блока данных)
  #define PDISPLx_SET_SYMBOL                    0x01 // Установка статического символа с кодом в байте 1 и цветом в байте 2 (0 - red, 1 - green, 2 - red+green)

// Серия из 2-х команд приводящая к установке изображения символа с заданным кодом
  #define PDISPLx_SET_SYMBOL_PTRN1              0x02 // Установка 1-й части  паттерна символа с кодом в байте 1 , байты паттерна в байтах 4..7
  #define PDISPLx_SET_SYMBOL_PTRN2              0x03 // Установка 2-й части  паттерна символа с кодом в байте 1 , байты паттерна в байтах 4..7


// Серия из 4-х команд приводящая к тому что символ становится анимированным
  #define PDISPLx_DIN_SYMBOL_SET1               0x04 // Фаза 1 установки динамического символа.
// В байте  1    -  номер символа, который выбран для отображения
// В байтах 2..3 -  период смены шагов
// В байтах 4..5 -  количество шагов
  #define PDISPLx_DIN_SYMBOL_SET2               0x05 // Фаза 2 установки динамического символа.
// В байте  2..3 - приращение по x
// В байтах 4..5 - приращение по y
  #define PDISPLx_DIN_SYMBOL_SET3               0x06 // Фаза 2 установки динамического символа.
// В байте  2..3 - начальное значение x
// В байтах 4..5 - начальное значение y
  #define PDISPLx_DIN_SYMBOL_SET4               0x07 // Фаза 2 установки динамического символа.
// В байте  1 - цвет символа (0 - красный, 1 - зеленый, 2 - оба цвета одновременно)



__packed  typedef union
{
  struct bits
  {
    uint8_t  btn_alarm   :     1;
    uint8_t  btn1        :     1;
    uint8_t  btn2        :     1;
    uint8_t  btn3        :     1;
    uint8_t  btn4        :     1;
    uint8_t  btn5        :     1;
    uint8_t  btn6        :     1;
    uint8_t  btn7        :     1;
    uint8_t  btn8        :     1;
    uint8_t  btn9        :     1;
    uint8_t  btn10       :     1;
    uint8_t  btn11       :     1;
    uint8_t  btn12       :     1;
    uint8_t  btn13       :     1;
    uint8_t  btn14       :     1;
    uint8_t  btn15       :     1;
    uint8_t  btn16       :     1;
    uint8_t  btn_STOP    :     1;
    uint8_t  slnd1_sw    :     1;
    uint8_t  slnd2_sw    :     1;
    uint8_t  overload_sw :     1;
    uint8_t  mot_overheat:     1;
    uint8_t  input_1     :     1;  // Зарезервировано
    uint8_t  input_2     :     1;  // Зарезервировано
    uint8_t  input_3     :     1;  // Зарезервировано
    uint8_t  ESC1        :     1;
    uint8_t  ESC2        :     1;
    uint8_t  ESC3        :     1;
    uint8_t  ESC4        :     1;
    uint8_t  ESC5        :     1;
    uint8_t  ESC6        :     1;
    uint8_t  ESC7        :     1;
    uint8_t  ESC8        :     1;
    uint8_t  ESC9        :     1;
    uint8_t  reset_f     :     1;  // Флаг устанавливаемый после сброса контроллера платформы
    uint8_t  inv_brk     :     1;  // Сигнал с инвертера о стотоянии его сигнала управления тормозами
    uint8_t  inv_alr     :     1;  // Сигнал с инвертера о стотоянии его сигнала ALARM
    uint8_t  sfd_diagn   :     1;  // Сигнал диагностики заваривания контактов устройств безопасности
  };
  struct words
  {
    uint32_t w1;
    uint32_t w2;
  };
}
T_can_platf_state;


//  Структура с  данными состояния контроллера остановки
__packed  typedef struct
{
  uint8_t   x1_undef      : 1;
  uint8_t   x1_state      : 1;
  uint8_t   x2_undef      : 1;
  uint8_t   x2_state      : 1;
  uint8_t   x3_undef      : 1;
  uint8_t   x3_state      : 1;
  uint8_t   x4_undef      : 1;
  uint8_t   x4_state      : 1;
  uint8_t   door_undef    : 1;
  uint8_t   door_state    : 1;  // 0 - дверь открыта. 1 - дверь зарыта
  uint8_t   lock_undef    : 1;
  uint8_t   lock_state    : 1;
  uint8_t   land_undef    : 1;
  uint8_t   land_state    : 1;
  uint8_t   btn_press     : 1;
  uint8_t   btn_lng_press : 1;
}
T_can_land_state;

// Структура команды контроллеру остановки
__packed typedef struct
{
  uint8_t OUTEML  :  1; // Состояние сигнала OUTEML
  uint8_t OUT1    :  1; // Состояние сигнала OUT1
  uint8_t OUT2    :  1; // Состояние сигнала OUT2
  uint8_t OUT3    :  1; // Состояние сигнала OUT3
  uint8_t DSENSEN :  1; // Состояние сигнала DSENSEN  Подача питания на датчик двери
  uint8_t OBLED1  :  1; // Состояние сигнала OBLED1
  uint8_t OBLED2  :  1; // Состояние сигнала OBLED2
  uint8_t W1LED   :  1; // Состояние сигнала 1WLED
  uint8_t MUTE    :  1; // Состояние сигнала MUTE
}
T_can_land_cmd_bits;

__packed typedef struct
{
  uint8_t  new_data;
  struct
  {
    T_can_land_cmd_bits data;
    T_can_land_cmd_bits mask;
  }
  cmd_data;

}
T_can_land_cmd;



// Структура CAN пакетов с данными от линейного энкодера на платформе
typedef __packed struct
{
  uint8_t   linenc_present;
  int32_t   position_mm;
  int16_t   speed_mm_sec;

}
T_lin_encoder_pack;

// Структура CAN пакетов с данными от квадратурного энкодера на платформе
typedef  __packed struct
{
  float    position_m;
  float    speed_m_sec;
}
T_quad_encoder_pack;



// Структура CAN пакетов с управляющими сигналами для платформы
typedef  __packed struct
{
  uint8_t   system_state : 8;
  uint16_t  platf_pos    : 16;
  uint8_t   inv_ctrl_en  : 1;   // Бит разрешения упраления внешними сигналами предназначенными для инвертера на плате платформы
  uint8_t   brake_rel    : 1;
  uint8_t   sfd_dagn     : 1;
  uint8_t   inv_rst      : 1;
  uint8_t   inv_stop     : 1;
  uint8_t   inv_rew      : 1;
  uint8_t   inv_fw       : 1;
  uint8_t   reserv       : 1;
  uint16_t  inv_speed    : 16;

} T_platform_control;




#endif // CAN_BUS_DEFS_H



