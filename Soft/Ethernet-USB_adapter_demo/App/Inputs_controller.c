// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.07.05
// 09:27:50
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "App.h"



uint32_t  smpl_enc_a;
uint32_t  smpl_enc_b;
uint32_t  smpl_enc_sw;


T_backpman3_inputs          inp;
T_backpman3_inputs          inp_prev;
T_backpman3_inputs_flags    inp_flags;


T_fp_measurement_results  fp_meas;

static T_input_cbl BACKPMAN3_sigs_cbls[] =
{
  { GEN_SW  ,&smpl_enc_a    ,&smpl_enc_a ,  0,  GEN_BNC_L  ,GEN_BNC_H  ,0    ,&inp.enc_a   ,&inp_prev.enc_a   ,&inp_flags.f_enc_a },
  { GEN_SW  ,&smpl_enc_b    ,&smpl_enc_b ,  0,  GEN_BNC_L  ,GEN_BNC_H  ,0    ,&inp.enc_b   ,&inp_prev.enc_b   ,&inp_flags.f_enc_b },
  { GEN_SW  ,&smpl_enc_sw   ,&smpl_enc_sw,  0,  GEN_BNC_L  ,GEN_BNC_H  ,0    ,&inp.enc_sw  ,&inp_prev.enc_sw  ,&inp_flags.f_enc_sw },
};

#define BACKPMAN3_SIGS_NUM (sizeof(BACKPMAN3_sigs_cbls)/sizeof(BACKPMAN3_sigs_cbls[0]))

static uint32_t  Inputs_do_processing(T_input_cbl *scbl);

/*------------------------------------------------------------------------------
  Процедура определения состояния сигнала

  Возвращает 1 если значение сигнала изменилось, иначе 0
 ------------------------------------------------------------------------------*/
static uint32_t Inputs_do_processing(T_input_cbl *scbl)
{
  uint8_t upper_sig, lower_sig;

  if (scbl->itype == GEN_SW)
  {
    // Обрабатываем сигнал с бистабильного датчика
    if (*scbl->p_smpl1 > scbl->lbound) scbl->pbncf.curr = 1;
    else scbl->pbncf.curr = 0;
  }
  else if (scbl->itype == ESC_SW)
  {
    // Обрабатываем сигнал с датчика обладающего неопределенным состоянием (контакты в цепи безопасности)
    if (*scbl->p_smpl1 > scbl->lbound) upper_sig = 1;
    else upper_sig = 0;

    if (*scbl->p_smpl2 > scbl->lbound) lower_sig = 1;
    else lower_sig = 0;

    if ((upper_sig) && (lower_sig))
    {
      scbl->pbncf.curr = 0; // 0 - замкнутое состояние
    }
    else if (((upper_sig) && (!lower_sig)) || ((!upper_sig) && (lower_sig)))
    {
      scbl->pbncf.curr = 1; // 1 - разомкнутое состояние
    }
    else
    {
      scbl->pbncf.curr = 0; // В данной схеме не может быть неопределенных состояний
    }

  }
  else return 0;


  if (scbl->pbncf.init == 0)
  {
    scbl->pbncf.init = 1;
    scbl->pbncf.prev = scbl->pbncf.curr;
    *scbl->val  = scbl->pbncf.curr;
    *scbl->val_prev =*scbl->val;
    return 0;
  }

  if (scbl->pbncf.prev != scbl->pbncf.curr)
  {
    scbl->pbncf.cnt = 0;
    scbl->pbncf.prev = scbl->pbncf.curr;
  }

  if (scbl->pbncf.curr == 0)
  {
    if (scbl->pbncf.cnt > scbl->l0_time)
    {
      *scbl->val_prev =*scbl->val;
      *scbl->val = scbl->pbncf.curr;
    }
    if (scbl->pbncf.cnt == scbl->l0_time) *scbl->flag = 1;
    scbl->pbncf.cnt++;
    return 1;
  }
  else if (scbl->pbncf.curr == 1)
  {
    if (scbl->pbncf.cnt > scbl->l1_time)
    {
      *scbl->val_prev =*scbl->val;
      *scbl->val = scbl->pbncf.curr;
    }
    if (scbl->pbncf.cnt == scbl->l1_time) *scbl->flag = 1;
    scbl->pbncf.cnt++;
    return 1;
  }
  else if (scbl->pbncf.curr == -1)
  {
    if (scbl->pbncf.cnt > scbl->lu_time)
    {
      *scbl->val_prev =*scbl->val;
      *scbl->val = scbl->pbncf.curr;
    }
    if (scbl->pbncf.cnt == scbl->lu_time) *scbl->flag = 1;
    scbl->pbncf.cnt++;
    return 1;
  }
  else
  {
    if (scbl->pbncf.cnt < MAX_UINT_32)
    {
      scbl->pbncf.cnt++;
    }
  }
  return 0;
}


/*-----------------------------------------------------------------------------------------------------
  Определение состояния входных цифровых и аналоговых сигналов


  \param void
-----------------------------------------------------------------------------------------------------*/
void Inputs_processing(void)
{
  smpl_enc_a  =  Get_smpl_enc_a ();
  smpl_enc_b  =  Get_smpl_enc_b ();
  smpl_enc_sw =  Get_smpl_enc_sw();

  for (uint32_t i=0; i < BACKPMAN3_SIGS_NUM; i++)
  {
    Inputs_do_processing(&BACKPMAN3_sigs_cbls[i]);
  }
}




