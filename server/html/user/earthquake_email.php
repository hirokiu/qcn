<?php
chdir("/var/www/boinc/sensor/html/user");
require_once("/var/www/boinc/sensor/html/inc/earthquake_email.inc");
$mag  = 2.6; 
$longitude = 174.853; 
$latitude = -41.191; 
$depth = 29.1; 
$n_stations = 10; 
$etime = 1378296253.989400; 
$dtime = 1378296269; 
$dt_detect  = 15.0; 
$edir       = 1378296253; 

earthquake_email($mag,$longitude,$latitude,$depth,$n_stations,$etime,$edir,$dtime,$dt_detect);

?>
