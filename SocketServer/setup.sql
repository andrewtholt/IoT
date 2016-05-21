drop table if exists mqtt_settings ;
drop table if exists mqtt_map ;

drop index if exists mqtt_map_forward ;
drop index if exists mqtt_map_reverse ;

-- 
-- can have multiple servers for load blancing or
-- redundancy
--
create table mqtt_settings (
    id integer primary key autoincrement,
    address varchar default "127.0.0.1",
    port int default 1883
);

-- hold a shorthand version of the long path,
-- this assists devices that have a small amount of
-- RAM in which to assemble messages.
--
create table mqtt_map (
    name varchar unique,
    path varchar
);

create index mqtt_map_forward on mqtt_map(name);
create index mqtt_map_reverse on mqtt_map(path);

-- 
-- Create default entry for MQTT server
--
insert into mqtt_settings (id) values (1);
-- 
-- Test mapping
--
insert into mqtt_map (name,path) values ('TEST_STATE','/home/outside/front/light/state');

