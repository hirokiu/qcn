<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.2; 
$longitude = -121.844; 
$latitude = 37.496; 
$depth = 17.1; 
$n_stations = 7; 
$etime = 1387488578.026237; 
$dtime = 1387488609; 
$dt_detect  = 31.0; 
$edir       = 1387488578; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
