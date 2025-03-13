<?php
  	//debuging
  	$GLOBALS['inDebug']=0;
  	
  	if($GLOBALS['inDebug']!=1){
  		// production settings
  		error_reporting(E_ALL ^ E_NOTICE);
  		$GLOBALS['globalConfigPath']='/var/www/globalconfig/';
  	}else{
  		error_reporting(E_ALL);
  		$GLOBALS['globalConfigPath']='/home/users/vali/work/web/globalconfig/';
  	}

  	// Include required database configuration
    include_once($GLOBALS['globalConfigPath'].'globalconfig.inc.php');
    include_once('include/adodb/adodb.inc.php');
    include_once('include/adodb/adodb-errorhandler.inc.php');

    // Create ADOdb database connection
    $conn = ADONewConnection('mysqli');
    $conn->Connect($dbPlutoAdminServer, $dbPlutoAdminUser, $dbPlutoAdminPass, $dbPlutoAdminDatabase) 
        or die('Could not connect to database');
    $conn->SetFetchMode(ADODB_FETCH_ASSOC);

	$ScenariosArray = getAssocArray('Template', 'PK_Template', 'Description', $conn, '', '');
	$ScenariosArray[''] = 'Undefined';

	$msg = ($_SERVER['QUERY_STRING'] == '') ? 'No parameters specified' : '';

	if(isset($_REQUEST['cgid'])){
		$cgid = (int)$_REQUEST['cgid'];
		if($cgid > 0){
            $query = 'SELECT Description, FK_Template FROM CommandGroup WHERE PK_CommandGroup = ?';
			$res = $conn->Execute($query, array($cgid));
			if($res && $res->RecordCount() > 0){
				$row = $res->FetchRow();
				$commandToSend = '/usr/pluto/bin/MessageSend localhost 0 0 10 '.$cgid;
				//exec($commandToSend);
				$msg = 'Scenario <b>'.$row['Description'].'</b> was executed.';
			} else {
				$msg = 'Invalid scenario.';
			}
		} else {
			$msg = 'Invalid scenario ID.';
		}
	} else {
		$msg = 'Parameter not recognised.';
	}

print $msg;

// function definitions

function getAssocArray($table, $keyField, $labelField, $conn, $whereClause='', $orderClause='')
{
	$retArray = array();
    $query = "SELECT $keyField, $labelField FROM $table $whereClause $orderClause";
	$res = $conn->Execute($query);
    
    if (!$res) {
        die('Query failed: ' . $conn->ErrorMsg());
    }
    
	while($row = $res->FetchRow()){
		$retArray[$row[$keyField]] = $row[$labelField];
	}
	return $retArray;
}

?>