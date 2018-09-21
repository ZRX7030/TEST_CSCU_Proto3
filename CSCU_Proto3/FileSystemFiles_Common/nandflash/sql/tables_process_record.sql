create table if not exists charge_order
(
	id						integer primary key  autoincrement,
	UUIDOwn					text default "", 
	OrderStatus				integer default 0, 
	CanAddr					integer default 0, 
	CmdSrc					integer default 0,
	StartReason				integer default 0, 
	StartTime				datetime default "", 
	EndTime					datetime default "", 
	TotalChargeTime			integer default 0,
	StartEnergy				integer default 0, 
	EndEnergy				integer default 0, 
	TotalChargeEnergy		integer default 0,
	PeakEnergy				integer default 0, 
	ValleyEnergy			integer default 0, 
	FlatEnergy				integer default 0, 
	TipEnergy				integer default 0,
	StartSoc				integer default 0, 
	StopSoc					integer default 0, 
	UUIDCloud				text default "",
	EventNo					text default "",
	BillCode				text default "",
	CardNo					text default "",
	customerID 				text default "", 
	VIN						text default "",
	CarLisence				text default "",
	CSCUStopReason			integer default 0, 
	CloudStopReason			integer default 0, 
	DevStopReason			integer default 0,
	OrderType				integer default 0,
	ChargeType				integer default 0,
	ChargeWay				integer default 0,
	LimitEnergy				integer default 0,
	GunNum					integer default 1,
	QueueGroup				integer default 0,
	QueueIndex				integer default 0,
	OrderSync				integer default 0
);

create table if not exists emergency_time
(
	id						integer primary key  autoincrement,
	EmergencyTime			datetime default "",
	RemoteEmergency			integer default 0,
	LocalEmergency			integer	default 0
);

create table if not exists plug_gun_table
(
	id						integer primary key autoincrement,
	canaddr					integer,
	insert_record_time		datetime,
	pull_record_time		datetime
);
create table if not exists terminal_online_table
(
	id						integer primary key autoincrement,
	canaddr					integer,
	charger_offline_time	datetime,
	charger_online_time		datetime
);
create table if not exists dc_cabinet_fault_table
(
	id						integer primary key autoincrement,
	canaddr					integer,
	module_id				integer,
	fault_code				integer,
	min_pdu_id				integer,
	max_pdu_id				integer,
	start_time				datetime,
	stop_time				datetime,
	serialnum 				integer,
	record_state 			integer,
	barcode					varchar(60) 
);
create table if not exists terminal_fault_table
(
	id						integer primary key autoincrement,
	canaddr					integer,
	fault_start_time		datetime,
	fault_stop_time			datetime,
	fault_code				integer
);

create table if not exists cscu_online_table
(
	id						integer primary key autoincrement,
	terminal_online_time	datetime,
	terminal_offline_time	datetime,
	offline_reason			varchar(50)
);

create table if not exists cscu_reboot_table       
(
	id						integer primary key  autoincrement, 
	record_time				datetime,
	flag					integer
);
create table if not exists bms_static_table 
(
	id								integer primary key autoincrement,
	canAddr							integer,
	guid_number						varchar(100),
	charager_id						integer,
	record_time						datetime,
	gbt27930_version				varchar(20),
	whole_max_charge_voltage			float,	
	battery_type					integer,
	battery_capacity				float,	
	battery_total_voltage			float,
	manufactor_name					varchar(100),
	battery_group_serial_number		varchar(100),
	battery_group_datetime			datetime,
	battery_group_charge_count		integer,
	battery_group_property			varchar(10),
	car_vin							varchar(30),
	bms_version						varchar(20),
	battery_single_voltage			float,
	battery_single_current			float,
	battary_total_energy			float,
	max_charge_voltage				float,
	max_allow_tempeture				float,
	whole_soc						integer,
	whole_current_voltage			float,
	max_out_voltage				float,
	min_out_voltage				float,
	max_out_current				float,
	min_out_current				float,
	bms_stop_reson					integer,
	bms_stop_fault					integer,
	bms_stop_error					integer,
	charger_stop_reason				integer,
	charger_stop_fault				integer,
	charger_stop_error				integer,
	stop_soc						integer,
	min_single_volatge				float,
	max_single_volatge				float,
	min_tempeture					float,
	max_tempeture					float,
	bms_error_package				varchar(100),
	charger_error_package			varchar(100)
);

create table if not exists format_data_table
(
	id						integer primary key autoincrement,
	canaddr					integer,
	interid					integer,
	slotnum					integer,
	serialnum				varchar(100),
	softversion1			varchar(100),
	softversion2			varchar(100),
	softversion3			varchar(100),
	hdwversion				varchar(100)
);

create table if not exists operate_record_table
(
	id 						integer primary key autoincrement,
	opt_type 				integer,
	opt_name 				varchar(50),
	opt_time 				datetime,
	opt_data 				varchar(50)
);

create table if not exists frozen_energy_resend
(
	id 						integer primary key autoincrement,
	can_addr				integer,
	uuid					text,
	event_no				text,
	upload_time 			datetime
);

create table if not exists power_curve
(
	id 						integer primary key autoincrement,
	can_addr				integer,
	curve_state				integer,
	curve_type				integer,
	bill_code				text,
	begin_time 				integer,
	end_time 				integer,
	suggest_value			text
);

create table if not exists charge_policy
(
	id 						integer primary key autoincrement,
	can_addr				integer,
	bill_code				text,
	begin_time 				integer,
	end_time 				integer,
	kwh_price				text,
	service_price			text
);

create table if not exists active_defend
(
	id 						integer primary key autoincrement,
	can_addr				integer,
	event_level				integer,
	trigger_reason			text,
	trigger_time			text,
	defend_time				text,
	defend_action			text,
	defend_result			text,
	fail_desc				text
);

create table if not exists cscu_order_alarm
(
	id 						integer primary key autoincrement,
	can_addr				integer,
	alarm_code				integer,
	alarm_content			text,
	alarm_time				text,
	order_uuid				text,
	order_code				text,
	alarm_valid				integer	
);
