<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.6; 
$longitude = 172.494; 
$latitude = -43.465; 
$depth = 2.2; 
$n_stations = 8; 
$etime = 1379651011.251209; 
$dtime = 1379651030; 
$dt_detect  = 18.7; 
$edir       = 1379651011; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
