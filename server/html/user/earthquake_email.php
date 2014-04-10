<?php
chdir("/var/www//boinc/sensor/html/user");
require_once("/var/www//boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 3.2; 
$longitude = -122.426; 
$latitude = 38.041; 
$depth = 18.9; 
$n_stations = 9; 
$etime = 1396296903.439055; 
$dtime = 1396296925; 
$dt_detect  = 21.6; 
$edir       = 1396296903; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
