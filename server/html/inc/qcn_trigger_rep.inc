<?php
// CMC this file will process remote server triggers and put them in the qcn_trigger_memory table as well as qcn_trigger_remote

function qcn_trigger_rep_process($basefile)
{
   $fname = "/tmp/" . basename($basefile,".php") . ".log";
   $body = "\n" . @file_get_contents('php://input');
   $ipaddr = "\n<ip_remote>" . $_SERVER['REMOTE_ADDR'] . "</ip_remote>\n<ip_fwd>" . $_SERVER['HTTP_X_FORWARDED_FOR'] . "</ip_fwd>\n";
   $fp = fopen($fname, "a");
   fwrite($fp, $ipaddr);
   fwrite($fp, $body);
   fclose($fp);
}

?>
