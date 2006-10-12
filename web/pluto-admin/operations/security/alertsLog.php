<?
function alertsLog($output,$securitydbADO,$dbADO) {
	// include language files
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/common.lang.php');
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/alertsLog.lang.php');
	
	/* @var $securitydbADO ADOConnection */
	/* @var $res ADORecordSet */
	
	$out='';
	$action = isset($_REQUEST['action'])?cleanString($_REQUEST['action']):'form';

	if (@$_SESSION['userLoggedIn']!=true) {
		header("Location: index.php?section=login");
	}
		$installationID = cleanInteger($_SESSION['installationID']);
	
	if ($action=='form') {
		$out.='<div class="err">'.(isset($_GET['error'])?strip_tags($_GET['error']):'').'</div>';
		$out.='
			<form action="index.php" method="post" name="alertsLog">
			<input type="hidden" name="section" value="alertsLog">
			<input type="hidden" name="action" value="add">
			';
		$query='
			SELECT 
				PK_Alert,
				Description, 
				UNIX_TIMESTAMP(DetectionTime) AS DetectionTime, 
				UNIX_TIMESTAMP(ExpirationTime) AS ExpirationTime, 
				IF(ResetTime IS NOT NULL,UNIX_TIMESTAMP(ResetTime),NULL) AS ResetTime,
				EK_Device,
				ResetBeforeExpiration,
				Benign,
				EK_Users
			FROM Alert
				INNER JOIN AlertType ON FK_Alerttype=PK_AlertType
			ORDER BY DetectionTime DESC';
		$out.=multipageAlertsLogs($query, 'index.php?section=alertsLog', (isset($_GET['page_no']))?$_GET['page_no']-1:0, 10,$securitydbADO,$dbADO);
		
		$out.='		
		</form>
		';
	} else {
		// no processing 
				
		header("Location: index.php?section=alertsLog&msg=Alerts types updated.");
	}
	
	$output->setMenuTitle($TEXT_SECURITY_CONST.' |');
	$output->setPageTitle($TEXT_ALERTS_LOG_CONST);
	$output->setNavigationMenu(array($TEXT_ALERTS_LOG_CONST=>'index.php?section=alertsLog'));				
	$output->setBody($out);
	$output->setTitle(APPLICATION_NAME.' :: '.$TEXT_ALERTS_LOG_CONST);			
	$output->output();  		
}

function multipageAlertsLogs($query, $url, $page_no, $art_pagina,$securitydbADO,$dbADO)
{
	// include language files
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/common.lang.php');
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/alertsLog.lang.php');
	
	$res=$securitydbADO->Execute($query);
	$total=$res->RecordCount();
	$max_pages = $total/$art_pagina;
	if( $max_pages - floor($max_pages) >= 0 )
		$max_pages = floor($max_pages) + 1;
	else
		$max_pages = floor( $max_pages );

	if($total!=0){
		$output= '<div align="center">';
		for($i=0;$i<$max_pages;$i++){
			$output.=($page_no==$i)?'<font color="red">'.($i+1).'</font> ':'<a href='.$url.'&page_no='.($i+1).'>[ '.($i+1).' ]</a> ';
		}
		$output.='</div>';
	}
	if($total==0){
		$output.=$TEXT_NO_RECORDS_CONST;
	}
	else{
		$output.=headerAlertLog();
		$art_index=0;
		$start = $page_no * $art_pagina;
		$res=$securitydbADO->Execute($query." LIMIT  $start,$art_pagina");
	    while($row = $res->FetchRow()){
			$art_index++;
			if($art_index-1 == $art_pagina)
		        break;
			$output.=formatAlertsLog($row, $art_index+$start,$securitydbADO,$dbADO);
		}
		$output.=footerAlertLog();
	}
	$output.='<div align="center">'.$TEXT_FOUND_CONST.': '.$total.'. ';
	if($total!=0)
		$output.= $TEXT_PAGE_CONST.' '.($page_no+1).' '.$TEXT_TOTAL_PAGES_CONST.' '.$max_pages.'.</div><br>';
	return $output;
}

function headerAlertLog()
{
	// include language files
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/common.lang.php');
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/alertsLog.lang.php');
	
	$output='<table border="0" align="center" cellspacing="0" cellpadding="2">
				<tr class="tablehead">
					<td align="center"><B>'.$TEXT_ALERT_TYPE_CONST.'</B></td>
					<td align="center"><B>'.$TEXT_DETECTION_TIME_CONST.'</B></td>
					<td align="center"><B>'.$TEXT_EXPIRATION_TIME_CONST.'</B></td>
					<td align="center"><B>'.$TEXT_RESET_BEFORE_EXPIRATION_CONST.'</B></td>
					<td align="center"><B>'.$TEXT_BENIGN_CONST.'</B></td>
					<td align="center"><B>'.$TEXT_RESET_TIME_CONST.'</B></td>
					<td align="center"><B>'.$TEXT_USER_CONST.'</B></td>
				</tr>';
	return $output;
}

function footerAlertLog()
{
	$output='</table>';
	return $output;
}

function formatAlertsLog($row, $art_index,$securitydbADO,$dbADO)
{
	$device='';
	
	$user='';
	if($row['EK_Users']!=''){
		$queryUsers='SELECT * FROM Users WHERE PK_Users=?';
		$resUsers=$dbADO->Execute($queryUsers,$row['EK_Users']);
		if($resUsers){
			$rowUsers=$resUsers->FetchRow();
			$user=$rowUsers['UserName'];
		}
	}

	$out='
		<tr class="'.(($art_index%2==0)?'alternate_back':'').'">
			<td align="center"><B>'.$row['Description'].'</B></td>
			<td align="center">'.date($GLOBALS['defaultDateFormat'],$row['DetectionTime']).'</td>
			<td align="center">'.date($GLOBALS['defaultDateFormat'],$row['ExpirationTime']).'</td>
			<td align="center"><input type="checkbox" name="checkb" '.(($row['ResetBeforeExpiration']==1)?'checked':'').' disabled></td>
			<td align="center"><input type="checkbox" name="checkb" '.(($row['Benign']==1)?'checked':'').' disabled></td>
			<td align="center">'.((is_null($row['ResetTime']))?'Not reseted':date($GLOBALS['defaultDateFormat'],$row['ResetTime'])).'</td>
			<td align="center">'.$user.'</td>
		</tr>
	';
	$queryAD='SELECT PK_Alert_Device,EK_Device,UNIX_TIMESTAMP(DetectionTime) AS DetectionTime  FROM Alert_Device WHERE FK_Alert=?';
	$resAD=$securitydbADO->Execute($queryAD,$row['PK_Alert']);
	while($rowAD=$resAD->FetchRow()){
		$resD=$dbADO->Execute('SELECT * FROM Device LEFT JOIN Device_Device_Related ON FK_Device=PK_Device WHERE PK_Device =?',$rowAD['EK_Device']);
		while($rowD=$resD->FetchRow()){
			// snapshot format: alert_[PK_Alert]_cam[camera].png
			$picPath=$GLOBALS['SecurityPicsPath'].'alert_'.$rowAD['PK_Alert_Device'].'_cam'.$rowD['FK_Device_Related'].'.png';
			$alertPic=(file_exists($picPath))?$picPath:APPROOT.'include/images/alert_no_pic.png';
		$out.='
			<tr bgcolor="'.(($art_index%2==0)?'#F0F3F8':'').'">
				<td align="center"><img src="include/image.php?imagepath='.$alertPic.'"></td>
				<td align="center">'.date($GLOBALS['defaultDateFormat'],$rowAD['DetectionTime']).'</td>
				<td align="left" colspan="5"><B>Device: </B><a href="index.php?section=editDeviceParams&deviceID='.$rowD['PK_Device'].'">'.$rowD['Description'].'</a></td>
			</tr>
			';
		}
	}	
	
	$queryNotifications='SELECT * FROM Notification WHERE FK_Alert=?';
	$resNotifications=$securitydbADO->Execute($queryNotifications,$row['PK_Alert']);
	while($rowNotification=$resNotifications->FetchRow()){
		$out.='
		<tr class="'.(($art_index%2==0)?'alternate_back':'').'">
			<td align="center"><B>Notification</B></td>
			<td align="center">'.date($GLOBALS['defaultDateFormat'],$rowNotification['NotificationTime']).'</td>
			<td align="left" colspan="2"><B>Info: </B>'.$rowNotification['Info'].'</td>
			<td align="left" colspan="3"><B>Result: </B>'.$rowNotification['Result'].'</td>
		</tr>
		';
	}
	
	return $out;
}

?>