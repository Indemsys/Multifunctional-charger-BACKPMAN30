// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2022-06-23
// 11:50:01
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"
#include   "BACKPMANv3_specifications.h"
#include   "BACKPMANv3_resources.h"
#include   "gx_display.h"
#include   "HMI_main_screen.h"

TX_THREAD                     hmi_thread;
#pragma data_alignment=8
uint8_t                       thread_hmi_stack[THREAD_HMI_STACK_SIZE];

GX_WINDOW_ROOT            *root;
GX_WINDOW                 *first_screen;
#define SCREEN_HANDLE      0x12345679

uint16_t                   frame_buffer[DISPLAY_1_X_RESOLUTION*DISPLAY_1_Y_RESOLUTION];
uint16_t                   display_buffer[DISPLAY_1_X_RESOLUTION*DISPLAY_1_Y_RESOLUTION] @ ".sram1";


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

  ptr_fr_buf = (uint16_t *)frame_buffer;
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

extern T_fp_measurement_results  fp_meas;

#define VAL_STR_LEN 8

char          str_psrc_v[VAL_STR_LEN];
GX_STRING     gx_str_psrc_v = {str_psrc_v , 0};

char          str_acc_v[VAL_STR_LEN];
GX_STRING     gx_str_acc_v = {str_acc_v , 0};

char          str_acc_i[VAL_STR_LEN];
GX_STRING     gx_str_acc_i = {str_acc_i , 0};

char          str_losses[VAL_STR_LEN];
GX_STRING     gx_str_losses = {str_losses , 0};

char          str_efficiency[VAL_STR_LEN];
GX_STRING     gx_str_efficiency = {str_efficiency , 0};

char          str_temper[VAL_STR_LEN];
GX_STRING     gx_str_temper = {str_temper , 0};

/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_HMI(ULONG initial_input)
{
  TFT_init();
  gx_system_initialize();

  gx_studio_display_configure(DISPLAY_1, _565rgb_driver_setup, LANGUAGE_RUSSIAN, DISPLAY_1_THEME_1,&root);
  root->gx_window_root_canvas->gx_canvas_memory = (ULONG*)frame_buffer;
  gx_studio_named_widget_create("window", (GX_WIDGET *) root, (GX_WIDGET **)&first_screen);
  gx_widget_show(root);
  gx_system_start();


  do
  {
    snprintf(str_psrc_v, VAL_STR_LEN, " %0.2f", fp_meas.flt_psrc_v);
    gx_str_psrc_v.gx_string_length = strlen(str_psrc_v);
    gx_prompt_text_set_ext(&window.window_Val1, &gx_str_psrc_v);

    snprintf(str_acc_v, VAL_STR_LEN, " %0.2f", fp_meas.flt_acc_v);
    gx_str_acc_v.gx_string_length = strlen(str_acc_v);
    gx_prompt_text_set_ext(&window.window_Val2, &gx_str_acc_v);

    snprintf(str_acc_i, VAL_STR_LEN, " %0.2f", fp_meas.flt_acc_i);
    gx_str_acc_i.gx_string_length = strlen(str_acc_i);
    gx_prompt_text_set_ext(&window.window_Val3, &gx_str_acc_i);

    snprintf(str_losses, VAL_STR_LEN, " %0.1f", fp_meas.loss);
    gx_str_losses.gx_string_length = strlen(str_losses);
    gx_prompt_text_set_ext(&window.window_Val4, &gx_str_losses);

    snprintf(str_efficiency, VAL_STR_LEN, " %0.1f", fp_meas.efficiency);
    gx_str_efficiency.gx_string_length = strlen(str_efficiency);
    gx_prompt_text_set_ext(&window.window_Val5, &gx_str_efficiency);

    snprintf(str_temper, VAL_STR_LEN, " %0.1f", fp_meas.t_sens);
    gx_str_temper.gx_string_length = strlen(str_temper);
    gx_prompt_text_set_ext(&window.window_Val6, &gx_str_temper);

    gx_system_canvas_refresh();
    Wait_ms(100);

  }
  while (1);

}

