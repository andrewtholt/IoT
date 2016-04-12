drop table if exists mqtt_settings ;
drop table if exists mqtt_map ;

-- 
-- can have multiplke server for load blancing or#
-- redundancy
--
create table mqtt_settings (
    id integer primary key autoincrement,
    address varchar default "127.0.0.1",
    port int default 1883
);

create table mqtt_map (
    name varchar,
    path varchar
);

-- 
-- Create default entry for MQTT server
--
insert into mqtt_settings (id) values (1);

