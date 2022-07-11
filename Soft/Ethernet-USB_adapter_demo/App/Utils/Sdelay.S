 SECTION .text : CODE (4)  


  PUBLIC Delay_m7

         ; Для Cortex-M4
         ;  (R0+1)*7
Delay_m7
         SUBS     r0,r0,#1   ; 1
         NOP                 ; 1
         NOP                 ; 1
         NOP                 ; 1
         CMP      r0,#0x00   ; 1
         BGT      Delay_m7   ; 2/1
         NOP                 ; 1
         NOP                 ; 1

         NOP                 ; 1
         NOP                 ; 1
         BX       lr         ; 2


        ALIGNROM  1
        END
