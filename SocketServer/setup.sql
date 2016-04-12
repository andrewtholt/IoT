
create table mqtt_settings (
    address varchar not null default "127.0.0.1",
    port int not null default 1883
);

create table mqtt_map (
    name varchar,
    path varchar
)
