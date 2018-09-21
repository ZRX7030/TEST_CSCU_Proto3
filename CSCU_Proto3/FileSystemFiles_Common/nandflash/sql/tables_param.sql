create table if not exists terminal_name_table
(
	id						integer primary key autoincrement,
	canaddr					integer,
	name					varchar(256)
);
create table if not exists charge_stop_name_table
(
	id						integer primary key autoincrement,
	code					integer,
	name					varchar(256)
);

create table if not exists order_start_name_table
(
	id						integer primary key autoincrement,
	code					integer,
	name					varchar(256)
);

create table if not exists order_stop_name_table
(
	id						integer primary key autoincrement,
	code					integer,
	name					varchar(256)
);
create table if not exists fault_name_table
(
	id						integer primary key autoincrement,
	code					integer,
	name					varchar(256)
);
create table if not exists order_cmdsrc_name_table
(
	id						integer primary key autoincrement,
	code					integer,
	name					varchar(256)
);

create table if not exists ammeter_param_table       
(
	id						integer primary key  autoincrement, 
	addr					varchar(13),
	dev_type				integer,
	vol_ratio				integer,
	cur_ratio				integer,
	enable					integer,
        fun_type                                integer
);
create table if not exists phasetype_param_table       
(
	id						integer primary key  autoincrement, 
	canaddr					integer,
	phase_type				integer
);
create table if not exists terminal_param_table       
(
	id						integer primary key  autoincrement, 
	canaddr					integer,
	strategy				integer,
	contrlo_mode			integer,
	aux_type				integer
);
create table if not exists terminal_code_param_table       
(
	id						integer primary key  autoincrement, 
	canaddr					integer,
	code					varchar(30)
);

create table if not exists tpfv_param_table       
(
	id						integer primary key  autoincrement, 
	time_seg				integer,
	start_hour				integer,
	start_minute			integer,
	stop_hour				integer,
	stop_minute				integer,
	limit_soc				integer,
	limit_current			integer
);

create table if not exists local_policy_table       
(
	id						integer primary key  autoincrement, 
	policy_index			integer,
	start_hour				integer,
	start_minute			integer,
	stop_hour				integer,
	stop_minute				integer,
	electric_fee			integer,
	service_fee				integer
);

create table if not exists fgpj_param_table       
(
	id						integer primary key  autoincrement, 
	time_seg				integer,
	start_hour				integer,
	start_minute			integer,
	stop_hour				integer,
	stop_minute				integer
);

create table if not exists singlephase_param_table       
(
	id						integer primary key  autoincrement, 
	canaddr					integer,
	duty_ratio				integer,
	overvolatge_threshold	integer,
	overcurrent_threshold	integer,
	undervolatge_threshold	integer,
	overtemp_thresold		integer
);
create table if not exists threephase_param_table       
(
	id						integer primary key  autoincrement, 
	canaddr					integer,
	duty_ratio				integer,
	overvolatge_threshold	integer,
	overcurrent_threshold	integer,
	undervolatge_threshold	integer,
	overtemp_thresold		integer
);
create table if not exists dcstatic_param_table       
(
	id						integer primary key  autoincrement, 
	canaddr					integer,
	eleclock_type			integer,
	vin_enable				integer,
	auxpower_type			integer,
	eleclock_enable			integer,
	max_gun_current			float,
	bms_pro_type 			integer,
	result					integer
);
create table if not exists dcdynamic_param_table       
(
	id						integer primary key  autoincrement, 
	canaddr					integer,
	work_type				integer,
	work_status				integer,
	strategy				integer,
	priority_level			integer,
	chargefinish_time		integer,
	result					integer
);
create table if not exists dcdoublesys_param_table       
(
	id						integer primary key  autoincrement, 
	canaddr					integer,
	group_value				integer,
	chargeterm_addr			integer,
	work_module				integer,
	switchmodule_type		integer,
	switch_time				integer,
	result					integer
);
create table if not exists activeprotection_param_table       
(
	id								integer primary key  autoincrement, 
	canaddr							integer,
	equstage_current				float,
	batteryequ_time					integer,
	heatout_thresold				integer,
	heatout_time					integer,
	heatout_disable					integer,
	single_overvolatge_thresold		float,
	single_overvolatge_suretime		integer,
	single_overvolatge_disable		integer,
	all_overvolatge_thresold		float,
	all_overvolatge_suretime		integer,
	all_overvolatge_disable			integer,
	overcurrent_thresold			float,
	overcurrent_suretime			integer,
	overcurrent_disable				integer,
	overtemp_thresold				integer,
	overtemp_suretime				integer,
	overtemp_disable				integer,
	lowtemp_thresold				integer,
	lowtemp_suretime				integer,
	lowtemp_disable					integer,
	bmsrelay_linkvol_thresold		float,
	bmsrelay_link_suretime			integer,
	bmsrelay_link_disable			integer,
	bmsrelay_openvol_thresold		float,
	bmsrelay_opencur_thresold		float,
	bmsrelay_open_suretime			integer,
	bmsrelay_open_disable			integer,
	overcharge_ratio				float,
	overcharge_energy				float,
	overcharge_suretime				integer,
	overcharge_disable				integer,
	bmsrepeat_suretime				integer,
	bmsrepeat_disable				integer,
	bmscheck_disable				integer,
	overvoltage_single_1 			float,
	overvoltage_single_2 			float,
	overvoltage_single_3 			float,
	overtemp_single_1 				integer,
	overtemp_single_2 				integer,
	overtemp_single_3 				integer,
	result							integer
);

create table if not exists dcflexcharge_param_table       
(
	id						integer primary key  autoincrement, 
	canaddr					integer,
	arg_type 				integer,
	cur_coef  				integer,
	soc_start				integer,
	soc_stop 				integer,
	temp_start				integer,
	temp_stop				integer,
	time					integer
);

create table if not exists ccu_param_table       
(
	id						integer primary key  autoincrement, 
	canaddr					integer,
	ccu_set_addr 			integer,
	start_addr 				integer,
	max_power  				float
);

create table if not exists queue_group_table       
(
	id						integer primary key  autoincrement, 
	group_id				integer,
	gun1					integer,
	gun2					integer,
	gun3					integer,
	gun4					integer,
	gun5					integer
);


CREATE TABLE IF NOT EXISTS ammeter_range_table
(
	id 						integer primary key autoincrement, 
	can_addr 				integer, 
	max_range 				text
);

CREATE TABLE IF NOT EXISTS acmeter_scale_table
(
	id 						integer primary key autoincrement,
	can_addr 				integer,
	ac_scale				text
);

create table if not exists chargegun_group_table
(
        id						integer primary key  autoincrement,
        group_id				integer,
        gun1					integer,
        gun2					integer,
        gun3					integer,
        gun4					integer,
        gun5					integer,
        gun6					integer,
        gun7					integer
);

create table if not exists terminal_name_show_table
(
        id						integer primary key autoincrement,
        canaddr					integer,
        name					varchar(256)
);

create table if not exists terminal_autocharge_set_table
(
        id						integer primary key autoincrement,
        canaddr					integer,
        autochargenum                     integer,
        isenable                                  integer
);

create table if not exists relay_set_table
(
        id                                              integer primary key autoincrement,
        device_type                                 integer,
        relay_type                     integer
);

create table if not exists terminal_name_multi_table
(
        id                      integer primary key autoincrement,
        canaddr                 integer,
        name                    varchar(256),
        multitype               integer
);

create table if not exists multi_gun_param_table
(
        id                      integer primary key autoincrement,
        multitype               integer,
        multiname               varchar(256),
        gunnumber               integer,
        gun1                    integer,
        gun2                    integer,
        gun3                    integer,
        gun4                    integer,
        gun5                    integer,
        gun6                    integer,
        gun7                    integer
);
