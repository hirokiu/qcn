<?php
// CMC this file will process remote server triggers and put them in the qcn_trigger_memory table as well as qcn_trigger_remote

require_once("util.inc");
require_once("db.inc");


/*

sensor.qcn_remote table

| id | script_name          | name                   | url                               | country | trigger_url                                         | download_url
|  1 | st_sensor_rep.php    | QCN Stanford Sensor    | http://qcn.stanford.edu/sensor    | US      | http://qcn.stanford.edu/sensor                      | http://qcn-upl.stanford.edu/trigger
|  2 | st_continual_rep.php | QCN Stanford Continual | http://qcn.stanford.edu/continual | US      | http://qcn.stanford.edu/continual                   | http://qcn-upl.stanford.edu/trigger/continual
|  3 | tw_sensor_rep.php    | QCN Taiwan Sensor      | http://qcn.twgrid.org/sensor      | TW      | http://qcn.stanford.edu/sensor/tw_sensor_rep.php    | http://qcn.twgrid.org/trigger
|  4 | tw_continual_rep.php | QCN Taiwan Continual   | http://qcn.twgrid.org/continual   | TW      | http://qcn.stanford.edu/sensor/tw_continual_rep.php | http://qcn.twgrid.org/trigger/continual
|  5 | mx_sensor_rep.php    | QCN Mexico Sensor      | http://www.ras.unam.mx/sensor     | MX      | http://qcn.stanford.edu/sensor/mx_sensor_rep.php    | http://www.ras.unam.mx/trigger
|  6 | mx_continual_rep.php | QCN Mexico Continual   | http://www.ras.unam.mx/continual  | MX      | http://qcn.stanford.edu/sensor/mx_continual_rep.php | http://www.ras.unam.mx/trigger/continual
|  7 | fr_sensor_rep.php    | QCN France Sensor      | http://emsc-csem.org/sensor       | FR      | http://qcn.stanford.edu/sensor/fr_sensor_rep.php    | http://emsc-csem.org/trigger 
|  8 | fr_continual_rep.php | QCN France Continual   | http://emsc-csem.org/continual    | FR      | http://qcn.stanford.edu/sensor/fr_continual_rep.php | http://emsc-csem.org/trigger/continual

*/

$g_TRIGMEMDB_LINK = 0;
$g_DB_LINK = 0;
$g_REMDB = "sensor.qcn_trigger_remote";
$g_TRIGMEMDB = "trigmem.qcn_trigger_memory";
$g_LOGFILE = "../../log_qcn-web/qcn_remote_trigger.log";
$g_lognum = 0; // file ID #
$g_server = array(
  array(
    'id' => 3,
    'script_name' => 'tw_sensor_rep.php',
    'name' => 'QCN Taiwan Sensor',
    'url' => 'http://qcn.twgrid.org/sensor',
    'download_url' => 'http://qcn.twgrid.org/trigger'
  ),
  array(
    'id' => 4,
    'script_name' => 'tw_continual_rep.php',
    'name' => 'QCN Taiwan Continual',
    'url' => 'http://qcn.twgrid.org/continual',
    'download_url' => 'http://qcn.twgrid.org/trigger/continual'
  ),
  array(
    'id' => 5,
    'script_name' => 'mx_sensor_rep.php',
    'name' => 'QCN Mexico Sensor',
    'url' => 'http://www.ras.unam.mx/sensor',
    'download_url' => 'http://www.ras.unam.mx/trigger'
  ),
  array(
    'id' => 6,
    'script_name' => 'mx_continual_rep.php',
    'name' => 'QCN Mexico Continual',
    'url' => 'http://www.ras.unam.mx/continual',
    'download_url' => 'http://www.ras.unam.mx/trigger/continual'
  ),
  array(
    'id' => 7,
    'script_name' => 'fr_sensor_rep.php',
    'name' => 'QCN France Sensor',
    'url' => 'http://emsc-csem.org/sensor',
    'download_url' => 'http://emsc-csem.org/trigger'
  ),
  array(
    'id' => 8,
    'script_name' => 'fr_continual_rep.php',
    'name' => 'QCN France Continual',
    'url' => 'http://emsc-csem.org/continual',
    'download_url' => 'http://emsc-csem.org/trigger/continual'
  )
);

function qcn_db_init()
{
    global $g_TRIGMEMDB_LINK, $g_DB_LINK;

    $config = get_config();
    $user = parse_config($config, "<trigmem_db_user>");
    $pass = parse_config($config, "<trigmem_db_passwd>");
    $db_name = parse_config($config, "<trigmem_db_name>");
    $host = parse_config($config, "<trigmem_db_host>");
    $g_TRIGMEMDB_LINK = mysql_pconnect($host, $user, $pass);
    if (!$g_TRIGMEMDB_LINK) {
        return 1;
    }
    if (!mysql_select_db($db_name, $g_TRIGMEMDB_LINK)) {
        echo "selecting $db_name\n";
        return 2;
    }

    $user = parse_config($config, "<db_user>");
    $pass = parse_config($config, "<db_passwd>");
    $db_name = parse_config($config, "<db_name>");
    $host = parse_config($config, "<db_host>");
    $g_DB_LINK = mysql_pconnect($host, $user, $pass);
    if (!$g_DB_LINK) {
        return 3;
    }
    if (!mysql_select_db($db_name, $g_DB_LINK)) {
        echo "selecting $db_name\n";
        return 4;
    }
    return 0;
}


function qcn_get_server_id($filename)
{
  global $g_server;
  // print_r($g_server);
  // print sizeof($g_server);
 
  $id = 0;
  for ($i = 0; $i < sizeof($g_server); $i++) {
    if (strpos($filename, $g_server[$i]['script_name'])) {
       $id = $g_server[$i]['id'];
       break; 
    }
  }
  return $id;
}

function qcn_trigger_rep_process($basefile)
{
   $server_id = qcn_get_server_id($basefile);
   if ($server_id) { // returned a valid server
     // parse the xml - only process if variety is trigger (i.e. not quakelist etc)
     // also discard if qlat < -90 and qlng < -360

     // get the server input string
     $string = @file_get_contents('php://input');

/*
     $string = "

<scheduler_request>
    <authenticator>26f1e24b3537129b0525703feb970762</authenticator>
    <hostid>601</hostid>
    <rpc_seqno>18031</rpc_seqno>
    <core_client_major_version>7</core_client_major_version>
    <core_client_minor_version>0</core_client_minor_version>
    <core_client_release>99</core_client_release>
    <platform_name>windows_intelx86</platform_name>
    <msg_from_host>
        <result_name>qcnc_053434_0</result_name>
        <time>1379947269</time>
<variety>trigger</variety>
<vr>7.33</vr>
<os>W32</os>
<sms>105</sms>
<ctime>1380038196.937414</ctime>
<fsig>3.272128</fsig>
<fmag>0.022373</fmag>
<file>qcnc_053434_000021_1379947263.zip</file>
<reset>0</reset>
<dt>0.020000</dt>
<follow>1</follow>
<mxy1p>0.111433</mxy1p>
<mz1p>0.045831</mz1p>
<mxy1a>0.011396</mxy1a>
<mz1a>0.019832</mz1a>
<mxy2a>0.007996</mxy2a>
<mz2a>0.011488</mz2a>
<mxy4a>0.011048</mxy4a>
<mz4a>0.010030</mz4a>
<tsync>0.000000</tsync>
<toff>0.000000</toff>
<wct>47929.03</wct>
<cpt>3.13</cpt>
<qlat>19.389413</qlat>
<qlng>-99.044881</qlng>

    </msg_from_host>
</scheduler_request>


";

     $string = "

<scheduler_request>
    <authenticator>dd9fc72b1a747f6458d4ab783a6276a7</authenticator>
    <hostid>630</hostid>
    <rpc_seqno>22900</rpc_seqno>
    <core_client_major_version>7</core_client_major_version>
    <core_client_minor_version>0</core_client_minor_version>
    <core_client_release>99</core_client_release>
    <platform_name>windows_x86_64</platform_name>
    <msg_from_host>
        <result_name>qcnc_053558_0</result_name>
        <time>1379951197</time>
<variety>trigger</variety>
<vr>7.33</vr>
<os>W64</os>
<sms>100</sms>
<ctime>1379951199.285035</ctime>
<fsig>3.005502</fsig>
<fmag>0.341353</fmag>
<file>qcnc_053558_000017_1379951199.zip</file>
<reset>0</reset>
<dt>0.020000</dt>
<mxy1p>0.312256</mxy1p>
<mz1p>0.093137</mz1p>
<tsync>1379950579.463584</tsync>
<toff>2.504157</toff>
<wct>4230.32</wct>
<cpt>16.83</cpt>
<qlat>52.216702</qlat>
<qlng>9.550000</qlng>

    </msg_from_host>
</scheduler_request>

     ";
*/

     // a simple parser such as the one built in to PHP5 should suffice for BOINC XML scheduler formats

     // this will return xml parsed into PHP objects, access via -> (note most are sub-values of msg_from_host)
     $xml = simplexml_load_string($string);

     insert_qcn_remote_trigger($server_id, $xml->hostid, $xml->msg_from_host);

   }
   qcn_log_close();
}


function insertValidRemoteTrigger($server_id, $hostid, &$mfh, $bTrigMem = false, $triggerid = 0)  
{
   global $g_TRIGMEMDB_LINK, $g_DB_LINK;
   global $g_REMDB, $g_TRIGMEMDB;
   global $g_server;

       $strUseDB = "";
       $sqllookup = "";
       $mylink = 0;
       // debug output to temp table 
       if ($bTrigMem) {
          $strUseDB = $g_TRIGMEMDB;
          $mylink = $g_TRIGMEMDB_LINK;
          $sqllookup = "SELECT id FROM " . $strUseDB 
              . " WHERE db_name='" . $g_server[$server_id]['id'] . "' AND file='" . BoincDb::escape_string($mfh->file) .  "'";
       }
       else {
          $strUseDB = $g_REMDB;
          $mylink = $g_DB_LINK;
          $sqllookup = "SELECT id FROM " . $strUseDB 
              . " WHERE qcn_remoteid=" . $server_id . " AND file='" . BoincDb::escape_string($mfh->file) .  "'";
         
     /* 
          $strsql = "insert into sensor.qcn_trigger_remote_xml (qcn_remoteid, hostid, valid, variety, result_name, latitude, longitude, xml) "
               . "values ($server_id, $hostid, 1," 
               . "'$mfh->variety', '$mfh->result_name', "
               . ($mfh->qlat ? "$mfh->qlat," : "NULL,")
               . ($mfh->qlng ? "$mfh->qlng," : "NULL,")
               . "'" . print_r($mfh, true) . "')";
          //qcn_log_error($strsql);
          mysql_query($strsql, $mylink);
      */
       }

       // valid trigger ie variety is trigger, and lat/lng are OK
       $result = mysql_query($sqllookup, $mylink);
       //print "\n" . $sqllookup . "\n";
       $idRemote = 0;
       if ($result && ($res = mysql_fetch_array($result))) {
           $idRemote = $res[0];
       }
       if ($result) mysql_free_result($result);

       if ($idRemote) { // do an update as we have the remote id
          $sql = "UPDATE " . $strUseDB . " SET " . qcn_remote_update_string($server_id, $hostid, $mfh, $bTrigMem, $triggerid) 
                     . " WHERE id=$idRemote";
       }
       else { // do an insert as thre is no existing remote id
          $sql = "INSERT INTO " . $strUseDB . " SET " . qcn_remote_update_string($server_id, $hostid, $mfh, $bTrigMem, $triggerid);
       }
       try { // this is an 'action query' so better to trap in a try since $result is non-existent, and print out mysql_error
          mysql_query($sql, $mylink);
          // return the appropriate id, either the updated one found, or new trigger insert
          if (!$idRemote && !$bTrigMem) {  // it was an insert
             $idRemote = mysql_insert_id($mylink);
          }
       }
       catch (Exception $e) {
          qcn_log_error($sql . "\n" . mysql_error());
       }
       return $idRemote;
}

function insert_qcn_remote_trigger($server_id, $hostid, &$mfh)
{
   global $g_TRIGMEMDB_LINK, $g_DB_LINK;

       $isValid = true;

       // NB: don't need to capture continual info, just variety = 'trigger'
       if ($mfh->variety != "trigger"
           || $mfh->qlat < -90.
           || $mfh->qlng < -360.
           || strpos($mfh->file, "'") 
           || strpos($mfh->file, '"') 
           || strpos($mfh->file, '<') 
           || strpos($mfh->file, '>') 
        ) {
           $isValid = false;
        } 

       if (!$isValid) return; // not a valid trigger ie failed one of the above conditions
       if (!qcn_db_init()) { // db liks were setup OK
          $triggerid = insertValidRemoteTrigger($server_id, $hostid, $mfh, false);  // regular db table
          insertValidRemoteTrigger($server_id, $hostid, $mfh, true, $triggerid); // trigmem "live" table

          mysql_close($g_TRIGMEMDB_LINK);
          mysql_close($g_DB_LINK);
       } 
}

function qcn_remote_update_string($server_id, $hostid, &$mfh, $memdb = false, $triggerid=0)
{
    return ($memdb ? "db_name='$server_id',triggerid=$triggerid," : "qcn_remoteid=$server_id,") 
         . "hostid=$hostid, ipaddr='" . $_SERVER['REMOTE_ADDR'] . "',"
         . "result_name='$mfh->result_name', time_trigger=$mfh->ctime, time_received=unix_timestamp(),"
         . "time_sync=$mfh->tsync, sync_offset=$mfh->toff, significance=$mfh->fsig, magnitude=$mfh->fmag,"
         . "dt=$mfh->dt, numreset=$mfh->reset, qcn_sensorid=$mfh->sms, "
         . ($memdb ? "" : "sw_version='$mfh->vr', os_type='$mfh->os',runtime_clock=$mfh->wct, runtime_cpu=$mfh->cpt,")
         . "varietyid="  . ($mfh->variety == "continual" ? 2 : 0) . ","
         . "latitude=$mfh->qlat, longitude=$mfh->qlng, file='$mfh->file',"
         . "mxy1p=$mfh->mxy1p, mz1p=$mfh->mz1p, "
         . "mxy1a=$mfh->mxy1a, mz1a=$mfh->mz1a, "
         . "mxy2a=$mfh->mxy2a, mz2a=$mfh->mz2a, "
         . "mxy4a=$mfh->mxy4a, mz4a=$mfh->mz4a ";

}

function qcn_log_error($errmsg)
{
   global $g_LOGFILE, $g_lognum;
   if (!$g_lognum) {
     $g_lognum = fopen($g_LOGFILE, "a");
   }
   fwrite($g_lognum, "\n\n" . date(DATE_RFC822) . "\n" . $errmsg . "\n");
}

function qcn_log_close()
{
   global $g_lognum;
   if ($g_lognum) { // then log is open
     fclose($g_lognum);
   }
}

/*

typical 'good' xml trigger input:

<scheduler_request>
    <authenticator>dd9fc72b1a747f6458d4ab783a6276a7</authenticator>
    <hostid>630</hostid>
    <rpc_seqno>22900</rpc_seqno>
    <core_client_major_version>7</core_client_major_version>
    <core_client_minor_version>0</core_client_minor_version>
    <core_client_release>99</core_client_release>
    <platform_name>windows_x86_64</platform_name>
    <msg_from_host>
        <result_name>qcnc_053558_0</result_name>
        <time>1379951197</time>
<variety>trigger</variety>
<vr>7.33</vr>
<os>W64</os>
<sms>100</sms>
<ctime>1379951199.285035</ctime>
<fsig>3.005502</fsig>
<fmag>0.341353</fmag>
<file>qcnc_053558_000017_1379951199.zip</file>
<reset>0</reset>
<dt>0.020000</dt>
<mxy1p>0.312256</mxy1p>
<mz1p>0.093137</mz1p>
<tsync>1379950579.463584</tsync>
<toff>2.504157</toff>
<wct>4230.32</wct>
<cpt>16.83</cpt>
<qltat>52.216702</qlat>
<qlng>9.550000</qlng>

    </msg_from_host>
</scheduler_request>


typical followup trigger xml input:
<scheduler_request>
    <authenticator>26f1e24b3537129b0525703feb970762</authenticator>
    <hostid>601</hostid>
    <rpc_seqno>18031</rpc_seqno>
    <core_client_major_version>7</core_client_major_version>
    <core_client_minor_version>0</core_client_minor_version>
    <core_client_release>99</core_client_release>
    <platform_name>windows_intelx86</platform_name>
    <msg_from_host>
        <result_name>qcnc_053434_0</result_name>
        <time>1379947269</time>
<variety>trigger</variety>
<vr>7.33</vr>
<os>W32</os>
<sms>105</sms>
<ctime>1379947262.937414</ctime>
<fsig>3.272128</fsig>
<fmag>0.022373</fmag>
<file>qcnc_053434_000021_1379947263.zip</file>
<reset>0</reset>
<dt>0.020000</dt>
<follow>1</follow>
<mxy1p>0.111433</mxy1p>
<mz1p>0.045831</mz1p>
<mxy1a>0.011396</mxy1a>
<mz1a>0.019832</mz1a>
<mxy2a>0.007996</mxy2a>
<mz2a>0.011488</mz2a>
<mxy4a>0.011048</mxy4a>
<mz4a>0.010030</mz4a>
<tsync>0.000000</tsync>
<toff>0.000000</toff>
<wct>47929.03</wct>
<cpt>3.13</cpt>
<qlat>19.389413</qlat>
<qlng>-99.044881</qlng>

    </msg_from_host>
</scheduler_request>


$try_replica = false;
$unixtimeArchive = mktime(0, 0, 0, date("n"), 1, date("Y")) - (60*24*3600);
$queryCount = 0;

    $config = get_config();
    $user = parse_config($config, "<db_user>");
    $pass = parse_config($config, "<db_passwd>");
    $host = parse_config($config, "<db_host>");

    <db_host>
    <db_user>
    <db_name>
    <db_passwd>
    <trigmem_db_user>
    <trigmem_db_name>
    <trigmem_db_passwd>
    <trigmem_db_host>
    <trigmem_db_port>


    $host = null;
    if ($try_replica == true) {
          $host = parse_config($config, "<replica_db_host>");
    }
    if ($host == null) {
          $host = parse_config($config, "<db_host>");
    }
    if ($host == null) {
        $host = "localhost";
    }
    $link = mysql_pconnect($host, $user, $pass);
    if (!$link) {
        return 1;
    }

*/



?>
