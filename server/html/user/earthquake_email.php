<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.5; 
$longitude = -124.869; 
$latitude = 37.274; 
$depth = 39.8; 
$n_stations = 7; 
$etime = 1379816537.717194; 
$dtime = 1379816624; 
$dt_detect  = 86.3; 
$edir       = 1379816537; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
