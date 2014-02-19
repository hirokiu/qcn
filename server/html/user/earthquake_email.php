<?php
chdir("/var/www//boinc/sensor/html/user");
require_once("/var/www//boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 7.4; 
$longitude = -123.359; 
$latitude = 37.210; 
$depth = 4.1; 
$n_stations = 7; 
$etime = 1391395231.187723; 
$dtime = 1391395285; 
$dt_detect  = 53.8; 
$edir       = 1391395231; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
