create table if not exists table_card_authentication
(
	id			integer primary key autoincrement,
	card_id		text,
	card_code	text,
	is_delete	integer
);

create table if not exists table_car_authentication
(
	id			integer primary key autoincrement,
	car_id		text,
	car_vin		text,
	priority	integer,
	car_no		text,
	is_delete	integer
);

create table if not exists table_update_time
(
	id			integer primary key autoincrement,
	card_update	datetime,
	car_update	datetime,
	query_time	datetime
);
