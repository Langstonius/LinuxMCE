<?php 

function eval_buffer($string) {
   ob_start();
   eval("$string[2];");
   $return = ob_get_contents();
   ob_end_clean();
   return $return;
}

function eval_print_buffer($string) {
   ob_start();
   eval("print $string[2];");
   $return = ob_get_contents();
   ob_end_clean();
   return $return;
}

function showSectionContentWithPhp($string) {
	
   $string = preg_replace_callback("/(<\?=)(.*?)\?>/si",
                                   "eval_print_buffer",$content);
   $string=preg_replace_callback("/(<\?php|<\?)(.*?)\?>/si",
                                 "eval_buffer",$string);
   return stripslashes($string);                                 	
} 

//---------------------------------------------------------------------------------------
//customized data format
//---------------------------------------------------------------------------------------
function formatDate($strData)
{
  $result = trim( $strData);
  $result = str_replace( "  ", " ", $result );
  //list( $aMonth, $aDay, $aYear, $theRest ) = explode( " ", $result, 4 );
  $result = date( "j M, Y",strtotime($result));
  return $result;
}
function formatTime($strData)
{
  $result = trim( $strData);
  $result = str_replace( "  ", " ", $result );
  //list( $aMonth, $aDay, $aYear, $theRest ) = explode(" ", $result, 4);
  $result = date( "G:i:s A", strtotime($result));
  return $result;
}
function formatNumber($nr) {return number_format($nr,0,',','.');}
//---------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------
//listing functions
//---------------------------------------------------------------------------------------
function getPagesMenu($query,$start,$items,$orderBy,$orderType,$url)
{
 $pages_menu = '';
 $urlStr = $GLOBALS['vRoot'].$url;
 $_SESSION['itemsPerPage'] = $items;

 if(($orderBy!='')&&(($orderType!=''))) {
   $_SESSION['sortOrder'] = $orderBy;
   $_SESSION['sortType']  = $orderType;
 }
 
 $res = @mysql_query($query);
 $rows = @(int)mysql_num_rows($res);
 if( ($rows>0) && ($rows>$items) ){
   
     $pages = $rows/$items;
     
     $prev = $start - $items;
     $crt_page = $start / $items;
     $first_page = $crt_page - $GLOBALS["prevNo"] + 1;
     $last_page  = $crt_page + $GLOBALS["nextNo"];
     
     //initialize if necessary
     if ($prev < 0) {$prev = 0;}
     if ($first_page < 1) {$first_page= 1;}
     if ($last_page > $pages) {$last_page = $pages;}
     
     if ($start!=0) {$pages_menu .= '<a class="smallb" href="'.$urlStr.'start='.$prev.'&orderBy='.$orderBy.'&orderType='.$orderType.'">&#171;</a>';}
     for ($i=$first_page; $i<=(int)$last_page; $i++)
     {
      if (($i-1)*$items==$start) {//crt page
       $crt_pos = $i;
       $pages_menu.= ' [<a class="smallnote" href="'.$urlStr.'start='.$items*($i-1).'&orderBy='.$orderBy.'&orderType='.$orderType.'"><b>'.$i.'</b></a>]';
      } 
      else {$pages_menu.= ' [<a class="small" href="'.$urlStr.'start='.$items*($i-1).'&orderBy='.$orderBy.'&orderType='.$orderType.'">'.$i.'</a>]';}
     }
     //last items page
     if (($items*($i-1)) != $rows)
      {
       if (($i-1)*$items==$start){
          $pages_menu.= ' [<a class="smallnote" href="'.$urlStr.'start='.$items*($i-1).'&orderBy='.$orderBy.'&orderType='.$orderType.'"><b>'.$i.'</b></a>]';
       }else{
          $pages_menu.= ' [<a class="small" href="'.$urlStr.'start='.$items*($i-1).'&orderBy='.$orderBy.'&orderType='.$orderType.'">'.$i.'</a>]';
       }
      }
     
    if (((@$crt_pos * $items) != $rows) && ($start!=($items*($i-1)))) {
      $pages_menu.= ' <a class="smallb" href="'.$urlStr.'start='.$items*(@$crt_pos).'&orderBy='.$orderBy.'&orderType='.$orderType.'">&#187;</a>';}
      
    $pages_menu = '<table cellpadding="3" cellspacing="0" border="0"><tr><td class="small">
                  '.$pages_menu.'</td></tr></table>';
 } 
 
 return $pages_menu;
}

//---------------------------------------------------------------------------------------

function die_admin($output,$message,$sourceScript,$query) {
$out=$message."<br />"."query:".$query;
$output->set_title(ApplicationName."::Error");
$output->set_body($out);
$output->do_output();
die();
}

function die_mesg_public($output,$message,$sourceScript,$query) {
	
	
	if ($GLOBALS["inDebug"]==1) {
		$queryTemp=explode("::",$query);
		if (count($queryTemp)==2) {
				$out="<h2>".$message."</h2><h3>Source:$sourceScript</h3><h4>SQL Error:".$queryTemp[0]."</h4><h4>".$queryTemp[1]."</h4>";
		}else{
				$out="<h2>".$message."</h2><h3>Source: $sourceScript</h3><h4>Error:".$query."</h4>";
		}		
		$output->setBody($out);
		$output->output();
	} else {
		$queryTemp=explode("::",$query);
		if (count($queryTemp)==2) {
				$mailBody=$message."\n\n Source:$sourceScript \n\n SQL Error:".$queryTemp[0]." \n\n ".$queryTemp[1]."";
		}else{
				$mailBody=$message." \n\n Source: $sourceScript \n\n ".$query;
		}	
		if (strlen($GLOBALS['sendErrorsTo'])>2) {
			mail($GLOBALS['sendErrorsTo'],"Error on site:".$GLOBALS['vRoot'],$mailBody);
		}
		$out="An error occured and an email about it was sent to the webadmin.<br />Sorry for the incovenience!";
		$output->setBody($out);
		$output->output();
	}
//$output->do_output();
die();
}


function getPagesNextPrev($pagesNumber,$currentPage) {
$output = "";
        if ($pagesNumber > 1) {
            $output.= "<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\"> ";
            $output.= "<tr>";
            $output.= "<td class=\"small\" width=\"25%\" nowrap>";
            if ($currentPage-5>1) {
             $startShowPrevNext  = $currentPage - 5;
            } else {
             $startShowPrevNext  = 1;
            }
            if ($currentPage+5<=$pagesNumber) {
             $endShowPrevNext = $currentPage+5;
            } else {
             $endShowPrevNext = $pagesNumber;
            }

            for ($jk=$startShowPrevNext;$jk<=$endShowPrevNext;$jk++) {
                if ($currentPage==$jk) {
                    $output.="[<b>" + $jk + "</b>]";
                } else {
                    $output .= "[<a class=\"small\" href=\"javascript:changePage(".$jk.");\">".$jk."</a>]";
                }
                $output .= "&nbsp;";
            }
            $output .= "</td>";
            $output .= "<td width=\"19%\" class=\"small\" align=\"right\" nowrap>Go to page:
                    <input class=\"small\" type=\"text\" name=\"gotoPageNo\" maxlength=\"3\" size=\"2\">&nbsp;<input type=\"button\" class=\"small\" value=\"Go\" onClick=\"javascript:formSubmit(this.form);\" id=button1 name=button1>
                    </td>
                    <td class=\"small\" align=\"right\" width=\"1%\"><img src=\"images/spacer.gif\" width=\"29\" height=\"8\"></td>
                    </tr>
                    </table>";
        } else {
            $output = "&nbsp;";
        }
        return $output;
} 

function cleanString($string,$len=1000) {
	return substr(mysql_escape_string(strip_tags(trim($string))),0,$len);
}

function cleanStringWithTags2Show($string,$len=1000) {
	return trim($string);
}

function cleanStringWithTags4DB($string,$len=1000) {
	return mysql_escape_string(trim($string));
}

function cleanInteger($number) {
	return (int)$number;
}

function cleanFloat($number) {
	return (float)$number;
}

function cleanArray($array2Parse) {
	$resultArray = array();
	foreach ($array2Parse as $elem) {
		if (cleanString((trim($elem)))!="") {
			$resultArray[]=$elem;			
		}
	}
	return $resultArray;
}

$GLOBALS['childsArray'] = array();

function getDeviceChildsArray($parentID,$dbADO) {
	if($parentID!=''){
		$queryGP = "SELECT * FROM Device WHERE FK_Device_ControlledVia = $parentID";
		$resGP = $dbADO->Execute($queryGP);
		
		if ($resGP) {
			while ($row=$resGP->FetchRow()) {
					$GLOBALS['childsArray'][]=$row['PK_Device'];
					$GLOBALS['childsArray'][]=getDeviceChildsArray($row['PK_Device'],$dbADO);
			}
		}
	}
	//return $childsArray;
}


$GLOBALS['childsDeviceCategoryNo']=0;
function getDeviceCategoryChildsNo($parentID,$dbADO) {
	$queryGP = "select * from DeviceCategory where FK_DeviceCategory_Parent = $parentID";
	$resGP = $dbADO->Execute($queryGP);
	
	if ($resGP) {
		while ($row=$resGP->FetchRow()) {
				$GLOBALS['childsDeviceCategoryNo']++;
				$GLOBALS['childsDeviceCategoryNo']+=getDeviceCategoryChildsNo($row['PK_DeviceCategory'],$dbADO);
		}
	}	
}

$GLOBALS['childsDeviceCategoryArray'] = array();

function getDeviceCategoryChildsArray($parentID,$dbADO) {
	$queryGP = "select * from DeviceCategory where FK_DeviceCategory_Parent = $parentID";
	$resGP = $dbADO->Execute($queryGP);
	
	if ($resGP) {
		while ($row=$resGP->FetchRow()) {
				$GLOBALS['childsDeviceCategoryArray'][]=$row['PK_DeviceCategory'];				
				$GLOBALS['childsDeviceCategoryArray'][]=getDeviceCategoryChildsArray($row['PK_DeviceCategory'],$dbADO);
		}
	}
}

function getDeviceCategoryChildsOptions($parentID,$parentName,$selectedValue,$notIn,$dbADO) {
	$whereNotIn = '';
	if (strlen($notIn)!='') {
		$whereNotIn=' and PK_DeviceCategory not in ('.$notIn.')';
	}
	$queryGP = "select * from DeviceCategory where FK_DeviceCategory_Parent = $parentID $whereNotIn order by Description Asc";
	$resGP = $dbADO->Execute($queryGP);
	$options='';
	if ($resGP) {
		while ($row=$resGP->FetchRow()) {
				$options.= '<option '.($selectedValue==$row['PK_DeviceCategory']?' selected="selected" ':'').' value="'.$row['PK_DeviceCategory'].'">'.$parentName.' - '.$row['Description'].'</option>';
				$options.= getDeviceCategoryChildsOptions($row['PK_DeviceCategory'],$parentName.' - '.$row['Description'],$selectedValue,$notIn,$dbADO);
		}
		$resGP->Close();
	}
	return $options;
}


function getEventCategoryChildsArray($parentID,$parentName,$selectedValue,$dbADO) {
	$queryGP = "select PK_EventCategory,Description from EventCategory where  FK_EventCategory_Parent = $parentID order by Description Asc";
	$resGP = $dbADO->Execute($queryGP);
	$EventCategoryOptions = '';
	if ($resGP) {
		while ($row=$resGP->FetchRow()) {				
				$EventCategoryOptions.= '<option '.($selectedValue==$row['PK_EventCategory']?' selected="selected" ':'').' value="'.$row['PK_EventCategory'].'">'.$parentName.' - '.$row['Description'].'</option>';
				$EventCategoryOptions.= getEventCategoryChildsArray($row['PK_EventCategory'],$parentName.' - '.$row['Description'],$selectedValue,$dbADO);
		}
	}	
	return $EventCategoryOptions;
}

function getValidOrbitersArray($installationID,$dbADO) {
		$orbiterID = $GLOBALS['rootOrbiterID'];
		$GLOBALS['childsDeviceCategoryArray'] = array();
		getDeviceCategoryChildsArray($orbiterID,$dbADO);
		$validDeviceCategoryOrbiters=cleanArray($GLOBALS['childsDeviceCategoryArray']);
		$validDeviceCategoryOrbiters[]=$orbiterID; //add orbiter root id to array
		
		$getOrbitersFromMasterDeviceList = 'SELECT PK_DeviceTemplate FROM DeviceTemplate WHERE FK_DeviceCategory IN ('.join(",",$validDeviceCategoryOrbiters).')';
		$resOrbitersFromMasterDeviceList = $dbADO->_Execute($getOrbitersFromMasterDeviceList);
		$arrayOrbitersFromMasterDeviceList = array();
		 if ($resOrbitersFromMasterDeviceList) {
		 	while ($rowOrbitersFromMasterDeviceList = $resOrbitersFromMasterDeviceList->FetchRow()) {
		 		$arrayOrbitersFromMasterDeviceList[]=$rowOrbitersFromMasterDeviceList['PK_DeviceTemplate'];
		 	}
		 }
		
		 
		$arrayOrbitersFromMasterDeviceList=cleanArray($arrayOrbitersFromMasterDeviceList);
		$queryOrbitersFromDevices = 'SELECT PK_Device FROM Device WHERE FK_DeviceTemplate in ('.join(",",$arrayOrbitersFromMasterDeviceList).') and FK_Installation = '.$installationID;
		$resOrbitersFromDevices = $dbADO->_Execute($queryOrbitersFromDevices);
		
		$orbitersFromDevicesArray = array();
		if ($resOrbitersFromDevices) {
			while ($rowOrbitersFromDevices = $resOrbitersFromDevices->FetchRow()) {
				$orbitersFromDevicesArray[] = $rowOrbitersFromDevices['PK_Device'];
			}
		}
		
		return $arrayOrbitersFromMasterDeviceList;
}


function InheritDeviceData($masterDeviceID,$insertID,$dbADO)
{
	$selectDeviceDatafromTemplate = 'SELECT FK_DeviceData, IK_DeviceData FROM DeviceTemplate_DeviceData WHERE FK_DeviceTemplate = ?';
	$resDeviceDatafromTemplate = $dbADO->Execute($selectDeviceDatafromTemplate,array($masterDeviceID));
	if ($resDeviceDatafromTemplate) {
		while($rowSelectDeviceDatafromTemplate = $resDeviceDatafromTemplate->FetchRow()){
			$queryInsertDevice = "INSERT INTO Device_DeviceData(FK_Device,FK_DeviceData,IK_DeviceData)
									VALUES(?,?,?)";
			$dbADO->Execute($queryInsertDevice,array($insertID,$rowSelectDeviceDatafromTemplate['FK_DeviceData'],$rowSelectDeviceDatafromTemplate['IK_DeviceData']));
		}
	}
}

function InheritCategoryDeviceData($masterDeviceID,$insertID,$dbADO)
{
	$getDeviceCategoryDeviceData='
	INSERT INTO Device_DeviceData
		(FK_Device, FK_DeviceData, IK_DeviceData)
	SELECT 
		PK_Device, DeviceCategory_DeviceData.FK_DeviceData, DeviceCategory_DeviceData.IK_DeviceData
	FROM Device
		INNER JOIN DeviceTemplate ON Device.FK_DeviceTemplate=PK_DeviceTemplate
		INNER JOIN DeviceCategory_DeviceData on DeviceCategory_DeviceData.FK_DeviceCategory=DeviceTemplate.FK_DeviceCategory
		LEFT JOIN Device_DeviceData ON FK_Device=PK_Device AND Device_DeviceData.FK_DeviceData=DeviceCategory_DeviceData.FK_DeviceData
	WHERE Device_DeviceData.FK_Device IS NULL AND PK_Device=?';
	$dbADO->Execute($getDeviceCategoryDeviceData,$insertID);

	$getParentCategoryDeviceData='
		INSERT INTO Device_DeviceData
			(FK_Device, FK_DeviceData, IK_DeviceData)
		SELECT 
			PK_Device, DeviceCategory_DeviceData.FK_DeviceData, DeviceCategory_DeviceData.IK_DeviceData
		FROM Device
			INNER JOIN DeviceTemplate ON Device.FK_DeviceTemplate=PK_DeviceTemplate
			INNER JOIN DeviceCategory ON DeviceTemplate.FK_DeviceCategory=PK_DeviceCategory
			INNER JOIN DeviceCategory_DeviceData on DeviceCategory_DeviceData.FK_DeviceCategory=DeviceCategory.FK_DeviceCategory_Parent
			LEFT JOIN Device_DeviceData ON FK_Device=PK_Device AND Device_DeviceData.FK_DeviceData=DeviceCategory_DeviceData.FK_DeviceData
		WHERE Device_DeviceData.FK_Device IS NULL AND PK_Device=?';
	$dbADO->Execute($getParentCategoryDeviceData,$insertID);
}

function createChildsForDeviceTemplate($masterDeviceID,$installationID,$insertID,$dbADO,$roomID)
{
	$queryDeviceTemplate='SELECT Description FROM DeviceTemplate WHERE PK_DeviceTemplate=?';
	$resDeviceTemplate=$dbADO->Execute($queryDeviceTemplate,$masterDeviceID);
	$rowDeviceTemplate=$resDeviceTemplate->FetchRow();
	$queryInsertDevice = "INSERT INTO Device(FK_Installation, Description, FK_Device_ControlledVia, FK_DeviceTemplate,FK_Room)
									values(?,?,?,?,?)";
	$dbADO->Execute($queryInsertDevice,array($installationID,$rowDeviceTemplate['Description'],$insertID,$masterDeviceID,$roomID));
	$insertChildID = $dbADO->Insert_ID();
	InheritDeviceData($masterDeviceID,$insertChildID,$dbADO);
	createChildsForControledViaDeviceTemplate($masterDeviceID,$installationID,$insertChildID,$dbADO,$roomID);
}

function createChildsForControledViaDeviceTemplate($masterDeviceID,$installationID,$insertID,$dbADO,$roomID)
{
	// check if DeviceTemplate controll anything
	$queryDeviceTemplate_DeviceTemplate_ControlledVia='SELECT * FROM DeviceTemplate_DeviceTemplate_ControlledVia
				WHERE FK_DeviceTemplate_ControlledVia = ? AND AutoCreateChildren = 1';
	$resDeviceTemplate_DeviceTemplate_ControlledVia=$dbADO->Execute($queryDeviceTemplate_DeviceTemplate_ControlledVia,$masterDeviceID);
	if($resDeviceTemplate_DeviceTemplate_ControlledVia->RecordCount()>0){
		// insert the children
		while($row=$resDeviceTemplate_DeviceTemplate_ControlledVia->FetchRow()){
			createChildsForDeviceTemplate($row['FK_DeviceTemplate'],$installationID,$insertID,$dbADO,$roomID);
		}
	}
}

function createChildsForControledViaDeviceCategory($masterDeviceID,$installationID,$insertID,$dbADO,$roomID)
{
	$getTemplateGategory='SELECT * FROM DeviceTemplate WHERE PK_DeviceTemplate=?';
	$resTemplateGategory=$dbADO->Execute($getTemplateGategory,$masterDeviceID);
	$rowCateg=$resTemplateGategory->FetchRow();
	$parentCateg=$rowCateg['FK_DeviceCategory'];
	
	// get Device Template from controlling Category
	$deviceTemplateArray=array();
	$queryDeviceTemplate_DeviceCategory_ControlledVia='SELECT FK_DeviceTemplate FROM DeviceTemplate_DeviceCategory_ControlledVia
				WHERE FK_DeviceCategory = ? AND AutoCreateChildren = 1';
	$res=$dbADO->Execute($queryDeviceTemplate_DeviceCategory_ControlledVia,$parentCateg);
	if($res->RecordCount()>0){
		while($row=$res->FetchRow()){
			// get all DeviceTemplates from category and create childs for them if necessary
			$deviceTemplateArray[]=$row['FK_DeviceTemplate'];
		}
	}
	foreach($deviceTemplateArray as $value){
		createChildsForDeviceTemplate($value,$installationID,$insertID,$dbADO,$roomID);
	}
}

// $device is the PK_Device of the device who has current options, if '' use default options
function getInstallWizardDeviceTemplates($step,$dbADO,$device='',$distro=0,$operatingSystem=0)
{
	if($distro!=0){
		$queryDistro='SELECT * FROM Distro WHERE PK_Distro=?';
		$resDistro=$dbADO->Execute($queryDistro,$distro);
		if($resDistro->RecordCount()>0){
			$rowDistro=$resDistro->FetchRow();
			$distroName=$rowDistro['Description'];
			$operatingSystem=$rowDistro['FK_OperatingSystem'];
		}else
			$distroName='';
	}
	
	if($operatingSystem!=0){
		$queryOperatingSystem='SELECT * FROM OperatingSystem WHERE PK_OperatingSystem=?';
		$resOperatingSystem=$dbADO->Execute($queryOperatingSystem,$operatingSystem);
		if($resOperatingSystem->RecordCount()>0){
			$rowOperatingSystem=$resOperatingSystem->FetchRow();
			$operatingSystemName=$rowOperatingSystem['Description'];
		}else
			$operatingSystemName='';
	}
			
	$out='<table>';
	$queryInstallWizard='
		SELECT InstallWizard.*,
			DeviceCategory.Description AS Category,
			DeviceTemplate.Description AS Template
		FROM InstallWizard
			INNER JOIN DeviceTemplate ON FK_DeviceTemplate=PK_DeviceTemplate
			INNER JOIN DeviceCategory ON FK_DeviceCategory=PK_DeviceCategory
		WHERE step=?';
	$resInstallWizard=$dbADO->Execute($queryInstallWizard,$step);
	if($resInstallWizard->RecordCount()==0){
		$out.='
			<tr>
				<td>&nbsp;</td>
			</tr>';
	}else{
		$out.='
			<tr class="normaltext">
				<td colspan="5"><b>What software modules do you want on this device?</b> &nbsp; The most common are selected by default.</td>
			</tr>';
	}
	$oldCategory='';
	$displayedTemplatesRequired=array();
	while($row=$resInstallWizard->FetchRow()){
		if($row['Category']!=$oldCategory){
			$oldCategory=$row['Category'];
			$displayCategory=$row['Category'];
		}else 
			$displayCategory='-';
		// check if the device actually exist to display actual entries
		if($device!=''){
			if($device!=@$_SESSION['CoreDCERouter']){
				$queryDevice='SELECT * FROM Device WHERE FK_DeviceTemplate=? AND FK_Device_ControlledVia=?';
				$resDevice=$dbADO->Execute($queryDevice,array($row['FK_DeviceTemplate'],$device));
			}else{
				// if device is DCE Router, check also childs of Core
				$queryDevice='SELECT * FROM Device WHERE FK_DeviceTemplate=? AND (FK_Device_ControlledVia=? OR FK_Device_ControlledVia=?)';
				$resDevice=$dbADO->Execute($queryDevice,array($row['FK_DeviceTemplate'],$_SESSION['CoreDCERouter'],$_SESSION['deviceID']));
			}
			$deviceTemplateChecked=($resDevice->RecordCount()==0)?'':'checked';
			$rowDevice=$resDevice->FetchRow();
			$oldDevice=$rowDevice['PK_Device'];
		}

		if($device==@$_SESSION['CoreDCERouter'] && @$_SESSION['isCoreFirstTime']==1){
			$isCoreFirstTime=1;
			unset($_SESSION['isCoreFirstTime']);
		}

		if($device==@$_SESSION['OrbiterHybridChild'] && @$_SESSION['isHybridFirstTime']==1){
			$isHybridFirstTime=1;
			unset($_SESSION['isHybridFirstTime']);
		}

		
		$out.='
			<tr class="normaltext">
				<td align="center"><b>'.$displayCategory.'</b></td>
				<td>&nbsp;&nbsp;&nbsp;&nbsp;'.$row['Template'].'</td>';
		$query='
			SELECT 
				InstallWizard_Distro.*,
				InstallWizard.Default
			FROM InstallWizard_Distro
				INNER JOIN InstallWizard ON FK_InstallWizard=PK_InstallWizard
			WHERE (FK_Distro IS NULL OR FK_Distro=?) AND (FK_OperatingSystem=? OR FK_OperatingSystem IS NULL) AND FK_InstallWizard=?';
		$res=$dbADO->Execute($query,array($distro,$operatingSystem,$row['PK_InstallWizard']));
		$rowIWD=$res->FetchRow();
		$templateIsChecked=((isset($deviceTemplateChecked) && @$isCoreFirstTime!=1)?$deviceTemplateChecked:(($rowIWD['Default']==1)?'checked':''));
		$out.='<td><input type="checkbox" '.(($res->RecordCount()==0)?'disabled':'').' '.$templateIsChecked.' name="device_'.$device.'_requiredTemplate_'.$row['FK_DeviceTemplate'].'" value="1"> 
			<input type="hidden" name="templateName_'.$row['FK_DeviceTemplate'].'" value="'.$row['Template'].'">
			<input type="hidden" name="oldDevice_'.$device.'_requiredTemplate_'.$row['FK_DeviceTemplate'].'" value="'.@$oldDevice.'">
		';
		if($res->RecordCount()==0)
			$out.='Not available for '.(($distro!=0)?$distroName:'').(($operatingSystem!=0)?' / '.$operatingSystemName:'');
		else{
			$displayedTemplatesRequired[]=$row['FK_DeviceTemplate'];
			$out.=$rowIWD['Comments'];
		}
		$out.='
				</td>
			</tr>
		';
	}

	$out.='	<input type="hidden" name="displayedTemplatesRequired_'.$device.'" value="'.(join(',',$displayedTemplatesRequired)).'">
	</table>';
	return $out;
}

function createInstallWizardDevices($step,$dbADO,$distro=0,$operatingSystem=0)
{
	$queryInstallWizard='
		SELECT InstallWizard.*,
			DeviceTemplate.Description AS Template
		FROM InstallWizard
			INNER JOIN DeviceTemplate ON FK_DeviceTemplate=PK_DeviceTemplate
		WHERE step=?';
	$resInstallWizard=$dbADO->Execute($queryInstallWizard,$step);
	if($resInstallWizard->RecordCount()==0){
		return NULL;
	}
	while($row=$resInstallWizard->FetchRow()){

		$query='
			SELECT 
				InstallWizard_Distro.*,
				InstallWizard.Default
			FROM InstallWizard_Distro
				INNER JOIN InstallWizard ON FK_InstallWizard=PK_InstallWizard
			WHERE (FK_Distro IS NULL OR FK_Distro=?) AND (FK_OperatingSystem=? OR FK_OperatingSystem IS NULL) AND FK_InstallWizard=?';
		$res=$dbADO->Execute($query,array($distro,$operatingSystem,$row['PK_InstallWizard']));
		$rowIWD=$res->FetchRow();

		if($res->RecordCount()!=0){
			if($rowIWD['Default']==1){
				$insertDevice='
					INSERT INTO Device 
						(Description, FK_DeviceTemplate, FK_Installation, FK_Device_ControlledVia,FK_Room) 
					VALUES (?,?,?,?,?)';
				$dbADO->Execute($insertDevice,array($row['Template'],$row['FK_DeviceTemplate'],$_SESSION['installationID'],$_SESSION['OrbiterHybridChild'],$_SESSION['coreRoom']));
				$insertHybridID=$dbADO->Insert_ID();
			}
			$res->MoveFirst();
		}
	}
}

function CheckValidCode($code,$dbADO)
{
	list($device, $pin) = explode("-", $code);
	$sql = "SELECT ActivationCode
			FROM Device
	 			INNER JOIN Installation ON Device.FK_Installation = Installation.PK_Installation
			WHERE Device.PK_Device = '$device' LIMIT 1";
	$res = $dbADO->Execute($sql);
	if ($res->RecordCount() == 0)
		return false;
	$row = $res->FetchRow();
	if ($row['ActivationCode'] !== $pin)
		return false;
	return true;
}

// return an array of packages
function GetActivationSh($code,$param='install',$otherParam='')
{
	global $dbPlutoMainUser;
	global $dbPlutoMainPass;
	global $dbPlutoMainServer;


	list($device, $pin) = explode("-", $code);
#	echo("/usr/pluto/bin/ConfirmDependencies -r -h ".$dbPlutoMainServer." -u ".$dbPlutoMainUser." -p ".$dbPlutoMainPass." -d $device install");
	exec("/usr/pluto/bin/ConfirmDependencies -r -D pluto_main -h $dbPlutoMainServer -u $dbPlutoMainUser -p $dbPlutoMainPass -d $device $otherParam $param", $result, $retcode);
	// || die("ERROR. Can't generate answer: $device:$pin");

	if ($retcode == 0)
		$return = "OK\n" . implode("\n", (array)$result) . "\n#EOF";
	else
		$return = "ERROR. Something went terribly wrong while generating the activation script. The installation can not continue. Please contact Plutohome to solve this problem. Thank you.";
	return $return;
}

function GetInitialData($installation,$user)
{
	exec("/usr/pluto/bin/GetInitialData.sh " . $installation . " " . $user, $result);
//		|| die("ERROR. Can't generate: $installation:$user");

	return $result;
}

function getMediaDirectorOrbiterChild($MD_PK_Device,$dbADO)
{
	$DTArray=getDeviceTemplatesFromCategory($GLOBALS['rootOrbiterID'],$dbADO);
	if(count($DTArray)==0)
		return null;
	$getOrbiterChild='SELECT * FROM Device WHERE FK_DeviceTemplate IN ('.join(',',$DTArray).') AND FK_Device_ControlledVia=?';
	$resOrbiterChild=$dbADO->Execute($getOrbiterChild,array($MD_PK_Device));
	if($resOrbiterChild->RecordCount()!=0){
		$rowOrbiterChild=$resOrbiterChild->FetchRow();
		return $rowOrbiterChild['PK_Device'];
	}
	return null;
}

function deleteDevice($PK_Device,$dbADO)
{
	getDeviceChildsArray($PK_Device,$dbADO);
	$toDelete = cleanArray($GLOBALS['childsArray']);
	$toDelete[]=$PK_Device;
	if (!is_array($toDelete)) {
		$toDelete=array();
	}
	foreach ($toDelete as $elem) {
	
		$arrayFKDeviceTables=array('CommandGroup_Command','Device_Command','Device_CommandGroup','Device_DeviceData','Device_DeviceGroup','Device_Device_Related','Device_EntertainArea','Device_HouseMode','Device_Orbiter','Device_StartupScript','Device_Users','InfraredGroup_Command');
		if($elem!=''){
			foreach($arrayFKDeviceTables AS $tablename){	
				$queryDelFromTable='DELETE FROM '.$tablename.' WHERE FK_Device='.$elem;
				$dbADO->Execute($queryDelFromTable);
			}		
			$queryDelDevice = 'DELETE FROM Device WHERE PK_Device = '.$elem;
			$dbADO->_Execute($queryDelDevice);
		}
	}

}

function addDeviceToEntertainArea($deviceID,$entArea,$dbADO)
{
	$queryDE='SELECT * FROM Device_EntertainArea WHERE FK_Device=? AND FK_EntertainArea=?';
	$resDE=$dbADO->Execute($queryDE,array($deviceID,$entArea));
	if($resDE->RecordCount()==0){
		$insertDeviceEntertainArea='INSERT INTO Device_EntertainArea (FK_Device, FK_EntertainArea) VALUES (?,?)';
		$dbADO->Execute($insertDeviceEntertainArea,array($deviceID,$entArea));
	}
					
	if(isOrbiter($deviceID,$dbADO) || isMediaDirector($deviceID,$dbADO)){
		$queryChilds='SELECT * FROM Device WHERE FK_Device_ControlledVia=? AND FK_Installation=?';
		$resChilds=$dbADO->Execute($queryChilds,array($deviceID,$_SESSION['installationID']));
		if($resChilds->RecordCount()>0){
			while($rowChilds=$resChilds->FetchRow()){
				addDeviceToEntertainArea($rowChilds['PK_Device'],$entArea,$dbADO);
			}
		}
	}
}

function isOrbiter($deviceID,$dbADO)
{
	$orbitersArray=array();
	$orbitersArray=getValidOrbitersArray($_SESSION['installationID'],$dbADO);
	if(in_array($deviceID,$orbitersArray))
		return true;
	return false;
}

function isMediaDirector($deviceID,$dbADO)
{
	$getTemplate='SELECT FK_DeviceTemplate,FK_Device_ControlledVia FROM Device WHERE PK_Device=?';
	$resTemplate=$dbADO->Execute($getTemplate,$deviceID);
	if($resTemplate->RecordCount()>0){
		$row=$resTemplate->FetchRow();
		$DeviceTemplate=$row['FK_DeviceTemplate'];
	}else
		return false;
	if($DeviceTemplate==$GLOBALS['rootGenericMediaDirector']){
		return true;
	}
	elseif($row['FK_Device_ControlledVia']=='' )
		return false;
	else{
		return isMediaDirector($row['FK_Device_ControlledVia'],$dbADO);
	}
}

function addMediaCommandGroup($optionName,$FK_EntertainArea, $installationID,$dbADO)
{
	$arrayID=$GLOBALS['ArrayIDForMedia'];
	
	$getTemplateID='SELECT * FROM Template WHERE Description=?';
	$resTemplateID=$dbADO->Execute($getTemplateID,'Media Wiz - '.$optionName);
	$rowTemplateID=$resTemplateID->FetchRow();
	$FK_Template=$rowTemplateID['PK_Template'];

	$insertScenario='INSERT INTO CommandGroup (FK_Array, FK_Installation, Description,FK_Template) VALUES (?,?,?,?)';
	$dbADO->Execute($insertScenario,array($arrayID,$installationID,$optionName,$FK_Template));
	$insertID=$dbADO->Insert_ID();

	$insertDeviceCommandGroup='INSERT INTO CommandGroup_EntertainArea (FK_EntertainArea, FK_CommandGroup,Sort) VALUES (?,?,?)';
	$dbADO->Execute($insertDeviceCommandGroup,array($FK_EntertainArea,$insertID,$insertID));
	$CG_C_insertID=$dbADO->Insert_ID();
	$queryInsertCommandGroup_Command = "
		INSERT INTO CommandGroup_Command 
			(FK_CommandGroup,FK_Command,FK_Device) 
		VALUES(?,?,?)";								
	if($optionName=='TV'){
		$queryMediaPlugin='SELECT * FROM Device WHERE FK_Installation=? AND FK_DeviceTemplate=?';
		$resMediaPlugin=$dbADO->Execute($queryMediaPlugin,array($installationID,$GLOBALS['rootMediaPlugin']));
		$rowMediaPlugin=$resMediaPlugin->FetchRow();
		$mediaPluginID=$rowMediaPlugin['PK_Device'];

		$dbADO->Execute($queryInsertCommandGroup_Command,array($insertID,$GLOBALS['commandMHPlayMedia'],$mediaPluginID));
		$CG_C_insertID=$dbADO->Insert_ID();

		$insertCommandParam='INSERT INTO CommandGroup_Command_CommandParameter (FK_CommandGroup_Command,FK_CommandParameter,IK_CommandParameter) VALUES (?,?,?)';
		$dbADO->Execute($insertCommandParam,array($CG_C_insertID,$GLOBALS['commandParamPK_MediaType'],1));

	}elseif($optionName=='playlists'){
		$dbADO->Execute($queryInsertCommandGroup_Command,array($insertID,$GLOBALS['commandGotoScreen'],$GLOBALS['localOrbiter']));
		$CG_C_insertID=$dbADO->Insert_ID();
		$insertCommandParam='INSERT INTO CommandGroup_Command_CommandParameter (FK_CommandGroup_Command,FK_CommandParameter,IK_CommandParameter) VALUES (?,?,?)';
		$dbADO->Execute($insertCommandParam,array($CG_C_insertID,$GLOBALS['commandParamPK_DesignObj'],$GLOBALS['butViewDesignObj']));

	}else{

		$dbADO->Execute($queryInsertCommandGroup_Command,array($insertID,$GLOBALS['commandSetVar'],$GLOBALS['localOrbiter']));
		$CG_C_insertID=$dbADO->Insert_ID();

		$insertCommandParam='INSERT INTO CommandGroup_Command_CommandParameter (FK_CommandGroup_Command,FK_CommandParameter,IK_CommandParameter) VALUES (?,?,?)';
		$dbADO->Execute($insertCommandParam,array($CG_C_insertID,$GLOBALS['commandParameterVariableNumber'],2));
		$dbADO->Execute($insertCommandParam,array($CG_C_insertID,$GLOBALS['commandParameterValueToAsign'],$optionName));

		$dbADO->Execute($queryInsertCommandGroup_Command,array($insertID,$GLOBALS['commandGotoScreen'],$GLOBALS['localOrbiter']));
		$CG_C_insertID=$dbADO->Insert_ID();
		$dbADO->Execute($insertCommandParam,array($CG_C_insertID,$GLOBALS['commandParamPK_DesignObj'],$GLOBALS['mnuMediaFileListDesignObj']));

		$dbADO->Execute($queryInsertCommandGroup_Command,array($insertID,$GLOBALS['commandSetVar'],$GLOBALS['localOrbiter']));
		$CG_C_insertID=$dbADO->Insert_ID();

		switch ($optionName){
			case 'music':
			$parmValue=4;
			break;
			case 'movies':
			$parmValue=3;
			break;
			case 'videos':
			$parmValue=5;
			break;
			case 'pictures':
			$parmValue=7;
			break;
		}
		$dbADO->Execute($insertCommandParam,array($CG_C_insertID,$GLOBALS['commandParameterVariableNumber'],14));
		$dbADO->Execute($insertCommandParam,array($CG_C_insertID,$GLOBALS['commandParameterValueToAsign'],$parmValue));
	}
}

function dbQuery($query,$conn,$error='MySQL error')
{
	$res=mysql_query($query,$conn) or die($error.": ".mysql_error($conn));
	return $res;
}

function getDeviceTemplatesFromCategory($categoryID,$dbADO)
{
	getDeviceCategoryChildsArray($categoryID,$dbADO);
	$GLOBALS['childsDeviceCategoryArray']=cleanArray($GLOBALS['childsDeviceCategoryArray']);
	$GLOBALS['childsDeviceCategoryArray'][]=$categoryID;
	
	$queryDeviceTemplate='
		SELECT * FROM DeviceTemplate 
			WHERE FK_DeviceCategory IN ('.join(',',$GLOBALS['childsDeviceCategoryArray']).')
		ORDER BY Description ASC';
	$resDeviceTemplate=$dbADO->Execute($queryDeviceTemplate);
	$DTArray=array();
	while($rowDeviceCategory=$resDeviceTemplate->FetchRow()){
		$DTArray[]=$rowDeviceCategory['PK_DeviceTemplate'];
	}
	return $DTArray;
}

function addScenariosToRoom($roomID, $installationID, $dbADO)
{
	$insertTelecomScenario='INSERT INTO CommandGroup (FK_Array, FK_Installation, Description,FK_Template,Hint) SELECT '.$GLOBALS['ArrayIDCommunicationScenarios'].','.$installationID.',\'Phone\','.$GLOBALS['TelecomScenariosTemplate'].',Description FROM Room WHERE PK_Room=?';
	$dbADO->Execute($insertTelecomScenario,$roomID);
	$cgID=$dbADO->Insert_ID();

	$insertCG_R='INSERT INTO CommandGroup_Room (FK_Room, FK_CommandGroup,Sort) VALUES (?,?,?)';
	$dbADO->Execute($insertCG_R,array($roomID,$cgID,$cgID));

	$queryInsertCommandGroup_Command = "INSERT INTO CommandGroup_Command (FK_CommandGroup,FK_Command,FK_Device) VALUES(?,?,?)";
	$dbADO->Execute($queryInsertCommandGroup_Command,array($cgID,$GLOBALS['commandGotoScreen'],$GLOBALS['localOrbiter']));
	$CG_C_insertID=$dbADO->Insert_ID();

	$insertCommandParam='INSERT INTO CommandGroup_Command_CommandParameter (FK_CommandGroup_Command,FK_CommandParameter,IK_CommandParameter) VALUES (?,?,?)';
	$dbADO->Execute($insertCommandParam,array($CG_C_insertID,$GLOBALS['commandParamID'],''));
	$dbADO->Execute($insertCommandParam,array($CG_C_insertID,$GLOBALS['commandParamPK_DesignObj'],$GLOBALS['TelecomMenu']));
	$dbADO->Execute($insertCommandParam,array($CG_C_insertID,$GLOBALS['commandParamDesignObjCurrentScreen'],''));
	$dbADO->Execute($insertCommandParam,array($CG_C_insertID,$GLOBALS['commandParamPK_Device'],''));
	$dbADO->Execute($insertCommandParam,array($CG_C_insertID,$GLOBALS['commandParamStoreVariables'],''));

	$insertCG_Room='INSERT INTO CommandGroup_Room (FK_CommandGroup, FK_Room,Sort) VALUES (?,?,?)';
	$insertCG_C='
		INSERT INTO CommandGroup_Command 
			(FK_CommandGroup,FK_Command,TurnOff,OrderNum,FK_Device)
		VALUES
			(?,?,?,?,?)';
	$insertCG_C_CP='
		INSERT INTO CommandGroup_Command_CommandParameter 
			(FK_CommandGroup_Command,FK_CommandParameter,IK_CommandParameter)
		VALUES
			(?,?,?)';
	
	$selectArmDisarm='SELECT * FROM CommandGroup WHERE FK_Template=? AND FK_Installation=?';
	$resArmDisarm=$dbADO->Execute($selectArmDisarm,array($GLOBALS['SecurityArmDisarmTemplate'],$installationID));
	if($resArmDisarm->RecordCount()){
		$rowArmDisarm=$resArmDisarm->FetchRow();
		$armDisarmCG=$rowArmDisarm['PK_CommandGroup'];
	}else{
		$insertCommandGroup='
				INSERT INTO CommandGroup 
					(FK_Array,FK_Installation,Description,CanTurnOff,AlwaysShow,CanBeHidden,FK_Template)
				VALUES
					(?,?,?,?,?,?,?)';
		$dbADO->Execute($insertCommandGroup,array($GLOBALS['ArrayIDForSecurity'],$installationID,'Security Panel',0,0,0,$GLOBALS['SecurityArmDisarmTemplate']));
		$armDisarmCG=$dbADO->Insert_ID();

		$dbADO->Execute($insertCG_C,array($armDisarmCG,$GLOBALS['commandGotoScreen'],0,0,$GLOBALS['localOrbiter']));
		$cg_cID=$dbADO->Insert_ID();
		$dbADO->Execute($insertCG_C_CP,array($cg_cID,$GLOBALS['commandParamPK_DesignObj'],$GLOBALS['mnuSecurityPanel']));
	}
	$dbADO->Execute($insertCG_Room,array($armDisarmCG,$roomID,$armDisarmCG));
	
}

function htmlPulldown($contentArray,$name,$selectedValue='None',$unselectedLabel='- Please select -',$jsCode='')
{
	if(!is_array($contentArray))
		return '';	// error
	$out='<select name="'.$name.'" '.$jsCode.'>
		<option value="0">'.$unselectedLabel.'</option>';
	foreach ($contentArray AS $key=>$label){
		$out.='<option value="'.$key.'" '.(($key==$selectedValue)?'selected':'').'>'.$label.'</option>';
	}
	$out.='</select>';
	return $out;
}

function getAssocArray($table,$keyField,$labelField,$dbADO,$whereClause='',$orderClause='')
{
	$retArray=array();
	$res=$dbADO->Execute("SELECT $keyField,$labelField FROM $table $whereClause $orderClause");
	while($row=$res->FetchRow()){
		$cleanField=(strpos($labelField,'.')!==false)?substr($labelField,strpos($labelField,'.')+1):$labelField;
		$cleanField=(strpos($cleanField,' AS ')!==false)?substr($cleanField,strpos($cleanField,' AS ')+4):$cleanField;
		$cleanField=trim($cleanField);

		$retArray[$row[$keyField]]=$row[$cleanField];
	}
	return $retArray;
}

function createDevice($FK_DeviceTemplate,$FK_Installation,$controlledBy,$roomID,$dbADO,$childOfMD=0)
{
	$orbiterID=getMediaDirectorOrbiterChild($controlledBy,$dbADO);
	
	$parentID=($childOfMD==0)?$orbiterID:$controlledBy;
	$insertDevice="INSERT INTO Device (Description, FK_DeviceTemplate, FK_Installation,FK_Device_ControlledVia) SELECT Description,PK_DeviceTemplate,$FK_Installation,$parentID FROM DeviceTemplate WHERE PK_DeviceTemplate='$FK_DeviceTemplate'";
	$dbADO->Execute($insertDevice);
	$insertID=$dbADO->Insert_ID();
	
	InheritDeviceData($FK_DeviceTemplate,$insertID,$dbADO);
	createChildsForControledViaDeviceTemplate($FK_DeviceTemplate,$FK_Installation,$insertID,$dbADO,$roomID);
	createChildsForControledViaDeviceCategory($FK_DeviceTemplate,$FK_Installation,$insertID,$dbADO,$roomID);

}

function delInstallation($installationID,$dbADO)
{
	$installationFKArray=array('CommandGroup','Criteria','Device','DeviceGroup','EventHandler','Floorplan','Household_Installation','InfraredGroup_Command','InfraredGroup_Command_Preferred','Installation_RepositorySource_URL','Installation_Users');
	foreach($installationFKArray AS $tablename){	
		$queryDelFromTable='DELETE FROM '.$tablename.' WHERE FK_Installation='.$installationID;
		$dbADO->Execute($queryDelFromTable);
	}
	$dbADO->Execute('DELETE FROM Installation WHERE PK_Installation=?',$installationID);
}

function dbConnection($dbServer,$dbUser,$dbPass,$dbDatabase)
{
  $dbADO = &ADONewConnection('mysql');
  $dbADO->NConnect($dbServer,urlencode($dbUser),urlencode($dbPass),urlencode($dbDatabase)); 
  
  return $dbADO;
}

function getFieldsAsArray($tableName,$fields,$dbADO,$filter='',$orderBy='')
{
	$fieldsArray=explode(',',$fields);
	$res=$dbADO->execute("SELECT $fields FROM $tableName $filter $orderBy");
	$result=array();
	while($row=$res->Fetchrow()){
		foreach ($fieldsArray AS $field){
			$cleanField=(strpos($field,'.')!==false)?substr($field,strpos($field,'.')+1):$field;
			$cleanField=(strpos($cleanField,' AS ')!==false)?substr($cleanField,strpos($cleanField,' AS ')+4):$cleanField;
			$cleanField=trim($cleanField);

			$result[$cleanField][]=$row[$cleanField];
		}
	}
	return $result;	
}


// display array nicely formated - used for debug only
function print_array($arr)
{
	print '<pre>';
	print_r($arr);
	print '</pre>';
}


function getmicrotime() 
{ 
    list($usec, $sec) = explode(" ", microtime()); 
    return ((float)$usec + (float)$sec); 
}

function pulldownFromArray($valuesArray,$name,$selectedValue,$extra='',$valueKey='key',$zeroValueDescription='- Please select -',$highlightValue=-1)
{
	$out='<select name="'.$name.'" "'.$extra.'">';
	if($zeroValueDescription!='')
		$out.='<option value="0">'.$zeroValueDescription.'</option>';
	foreach ($valuesArray AS $key=>$value){
		$optionValue=($valueKey=='key')?$key:$value;
		$out.='<option value="'.$optionValue.'" '.(($optionValue==$selectedValue)?'selected':'').' '.((in_array($optionValue,explode(',',$highlightValue)))?'style="background:lightgreen;"':'').'>'.$value.'</option>
		';
	}
	$out.='</select>';
	return $out;
}

function getUnixStamp($date)
{
	$dateArray=explode(' ',$date);
	$dateParts=explode('-',$dateArray[0]);
	$timeParts=explode(':',$dateArray[1]);

	return mktime($timeParts[0],$timeParts[1],$timeParts[2],$dateParts[1],$dateParts[2],$dateParts[0]);
}

function convertMac($mac)
{
	return exec('/usr/pluto/bin/convert_mac "' . $mac. '"');
	
	$macArray=explode(':',$mac);
	if(count($macArray)!=6)
		return false;
		
		
	$int64 = 0;
	for ($i = 0; $i < 6; $i++){
	    $value = hexdec($macArray[$i]);
	    $power = pow(256, 5-$i);
	    $int64 += $power * $value;
	    #$int64 *= 256;
	    #$int64 += $value;
	    echo $int64. " ";
	}
	
	return $int64;
}


function isMacRange($adr){
	$parts=explode('-',$adr);
	$no=count($parts);
	if($no==1){
		return checkMAC(trim($parts[0]));
	}elseif($no==2){
		if(checkMAC(trim($parts[0])) && checkMAC(trim($parts[1]))){
			return true;
		}
	}else{
		return false;
	}
	
	return false;
}

function checkMAC($addr){
	$rc = false;
	if ((strlen($addr)==17) && (!preg_match('/[^A-Fa-f0-9\:]/',$addr)))
	{
		$mac = explode(":", $addr);
		if (count($mac)==6)
		{
			$rc = true;
			for ($i=0;$i<=5;$i++)
			{
				if ((strlen($mac[$i])!=2)		||
				(hexdec($mac[$i])<0)	||
				(hexdec($mac[$i]>255)))
				{
					$rc = false;
					break;
				}
			}
		}
	}
	return $rc;
}

function IntToMac($nr){
	return exec('/usr/pluto/bin/convert_mac "' . $nr. '"');
	$hexArr=array();
	$pos=0;
	while(floor($nr)>0){
		$hexdigit = $nr % 256;
		if ($hexdigit < 0)
			$hexdigit += 256;
		echo '<br>'.$hexdigit;
		$hexArr[5-$pos]=dechex($hexdigit);
		$nr /= 256;
		$pos++;
	}

	if(count($hexArr)!=6){
		return false;
	}
	
	return join(':',$hexArr);
}
?>
