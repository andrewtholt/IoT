drop table if exists fred_variables ;

drop index if exists mqtt_map_forward ;
drop index if exists mqtt_map_reverse ;

-- 
-- can have multiple servers for load blancing or
-- redundancy
--
create table fred_variables (
    id integer primary key autoincrement,
    name varchar unique,
    value varchar
);

