create table if not exists data_retry_table
(
   id                   integer primary key autoincrement,
   fail_time            datetime,
   primeval_data		varchar(500)
);
create table if not exists status_save_table       
(
	id						integer primary key  autoincrement, 
	canaddr					integer,
	data					blob
);
create table if not exists charge_save_table       
(
	id						integer primary key  autoincrement, 
	canaddr					integer,
	data					blob
);


