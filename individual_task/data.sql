create database payments;

create table if not exists account
(
	id serial not null
		constraint account_pkey
			primary key,
	login varchar(30) not null,
	pass varchar(30) not null,
	is_client boolean default true not null,
	name varchar(50) not null
);

create table if not exists bill
(
	id serial not null
		constraint bill_pkey
			primary key,
	month integer not null,
	year integer not null,
	meters_data varchar(6),
	client_id integer not null
		constraint bill_account_id_fk
			references account
);


INSERT INTO public.account (id, login, pass, is_client, name) VALUES (1, 'admin', 'admin', false, 'Petr Petrov');
INSERT INTO public.account (id, login, pass, is_client, name) VALUES (2, 'client1', 'passw', true, 'Ivan Ivanov');
INSERT INTO public.account (id, login, pass, is_client, name) VALUES (4, 'client3', 'passw', true, 'Oleg');
INSERT INTO public.account (id, login, pass, is_client, name) VALUES (3, 'client2', 'passw', true, 'Gleb');
INSERT INTO public.bill (id, month, year, meters_data, client_id) VALUES (1, 10, 2018, '100254', 2);
INSERT INTO public.bill (id, month, year, meters_data, client_id) VALUES (5, 9, 2018, '100250', 2);
INSERT INTO public.bill (id, month, year, meters_data, client_id) VALUES (6, 8, 2018, '100200', 2);
INSERT INTO public.bill (id, month, year, meters_data, client_id) VALUES (7, 7, 2018, '100000', 2);
INSERT INTO public.bill (id, month, year, meters_data, client_id) VALUES (8, 10, 2018, '110457', 3);
INSERT INTO public.bill (id, month, year, meters_data, client_id) VALUES (9, 8, 2018, '110195', 3);
INSERT INTO public.bill (id, month, year, meters_data, client_id) VALUES (10, 7, 2018, '109921', 3);
INSERT INTO public.bill (id, month, year, meters_data, client_id) VALUES (11, 6, 2018, '109850', 3);
INSERT INTO public.bill (id, month, year, meters_data, client_id) VALUES (12, 9, 2018, '201684', 4);
INSERT INTO public.bill (id, month, year, meters_data, client_id) VALUES (13, 7, 2018, '201103', 4);
INSERT INTO public.bill (id, month, year, meters_data, client_id) VALUES (14, 6, 2018, '200845', 4);
INSERT INTO public.bill (id, month, year, meters_data, client_id) VALUES (15, 11, 2018, '100300', 2);
INSERT INTO public.bill (id, month, year, meters_data, client_id) VALUES (16, 12, 2018, '100400', 2);
