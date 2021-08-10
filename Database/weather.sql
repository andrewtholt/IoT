
drop table if exists weather;

-- create table weather (
--     id INTEGER PRIMARY KEY AUTOINCREMENT,
-- 
--     temperature INTEGER DEFAULT 0,
--     humidity INTEGER DEFAULT 0,
--     pressure INTEGER DEFAULT 0,
--     light_level INTEGER DEFAULT 0,
--     dt datetime DEFAULT current_timestamp
-- );
-- 
-- insert into weather (temperature, humidity, pressure, light_level ) 
--    values( 1796,52,101262, 95);

create table weather (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT,
    data INTEGER,
    time INTERGER DEFAULT 0
--     dt datetime DEFAULT current_timestamp
);

CREATE INDEX by_time ON weather(time);
-- CREATE INDEX by_time ON weather(dt);

