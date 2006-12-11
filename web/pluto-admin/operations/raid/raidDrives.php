<?
function raidDrives($output,$dbADO) {
	// include language files
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/common.lang.php');
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/raid.lang.php');

	/* @var $dbADO ADOConnection */
	/* @var $rs ADORecordSet */
	//$dbADO->debug=true;
	$out='';
	$action = isset($_REQUEST['action'])?cleanString($_REQUEST['action']):'form';
	$installationID = (int)@$_SESSION['installationID'];
	$deviceID=@$_REQUEST['deviceID'];
	
	$raidDrivesCategories=getDescendantsForCategory($GLOBALS['RaidDrives'],$dbADO);
	$raidDrivesTemplates=getAssocArray('DeviceTemplate','PK_DeviceTemplate','Description',$dbADO,'WHERE FK_DeviceCategory IN ('.join(',',$raidDrivesCategories).')');

	if ($action == 'form') {
		$out.='
	<script>
	function formReload(){
		document.raidDrives.action.value="form";
		document.raidDrives.submit();
	}	
	</script>		
	<div class="err" align="center">'.(isset($_GET['error'])?strip_tags($_GET['error']):'').'</div>
	<div class="confirm" align="center"><B>'.@$_GET['msg'].'</B></div>
	<form action="index.php" method="POST" name="raidDrives">
	<input type="hidden" name="section" value="raidDrives">
	<input type="hidden" name="action" value="update">	
	<input type="hidden" name="deviceID" value="'.$deviceID.'">
	<a href="index.php?section=raid">'.$TEXT_BACK_CONST.'</a>
';
	$fields='
		FK_DeviceTemplate,
		PK_Device,Device.Description AS Description,
		DeviceTemplate.Description AS Type,
		DDD1.IK_DeviceData AS Status,
		DDD2.IK_DeviceData AS RAIDStatus,
		DDD3.IK_Devicedata AS NoOfDisks,
		DDD4.IK_DeviceData AS BlockDevice,
		DDD5.IK_DeviceData AS RaidSize';
				
	$join='
		INNER JOIN DeviceTemplate ON FK_DeviceTemplate=PK_DeviceTemplate 
		LEFT JOIN Device_DeviceData DDD1 on DDD1.FK_Device=PK_Device AND DDD1.FK_DeviceData='.$GLOBALS['State'].' 
		LEFT JOIN Device_DeviceData DDD2 on DDD2.FK_Device=PK_Device AND DDD2.FK_DeviceData='.$GLOBALS['RAIDStatus'].' 
		LEFT JOIN Device_DeviceData DDD3 on DDD3.FK_Device=PK_Device AND DDD3.FK_DeviceData='.$GLOBALS['NoofDisks'].'
		LEFT JOIN Device_DeviceData DDD4 on DDD4.FK_Device=PK_Device AND DDD4.FK_DeviceData='.$GLOBALS['BlockDevice'].'
		LEFT JOIN Device_DeviceData DDD5 on DDD5.FK_Device=PK_Device AND DDD5.FK_DeviceData='.$GLOBALS['DriveSize'];	

	$data=getFieldsAsArray('Device',$fields,$dbADO,$join.' WHERE PK_Device='.$deviceID);
	$raidTemplate=$data['FK_DeviceTemplate'][0];
	
	$RAIDStatus=(int)$data['RAIDStatus'][0];
	$createArrayBtn=($RAIDStatus==0)?'<input type="submit" class="button" name="createArray" value="'.$TEXT_CREATE_ARRAY_CONST.'">':'-';
		
	if(count($data)==0){
		$out.='
		<table>
		<tr class="err">
			<td>'.$TEXT_ERROR_INVALID_ID_CONST.'</td>
		</tr>
		</table>';
	}else{
		
		$out.='
		<table>
		<tr>
			<td colspan="5">
			<table>
			<tr class="alternate_back">
				<td align="center"><B>'.$TEXT_ID_CONST.'</B></td>
				<td align="center"><B>'.$TEXT_RAID_CONST.'</B></td>
				<td align="center"><B>'.$TEXT_RAID_TYPE_CONST.'</B></td>
				<td align="center"><B>'.$TEXT_BLOCK_DEVICE_CONST.'</B></td>
				<td align="center"><B>'.$TEXT_NO_OF_DRIVES_CONST.'</B></td>
				<td align="center"><B>'.$TEXT_SIZE_CONST.'</B></td>
				<td align="center"><B>'.$TEXT_RAID_STATUS_CONST.'</B></td>
				<td align="center"><B>'.$TEXT_ACTION_CONST.'</B></td>				
			</tr>		
			<tr class="alternate_back">
				<td>'.$data['PK_Device'][0].'</td>
				<td>'.$data['Description'][0].'</td>
				<td>'.$data['Type'][0].'</td>
				<td align="center">'.$data['BlockDevice'][0].'</td>
				<td align="center">'.$data['NoOfDisks'][0].'</td>
				<td align="center">'.$data['RaidSize'][0].'</td>
				<td align="center">'.$data['Status'][0].'</td>		
				<td align="center">'.$createArrayBtn.'</td>
				</tr>
			</table>
			</td>
		</tr>		
		<tr class="tablehead">
			<td align="center">'.$TEXT_ID_CONST.'</td>
			<td align="center">'.$TEXT_DRIVE_CONST.'</td>
			<td align="center">'.$TEXT_CAPACITY_CONST.'</td>
			<td align="center">'.$TEXT_TYPE_CONST.'</td>
			<td align="center">'.$TEXT_DRIVE_STATUS_CONST.'</td>
			<td align="center">'.$TEXT_ACTION_CONST.'</td>
		</tr>
		';
		$fields='
			PK_Device,
			Device.Description AS Description,
			DDD1.IK_DeviceData AS Capacity,
			DDD2.IK_DeviceData AS Status,
			DDD3.IK_DeviceData AS Type';
		$join='
			INNER JOIN DeviceTemplate ON FK_DeviceTemplate=PK_DeviceTemplate 
			LEFT JOIN Device_DeviceData DDD1 on DDD1.FK_Device=PK_Device AND DDD1.FK_DeviceData='.$GLOBALS['DriveSize'].' 
			LEFT JOIN Device_DeviceData DDD2 on DDD2.FK_Device=PK_Device AND DDD2.FK_DeviceData='.$GLOBALS['State'].' 
			LEFT JOIN Device_DeviceData DDD3 on DDD3.FK_Device=PK_Device AND DDD3.FK_DeviceData='.$GLOBALS['Spare'];
		$drivesData=getFieldsAsArray('Device',$fields,$dbADO,$join.' WHERE FK_DeviceCategory IN ('.join(',',$raidDrivesCategories).') AND FK_Device_ControlledVia='.$deviceID);
		
		if(count($drivesData)==0){
			$out.='
			<tr class="alternate_back">
				<td colspan="6">'.$TEXT_NO_RECORDS_CONST.'</td>
			</tr>';
		}

		$raidDrives=array();
		$noDrives=count(@$drivesData['PK_Device']);
		for ($i=0;$i<$noDrives;$i++){
			$raidDrives[]=$drivesData['PK_Device'][$i];

			$confirmation=$TEXT_CONFIRM_DELETE_DRIVE_CONST;
			$noDelete=0;
			if($RAIDStatus>0 && $drivesData['Type'][$i]!=1){
				switch($data['FK_DeviceTemplate'][0]){
					case $GLOBALS['Raid1']:
						if($noDrives!=2){
							$confirmation=$TEXT_RAID1_DELETE_DRIVE_CONST;
						}else{
							$noDelete=1;
						}
					break;
					case $GLOBALS['Raid5']:
						if($data['Type'][0]==0){
							if($data['NoOfDisks'][0]==$noDrives && $noDrives>2){
								$confirmation=$TEXT_RAID5_DELETE_DRIVE_CONST.' '.($data['NoOfDisks'][0]-1);
							}else{
								$noDelete=1;
							}
						}
					break;
					default:
						if($noDrives==1){
							$noDelete=1;
						}
					break;
				}
			}
	
			$deleteFunction=($noDelete==1)?'alert(\''.addslashes($TEXT_DELETE_NOT_POSIBLE_CONST).'\');return false;':'if(!confirm(\''.addslashes($confirmation).'\'))return false;';		
			
			$class=($i%2!=0)?'alternate_back':'';
			$out.='
			<tr class="alternate_back">
				<td align="center">'.$drivesData['PK_Device'][$i].'</td>
				<td align="center">'.$drivesData['Description'][$i].'</td>
				<td align="center">'.$drivesData['Capacity'][$i].'</td>
				<td align="center">'.(($drivesData['Type'][$i]==1)?'spare':'active').' disk</td>
				<td align="center">'.$drivesData['Status'][$i].'</td>
				<td align="center">
				<input type="button" class="button_fixed" name="edit_'.$drivesData['PK_Device'][$i].'" value="'.$TEXT_ADVANCED_CONST.'"  onClick="self.location=\'index.php?section=editDeviceParams&deviceID='.$drivesData['PK_Device'][$i].'\';"><br>
				<input type="submit" class="button_fixed" name="delete_'.$drivesData['PK_Device'][$i].'" value="'.$TEXT_DELETE_CONST.'"  onClick="'.$deleteFunction.'">
				</td>
			</tr>';
		}
		$availableDrives=getAvailableDrives();

		if(($RAIDStatus==2 && $raidTemplate!=$GLOBALS['Raid0']) || $RAIDStatus!=2){
			$spareChecked=($RAIDStatus!=0)?'checked onClick="this.checked=true;"':'';
			$asSpareCheckbox=($raidTemplate!=$GLOBALS['Raid0'])?$TEXT_AS_SPARE_DISK_CONST.' <input type="checkbox" name="spare" value="1" '.$spareChecked.'>':'';
			$out.='
			<tr>
				<td align="center" colspan="5"> <B>'.$TEXT_AVAILABLE_DRIVES_CONST.'</B> '.pulldownFromArray($availableDrives,'drive','').' '.$asSpareCheckbox.' <input type="submit" class="button" name="add" value="'.$TEXT_ADD_CONST.'"></td>
			</tr>';
			
		}
		$out.='
		</table>
		<input type="hidden" name="raidDrives" value="'.join(',',$raidDrives).'">
		';
	}
	$out.='
	</form>
	';
	} else {
		// check if the user has the right to modify installation
		$canModifyInstallation = getUserCanModifyInstallation($_SESSION['userID'],$_SESSION['installationID'],$dbADO);
		if (!$canModifyInstallation){
			header("Location: index.php?section=raidDrives&error=$TEXT_NOT_AUTHORISED_TO_MODIFY_INSTALLATION_CONST");
			exit(0);
		}

		if(isset($_POST['add']) && @$_POST['drive']!='0'){
			$spare=(int)@$_POST['spare'];
			$newDrive=createDevice($GLOBALS['RaidHardDrive'],$installationID,$deviceID,NULL,$dbADO);
			set_device_data($newDrive,$GLOBALS['Spare'],$spare,$dbADO);
			
			list($name,$capacity)=explode(',',$_POST['drive']);
			$dbADO->Execute('UPDATE Device SET Description=? WHERE PK_Device=?',array($name,$newDrive));
			set_device_data($newDrive,$GLOBALS['DriveSize'],$capacity,$dbADO);
			set_device_data($newDrive,$GLOBALS['BlockDevice'],$name,$dbADO);
			
			// if is spare and raid is created or in progress
			if($spare==1){
				$cmd='sudo -u root /usr/pluto/bin/add_spare.sh '.$newDrive.' '.$deviceID;
				exec_batch_command($cmd);
			}
			
			header("Location: index.php?section=raidDrives&deviceID=$deviceID&msg=".urlencode($TEXT_DRIVE_ADDED_CONST));
			exit();				
		}
		
		$raidDrives=explode(',',$_POST['raidDrives']);
		foreach ($raidDrives AS $drive){
			if(isset($_POST['delete_'.$drive])){
				
				$cmd='sudo -u root /usr/pluto/bin/delete_drive.sh 0 '.$deviceID.' '.$drive;
				exec_batch_command($cmd);
				
				deleteDevice($drive,$dbADO);
				
				header("Location: index.php?section=raidDrives&deviceID=$deviceID&msg=".urlencode($TEXT_DRIVE_DELETED_CONST));
				exit();				
			}
		}
		
		if(isset($_POST['createArray'])){
			if(count($raidDrives)<2){
				header("Location: index.php?section=raidDrives&deviceID=$deviceID&error=".urlencode($TEXT_NO_DRIVES_FOR_ARRAY_CONST));
				exit();				
			}
		
			
			$raidData=getFieldsAsArray('Device','FK_DeviceTemplate',$dbADO,'WHERE PK_Device='.$deviceID);
			$raidTemplate=$raidData['FK_DeviceTemplate'][0];
			switch ($raidTemplate){
				case $GLOBALS['Raid0']:
					$cmd='sudo -u root /usr/pluto/bin/create_RAID0.sh '.$deviceID;
				break;				
				case $GLOBALS['Raid1']:
					$cmd='sudo -u root /usr/pluto/bin/create_RAID1.sh '.$deviceID;
				break;				
				case $GLOBALS['Raid5']:
					$cmd='sudo -u root /usr/pluto/bin/create_RAID5.sh '.$deviceID;
				break;				
			}
			
			exec_batch_command($cmd);
			
			set_device_data($deviceID,$GLOBALS['RAIDStatus'],2,$dbADO);			
			set_device_data($deviceID,$GLOBALS['NoofDisks'],count($raidDrives),$dbADO);
			set_device_data($deviceID,$GLOBALS['State'],'Done',$dbADO);
			
			header("Location: index.php?section=raidDrives&deviceID=$deviceID&msg=".urlencode($TEXT_RAID_ARRAY_CREATED_CONST));
			exit();				
		}
		
		header("Location: index.php?section=raidDrives&deviceID=$deviceID&msg=".urlencode(@$msg));
		exit();
	}

	$output->setMenuTitle($TEXT_ADVANCED_CONST.' |');
	$output->setPageTitle($TEXT_RAID_DRIVES_CONST);

	$output->setScriptCalendar('null');
	$output->setNavigationMenu(array($TEXT_RAID_DRIVES_CONST=>'index.php?section=raidDrives&deviceID='.$deviceID));
	$output->setBody($out);
	$output->setTitle(APPLICATION_NAME);
	$output->output();
}

function getAvailableDrives(){
	$cmd='sudo -u root /usr/pluto/bin/scan_drives.sh';
	$drives=exec_batch_command($cmd,1);
	
	// TODO: remove
	//$drives='/dev/hdd1,80.0 G;/dev/hdd2,80.0 G;/dev/hdd3,120.0 G;/dev/hdd4,160.0 G;';
	
	$drivesArray=explode(';',$drives);
	$drivesAssoc=array();
	foreach ($drivesArray AS $drive){
		if(trim($drive)!=''){
			$drivesAssoc[$drive]=$drive;
		}
	}
	
	return $drivesAssoc;
}
?>
