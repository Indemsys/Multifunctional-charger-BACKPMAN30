#include   "App.h"


typedef struct
{
    GPIO_TypeDef  *gpio;
    uint8_t        pinn;

} T_out_pin;


const T_out_pin out_pins[OUTS_NUM] =
{
  {GPIOC,  6 },   // 0    OUT1
  {GPIOC,  7 },   // 1    OUT2
  {GPIOE, 14 },   // 2    OUT3
  {GPIOE, 15 },   // 3    OUT4
  {GPIOA, 15 },   // 4    LED_GREEN (LED_GR)
  {GPIOB, 10 },   // 5    LED_RED   (LED_RD)
  {GPIOD,  6 },   // 6    LCD_LIGHT (OLEDV)
  {GPIOD, 11 },   // 7    LEDCAN0
};


// Управляющая структура машины состояний управляемой шаблоном
typedef struct
{
    uint32_t  init_state;
    uint32_t  counter;
    int32_t  *pattern_start_ptr;  // Указатель на массив констант являющийся цепочкой состояний (шаблоном)
                                  // Если значение в массиве = 0xFFFFFFFF, то процесс обработки завершается
    // Если значение в массиве = 0x00000000, то вернуть указатель на начало цепочки
    int32_t   *pttn_ptr;          // Текущая позиция в цепочке состояний
    uint32_t  period;             // Периодичность вызова машины состояний в  мс

} T_outs_ptrn;


T_outs_ptrn outs_cbl[OUTS_NUM];

#define __ON   1
#define _OFF   0
//  Шаблон состоит из массива груп слов.
//  Первое слово в группе - значение
//  Второе слово в группе - длительность интервала времени в мс или специальный маркер остановки(0xFFFFFFFF) или цикла(0x00000000)

const int32_t   OUT_BLINK[] =
{
  __ON, 100,
  _OFF, 200,
  0, 0
};

const int32_t   OUT_ON[] =
{
  __ON, 1000,
  __ON, 0xFFFFFFFF
};

const int32_t   OUT_OFF[] =
{
  _OFF, 1000,
  _OFF, 0xFFFFFFFF
};

const int32_t   OUT_UNDEF_BLINK[] =
{
  __ON, 30,
  _OFF, 900,
  _OFF, 0
};

const int32_t   OUT_3_BLINK[] =
{
  __ON, 50,
  _OFF, 50,
  __ON, 50,
  _OFF, 50,
  __ON, 50,
  _OFF, 250,
  _OFF, 0
};


const int32_t   OUT_3_OFF_BLINK[] =
{
  _OFF, 50,
  __ON, 50,
  _OFF, 50,
  __ON, 50,
  _OFF, 50,
  __ON, 0xFFFFFFFF
};



const int32_t   OUT_CAN_ACTIVE_BLINK[] =
{
  _OFF, 10,
  __ON, 50,
  _OFF, 10,
  __ON, 50,
  _OFF, 10,
  __ON, 50,
  _OFF, 10,
  __ON, 50,
  _OFF, 10,
  __ON, 50,
  _OFF, 10,
  __ON, 0xFFFFFFFF
};



typedef struct
{
    uint32_t num;
    const int32_t *pttrn;

} T_land_indication_patterns;


static TX_MUTEX    outputs_mutex;

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t Outputs_mutex_create(void)
{
  if (tx_mutex_create(&outputs_mutex, "OutputsSM", TX_INHERIT) != TX_SUCCESS) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param out_num
-----------------------------------------------------------------------------------------------------*/
void Set_output_blink(uint32_t out_num)
{
  Outputs_set_pattern(OUT_BLINK, out_num, SCAN_PERIOD);
}

void Set_output_on(uint32_t out_num)
{
  Outputs_set_pattern(OUT_ON, out_num, SCAN_PERIOD);
}

void Set_output_off(uint32_t out_num)
{
  Outputs_set_pattern(OUT_OFF, out_num, SCAN_PERIOD);
}

void Set_output_blink_undef(uint32_t out_num)
{
  Outputs_set_pattern(OUT_UNDEF_BLINK, out_num, SCAN_PERIOD);
}

void Set_output_blink_3(uint32_t out_num)
{
  Outputs_set_pattern(OUT_3_BLINK, out_num, SCAN_PERIOD);
}

void Set_output_can_active_blink(uint32_t out_num)
{
  Outputs_set_pattern(OUT_CAN_ACTIVE_BLINK, out_num, SCAN_PERIOD);
}

void Set_output_off_blink_3(uint32_t out_num)
{
  Outputs_set_pattern(OUT_3_OFF_BLINK, out_num, SCAN_PERIOD);
}



/*-----------------------------------------------------------------------------------------------------


  \param num
  \param state
-----------------------------------------------------------------------------------------------------*/
void Set_output_state(uint32_t out_num, uint32_t state)
{

  if (state == 0)
  {
    out_pins[out_num].gpio->BSRR = LSHIFT(1, out_pins[out_num].pinn)<< 16;
  }
  else
  {
    out_pins[out_num].gpio->BSRR = LSHIFT(1, out_pins[out_num].pinn);
  }
}



/*-----------------------------------------------------------------------------------------------------
  Инициализация шаблона для машины состояний выходного сигнала

  Шаблон состоит из массива груп слов.
  Первое слово в группе - значение сигнала
  Второе слово в группе - длительность интервала времени в  мс
    интервал равный 0x00000000 - означает возврат в начало шаблона
    интервал равный 0xFFFFFFFF - означает застывание состояния


  \param pttn    - указатель на запись шаблоне
  \param n       - номер сигнала
  \param period  - периодичность вызова машины состояний в мкс
-----------------------------------------------------------------------------------------------------*/
void Outputs_set_pattern(const int32_t *pttn, uint32_t n, uint32_t period)
{

  if (n >= OUTS_NUM) return;


  if (tx_mutex_get(&outputs_mutex, 10) != TX_SUCCESS)  return;

  if ((pttn != 0) && (outs_cbl[n].pattern_start_ptr != (int32_t *)pttn))
  {
    outs_cbl[n].pattern_start_ptr = (int32_t *)pttn;
    outs_cbl[n].pttn_ptr = (int32_t *)pttn;
    outs_cbl[n].period = period;
    Set_output_state(n,*outs_cbl[n].pttn_ptr);
    outs_cbl[n].pttn_ptr++;
    outs_cbl[n].counter =(*outs_cbl[n].pttn_ptr) * 1000ul / period;
    outs_cbl[n].pttn_ptr++;
  }
  tx_mutex_put(&outputs_mutex);

}


/*-----------------------------------------------------------------------------------------------------
   Автомат состояний выходных сигналов

-----------------------------------------------------------------------------------------------------*/
void Outputs_state_automat(void)
{
  uint32_t        duration;
  uint32_t        output_state;
  uint32_t        n;


  if (tx_mutex_get(&outputs_mutex, 10) != TX_SUCCESS)  return;

  for (n = 0; n < OUTS_NUM; n++)
  {


    // Управление состоянием выходного сигнала
    if (outs_cbl[n].counter) // Отрабатываем шаблон только если счетчик не нулевой
    {
      outs_cbl[n].counter--;
      if (outs_cbl[n].counter == 0)  // Меняем состояние сигнала при обнулении счетчика
      {
        if (outs_cbl[n].pattern_start_ptr != 0)  // Проверяем есть ли назначенный шаблон
        {
          output_state =*outs_cbl[n].pttn_ptr;   // Выборка значения состояния выхода
          outs_cbl[n].pttn_ptr++;
          duration =*outs_cbl[n].pttn_ptr;       // Выборка длительности состояния
          outs_cbl[n].pttn_ptr++;                // Переход на следующий элемент шаблона
          if (duration != 0xFFFFFFFF)
          {
            if (duration == 0)  // Длительность равная 0 означает возврат указателя элемента на начало шаблона и повторную выборку
            {
              outs_cbl[n].pttn_ptr = outs_cbl[n].pattern_start_ptr;
              output_state =*outs_cbl[n].pttn_ptr;
              outs_cbl[n].pttn_ptr++;
              outs_cbl[n].counter =(*outs_cbl[n].pttn_ptr) * 1000ul / outs_cbl[n].period;
              outs_cbl[n].pttn_ptr++;
              Set_output_state(n , output_state);
            }
            else
            {
              outs_cbl[n].counter = duration * 1000ul / outs_cbl[n].period;
              Set_output_state(n ,output_state);
            }
          }
          else
          {
            // Обнуляем счетчик и таким образом выключаем обработку паттерна
            Set_output_state(n , output_state);
            outs_cbl[n].counter = 0;
            outs_cbl[n].pattern_start_ptr = 0;
          }
        }
        else
        {
          // Если нет шаблона обнуляем состояние выходного сигнала
          Set_output_state(n, 0);
        }
      }
    }
  }
  tx_mutex_put(&outputs_mutex);
}


