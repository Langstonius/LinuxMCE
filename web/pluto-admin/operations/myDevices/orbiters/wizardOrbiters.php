<?
function wizardOrbiters($output,$dbADO) {
	global $dbPlutoMainDatabase,$excludedData;
	/* @var $dbADO ADOConnection */
	/* @var $rs ADORecordSet */
//	$dbADO->debug=true;
	$userID = (int)@$_SESSION['userID'];
	$out='';
	$action = isset($_REQUEST['action'])?cleanString($_REQUEST['action']):'form';
	$installationID = (int)@$_SESSION['installationID'];
	
	$deviceCategory=$GLOBALS['rootOrbiterID'];
	
	$orbitersDTArray=getDeviceTemplatesFromCategory($deviceCategory,$dbADO,1);
	$roomsArray=getAssocArray('Room','PK_Room','Description',$dbADO,'WHERE FK_Installation='.$installationID, 'ORDER BY Description ASC');

	if(isset($_REQUEST['lastAdded'])){
		$newOrbiterAlert='alert("This device requires some advance preparation, which can take several minutes. Your Core is doing this now and you will see a message on all orbiters and media directors notifying you when it\'s done. Please wait to use the device until then.");
		';
	}
	if ($action == 'form') {
		$out.=setLeftMenu($dbADO).'
	<script>
			function windowOpen(locationA,attributes) {
				window.open(locationA,\'\',attributes);
			}
		'.@$newOrbiterAlert.'

	</script>
	<div class="err">'.(isset($_GET['error'])?strip_tags($_GET['error']):'').'</div>
	<div class="confirm"><B>'.@stripslashes($_GET['msg']).'</B></div>
	<form action="index.php" method="POST" name="devices">
	<input type="hidden" name="section" value="wizardOrbiters">
	<input type="hidden" name="action" value="add">	
	<h3  align="left">Orbiters</h3>
	Download <a href="index.php?section=orbitersWin">Orbiter Win Installer</a><br>
	Privacy settings: <a href="index.php?section=usersOrbiters">restrict acces to users</a>
		<div id="preloader" style="display:;">
			<table width="100%">
				<tr>
					<td align="center">Loading, please wait ...</td>
				</tr>
			</table>
		</div>
		
		<div id="content" style="display:none;">
		<table border="0" align="center">
			<tr>
				<td colspan="2" align="center">
					<input type="submit" class="button" name="QuickRegenAll" value="Quick Regen All"> 
					<input type="submit" class="button" name="FullRegenAll" value="Full Regen All"> 
					<input type="checkbox" name="reset_all" value="1"> Reset Router when done regenerating</td>
			</tr>';
		
		
		
			$displayedDevices=array();
			$DeviceDataToDisplay=array();
			$DeviceDataDescriptionToDisplay=array();

			$orbiterDD=array();
			// WARNING: hard-coded values
			$orbiterDD[]=3;			// default user
			$orbiterDD[]=20;		// No effects
			$orbiterDD[]=21;		// Main 
			$orbiterDD[]=22;		// Sleeping menu
			$orbiterDD[]=23;		// Screen saver menu
			$orbiterDD[]=24;		// Skin
			$orbiterDD[]=25;		// Size
			$orbiterDD[]=26;		// Language
			$orbiterDD[]=56;		// Timeout
			$orbiterDD[]=84;		// Leave Monitor on for OSD
			$orbiterDD[]=91;		// Main Menu
			$orbiterDD[]=104;		// UI
			$orbiterDD[]=111;		// Using Infrared

			$excludedData['standard_roaming_orbiters']=array('state',84);
			$excludedData['mobile_orbiters']=array(84,20,'room','wifi',56);
			$excludedData['on_screen_orbiters']=array('dt','ip_mac','wifi','state',25);
			
			$queryData='
					SELECT 
						IF(FK_DeviceCategory=2,\'mobile_orbiters\',IF(Device.FK_DeviceTemplate=62,\'on_screen_orbiters\',\'standard_roaming_orbiters\')) AS OrbiterGroup,
						Device.*, 
						DeviceTemplate.Description AS TemplateName, 
						DeviceCategory.Description AS CategoryName, 
						FK_DeviceCategory,
						Manufacturer.Description AS ManufacturerName, 
						IsIPBased, 
						RegenInProgress,RegenStatus,RegenPercent, 
						DeviceData.Description AS ddDescription, 
						ParameterType.Description AS typeParam, 
						Device_DeviceData.IK_DeviceData,
						Device_DeviceData.FK_DeviceData,
						ShowInWizard,
						ShortDescription,
						AllowedToModify,
						DeviceTemplate_DeviceData.Description AS Tooltip 
					FROM Device 
					INNER JOIN DeviceTemplate ON Device.FK_DeviceTemplate=PK_DeviceTemplate 
					INNER JOIN DeviceCategory ON FK_DeviceCategory=PK_DeviceCategory 
					INNER JOIN Manufacturer ON FK_Manufacturer=PK_Manufacturer 
					LEFT JOIN Orbiter ON PK_Orbiter=PK_Device 
					INNER JOIN Device_DeviceData ON Device_DeviceData.FK_Device=PK_Device
					INNER JOIN DeviceData ON Device_DeviceData.FK_DeviceData=PK_DeviceData
					INNER JOIN ParameterType ON FK_ParameterType = PK_ParameterType 
					LEFT JOIN DeviceTemplate_DeviceData ON DeviceTemplate_DeviceData.FK_DeviceData=Device_DeviceData.FK_DeviceData AND DeviceTemplate_DeviceData.FK_DeviceTemplate=Device.FK_DeviceTemplate
					WHERE Device.FK_DeviceTemplate IN ('.join(',',array_keys($orbitersDTArray)).') AND Device_DeviceData.FK_DeviceData IN ('.join(',',$orbiterDD).') AND FK_Installation=?
					ORDER BY OrbiterGroup ASC, Device.Description ASC';

			$resDevice=$dbADO->Execute($queryData,$installationID);

			$orbiterGroupDisplayed='';
			$orbiterDisplayed='';
			$PingTest=0;
			$isOSD=0;
			$regenArray=array();
			$content=array();
			$properties=array();
			while($rowD=$resDevice->FetchRow()){
				$orbiterGroupDisplayed=$rowD['OrbiterGroup'];

				if(!in_array($rowD['FK_DeviceData'],$DeviceDataToDisplay)){
					$DeviceDataToDisplay[]=$rowD['FK_DeviceData'];
				}
				
				$PingTest=$rowD['PingTest'];
				$regenArray[$rowD['PK_Device']]['regen']=(int)$rowD['RegenInProgress'];
				$regenArray[$rowD['PK_Device']]['status']=$rowD['RegenStatus'];
				$regenArray[$rowD['PK_Device']]['percent']=$rowD['RegenPercent'];
				
				if($rowD['FK_DeviceData']==84 && @$ddValue==1){
					$isOSD=1;
				}


				if(!in_array($rowD['PK_Device'],$displayedDevices)){
					$displayedDevices[]=$rowD['PK_Device'];
				}


				if(!in_array('dt',$excludedData[$orbiterGroupDisplayed])){
					$content[$orbiterGroupDisplayed][$rowD['PK_Device']]['dt']='
							<tr>
								<td align="right"><B>DeviceTemplate</B></td>
								<td align="left" title="Category: '.$rowD['CategoryName'].', manufacturer: '.$rowD['ManufacturerName'].'">'.$rowD['TemplateName'].'</td>
							</tr>
							';
				}

				if(!in_array('state',$excludedData[$orbiterGroupDisplayed])){
					$content[$orbiterGroupDisplayed][$rowD['PK_Device']]['state']='
							<tr>
								<td align="right"><B>State</B></td>
								<td>'.getStateFormElement($rowD['PK_Device'],'State_'.$rowD['PK_Device'],$rowD['State'],$dbADO).'</td>
							</tr>
							';
				}

				if(!in_array('description',$excludedData[$orbiterGroupDisplayed])){
					$content[$orbiterGroupDisplayed][$rowD['PK_Device']]['description']='
							<tr>
								<td align="right"><B>Description</B></td>
								<td align="left"><input type="text" name="description_'.$rowD['PK_Device'].'" value="'.stripslashes($rowD['Description']).'"> # '.$rowD['PK_Device'].'</td>
							</tr>';
				}

				if($rowD['IsIPBased']==1 && !in_array('ip_mac',$excludedData[$orbiterGroupDisplayed])){
					$content[$orbiterGroupDisplayed][$rowD['PK_Device']]['ip_mac']='
								<tr>
									<td align="right"><B>IP</B></td>
									<td><input type="text" name="ip_'.$rowD['PK_Device'].'" value="'.$rowD['IPaddress'].'"></td>
								</tr>
								<tr>
									<td align="right"><B>MAC</B></td>
									<td><input type="text" name="mac_'.$rowD['PK_Device'].'" value="'.$rowD['MACaddress'].'"></td>
								</tr>';
				}
				
				if(!in_array('room',$excludedData[$orbiterGroupDisplayed])){
					$content[$orbiterGroupDisplayed][$rowD['PK_Device']]['room']='
							<tr>
								<td align="right"><B>Room</B></td>
								<td>'.pulldownFromArray($roomsArray,'room_'.$rowD['PK_Device'],$rowD['FK_Room']).'</td>
							</tr>
							';
				}

				if(!in_array($rowD['FK_DeviceData'],$excludedData[$orbiterGroupDisplayed])){
					@$content[$orbiterGroupDisplayed][$rowD['PK_Device']]['dd'].=formatDDRows($rowD,$dbADO);
				}


				$properties[$rowD['PK_Device']]['regenArray']=$regenArray[$rowD['PK_Device']];
				$properties[$rowD['PK_Device']]['PingTest']=$PingTest;
				$properties[$rowD['PK_Device']]['isOSD']=$isOSD;

			}

			$content['mobile_orbiters']=(@$content['mobile_orbiters']=='')?'<tr><td colspan="2" align="center">No orbiters in this category.</td></tr>':$content['mobile_orbiters'];
			$content['standard_roaming_orbiters']=(@$content['standard_roaming_orbiters']=='')?'<tr><td colspan="2" align="center">No orbiters in this category.</td></tr>':$content['standard_roaming_orbiters'];
			$content['on_screen_orbiters']=(@$content['on_screen_orbiters']=='')?'<tr><td colspan="2" align="center">No orbiters in this category.</td></tr>':$content['on_screen_orbiters'];
			$out.='
				<tr>
					<td bgcolor="lightblue" colspan="2" align="center"><B>Mobile phone orbiters</B></td>
				</tr>
				<tr>
					<td colspan="2" align="center">'.orbiterTable($content['mobile_orbiters'],'mobile_orbiters',$properties).'</td>
				</tr>
				<tr>
					<td bgcolor="lightblue" colspan="2" align="center"><B>Standard roaming orbiters</B></td>
				</tr>
				<tr>
					<td colspan="2" align="center">'.orbiterTable($content['standard_roaming_orbiters'],'standard_roaming_orbiters',$properties).'</td>
				</tr>
				<tr>
					<td bgcolor="lightblue" colspan="2" align="center"><B>On-screen displays for media directors</B></td>
				</tr>
				<tr>
					<td colspan="2" align="center">'.orbiterTable($content['on_screen_orbiters'],'on_screen_orbiters',$properties).'</td>
				</tr>
			
				
			';
			
			
			$out.='
				<input type="hidden" name="DeviceDataToDisplay" value="'.join(',',$DeviceDataToDisplay).'">
				<input type="hidden" name="displayedDevices" value="'.join(',',$displayedDevices).'">';
			$out.='
			</table>
			<table align="center" border="0">
				<tr>
					<td colspan="2">&nbsp;</td>
				</tr>
				<tr>
					<td colspan="2" align="center">'.pulldownFromArray($orbitersDTArray,'deviceTemplate',0).'
					<input type="submit" class="button" name="add" value="Add orbiter"  ></td>
				</tr>
			</table>
			</div>
		</form>
		<script>
		 	var frmvalidator = new formValidator("devices");
 //			frmvalidator.addValidation("Description","req","Please enter a device description");			
//	 		frmvalidator.addValidation("masterDevice","dontselect=0","Please select a Device Template!");			
		</script>
	
	</form>
	';
		$output->setScriptInBody('onLoad="document.getElementById(\'preloader\').style.display=\'none\';document.getElementById(\'content\').style.display=\'\';";');			
	} else {
		// check if the user has the right to modify installation
		$canModifyInstallation = getUserCanModifyInstallation($_SESSION['userID'],$_SESSION['installationID'],$dbADO);
		if (!$canModifyInstallation){
			header("Location: index.php?section=devices&error=You are not authorised to change the installation.");
			exit(0);
		}

		if(isset($_POST['QuickRegenAll'])){
			$ResetRouter=((int)@$_POST['reset_all']==1)?' 24 1':'';
			setOrbitersNeedConfigure($installationID,$dbADO);
				
			$commandToSend='/usr/pluto/bin/MessageSend localhost -targetType template 0 '.$GLOBALS['OrbiterPlugIn'].' 1 266 2 0 21 "-a"'.$ResetRouter;
			exec($commandToSend);
			$regen='Q_ALL';
		}		
		if(isset($_POST['FullRegenAll'])){
			$ResetRouter=((int)@$_POST['reset_all']==1)?' 24 1':'';
			setOrbitersNeedConfigure($installationID,$dbADO);
				
			$commandToSend='/usr/pluto/bin/MessageSend localhost -targetType template 0 '.$GLOBALS['OrbiterPlugIn'].' 1 266 2 0 21 "-r"'.$ResetRouter;
			exec($commandToSend);
			$regen='F_ALL';
		}		

		$displayedDevicesArray=explode(',',$_POST['displayedDevices']);
		foreach($displayedDevicesArray as $value){
			if(isset($_POST['delete_'.$value])){
				deleteDevice($value,$dbADO); 
			}
			if(isset($_POST['quickRegen_'.$value])){
				$updateOrbiter='UPDATE Orbiter SET Regen=1 WHERE PK_Orbiter=?';
				$dbADO->Execute($updateOrbiter,$value); 
				$dbADO->Execute('UPDATE Device SET NeedConfigure=1 WHERE PK_Device=?',$value);
				$updateOrbiters=true;
				$ResetRouter=((int)@$_POST['reset_'.$value]==1)?' 24 1':'';
				
				$commandToSend='/usr/pluto/bin/MessageSend localhost -targetType template '.$value.' '.$GLOBALS['OrbiterPlugIn'].' 1 266 2 '.$value.' 21 '.$ResetRouter;
				exec($commandToSend,$tmp);
				$regen='Q_'.$value;
			}
			if(isset($_POST['fullRegen_'.$value])){
				$updateOrbiter='UPDATE Orbiter SET Modification_LastGen=0 WHERE PK_Orbiter=?';
				$dbADO->Execute($updateOrbiter,$value); 
				$dbADO->Execute('UPDATE Device SET NeedConfigure=1 WHERE PK_Device=?',$value);
				$updateOrbiters=true;
				$ResetRouter=((int)@$_POST['reset_'.$value]==1)?' 24 1':'';
				
				$commandToSend='/usr/pluto/bin/MessageSend localhost -targetType template '.$value.' '.$GLOBALS['OrbiterPlugIn'].' 1 266 2 '.$value.' 21 "-r"'.$ResetRouter;
				exec($commandToSend);
				$regen='F_'.$value;
			}
		}
		
		if(isset($_POST['update']) || isset($updateOrbiters) || $action=='externalSubmit'){

			$DeviceDataToDisplayArray=explode(',',$_POST['DeviceDataToDisplay']);
			foreach($displayedDevicesArray as $key => $value){
				$deviceTemplate=(int)@$_POST['deviceTemplate_'.$value];
				$description=@$_POST['description_'.$value];
				if(isset($_POST['ip_'.$value])){
					$ip=$_POST['ip_'.$value];
					$mac=$_POST['mac_'.$value];
					$updateMacIp=",IPaddress='$ip', MACaddress='$mac'";
				}else{
					$updateMacIp='';
				}
				$updateQuery='';
				
				if(isset($_POST['room_'.$value])){
					$room=((int)@$_POST['room_'.$value]!=0)?(int)@$_POST['room_'.$value]:'NULL';
					$updateQuery.=',FK_Room='.$room.'';
				}
				
				if(isset($_POST['PingTest_'.$value])){
					$pingTest=(int)@$_POST['PingTest_'.$value];
					$updateQuery.=',PingTest='.$pingTest;
				}
				
				$State= (isset($_POST['State_'.$value]))?cleanString($_POST['State_'.$value]):getStateValue('State_'.$value);
				$updateQuery.=',State=\''.$State.'\'';
				
				$updateDevice='UPDATE Device SET Description=? '.$updateQuery.' '.@$updateMacIp.' WHERE PK_Device=?';
				$dbADO->Execute($updateDevice,array($description,$value));

				foreach($DeviceDataToDisplayArray as $ddValue){
					if($ddValue!=91){
						if($ddValue!=56){
							$deviceData=(isset($_POST['deviceData_'.$value.'_'.$ddValue]))?$_POST['deviceData_'.$value.'_'.$ddValue]:NULL;
						}else{
							$deviceData=(isset($_POST['timeoutSS_'.$value]))?$_POST['timeoutSS_'.$value].','.$_POST['timeoutPO_'.$value]:NULL;
						}
						if(!is_null($deviceData)){
							$oldDeviceData=$_POST['oldDeviceData_'.$value.'_'.$ddValue];
							if($oldDeviceData!=$deviceData){
								if($oldDeviceData=='NULL'){
									$insertDDD='
										INSERT INTO Device_DeviceData 
											(FK_Device, FK_DeviceData, IK_DeviceData)
										VALUES 
											(?,?,?)';
									$dbADO->Execute($insertDDD,array($value,$ddValue,$deviceData));
								}
								else{
									$updateDDD='
										UPDATE Device_DeviceData 
											SET IK_DeviceData=? 
										WHERE FK_Device=? AND FK_DeviceData=?';
									$dbADO->Execute($updateDDD,array($deviceData,$value,$ddValue));
								}
							}
							if($ddValue==$GLOBALS['Size']){
								$sizeArray=getFieldsAsArray('Size','Width,Height',$dbADO,'WHERE PK_Size='.$deviceData);
								if(count($sizeArray)>0){
									$resX=$sizeArray['Width'][0];
									$resY=$sizeArray['Height'][0];
									$dbADO->Execute('UPDATE Device_DeviceData SET IK_DeviceData=? WHERE FK_Device=? AND FK_DeviceData=?',array($resX,$value,$GLOBALS['ScreenWidth']));
									$dbADO->Execute('UPDATE Device_DeviceData SET IK_DeviceData=? WHERE FK_Device=? AND FK_DeviceData=?',array($resY,$value,$GLOBALS['ScreenHeight']));
								}
							}
						}
					}
				}
				
				// main menu
				$mainMenu=((int)@$_POST['mainMenu_'.$value]>0)?(int)$_POST['mainMenu_'.$value]:'';
				$dbADO->Execute('UPDATE Device_DeviceData SET IK_DeviceData=? WHERE FK_Device=? AND FK_DeviceData=?',array($mainMenu,$value,91));
			}
		}
		
		if(isset($_POST['add'])){
			$deviceTemplate=(int)$_POST['deviceTemplate'];
			if($deviceTemplate!=0){
				$cmd='sudo -u root /usr/pluto/bin/CreateDevice -h localhost -D '.$dbPlutoMainDatabase.' -d '.$deviceTemplate.' -i '.$installationID.' -C 0';
				$insertID=exec($cmd);				
				$suffix='&lastAdded='.$insertID;
				
				// full regen
				$regenCmd='/usr/pluto/bin/MessageSend localhost -targetType template '.$insertID.' '.$GLOBALS['OrbiterPlugIn'].' 1 266 2 '.$insertID.' 21 "-r"';
				exec($regenCmd);
				
				$regen='F_ALL';
			}
		}
		
		$commandMessage=(isset($commandToSend))?'<br>Command sent: '.$commandToSend:'';
		header("Location: index.php?section=wizardOrbiters&msg=Orbiters updated.$commandMessage&regen=$regen".@$suffix);		
	}

	$output->setScriptCalendar('null');
	$output->setBody($out);
	$output->setTitle(APPLICATION_NAME.' :: Orbiters');
	$output->output();
}


// format and return device data rows
function formatDDRows($rowD,$dbADO)
{
	$ddHTML='';
	$selectedMenu=0;
	$ddValue=$rowD['IK_DeviceData'];
	if((@$rowD['ShowInWizard']==1 || @$rowD['ShowInWizard']=='') && $rowD['FK_DeviceData']!=91){
		if($rowD['FK_DeviceData']!=56){
			$ddHTML.='
				<tr>
					<td align="right"><b>'.((@$rowD['ShortDescription']!='')?$rowD['ShortDescription']:str_replace('PK_','',$rowD['ddDescription'])).'</b> '.((@$rowD['Tooltip']!='')?'<img src="include/images/tooltip.gif" title="'.@$rowD['Tooltip'].'" border="0" align="middle">':'').'</td>
					<td align="left">';
			switch($rowD['typeParam']){
				case 'int':
				if(in_array($rowD['ddDescription'],$GLOBALS['DeviceDataLinkedToTables']))
				{
					$tableName=str_replace('PK_','',$rowD['ddDescription']);
					if(!isset($GLOBALS['ddTableArray_'.$tableName])){
						if($tableName!='Users')
							$GLOBALS['ddTableArray_'.$tableName]=getAssocArray($tableName,$rowD['ddDescription'],'Description',$dbADO,'','ORDER BY Description ASC');
						else
							$GLOBALS['ddTableArray_'.$tableName]=getAssocArray('Users','PK_Users','UserName',$dbADO,'','ORDER BY UserName ASC');
					}
					
					$ddHTML.=pulldownFromArray($GLOBALS['ddTableArray_'.$tableName],'deviceData_'.$rowD['PK_Device'].'_'.$rowD['FK_DeviceData'],$ddValue,((isset($rowD['AllowedToModify']) && $rowD['AllowedToModify']==0)?'disabled':'').' '.(($rowD['FK_DeviceData']==24)?'onChange="document.wizardOrbiters.action.value=\'form\';document.wizardOrbiters.submit();"':''),'key','- Please select -');
				}
				else
					$ddHTML.='<input type="text" name="deviceData_'.$rowD['PK_Device'].'_'.$rowD['FK_DeviceData'].'" value="'.@$ddValue.'" '.((isset($rowD['AllowedToModify']) && $rowD['AllowedToModify']==0)?'disabled':'').'>';
				break;
				case 'bool':
					$ddHTML.='<input type="checkbox" name="deviceData_'.$rowD['PK_Device'].'_'.$rowD['FK_DeviceData'].'" value="1" '.((@$ddValue!=0)?'checked':'').' '.((isset($rowD['AllowedToModify']) && $rowD['AllowedToModify']==0)?'disabled':'').'>';
				break;
				default:
					$ddHTML.='<input type="text" name="deviceData_'.$rowD['PK_Device'].'_'.$rowD['FK_DeviceData'].'" value="'.@$ddValue.'" '.((isset($rowD['AllowedToModify']) && $rowD['AllowedToModify']==0)?'disabled':'').'>';
			}

			$ddHTML.='</td>
				</tr>';
		}
		else{
			$parts=explode(',',$ddValue);
			$ddHTML.='
					<tr>
						<td align="right"><B>Seconds before screen saver</B></td>
						<td><input type="text" name="timeoutSS_'.$rowD['PK_Device'].'" value="'.@$parts[0].'"></td>
					</tr>
						<tr>
						<td align="right"><B>Seconds before power off</B></td>
						<td><input type="text" name="timeoutPO_'.$rowD['PK_Device'].'" value="'.@$parts[1].'"></td>
					</tr>							';			
		}
		$ddHTML.='
					<input type="hidden" name="oldDeviceData_'.$rowD['PK_Device'].'_'.$rowD['FK_DeviceData'].'" value="'.((!is_null($rowD['IK_DeviceData']))?$ddValue:'NULL').'">';					

		unset($ddValue);
	}

	return $ddHTML;
}

function displayButtons($orbiter,$RegenInProgress){
	$out='';
			
	$out.='
		<tr>
			<td align="right"><input type="checkbox" name="reset_'.$orbiter.'" value="1"></td>
			<td>Reset Router when done regenerating</td>
		</tr>';
	if(@$RegenInProgress==1){
		$out.='
			<tr>
				<td colspan="2" align="center"><iframe src="index.php?section=orbiterRegenInProgress&orbiterID='.$orbiter.'" style="width:100%;height:80px;border:0;"></iframe></td>
			</tr>
		';
	}

	$out.='
			<tr>
				<td align="center" colspan="2">
					<input type="submit" class="button" name="quickRegen_'.$orbiter.'" value="Quick regen"  >&nbsp;&nbsp;
					<input type="submit" class="button" name="fullRegen_'.$orbiter.'" value="Full regen"  >&nbsp;&nbsp;
					<input type="button" class="button" name="customBg" value="Custom Bg"  onclick="windowOpen(\'index.php?section=customBackground&oID='.$orbiter.'\',\'width=600,height=400,scrollbars=1,resizable=1\');">&nbsp;&nbsp;
					<input type="submit" class="button" name="update" value="Update"  >&nbsp;&nbsp;
					<input type="button" class="button" name="edit_'.$orbiter.'" value="Adv"  onClick="self.location=\'index.php?section=editDeviceParams&deviceID='.$orbiter.'\';">&nbsp;&nbsp;
					<input type="submit" class="button" name="delete_'.$orbiter.'" value="Delete"  onclick="if(!confirm(\'Are you sure you want to delete this orbiter?\'))return false;">
				</td>
			</tr>	
	';
	return $out;
}

function displayWiFiRow($orbiter,$isOSD,$PingTest){
	$out='';
	
	if($isOSD==0){
		$out.='
		<tr>
			<td align="right"><B>This device uses a Wi-Fi connection</B></td>
			<td><input type="checkbox" name="PingTest_'.$orbiter.'" value="1" '.(($PingTest==1)?'checked':'').'></td>
		</tr>';			
	}
	
	return $out;
}

function orbiterTable($content,$orbiterGroupDisplayed,$properties){
	
	global $excludedData;
	$out='';
	$pos=0;
	foreach ($content AS $orbiter=>$valueArray){
		$pos++;
		$color=($pos%2==1)?'#EEEEEE':'#FFFFFF';
		$out.='<table width="100%" bgcolor="'.$color.'">
			<tr>';
			foreach ($valueArray AS $row){
				$out.=$row;
			}
			$isOSD=$properties[$orbiter]['isOSD'];
			$PingTest=$properties[$orbiter]['PingTest'];
			$regenArray=$properties[$orbiter]['regenArray'];
			if(!in_array('wifi',$excludedData[$orbiterGroupDisplayed])){
				$out.=displayWiFiRow($orbiter,$PingTest,$isOSD);
			}

			$regenQueued=substr(@$_REQUEST['regen'],2);
			$regenBox=($regenQueued==$orbiter || $regenQueued=='ALL' || $regenArray['regen']==1)?1:0;
			$out.=displayButtons($orbiter,$regenBox);
			
			
		$out.='</tr>
		</table>';
	}

	
	return $out;
}
?>