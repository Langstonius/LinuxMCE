<?
session_name('Pluto');
session_start();
require('../include/config/config.inc.php');

	$oldParams=$_SERVER['QUERY_STRING'];
	$newParams=(strpos($oldParams,'&')!==false)?substr($oldParams,strpos($oldParams,'&')+1):$oldParams;

	if(!isset($_SESSION['userID'])){
		header("Location: http://plutohome.com/support/mantis/");
		exit();
	}else{
		header("Location: http://plutohome.com/support/mantis/login.php?username=".$_SESSION['username']."&password=".$_SESSION['extPassword']."&login=1&return=main_page.php");
	}
$out='

';
		
?>