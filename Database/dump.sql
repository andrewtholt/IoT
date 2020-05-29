.headers on
.mode csv
.output data.csv
SELECT time, name, data FROM weather where name = 'home_office_temperature';
.quit
