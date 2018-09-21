#ifndef AMMETER_H
#define AMMETER_H

#include "GeneralData/GeneralData.h"

/***********************充电终端遥信******************************/
InfoAddr Addr_Vol_A_Term = 0x205021;//A相电压
InfoAddr Addr_Vol_B_Term = 0x205022;//B相电压
InfoAddr Addr_Vol_C_Term = 0x205023;//C相电压
InfoAddr Addr_Cur_A_Term = 0x205024;//A相电流
InfoAddr Addr_Cur_B_Term = 0x205025;//B相电流
InfoAddr Addr_Cur_C_Term = 0x205026;//C相电流
InfoAddr Addr_Power_Term = 0x205027;//总有功功率
InfoAddr Addr_rePower_Term = 0x205028;//A相电压
InfoAddr Addr_PowerFactor_Term = 0x205029;//A相电压
InfoAddr Addr_Cur_0_Term = 0x20502A;//A相电压
InfoAddr Addr_Vol_unbalance_Term = 0x20502B;//A相电压
InfoAddr Addr_Cur_unbalance_Term = 0x20502C;//A相电压
InfoAddr Addr_harm_distortion_Term = 0x20502D;//谐波畸变率
InfoAddr Addr_Power_A_Term = 0x20502E;//A相电压
InfoAddr Addr_Power_B_Term = 0x20502F;//A相电压
InfoAddr Addr_Power_C_Term = 0x205030;//A相电压
InfoAddr Addr_rePower_A_Term = 0x205031;//A相电压
InfoAddr Addr_rePower_B_Term = 0x205032;//A相电压
InfoAddr Addr_rePower_C_Term = 0x205033;//A相电压
InfoAddr Addr_PowerFactor_A_Term = 0x205034;//A相电压
InfoAddr Addr_PowerFactor_B_Term = 0x205035;//A相电压
InfoAddr Addr_PowerFactor_C_Term = 0x205036;//A相电压
InfoAddr Addr_active_absorb_energy_Term = 0x205037;
InfoAddr Addr_active_liberate_energy_Term = 0x205038;
InfoAddr Addr_reactive_sensibility_energy_Term = 0x205039;
InfoAddr Addr_reactive_capacity_energy_Term = 0x20503A;


#endif // AMMETER_H
