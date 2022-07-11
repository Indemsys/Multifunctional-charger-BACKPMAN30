﻿// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020-12-14
// 15:35:51
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"


/*-----------------------------------------------------------------------------------------------------


  \param c

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t ascii_to_hex(uint8_t c)
{
  if (c >= '0' && c <= '9')      return (c - '0') & 0x0f;
  else if (c >= 'a' && c <= 'f') return (c - 'a' + 10) & 0x0f;
  else                           return (c - 'A' + 10) & 0x0f;
}

/*-----------------------------------------------------------------------------------------------------


  \param c

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t hex_to_ascii(uint8_t c)
{
  c = c & 0xf;
  if (c <= 9) return (c + 0x30);
  return (c + 'A' - 10);
}

/*-------------------------------------------------------------------------------------------------------------
   Отсечение пробелов спереди и сзади и снятие кавычек если есть
-------------------------------------------------------------------------------------------------------------*/
char* Trim_and_dequote_str(char *str)
{
  int i;

  while (*str == ' ')
  {
    str++;
    if (*str == 0) return str;
  }

  for (i =(strlen(str)- 1); i > 0; i--)
  {
    if (str[i] != ' ') break;
    str[i] = 0;
  }

  if ((str[0] == '"') && (str[strlen(str)- 1] == '"'))
  {
    str[strlen(str)- 1] = 0;
    str++;
  }
  return str;
}

/*-------------------------------------------------------------------------------------------------------------

  Читать в строку str данные из буфера buf до появления нуля или комбинацией "\r\n" или "\n\r", но не длинее len символов.
  Произвести смещение указателя на буфер buf
  Символы \r и \n из строки удаляються

  Возвращает -1 если в начале файла обнаружен байт  0
-------------------------------------------------------------------------------------------------------------*/
int Read_cstring_from_buf(char **buf, char *str, uint32_t len)
{
  int32_t  pos;
  char    c;
  char    *ptr;

  ptr =*buf;
  pos = 0;
  do
  {
    c =*ptr;      // Читаем символ из текущей позиции
    if (c == 0)
    {
      if (pos == 0)
      {
        *buf = ptr;
        return (-1);  // Возвращаем -1 если первый символ в первой прозиции = 0
      }
      // Удаляем из предыдущей позиции строки символы '\r' и '\n' чтобы их не осталось в строке
      if ((str[pos - 1] == '\r') || (str[pos - 1] == '\n'))
      {
        str[pos - 1] = 0;
      }
      *buf = ptr; // Возвращаем указатель на начале следующей подстроки
      return (len);
    }
    str[pos++] = c; // Записываем символ в строку
    ptr++;          // Смещаем текущую позицию в строке
    str[pos] = 0;   // Добавляем 0 чтобы строка всегда оканчивалась нулем
    if (pos > 1)    // Проверяем на наличие шаблоново окончания строки
    {
      if (strcmp(&str[pos - 2], "\r\n") == 0)
      {
        str[pos - 2] = 0;
        *buf = ptr;
        return (pos - 2);
      }
      if (strcmp(&str[pos - 2], "\n\r") == 0)
      {
        str[pos - 2] = 0;
        ptr--;
        *buf = ptr;
        return (pos - 2);
      }
    }
    if (pos >= len)
    {
      *buf = ptr;
      return (len);
    }
  }while (1);
}

 /*-----------------------------------------------------------------------------------------------------
  Поиск строки в буфере заканчивающейся 0 или одним из символов "\r" и "\n"
  Символы "\r" и "\n" в буфере заменяются на 0

  \param **buf      Указатель на буфер. После вызова функции сдвигается к началу следующей строки
  \param *buf_len   Указатель на длину буфера. Если найдена строка указатель возвращает количество оставшихся необработанных данных

  \return char*     Указатель на начало строки оканчивающейся нулем

  Пример:
  Обработка такого буфера -  {0, '1' , '2' , '3' , '\r', '\n' , '4' , '5' , '6', '\r', '7' , '8' , '\n', 0 , 'q', 'w' , 0 , 0 , 0 , 'e', 'r'};
  Приведет к выводу следующих строк: "123" "456" "78" "qw" "e"
-----------------------------------------------------------------------------------------------------*/
uint8_t* Isolate_string_in_buf(uint8_t **buf, uint32_t *buf_len)
{
  uint8_t    *ptr;
  int32_t     pos;
  uint8_t     ch;
  int         cnt;
  int         len;

  len =*buf_len;
  if (len < 2) return 0;
  cnt = 0;
  ptr =*buf;
  pos = 0;
  do
  {
    ch = ptr[pos];      // Читаем символ из текущей позиции
    if (ch == 0)
    {
      if (cnt == 0)
      {
        // Первые нули пропускаем идя до конца буфера
        if (pos < (len-2))
        {
          pos++;
        }
        else
        {
          return 0;
        }
      }
      else
      {
        *buf = &ptr[pos];
        *buf_len = len - pos;
        return &ptr[pos-cnt];
      }
    }
    else if ((ch == '\r') || (ch == '\n'))
    {
      ptr[pos] = 0;
    }
    else
    {
      if (pos < (len-1))
      {
        cnt++;
        pos++;
      }
      else
      {
        ptr[pos] = 0;
        *buf = &ptr[pos];
        *buf_len = len - pos;
        return &ptr[pos-cnt];
      }
    }
  }while (1);
}

/*-----------------------------------------------------------------------------------------------------


  \param str

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Extract_number_from_string(char *str)
{
  uint32_t num = 0;

  while (isdigit(*str))
  {
    num = (num * 10) + (*str - '0');
    str++;
  }
  return num;
}
