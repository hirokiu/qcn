<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.5; 
$longitude = 174.282; 
$latitude = -41.731; 
$depth = 11.9; 
$n_stations = 8; 
$etime = 1375016834.711047; 
$dtime = 1375016872; 
$dt_detect  = 37.3; 
$edir       = 1375016834; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
