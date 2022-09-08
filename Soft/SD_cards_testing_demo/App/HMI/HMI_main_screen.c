// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-06-23
// 11:50:01
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "BACKPMANv3_specifications.h"
#include   "BACKPMANv3_resources.h"
#include   "gx_display.h"
#include   "HMI_main_screen.h"

extern GX_STUDIO_DISPLAY_INFO BACKPMANv3_display_table[];
extern SD_HandleTypeDef   hsd_sdmmc[];

TX_THREAD                     hmi_thread  @ "DTCM";
#pragma data_alignment=8
uint8_t                       thread_hmi_stack[THREAD_HMI_STACK_SIZE] @ "DTCM";

GX_WINDOW_ROOT            *root;
GX_WINDOW                 *net_info_screen;
GX_WINDOW                 *live_values_src_screen;
GX_WINDOW                 *sdcard_src_screen;
#define SCREEN_HANDLE      0x12345679

uint16_t                   frame_buffer[DISPLAY_1_X_RESOLUTION*DISPLAY_1_Y_RESOLUTION]   @ "SRAM2";
uint16_t                   display_buffer[DISPLAY_1_X_RESOLUTION*DISPLAY_1_Y_RESOLUTION] @ "SRAM1";

extern T_fp_measurement_results  fp_meas;

#define       VAL_STR_LEN 32

char          str_1[VAL_STR_LEN];
char          str_2[VAL_STR_LEN];
char          str_3[VAL_STR_LEN];
char          str_4[VAL_STR_LEN];
char          str_5[VAL_STR_LEN];
char          str_6[VAL_STR_LEN];
char          str_7[VAL_STR_LEN];
char          str_8[VAL_STR_LEN];
char          str_9[VAL_STR_LEN];
char          str_10[VAL_STR_LEN];

GX_STRING     gx_str_1 = {str_1 , 0};
GX_STRING     gx_str_2 = {str_2 , 0};
GX_STRING     gx_str_3 = {str_3 , 0};
GX_STRING     gx_str_4 = {str_4 , 0};
GX_STRING     gx_str_5 = {str_5 , 0};
GX_STRING     gx_str_6 = {str_6 , 0};
GX_STRING     gx_str_7 = {str_7 , 0};
GX_STRING     gx_str_8 = {str_8 , 0};
GX_STRING     gx_str_9 = {str_9 , 0};
GX_STRING     gx_str_10 = {str_10 , 0};


static void Thread_HMI(ULONG initial_input);
/*-----------------------------------------------------------------------------------------------------
  Переносим веь буфер на дисплей

  \param canvas
  \param dirty
-----------------------------------------------------------------------------------------------------*/
static void _565rgb_buffer_toggle(GX_CANVAS *canvas, GX_RECTANGLE *dirty)
{
  uint16_t *ptr_fr_buf;
  uint16_t *ptr_disp_buf;

  ptr_fr_buf   = (uint16_t *)frame_buffer;
  ptr_disp_buf = (uint16_t *)display_buffer;

  for (uint32_t i=0; i < (DISPLAY_1_X_RESOLUTION * DISPLAY_1_Y_RESOLUTION); i++)
  {
    uint16_t v =*ptr_fr_buf;
    *ptr_disp_buf =(v >> 8) | ((v << 8) & 0xFF00);
    ptr_fr_buf++;
    ptr_disp_buf++;
  }

  TFT_wr_data_buf((uint8_t *)display_buffer, sizeof(display_buffer));
}

/*-----------------------------------------------------------------------------------------------------


  \param display

  \return UINT
-----------------------------------------------------------------------------------------------------*/
static UINT _565rgb_driver_setup(GX_DISPLAY *display)
{
  _gx_display_driver_565rgb_setup(display, (VOID *)SCREEN_HANDLE, _565rgb_buffer_toggle);


  TFT_wr__cmd(0x29); // display ON
  TFT_Set_rect(0, 0, DISPLAY_1_X_RESOLUTION-1, DISPLAY_1_Y_RESOLUTION-1);
  TFT_wr__cmd(0x2C); // Команда записи в дисплей
                     //
  return (GX_SUCCESS);
}

/*-----------------------------------------------------------------------------------------------------


  \param window
  \param event_ptr

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT main_screen_event_process(GX_WINDOW *window, GX_EVENT *event_ptr)
{
  UINT status;
  switch (event_ptr->gx_event_type)
  {

  default:
    //ITM_EVENT8(1,1);
    status = gx_window_event_process(window, event_ptr);
    //ITM_EVENT8(1,0);
    return status;
  }

  return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Thread_HMI_create(void)
{
  tx_thread_create(&hmi_thread, "HMI", Thread_HMI,
                   0,
                   (void *)thread_hmi_stack, // stack_start
                   THREAD_HMI_STACK_SIZE,    // stack_size
                   THREAD_HMI_PRIORITY,      // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
                   THREAD_HMI_PRIORITY,      // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
                   TX_NO_TIME_SLICE,
                   TX_AUTO_START);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Show_live_values_screen(void)
{
  snprintf(str_1, VAL_STR_LEN, " %0.2f", fp_meas.flt_psrc_v);
  gx_str_1.gx_string_length = strlen(str_1);
  gx_prompt_text_set_ext(&window.window_Val1,&gx_str_1);

  snprintf(str_2, VAL_STR_LEN, " %0.2f", fp_meas.flt_acc_v);
  gx_str_2.gx_string_length = strlen(str_2);
  gx_prompt_text_set_ext(&window.window_Val2,&gx_str_2);

  snprintf(str_3, VAL_STR_LEN, " %0.2f", fp_meas.flt_acc_i);
  gx_str_3.gx_string_length = strlen(str_3);
  gx_prompt_text_set_ext(&window.window_Val3,&gx_str_3);

  snprintf(str_4, VAL_STR_LEN, " %0.1f", fp_meas.loss);
  gx_str_4.gx_string_length = strlen(str_4);
  gx_prompt_text_set_ext(&window.window_Val4,&gx_str_4);

  snprintf(str_5, VAL_STR_LEN, " %0.1f", fp_meas.efficiency);
  gx_str_5.gx_string_length = strlen(str_5);
  gx_prompt_text_set_ext(&window.window_Val5,&gx_str_5);

  snprintf(str_6, VAL_STR_LEN, " %0.1f", fp_meas.t_sens);
  gx_str_6.gx_string_length = strlen(str_6);
  gx_prompt_text_set_ext(&window.window_Val6,&gx_str_6);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Refresh_CDC_ECM_link_info(void)
{
  if (g_uinf.inserted == 0)
  {
    snprintf(str_1, VAL_STR_LEN, " ");
    snprintf(str_2, VAL_STR_LEN, " ");
    snprintf(str_3, VAL_STR_LEN, " ");
    snprintf(str_4, VAL_STR_LEN, "CDC ECM host mode");
    snprintf(str_5, VAL_STR_LEN, " ");
    snprintf(str_6, VAL_STR_LEN, " ");
    snprintf(str_7, VAL_STR_LEN, " ");
    snprintf(str_8, VAL_STR_LEN, "DISCONNECTED");

    gx_prompt_text_color_set(&window_LAN_info.window_LAN_info_Val_Connection,GX_COLOR_ID_RED, GX_COLOR_ID_RED, GX_COLOR_ID_RED);
  }
  else
  {
    ECM_Get_MAC(str_3, VAL_STR_LEN);
    snprintf(str_1, VAL_STR_LEN, "%04X", g_uinf.idVendor);
    snprintf(str_2, VAL_STR_LEN, "%04X", g_uinf.idProduct);

    if (Is_ECM_usb_link_up() == NX_FALSE)
    {
      snprintf(str_4, VAL_STR_LEN, " ");
      snprintf(str_5, VAL_STR_LEN, " ");
      snprintf(str_6, VAL_STR_LEN, " ");
      snprintf(str_7, VAL_STR_LEN, " ");
      snprintf(str_8, VAL_STR_LEN, "DISCONNECTED");

      gx_prompt_text_color_set(&window_LAN_info.window_LAN_info_Val_Connection,GX_COLOR_ID_RED, GX_COLOR_ID_RED, GX_COLOR_ID_RED);
    }
    else
    {
      if (wvar.en_dhcp_client)
      {
        snprintf(str_4, VAL_STR_LEN, "ECM DHCP client");
      }
      else
      {
        snprintf(str_4, VAL_STR_LEN, "ECM Static IP");
      }
      ECM_Get_MASK_IP(str_5, str_6, VAL_STR_LEN);
      ECM_Get_Gateway_IP(str_7, VAL_STR_LEN);

      snprintf(str_8, VAL_STR_LEN, "CONNECTED");

      gx_prompt_text_color_set(&window_LAN_info.window_LAN_info_Val_Connection,GX_COLOR_ID_GREEN, GX_COLOR_ID_GREEN, GX_COLOR_ID_GREEN);
    }
  }

  gx_str_1.gx_string_length = strlen(str_1);
  gx_str_2.gx_string_length = strlen(str_2);
  gx_str_3.gx_string_length = strlen(str_3);
  gx_str_4.gx_string_length = strlen(str_4);
  gx_str_5.gx_string_length = strlen(str_5);
  gx_str_6.gx_string_length = strlen(str_6);
  gx_str_7.gx_string_length = strlen(str_7);
  gx_str_8.gx_string_length = strlen(str_8);


  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_VID        ,&gx_str_1);
  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_PID        ,&gx_str_2);
  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_MAC        ,&gx_str_3);
  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_DHCP       ,&gx_str_4);
  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_IP         ,&gx_str_5);
  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_MASK       ,&gx_str_6);
  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_GATE       ,&gx_str_7);
  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_Connection ,&gx_str_8);

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Refresh_RNDIS_link_info(void)
{
  if (g_uinf.inserted == 0)
  {
    snprintf(str_1, VAL_STR_LEN, " ");
    snprintf(str_2, VAL_STR_LEN, " ");
    snprintf(str_3, VAL_STR_LEN, " ");
    snprintf(str_4, VAL_STR_LEN, "RNDIS device mode");
    snprintf(str_5, VAL_STR_LEN, " ");
    snprintf(str_6, VAL_STR_LEN, " ");
    snprintf(str_7, VAL_STR_LEN, " ");
    snprintf(str_8, VAL_STR_LEN, "DISCONNECTED");

    gx_prompt_text_color_set(&window_LAN_info.window_LAN_info_Val_Connection,GX_COLOR_ID_RED, GX_COLOR_ID_RED, GX_COLOR_ID_RED);
  }
  else
  {
    RNDIS_Get_MAC(str_3, VAL_STR_LEN);
    snprintf(str_1, VAL_STR_LEN, "%04X", g_uinf.idVendor);
    snprintf(str_2, VAL_STR_LEN, "%04X", g_uinf.idProduct);

    if (Is_RNDIS_network_active() == NX_FALSE)
    {
      snprintf(str_4, VAL_STR_LEN, " ");
      snprintf(str_5, VAL_STR_LEN, " ");
      snprintf(str_6, VAL_STR_LEN, " ");
      snprintf(str_7, VAL_STR_LEN, " ");
      snprintf(str_8, VAL_STR_LEN, "DISCONNECTED");

      gx_prompt_text_color_set(&window_LAN_info.window_LAN_info_Val_Connection,GX_COLOR_ID_RED, GX_COLOR_ID_RED, GX_COLOR_ID_RED);
    }
    else
    {
      if (wvar.rndis_config == RNDIS_CONFIG_PRECONFIGURED_DHCP_SERVER)
      {
        snprintf(str_4, VAL_STR_LEN, "RNDIS DHCP server");
      }
      else
      {
        snprintf(str_4, VAL_STR_LEN, "RNDIS Static IP");
      }
      RNDIS_Get_MASK_IP(str_5, str_6, VAL_STR_LEN);
      RNDIS_Get_Gateway_IP(str_7, VAL_STR_LEN);

      snprintf(str_8, VAL_STR_LEN, "CONNECTED");

      gx_prompt_text_color_set(&window_LAN_info.window_LAN_info_Val_Connection,GX_COLOR_ID_GREEN, GX_COLOR_ID_GREEN, GX_COLOR_ID_GREEN);
    }
  }

  gx_str_1.gx_string_length = strlen(str_1);
  gx_str_2.gx_string_length = strlen(str_2);
  gx_str_3.gx_string_length = strlen(str_3);
  gx_str_4.gx_string_length = strlen(str_4);
  gx_str_5.gx_string_length = strlen(str_5);
  gx_str_6.gx_string_length = strlen(str_6);
  gx_str_7.gx_string_length = strlen(str_7);
  gx_str_8.gx_string_length = strlen(str_8);


  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_VID        ,&gx_str_1);
  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_PID        ,&gx_str_2);
  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_MAC        ,&gx_str_3);
  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_DHCP       ,&gx_str_4);
  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_IP         ,&gx_str_5);
  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_MASK       ,&gx_str_6);
  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_GATE       ,&gx_str_7);
  gx_prompt_text_set_ext(&window_LAN_info.window_LAN_info_Val_Connection ,&gx_str_8);

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Show_NET_info_screen(void)
{
  if (g_usb_mode == USB_MODE_HOST)
  {
    Refresh_CDC_ECM_link_info();
  }
  else
  {
    Refresh_RNDIS_link_info();
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void  Show_SD_info_screen(void)
{
  uint32_t status = Get_FS_init_res()->fx_media_open_result;

  if (status != FX_SUCCESS)
  {
    snprintf(str_1, VAL_STR_LEN, "SD error");
    snprintf(str_2, VAL_STR_LEN, " ");
    snprintf(str_3, VAL_STR_LEN, " ");
    snprintf(str_4, VAL_STR_LEN, " ");
    snprintf(str_5, VAL_STR_LEN, " ");
    snprintf(str_6, VAL_STR_LEN, " ");
    snprintf(str_7, VAL_STR_LEN, " ");
    snprintf(str_8, VAL_STR_LEN, " ");
    snprintf(str_9, VAL_STR_LEN, " ");
    snprintf(str_10, VAL_STR_LEN, " ");

    gx_prompt_text_color_set(&window_SD_info.window_SD_info_Val_SD_status,GX_COLOR_ID_RED, GX_COLOR_ID_RED, GX_COLOR_ID_RED);
  }
  else
  {
    snprintf(str_1, VAL_STR_LEN, "SD initialised");

    // ManufID
    snprintf(str_2, VAL_STR_LEN, "%02X ",SDcard_CID.ManufacturerID);
    // OID
    snprintf(str_3, VAL_STR_LEN, "%02X ",SDcard_CID.OEM_AppliID);
    // NAME
    char c1 =(SDcard_CID.ProdName1) & 0xFF;
    char c2 =(SDcard_CID.ProdName1>>8) & 0xFF;
    char c3 =(SDcard_CID.ProdName1>>16) & 0xFF;
    char c4 =(SDcard_CID.ProdName1>>24) & 0xFF;
    char c5 = SDcard_CID.ProdName2;
    snprintf(str_4, VAL_STR_LEN, "%c%c%c%c%c ",c1, c2, c3, c4 , c5);
    // Rev
    snprintf(str_5, VAL_STR_LEN, "%d ",SDcard_CID.ProdRev);
    // Serial Number
    snprintf(str_6, VAL_STR_LEN, "%08X ",SDcard_CID.ProdSN);
    // ManufactDate
    int32_t year  = 2000 +((SDcard_CID.ManufactDate >> 4) & 0xFF);
    int32_t month = SDcard_CID.ManufactDate & 0x0F;
    snprintf(str_7, VAL_STR_LEN, "%d.%d ",year , month);

    uint64_t sz = (uint64_t)hsd_sdmmc[0].SdCard.BlockNbr * (uint64_t)hsd_sdmmc[0].SdCard.LogBlockSize;

    uint32_t s1 = sz % 1000ll;
    uint32_t s2 =(sz / 1000ll)% 1000;
    uint32_t s3 =(sz / 1000000ll)% 1000;
    uint32_t s4 =(sz / 1000000000ll);

    snprintf(str_8, VAL_STR_LEN, "%d %03d %03d %03d",s4, s3,s2, s1);

    sz = Get_media_total_size();
    s1 = sz % 1000ll;
    s2 =(sz / 1000ll)% 1000;
    s3 =(sz / 1000000ll)% 1000;
    s4 =(sz / 1000000000ll);
    snprintf(str_9, VAL_STR_LEN, "%d %03d %03d %03d",s4, s3,s2, s1);


  }

  gx_str_1.gx_string_length = strlen(str_1);
  gx_str_2.gx_string_length = strlen(str_2);
  gx_str_3.gx_string_length = strlen(str_3);
  gx_str_4.gx_string_length = strlen(str_4);
  gx_str_5.gx_string_length = strlen(str_5);
  gx_str_6.gx_string_length = strlen(str_6);
  gx_str_7.gx_string_length = strlen(str_7);
  gx_str_8.gx_string_length = strlen(str_8);
  gx_str_9.gx_string_length = strlen(str_9);
  gx_str_10.gx_string_length = strlen(str_10);


  gx_prompt_text_set_ext(&window_SD_info.window_SD_info_Val_SD_status      ,&gx_str_1);
  gx_prompt_text_set_ext(&window_SD_info.window_SD_info_Val_ManufID        ,&gx_str_2);
  gx_prompt_text_set_ext(&window_SD_info.window_SD_info_Val_OID            ,&gx_str_3);
  gx_prompt_text_set_ext(&window_SD_info.window_SD_info_Val_NAME           ,&gx_str_4);
  gx_prompt_text_set_ext(&window_SD_info.window_SD_info_Val_Rev            ,&gx_str_5);
  gx_prompt_text_set_ext(&window_SD_info.window_SD_info_Val_SN             ,&gx_str_6);
  gx_prompt_text_set_ext(&window_SD_info.window_SD_info_Val_Manuf_Data     ,&gx_str_7);
  gx_prompt_text_set_ext(&window_SD_info.window_SD_info_Val_Card_Capacity  ,&gx_str_8);
  gx_prompt_text_set_ext(&window_SD_info.window_SD_info_Val_FS_Capacity    ,&gx_str_9);
  gx_prompt_text_set_ext(&window_SD_info.window_SD_info_Val_Misc           ,&gx_str_10);

}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Hide_all_screens(void)
{
  gx_widget_hide(net_info_screen);
  gx_widget_hide(live_values_src_screen);
  gx_widget_hide(sdcard_src_screen);
}

/*-----------------------------------------------------------------------------------------------------


  \param n
-----------------------------------------------------------------------------------------------------*/
static void Refresh_screen(uint32_t n)
{
  switch (n)
  {
  case 0:
    Show_NET_info_screen();
    break;
  case 1:
    Show_live_values_screen();
    break;
  case 2:
    Show_SD_info_screen();
    break;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param n
-----------------------------------------------------------------------------------------------------*/
static void Change_screen(uint32_t n)
{
  Hide_all_screens();
  switch (n)
  {
  case 0:
    gx_widget_show(net_info_screen);
    break;
  case 1:
    gx_widget_show(live_values_src_screen);
    break;
  case 2:
    gx_widget_show(sdcard_src_screen);
    break;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_HMI(ULONG initial_input)
{
  static uint32_t scr_number = 2;
  static uint32_t prev_encoder_val = 0;

  TFT_init();
  gx_system_initialize();

  BACKPMANv3_display_table[0].canvas_memory =  (ULONG *)frame_buffer;
  gx_studio_display_configure(DISPLAY_1, _565rgb_driver_setup, LANGUAGE_RUSSIAN, DISPLAY_1_THEME_1,&root);
  gx_studio_named_widget_create("window_LAN_info", (GX_WIDGET *) root, (GX_WIDGET **)&net_info_screen);
  gx_studio_named_widget_create("window", (GX_WIDGET *) root, (GX_WIDGET **)&live_values_src_screen);
  gx_studio_named_widget_create("window_SD_info", (GX_WIDGET *) root, (GX_WIDGET **)&sdcard_src_screen);
  Hide_all_screens();
  gx_widget_show(root);
  gx_system_start();

  Change_screen(scr_number);

  do
  {
    // Обновить содержимое экранов
    Refresh_screen(scr_number);

    // Реагировать на команды управления экранами
    if (prev_encoder_val < g_encoder_counter)
    {
      if (scr_number < 2)
      {
        scr_number++;
      }
      else
      {
        scr_number = 0;
      }
      Change_screen(scr_number);
      Refresh_screen(scr_number);

    }
    else if (prev_encoder_val > g_encoder_counter)
    {
      if (scr_number > 0)
      {
        scr_number--;
      }
      else
      {
        scr_number = 2;
      }
      Change_screen(scr_number);
      Refresh_screen(scr_number);

    }

    prev_encoder_val = g_encoder_counter;

    //ITM_EVENT8(2,1);
    gx_system_canvas_refresh();
    //ITM_EVENT8(2,0);
    Wait_ms(100);

  }while (1);

}


