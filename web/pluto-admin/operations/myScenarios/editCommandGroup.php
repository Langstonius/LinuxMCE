<?php
function editCommandGroup($output,$dbADO) {
	// include language files
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/common.lang.php');
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/editCommandGroup.lang.php');
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/scenarioWizard.lang.php');
	
	/* @var $dbADO ADOConnection */
	/* @var $insertRs ADORecordSet */
	/* @var $output Template */

	$action = isset($_REQUEST['action'])?cleanString($_REQUEST['action']):'form';
	$out='';
	$installationID = (int)@$_SESSION['installationID'];
	$commandGroupID = @(int)$_REQUEST['cgID'];
	$from = isset($_REQUEST['from'])?cleanString($_REQUEST['from']):'';
	$hook='';
	$errNotice='';
	
	if(isset($_REQUEST['dcgID'])){
		$commandGroupID = (int)$_REQUEST['dcgID'];
		
		$canModifyInstallation = getUserCanModifyInstallation($_SESSION['userID'],$installationID,$dbADO);
		if (!$canModifyInstallation) {
			header("Location: index.php?section=editCommandGroup&cgID=$commandGroupID&error=$TEXT_NOT_AUTHORISED_TO_MODIFY_INSTALLATION_CONST");
			exit();
		}
		
		deleteCommandGroup($commandGroupID,$dbADO);
		setOrbitersNeedConfigure($installationID,$dbADO);
		header("Location: index.php?section=myScenarios&msg=$TEXT_SCENARIO_DELETED_CONST");
	}
	
		
	if ($action=='form') {
		$query = "SELECT * FROM CommandGroup WHERE PK_CommandGroup = ? AND FK_Installation = ?";
		$resCommandGroupDetails = $dbADO->Execute($query,array($commandGroupID,$installationID));
		
		$filePath=$GLOBALS['scenariosPicsPath'].$commandGroupID.'.png';
		if(file_exists($filePath)){
			$randNumber=rand(0,99999);
			$scenarioImg='<img src="include/image.php?imagepath='.$filePath.'&rand='.$randNumber.'"><br>';
		}else
			$scenarioImg='&nbsp;';		
		
		if (!$resCommandGroupDetails || $resCommandGroupDetails->RecordCount()!=1) {
			$out=$TEXT_ERROR_SCENARIO_DOES_NOT_EXISTS_CONST;
		} else {
			$rowCommandGroupDetails = $resCommandGroupDetails->FetchRow();
			
			$out.='
			<script>
				function windowOpen(locationA,attributes) {
					window.open(locationA,\'\',attributes);
				}
			</script>
			
			<form action="index.php" method="POST" name="editCommandGroup" enctype="multipart/form-data">
				<input type="hidden" name="section" value="editCommandGroup">
				
				<input type="hidden" name="action" value="add">	
				<input type="hidden" name="cgID" value="'.$commandGroupID.'">
			
				<div align="center" class="confirm"><B>'.stripslashes(@$_REQUEST['msg']).'</B></div>
				<div align="center" class="err"><B>'.stripslashes(@$_REQUEST['error']).'</B></div>
			
			<fieldset><legend>'.$TEXT_SCENARIO_DATA_CONST.'</legend>
				<table border="0">
						<tr>
							<td><B>'.$TEXT_DESCRIPTION_CONST.'</B> *</td>
							<td><textarea name="description" style="width:100%;" rows="3">'.$rowCommandGroupDetails['Description'].'</textarea></td>
						</tr>
						<tr>
							<td><B>'.$TEXT_HINT_CONST.'</B></td>
							<td><input type="text" style="width:100%;" name="hint" value="'.$rowCommandGroupDetails['Hint'].'"></td>
						</tr>
						<tr>
							<td><B>'.$TEXT_ARRAY_TYPE_CONST.'</B></td>
							<td>
								<select name="arrayType" style="width:100%;">';
			
			$queryArrays = "SELECT * FROM Array Where CommandGroup = 1 order by Description Asc";
			$resArrays = $dbADO->Execute($queryArrays);
			
						while ($rowArray=$resArrays->FetchRow()) {
							$out.='<option '.($rowArray['PK_Array']==$rowCommandGroupDetails['FK_Array']?" selected='selected' ":'').' value="'.$rowArray['PK_Array'].'">'.$rowArray['Description'].'</option>';
						}
						
			$out.='
								</select>
						</td></tr>
						<tr>
					<td><B>'.$TEXT_DESIGN_OBJ_CONST.'</B></td>
					<td>
						'.(@(int)$_SESSION['editCommandGroup']['designObjID']!=0
								?$TEXT_DESIGN_OBJ_SELECTED_CONST.': <b>'.$_SESSION['editCommandGroup']['designObjName'].'</b>&nbsp;&nbsp;<br />'
								:(
									isset($_GET['smsg'])
									?'<b>'.strip_tags($_GET['smsg']).'</b>'
									:'')
								 ).' 
								 <input type="text" name="designObjID" size="20" value="'.(isset($_SESSION['editCommandGroup']['designObjID']) && $_SESSION['editCommandGroup']['designObjID'] != 0 ?$_SESSION['editCommandGroup']['designObjID']:$rowCommandGroupDetails['FK_DesignObj']).'">
								 <input type="submit" class="button" name="searchCommandGroup" value="'.$TEXT_SEARCH_CONST.' [...]"  >
					</td>
				</tr>
				<tr>
					<td><B>'.$TEXT_TEMPLATE_CONST.'</B></td>
					<td>
						<select name="template" style="width:100%;">
							<option value="0">'.$TEXT_PLEASE_SELECT_CONST.'</option>';
	
							$queryTemplates = "SELECT * FROM Template order by Description Asc";
							$resTemplates = $dbADO->Execute($queryTemplates);
										while ($rowTemplates=$resTemplates->fetchRow()) {
											$out.='<option '.($rowTemplates['PK_Template']==$rowCommandGroupDetails['FK_Template']?" selected='selected' ":'').' value="'.$rowTemplates['PK_Template'].'">'.$rowTemplates['Description'].'</option>';
										}
						
							$out.='
						</select>
				</td></tr>
				<tr>
					<td><B>'.(($rowCommandGroupDetails['FK_Array']==$GLOBALS['ArrayIDForMedia'])?$TEXT_ENTERTAINMENT_AREA_CONST:$TEXT_ROOM_CONST).'</B></td>
					<td>';
					if($rowCommandGroupDetails['FK_Array']==$GLOBALS['ArrayIDForMedia']){
						$queryCheckedEntArea='SELECT * FROM CommandGroup_EntertainArea WHERE FK_CommandGroup=?';
						$resCheckedEntArea=$dbADO->Execute($queryCheckedEntArea,$commandGroupID);
						$checkedEntArea=array();
						while($rowCheckedEntArea=$resCheckedEntArea->FetchRow()){
							$checkedEntArea[]=$rowCheckedEntArea['FK_EntertainArea'];
						}
						$queryEntArea='
							SELECT EntertainArea.* FROM EntertainArea
								INNER JOIN Room ON FK_Room=PK_Room
							WHERE 
								FK_Installation=?
							ORDER BY EntertainArea.Description ASC';
						$resEntArea=$dbADO->Execute($queryEntArea,$installationID);
						$displayedEntArea=array();
						$EntAreaOldValues=array();
						while($rowEntArea=$resEntArea->FetchRow()){
							$displayedEntArea[]=$rowEntArea['PK_EntertainArea'];
							$EntAreaOldValues[]=(in_array($rowEntArea['PK_EntertainArea'],$checkedEntArea))?1:0;
							$out.='<input type="checkbox" value="1" name="entArea_'.$rowEntArea['PK_EntertainArea'].'" '.((in_array($rowEntArea['PK_EntertainArea'],$checkedEntArea))?'checked':'').'> '.$rowEntArea['Description'].'<br>';
						}
						$out.='<input type="hidden" name="displayedEntArea" value="'.join(",",$displayedEntArea).'">';
						$out.='<input type="hidden" name="EntAreaOldValues" value="'.join(",",$EntAreaOldValues).'">';
					}else{
						$queryCheckedRooms='SELECT * FROM CommandGroup_Room WHERE FK_CommandGroup=?';
						$resCheckedRooms=$dbADO->Execute($queryCheckedRooms,$commandGroupID);
						$checkedRooms=array();
						while($rowCheckedRooms=$resCheckedRooms->FetchRow()){
							$checkedRooms[]=$rowCheckedRooms['FK_Room'];
						}
						$queryRooms='SELECT Room.* FROM Room WHERE FK_Installation=? ORDER BY Room.Description ASC';
						$resRooms=$dbADO->Execute($queryRooms,$installationID);
						$displayedRooms=array();
						$roomsOldValues=array();
						while($rowRooms=$resRooms->FetchRow()){
							$displayedRooms[]=$rowRooms['PK_Room'];
							$roomsOldValues[]=(in_array($rowRooms['PK_Room'],$checkedRooms))?1:0;
							$out.='<input type="checkbox" value="1" name="room_'.$rowRooms['PK_Room'].'" '.((in_array($rowRooms['PK_Room'],$checkedRooms))?'checked':'').'> '.$rowRooms['Description'].'<br>';
						}
						$out.='<input type="hidden" name="displayedRooms" value="'.join(",",$displayedRooms).'">';
						$out.='<input type="hidden" name="roomsOldValues" value="'.join(",",$roomsOldValues).'">';
					}
			$out.='</td>
				</tr>
				<tr>
					<td>'.$scenarioImg.'<B>'.$TEXT_PICTURE_CONST.' **</B></td>
					<td><input type="file" name="pic"></td>
				</tr>
				<tr>
					<td colspan="2">
						<input type="submit" class="button_fixed" name="submitX" value="'.$TEXT_SAVE_CONST.'"  >
						<input type="reset" class="button_fixed" name="cancelBtn" value="'.$TEXT_CANCEL_CONST.'">
						<input type="button" class="button_fixed" name="deleteCG" value="'.$TEXT_DELETE_SCENARIO_CONST.'" onClick="if(confirm(\''.$TEXT_DELETE_SCENARION_CONFIRMATION_CONST.'\'))self.location=\'index.php?section=editCommandGroup&dcgID='.$commandGroupID.'\'">
					</td>
				</tr>
				</table>
			</fieldset>
			<em>
			* '.$TEXT_REQUIRED_FIELDS_CONST.'<BR>
			** '.$TEXT_PICTURE_RECOMMENDED_CONST.'</em><br><br>
			
			<fieldset><legend>'.$TEXT_SCENARIO_COMMANDS_CONST.'</legend>
			<table>
				';
						
				$selectCommandsAssigned = "
				SELECT CommandGroup_Command.* FROM 
					CommandGroup_Command				
					LEFT JOIN 
						Device ON CommandGroup_Command.FK_Device = Device.PK_Device						
					WHERE 
						(Device.FK_Installation = ? OR Device.FK_Installation IS NULL)
					AND CommandGroup_Command.FK_CommandGroup = ?
				ORDER BY OrderNum ASC
				";
				$resCommandAssigned = $dbADO->Execute($selectCommandsAssigned,array($installationID,$commandGroupID));

				if ($resCommandAssigned->RecordCount()>0) {
				$out.='<tr>
					<td colspan="2">
							<fieldset>
								<legend>'.$TEXT_COMMANDS_CONST.'</legend>
								<table>
									';
				
				while ($rowCommandAssigned = $resCommandAssigned->FetchRow()) {
					$out.='
						<tr>
							<td valign="top">
								<a name="hook_'.$rowCommandAssigned['PK_CommandGroup_Command'].'"></a>
								<input type="hidden" name="device_'.$rowCommandAssigned['PK_CommandGroup_Command'].'" value="'.$rowCommandAssigned['FK_Device'].'">
								'.getDeviceNameForScenarios($rowCommandAssigned['FK_Device'],$dbADO).'</td>';

					 	$out.='
					 		</td>
						<td>
						';
					if($rowCommandAssigned['FK_Device']!='-300'){
						$query = "SELECT PK_Command,Command.Description
								FROM Device 
									INNER JOIN DeviceTemplate_DeviceCommandGroup ON Device.FK_DeviceTemplate = DeviceTemplate_DeviceCommandGroup.FK_DeviceTemplate 
									INNER JOIN DeviceCommandGroup_Command ON DeviceTemplate_DeviceCommandGroup.FK_DeviceCommandGroup = DeviceCommandGroup_Command.FK_DeviceCommandGroup 
									INNER JOIN Command on DeviceCommandGroup_Command.FK_Command = Command.PK_Command
							   WHERE PK_Device=?";
						$resNewCommand = $dbADO->Execute($query,array($rowCommandAssigned['FK_Device']));
					}else{
						$query = "
							SELECT PK_Command,Command.Description
								FROM Command 
									INNER JOIN DeviceCommandGroup_Command ON 
										DeviceCommandGroup_Command.FK_Command = Command.PK_Command
									INNER JOIN DeviceTemplate_DeviceCommandGroup ON 
										DeviceTemplate_DeviceCommandGroup.FK_DeviceCommandGroup = DeviceCommandGroup_Command.FK_DeviceCommandGroup 
							   WHERE FK_DeviceTemplate=?
									ORDER BY Command.Description ASC";
						$resNewCommand = $dbADO->Execute($query,array(cleanInteger($GLOBALS['deviceTemplateOrbiter'])));
					}
						if ($resNewCommand) {
							$out.='<select name="deviceCommand_'.$rowCommandAssigned['PK_CommandGroup_Command'].'" onChange="this.form.submit();">';
									while ($rowNewCommand = $resNewCommand->FetchRow()) {
										$out.='<option '.($rowCommandAssigned['FK_Command']==$rowNewCommand['PK_Command']?'selected="selected"':'').' value="'.$rowNewCommand['PK_Command'].'">'.$rowNewCommand['Description'].'</option>';
									}
									if ($resNewCommand->RecordCount()==0) {
										$out.='<option value="0">-'.$TEXT_SCENARIO_NO_COMMAND_CONST.'-</option>';
									}
						$out.='</select>';
						}
						
						
							$query = "SELECT Command_CommandParameter.FK_CommandParameter,Command_CommandParameter.Description as C_CP_Description,CommandParameter.Description as CP_Description,
										CommandGroup_Command_CommandParameter.IK_CommandParameter,
										ParameterType.Description as PT_Description
											FROM Command_CommandParameter 
												INNER JOIN Command on FK_Command = PK_Command
												INNER JOIN CommandParameter ON Command_CommandParameter.FK_CommandParameter = CommandParameter.PK_CommandParameter
												INNER JOIN ParameterType ON CommandParameter.FK_ParameterType = ParameterType.PK_ParameterType
												LEFT JOIN CommandGroup_Command_CommandParameter ON CommandGroup_Command_CommandParameter.FK_CommandParameter = Command_CommandParameter.FK_CommandParameter
									  WHERE FK_Command = ? AND FK_CommandGroup_Command =?
									  ORDER BY CommandParameter.Description asc
								";
							$resSelectParameters = $dbADO->Execute($query,array($rowCommandAssigned['FK_Command'],$rowCommandAssigned['PK_CommandGroup_Command']));

								if ($resSelectParameters) {
									$out.='<table>';
									while ($rowSelectParameters=$resSelectParameters->FetchRow()) {
										$out.="<tr ".(strlen(trim($rowSelectParameters['CP_Description']))==0?" bgColor='lightgreen' ":"").">
												<td>#{$rowSelectParameters['FK_CommandParameter']} <span title=\"{$rowSelectParameters['C_CP_Description']}\">{$rowSelectParameters['CP_Description']}</span> ({$rowSelectParameters['PT_Description']})</td>
												<td><input type='text' name=\"CommandParameterValue_{$rowCommandAssigned['PK_CommandGroup_Command']}_{$rowSelectParameters['FK_CommandParameter']}\" value=\"{$rowSelectParameters['IK_CommandParameter']}\" >".'</td></tr>';
									}
									$out.='</table>';
								}
								
						
					$out.='
					
							</td>
						<td valign="top">
						<input type="button" class="button" name="editA" value="'.$TEXT_REMOVE_CONST.'" onClick="if(confirm(\''.$TEXT_CONFIRM_DELETE_COMMAND_FROM_SCENARIO_CONST.'\'))windowOpen(\'index.php?section=deleteCommandFromCommandGroup_Command&from=editCommandGroup&cgID='.$rowCommandAssigned['PK_CommandGroup_Command'].'\',\'width=100,height=100,toolbars=true,resizable=1,scrollbars=1\');"">
						</td>						
					</tr>
					<tr>
						<td align="center" colspan="2"><input type="button" class="button" name="testCommand" value="'.$TEXT_TEST_COMMAND_CONST.'" onClick="self.location=\'index.php?section=editCommandGroup&cgID='.$commandGroupID.'&cgcID='.$rowCommandAssigned['PK_CommandGroup_Command'].'&action=testCommand\'"></td>
					</tr>
					';
				}
				
				$out.='
									<tr>
										<td colspan="3" align="center">
											<a name="hook_0"></a>
											<input type="submit" class="button" name="addNewDeviceButton" value="'.$TEXT_SAVE_CONST.'"  >
										</td>
									</tr>
								</table>
							</fieldset>
					</td>
				</tr>
				';
				}//end if there are no commands
				
				if ($rowCommandGroupDetails['FK_Array']==$GLOBALS['ArrayIDForLight']) {
						$devicesAllowed = getValidLightingObjectsArray($installationID,$dbADO);						
				}
				
				if ($rowCommandGroupDetails['FK_Array']==$GLOBALS['ArrayIDForClimate']) {
						$devicesAllowed = getValidClimateObjectsArray($installationID,$dbADO);						
				}
				
				if ($rowCommandGroupDetails['FK_Array']==$GLOBALS['ArrayIDForSecurity']) {
						$devicesAllowed = getValidSecurityObjectsArray($installationID,$dbADO);						
				}
				
				$out.='<tr>
					<td colspan="2"><a name="addDevice"></a>
					 Device:'.
				deviceForScenariosSelector('addNewDevice',cleanInteger(@$_REQUEST['newDevice']),$dbADO,1,'onChange="this.form.submit();"');
				
				if (isset($_REQUEST['newDevice']) && cleanInteger($_REQUEST['newDevice'])!=0) {
					if(cleanInteger(@$_REQUEST['newDevice']!='-300')){
						$query = "SELECT PK_Command,Command.Description
										FROM Device 
												INNER JOIN DeviceTemplate_DeviceCommandGroup ON Device.FK_DeviceTemplate = DeviceTemplate_DeviceCommandGroup.FK_DeviceTemplate 
												INNER JOIN DeviceCommandGroup_Command ON DeviceTemplate_DeviceCommandGroup.FK_DeviceCommandGroup = DeviceCommandGroup_Command.FK_DeviceCommandGroup 
												INNER JOIN Command on DeviceCommandGroup_Command.FK_Command = Command.PK_Command
									   WHERE PK_Device=?";
						$resNewCommand = $dbADO->Execute($query,array(cleanInteger($_REQUEST['newDevice'])));
					}else{
						$query = "
							SELECT PK_Command,Command.Description
								FROM Command 
									INNER JOIN DeviceCommandGroup_Command ON 
										DeviceCommandGroup_Command.FK_Command = Command.PK_Command
									INNER JOIN DeviceTemplate_DeviceCommandGroup ON 
										DeviceTemplate_DeviceCommandGroup.FK_DeviceCommandGroup = DeviceCommandGroup_Command.FK_DeviceCommandGroup 
							   WHERE FK_DeviceTemplate=?
									ORDER BY Command.Description ASC";
						$resNewCommand = $dbADO->Execute($query,array(cleanInteger($GLOBALS['deviceTemplateOrbiter'])));
					}
					if ($resNewCommand) {
						$out.=' Command: <select name="addNewDeviceCommand">
							<option value="0">-please select-</option>';
								while ($rowNewCommand = $resNewCommand->FetchRow()) {
									$out.='<option value="'.$rowNewCommand['PK_Command'].'">'.$rowNewCommand['Description'].'</option>';
								}
								if ($resNewCommand->RecordCount()==0) {
									$out.='<option value="0">-no command-</option>';
								}
					$resNewCommand->Close();
					$out.='</select>';
					}
				}
				$out.='
						<input type="submit" class="button" name="addNewDeviceButton" value="'.$TEXT_ADD_CONST.'"  >
					</td>
				</tr>
				</table>	
				</fieldset>
				<em>'.$TEXT_HIGHLIGHT_INFO_CONST.'</em><br>
				<br>
			
			</form>
			<script>
		 		var frmvalidator = new formValidator("editCommandGroup");
 				frmvalidator.addValidation("description","req","'.$TEXT_SCENARION_DESCRIPTION_REQUIRED_CONST.'");
			</script>
			';
		}
		
	} else {
		//check if current user canModifyInstallation
		$canModifyInstallation = getUserCanModifyInstallation($_SESSION['userID'],$installationID,$dbADO);
	
		
		$designObjID = cleanInteger(@$_POST['designObjID']);
		$_SESSION['editCommandGroup']['designObjID'] = 0;
		$_SESSION['editCommandGroup']['designObjName'] = '';
		
		
		$querySelectCommandGroup = "
							select  PK_DesignObj,Description from DesignObj
							where PK_DesignObj = ?";
		$rs = $dbADO->Execute($querySelectCommandGroup,array($designObjID));
		while ($row = $rs->FetchRow()) {
				$_SESSION['editCommandGroup']['designObjID'] = $row['PK_DesignObj'];
				$_SESSION['editCommandGroup']['designObjName'] = $row['Description'];
		}
		$rs->Close();
		
			
		if (isset($_POST['searchCommandGroup'])) {
			if ($_SESSION['editCommandGroup']['designObjID'] !=0) {
				header("Location: index.php?section=editCommandGroup&from=$from&cgID=$commandGroupID");
				exit();
			} else {
				header("Location: index.php?section=editCommandGroup&from=$from&cgID=$commandGroupID&smsg=Invalid+ID");
				exit();				
			}
		}
		
		
		
		//die();				
		if ($canModifyInstallation) {

			if(isset($_REQUEST['cgcID'])){
				// test command
				$cgcID=$_REQUEST['cgcID'];
				$queryCommands='
					SELECT CommandGroup_Command_CommandParameter.*, FK_Command,FK_Device 
					FROM CommandGroup_Command
					LEFT JOIN CommandGroup_Command_CommandParameter ON FK_CommandGroup_Command=PK_CommandGroup_Command 
					WHERE PK_CommandGroup_Command=?';

				$resCommands=$dbADO->Execute($queryCommands,$cgcID);
				$commandParmsArray=array();
				while($rowCommands=$resCommands->FetchRow()){
					$commandParmsArray[]=$rowCommands['FK_CommandParameter'].' "'.$rowCommands['IK_CommandParameter'].'"';
					$FK_Command=$rowCommands['FK_Command'];
					$FK_Device=$rowCommands['FK_Device'];
				}
				$commandToSend='/usr/pluto/bin/MessageSend localhost 0 '.$FK_Device.' 1 '.$FK_Command.' '.join(' ',$commandParmsArray);
				exec($commandToSend);
				
				header('Location: index.php?section=editCommandGroup&cgID='.$commandGroupID.'&msg='.$TEXT_SCENARIO_COMMAND_SENT_CONST.': '.$commandToSend.'#hook_'.$cgcID);
				exit();
			}
			
			
			$x=cleanInteger(@$_POST['addNewDevice']);
			$y=cleanInteger(@$_POST['addNewDeviceCommand']);
			
			if ($y!=0 && $x!=0) {
				$queryInsertCommandGroup_Command = "INSERT INTO CommandGroup_Command
				(FK_CommandGroup,FK_Command,FK_Device) values(?,?,?)";			
				$insertRs = $dbADO->Execute($queryInsertCommandGroup_Command,array($commandGroupID,$y,$x));			
				$hook='hook_'.$dbADO->Insert_ID();
			}
			
			$isModified = 0;
			if ($dbADO->Affected_Rows()>0) {
				$isModified=1;				
			}
			$selectCommandsAssigned = "
				SELECT CommandGroup_Command.* FROM 
					CommandGroup_Command				
					LEFT JOIN 
						Device ON CommandGroup_Command.FK_Device = Device.PK_Device						
					WHERE 
						(Device.FK_Installation = ? OR Device.FK_Installation IS NULL)
				AND CommandGroup_Command.FK_CommandGroup = ?
				";								

			$resCommandAssigned = $dbADO->Execute($selectCommandsAssigned,array($installationID,$commandGroupID));
			
			$parametersUpdatedAlert = $TEXT_PARAMETERS_NOT_UPDATED_CONST;
			
			if ($resCommandAssigned) {
				while ($rowCommandAssigned = $resCommandAssigned->FetchRow()) {
					
					$deviceSelected = isset($_POST['device_'.$rowCommandAssigned['PK_CommandGroup_Command']])?$_POST['device_'.$rowCommandAssigned['PK_CommandGroup_Command']]:$x;
					$commandSelected = isset($_POST['deviceCommand_'.$rowCommandAssigned['PK_CommandGroup_Command']])?$_POST['deviceCommand_'.$rowCommandAssigned['PK_CommandGroup_Command']]:$y;

					$updateCommandGroup_Command = 'UPDATE CommandGroup_Command SET FK_Device = ? WHERE PK_CommandGroup_Command = ? ';
					$resUpdateCommandGroup_Command  = $dbADO->Execute($updateCommandGroup_Command,array($deviceSelected,$rowCommandAssigned['PK_CommandGroup_Command']));
					
					if ($dbADO->Affected_Rows()==1) {//enter here only if the Device is changed
						$updateCommandGroup_Command = 'UPDATE CommandGroup_Command SET FK_Command = NULL WHERE PK_CommandGroup_Command = ? ';
						$resUpdateCommandGroup_Command  = $dbADO->Execute($updateCommandGroup_Command,array($rowCommandAssigned['PK_CommandGroup_Command']));
						//delete old parameters
						$deleteParamValues = 'delete from CommandGroup_Command_CommandParameter where 
									FK_CommandGroup_Command = ? 
						';
						$query = $dbADO->Execute($deleteParamValues,array($rowCommandAssigned['PK_CommandGroup_Command']));
				
					} elseif($commandSelected!=0) {
						$updateCommandGroup_Command = 'UPDATE CommandGroup_Command SET FK_Command = ? WHERE PK_CommandGroup_Command = ? ';
						$resUpdateCommandGroup_Command  = $dbADO->Execute($updateCommandGroup_Command,array($commandSelected,$rowCommandAssigned['PK_CommandGroup_Command']));
						if ($dbADO->Affected_Rows()==1) {//if we have changed the command, delete old values
							$deleteParamValues = 'delete from CommandGroup_Command_CommandParameter where 
									FK_CommandGroup_Command = ? 
							';
							$query = $dbADO->Execute($deleteParamValues,array($rowCommandAssigned['PK_CommandGroup_Command']));
						}
						//echo "here2";
						
					}
					//die();
					//command parameters

					$query = "SELECT Command_CommandParameter.FK_CommandParameter,Command_CommandParameter.Description as C_CP_Description,CommandParameter.Description as CP_Description,
										CommandGroup_Command_CommandParameter.IK_CommandParameter,
										ParameterType.Description as PT_Description
											FROM Command_CommandParameter 
												INNER JOIN Command on FK_Command = PK_Command
												INNER JOIN CommandParameter ON Command_CommandParameter.FK_CommandParameter = CommandParameter.PK_CommandParameter
												INNER JOIN ParameterType ON CommandParameter.FK_ParameterType = ParameterType.PK_ParameterType
												LEFT JOIN CommandGroup_Command_CommandParameter ON CommandGroup_Command_CommandParameter.FK_CommandParameter = Command_CommandParameter.FK_CommandParameter
									  WHERE FK_Command = ?
								";
							
					$resSelectParameters = $dbADO->Execute($query,array($commandSelected));
					
					if ($resSelectParameters) {
								while ($rowSelectParameters=$resSelectParameters->fetchRow()) {
									$value=cleanString(@$_POST['CommandParameterValue_'.$rowCommandAssigned['PK_CommandGroup_Command'].'_'.$rowSelectParameters['FK_CommandParameter']]);
									//see if we need to update or to insert
									$checkExists = "SELECT * FROM CommandGroup_Command_CommandParameter WHERE FK_CommandGroup_Command = ? AND  FK_CommandParameter = ?";
									$resExists=$dbADO->Execute($checkExists,array($rowCommandAssigned['PK_CommandGroup_Command'],$rowSelectParameters['FK_CommandParameter']));
									if ($resExists->RecordCount()==1) {
										$update = "UPDATE CommandGroup_Command_CommandParameter  SET  IK_CommandParameter  = ? WHERE FK_CommandGroup_Command = ? AND  FK_CommandParameter =? ";
										$dbADO->Execute($update,array($value,$rowCommandAssigned['PK_CommandGroup_Command'],$rowSelectParameters['FK_CommandParameter']));
									} else {										
										$insert = "INSERT INTO CommandGroup_Command_CommandParameter (IK_CommandParameter,FK_CommandGroup_Command,FK_CommandParameter) values(?,?,?)";
										$dbADO->Execute($insert,array($value,$rowCommandAssigned['PK_CommandGroup_Command'],$rowSelectParameters['FK_CommandParameter']));										
									}
									if ($dbADO->Affected_Rows()==1) {
										$parametersUpdatedAlert=$TEXT_PARAMETERS_UPDATED_CONST;
									}
									
								}
					}
							
					
				 }
			  }
			//die();
			
			$description = cleanString($_POST['description']);
			$hint = cleanString($_POST['hint']);
			$arrayID = cleanInteger($_POST['arrayType']);
			$designObjID = ($_SESSION['editCommandGroup']['designObjID']>0)?$_SESSION['editCommandGroup']['designObjID']:null;
			$templateID = (isset($_POST['template']) && $_POST['template']!=0)?cleanInteger($_POST['template']):NULL;
			
			// upload scenario picture
			$filePath=$GLOBALS['scenariosPicsPath'].$commandGroupID.'.png';
			if($_FILES['pic']['name']!=''){
				switch($_FILES['pic']['type']){
					case 'image/x-png':
					case 'image/png':
					case 'image/jpg':
					case 'image/pjpeg':
					case 'image/jpeg':						
						$invalidType=false;
					break;
					default:
						$invalidType=true;
					break;
				}
	
				if($invalidType===false){
					if(!move_uploaded_file($_FILES['pic']['tmp_name'],$filePath)){
						$errNotice.=$TEXT_UPLOAD_FAILED_CONST.' '.$filePath;
					}else{
						// convert to png and/or resize if necessary
						exec('chmod 777 -R '.$filePath);
						$imageinfo = @getimagesize( $filePath );
						if($imageinfo[0]!=160 && $imageinfo[1]!=160){
							$flag=resizeImage($filePath,$filePath,160,160,1);
						}					
					}
				}else 
					$errNotice.=$TEXT_INVALID_PNG_FILE_CONST.': '.$_FILES['pic']['name'].'<br>';
			}
			
			$updateCommandGroup = "
				UPDATE CommandGroup 
				SET Description = ?,FK_Array=?,FK_DesignObj =?,FK_Template=?, Hint=?
				WHERE PK_CommandGroup = ?";
			$res=$dbADO->Execute($updateCommandGroup,array($description,$arrayID,$designObjID,$templateID,$hint,$commandGroupID));
			$isModified=1;
			
			if ((@$_POST['addNewDevice']!=0) && cleanInteger(@$_POST['addNewDeviceCommand'])==0) {
				$addNewDevice =  cleanInteger($_POST['addNewDevice']);			
				header("Location: index.php?section=editCommandGroup&from=$from&cgID=$commandGroupID&newDevice=$addNewDevice#hook_0");
				exit();							
			}

			if($arrayID==$GLOBALS['ArrayIDForMedia']){
				
				$displayedEntArea =cleanString($_POST['displayedEntArea']);
				$EntAreaOldValues =cleanString($_POST['EntAreaOldValues']);
				
				if($displayedEntArea!=''){
					$EntAreaArray=explode(',',$displayedEntArea);
					$EntAreaOldValuesArray=explode(',',$EntAreaOldValues);
					foreach ($EntAreaArray AS $key => $value){
						if(@$_POST['entArea_'.$value]!=$EntAreaOldValuesArray[$key]){
							if(@$_POST['entArea_'.$value]==1){
								$insertCommandGroupEntArea='
									INSERT INTO CommandGroup_EntertainArea 
										(FK_CommandGroup, FK_EntertainArea,Sort)
									VALUES
										(?,?,?)';
								$dbADO->Execute($insertCommandGroupEntArea,array($commandGroupID,$value,$commandGroupID));
							}else{
								$deleteCommandGroupEntArea='
									DELETE FROM CommandGroup_EntertainArea 
									WHERE FK_CommandGroup=? AND FK_EntertainArea=?';
								$dbADO->Execute($deleteCommandGroupEntArea,array($commandGroupID,$value));
							}
						}
					}
				}
			}else{
				$displayedRooms =cleanString($_POST['displayedRooms']);
				$roomsOldValues =cleanString($_POST['roomsOldValues']);
				
				if($displayedRooms!=''){
					$roomsArray=explode(',',$displayedRooms);
					$roomsOldValuesArray=explode(',',$roomsOldValues);
					foreach ($roomsArray AS $key => $value){
						if(@$_POST['room_'.$value]!=$roomsOldValuesArray[$key]){
							if(@$_POST['room_'.$value]==1){
								$insertCommandGroupRoom='
									INSERT INTO CommandGroup_Room 
										(FK_CommandGroup, FK_Room,Sort)
									VALUES
										(?,?,?)';
								$dbADO->Execute($insertCommandGroupRoom,array($commandGroupID,$value,$commandGroupID));
							}else{
								$deleteCommandGroupRoom='
									DELETE FROM CommandGroup_Room 
									WHERE FK_CommandGroup=? AND FK_Room=?';
								$dbADO->Execute($deleteCommandGroupRoom,array($commandGroupID,$value));
							}
						}
					}
				}
			}
			setOrbitersNeedConfigure($installationID,$dbADO);			
			
			$errNotice=urlencode($errNotice);
			$out.="
			<script>
				alert('Command Group ".($isModified?"":"not")." updated! $parametersUpdatedAlert ');
			    //opener.document.forms.{$from}.action.value='form';
				//opener.document.forms.{$from}.submit();
				//self.close();
				document.location.href='index.php?section=editCommandGroup&from=$from&cgID=$commandGroupID&saved=1&error=$errNotice#$hook';
			</script>
			";
			$_SESSION['editCommandGroup']=array();
		}
		
		 
		
	}

	if(@$_REQUEST['saved']==1)
		$output->setScriptInBody("onLoad=\"javascript:top.treeframe.location.reload();\"");
		
	$output->setMenuTitle($TEXT_ADVANCED_CONST.' |');
	$output->setPageTitle($TEXT_EDIT_SCENARIO_CONST.' #'.$commandGroupID);
		
	$output->setBody($out);
	$output->setTitle(APPLICATION_NAME.' :: '.$TEXT_EDIT_MY_SCENARIO_CONST);			
	$output->output();
}
?>
