<?
function editComputingApplications($output,$dbADO,$mediadbADO) {
	/* @var $dbADO ADOConnection */
	/* @var $rs ADORecordSet */
//	$dbADO->debug=true;
	$out='';
	$action = isset($_REQUEST['action'])?cleanString($_REQUEST['action']):'form';
	$installationID = (int)@$_SESSION['installationID'];
	$mdID=(int)$_REQUEST['mdID'];
	$fixedPath='/home/mediapics';
	
	if ($action == 'form') {
		// build pics arrays
		$picsArray=getFieldsAsArray('Picture','PK_Picture,Extension',$mediadbADO);
		$fullPicArray=array();
		for($i=0;$i<count($picsArray['PK_Picture']);$i++)
			$fullPicArray[$picsArray['PK_Picture'][$i].'.'.$picsArray['Extension'][$i]]=$picsArray['PK_Picture'][$i];
		$picsByKey=array_flip($fullPicArray);
		
		
	$tableRows='';
	$appArray=getFieldsAsArray('Device_QuickStart','FK_QuickStartTemplate,PK_Device_QuickStart,Description,SortOrder,`Binary`,Arguments,EK_Picture',$dbADO,'WHERE FK_Device='.$mdID,'ORDER BY SortOrder ASC');
	$usedTemplates=array();
	for($i=0;$i<count(@$appArray['PK_Device_QuickStart']);$i++){
		if(!in_array($appArray['PK_Device_QuickStart'][$i],$usedTemplates)){
			$usedTemplates[]=$appArray['FK_QuickStartTemplate'][$i];
		}
		
		$color=($i%2!=0)?'#F0F3F8':'#FFFFFF';
		$pic=(!is_null($appArray['EK_Picture'][$i]))?'<img src="include/image.php?imagepath='.$fixedPath.'/'.$picsByKey[$appArray['EK_Picture'][$i]].'" align="middle">':'&nbsp;';
		$tableRows.='
			<tr bgcolor="'.$color.'">
				<td>'.$pic.'</td>
				<td>Description: <B>'.$appArray['Description'][$i].'</B><br>Path to binary: <B>'.$appArray['Binary'][$i].'</B><br>Arguments: <B>'.$appArray['Arguments'][$i].'</B></td>
				<td align="center">
					<input type="submit" class="button" value="U" name="u_'.$appArray['PK_Device_QuickStart'][$i].'" onClick="document.editComputingApplications.currentPos.value='.$appArray['SortOrder'][$i].';"><br>
					<input type="submit" class="button" value="D" name="d_'.$appArray['PK_Device_QuickStart'][$i].'" onClick="document.editComputingApplications.currentPos.value='.$appArray['SortOrder'][$i].';"><br>
					<input type="submit" class="button" name="del_'.$appArray['PK_Device_QuickStart'][$i].'" value="Delete" onClick="if(!confirm(\'Are you sure you want to delete this application?\'))return false;"></td>
			</tr>
		';
	}
	$tableRows.='<input type="hidden" name="apps" value="'.@join(',',@array_values($appArray['PK_Device_QuickStart'])).'">';
	
		
		
	$mdArray=getDevicesArrayFromCategory($GLOBALS['rootMediaDirectors'],$dbADO);
	unset($mdArray[$mdID]);
	$mdCheckboxes='';
	foreach ($mdArray AS $md=>$descr){
		$mdCheckboxes='<input type="checkbox" value="1" name="md_'.$md.'"> '.$descr.'<br>';
	}
	$mdCheckboxes.='<input type="hidden" name="mds" value="'.join(',',array_keys($mdArray)).'">';

	$tree=getQuickStartTree($usedTemplates,$dbADO);
	
		$out.='
	<script>
		function showPreview()
		{
			if(document.editComputingApplications.picture.selectedValue!=0){
				document.editComputingApplications.preview.src=\'include/image.php?imagepath='.$fixedPath.'/\'+document.editComputingApplications.picture.value;
				document.getElementById(\'previewBox\').style.display=\'\';
			}else{
				document.getElementById(\'previewBox\').style.display=\'none\';
			}
		}

		function windowOpen(locationA,attributes) {
			window.open(locationA,\'\',attributes);
		}

	</script>
	<div class="err" align="center">'.(isset($_GET['error'])?strip_tags($_GET['error']):'').'</div>
	<div align="center" class="confirm"><B>'.@$_REQUEST['msg'].'</B></div>
		
	<form action="index.php" method="POST" name="editComputingApplications" enctype="multipart/form-data">
		<input type="hidden" name="section" value="editComputingApplications">
		<input type="hidden" name="action" value="add">	
		<input type="hidden" name="mdID" value="'.$mdID.'">	
		<input type="hidden" name="currentPos" value="">
		
	<div align="center"><h3>Edit Computing Applications</h3>
	The following applications will appear on the �Computing� menu:</div><br>
	<table cellpadding="3" cellspacing="0" align="center">
		<tr>
			<td valign="top">'.$tree.'</td>
			<td valign="top"><table cellpadding="3" cellspacing="0" align="center">
			<tr bgcolor="lightblue">
				<td align="center" colspan="2"><B>Application</B></td>
				<td align="center"><B>Action</B></td>
			</tr>
			'.$tableRows.'
			<tr>
				<td>&nbsp;</td>
			</tr>
			<tr bgcolor="lightblue">
				<td colspan="3"><B>Add new application</B></td>
			</tr>
			<tr>
				<td><B>Select icon</B></td>
				<td><input type="file" name="picture" value=""></td>
			</tr>
			<tr>
				<td><B>Description</B></td>
				<td><input type="text" name="description"></td>
			</tr>
			<tr>
				<td><B>Path to binary</B></td>
				<td><input type="text" name="path"></td>
			</tr>
			<tr>
				<td><B>Arguments</B></td>
				<td><input type="text" name="arguments"></td>
			</tr>
			<tr>
				<td><B>Also add to media directors</B></td>
				<td>'.$mdCheckboxes.'</td>
			</tr>
			<tr>
				<td colspan="2" align="center"><input type="submit" class="button" name="add" value="Add"> <input type="button" class="button" name="close" value="Close" onclick="self.close();"></td>
			</tr>
		</table></td>
		</tr>	
	</table>
	</form>
	';
	} else {
		// check if the user has the right to modify installation
		$canModifyInstallation = getUserCanModifyInstallation($_SESSION['userID'],$_SESSION['installationID'],$dbADO);
		if (!$canModifyInstallation){
			header("Location: index.php?section=editComputingApplications&error=You are not authorised to change the installation.");
			exit(0);
		}
		
		if($action=='addFromTemplate'){
			$qst=$_REQUEST['qst'];
			$dbADO->Execute("INSERT INTO Device_QuickStart (FK_Device,Description,`Binary`,Arguments,FK_QuickStartTemplate) SELECT $mdID,Description,`Binary`,Arguments,? FROM QuickStartTemplate WHERE PK_QuickStartTemplate=?",array($qst,$qst));
		}
		
		$description=stripslashes($_POST['description']);
		$path=stripslashes($_POST['path']);
		$arguments=stripslashes($_POST['arguments']);
		if($description!=''){
			$picture=NULL;
			if($_FILES['picture']['name']!=''){
				$extension=substr($_FILES['picture']['name'],strrpos($_FILES['picture']['name'],'.')+1);
				$mediadbADO->Execute('INSERT INTO Picture (Extension) VALUES (?)',$extension);
				$picture=$mediadbADO->Insert_ID();
				if(!@move_uploaded_file($_FILES['picture']['tmp_name'],$fixedPath.'/'.$picture.'.'.$extension)){
					$mediadbADO->Execute('DELETE FROM Picture WHERE PK_Picture=?',$picture);
					$picture=NULL;
					$err='Picture not uploaded, check the rights for '.$fixedPath;
				}
				
			}
			$addQuery='
				INSERT IGNORE INTO Device_QuickStart
					(FK_Device,Description,`Binary`,Arguments,EK_Picture)
				VALUES
					(?,?,?,?,?)';
			$dbADO->Execute($addQuery,array($mdID,$description,$path,$arguments,$picture));
			$dbADO->Execute('UPDATE Device_QuickStart SET SortOrder=? WHERE PK_Device_QuickStart=?',array($dbADO->Insert_ID(),$dbADO->Insert_ID()));
			
			$mds=explode(',',$_POST['mds']);
			foreach ($mds AS $md){
				if(isset($_POST['md_'.$md])){
					$dbADO->Execute($addQuery,array($md,$description,$path,$arguments,$picture));
					$dbADO->Execute('UPDATE Device_QuickStart SET SortOrder=? WHERE PK_Device_QuickStart=?',array($dbADO->Insert_ID(),$dbADO->Insert_ID()));
				}
			}
		}

		$apps=explode(',',$_POST['apps']);
		$currentPos=(int)$_POST['currentPos'];
		foreach ($apps AS $app){
			if((int)$app!=0 && isset($_POST['del_'.$app])){
				$dbADO->Execute('DELETE FROM Device_QuickStart WHERE PK_Device_QuickStart=?',$app);
			}

			// move up
			if((int)$app!=0 && isset($_POST['u_'.$app])){
				$res=$dbADO->Execute('SELECT * FROM Device_QuickStart WHERE FK_Device=? AND SortOrder<? ORDER BY SortOrder DESC LIMIT 0,1',array($mdID,$currentPos));
				if($res->RecordCount()>0){
					$row=$res->FetchRow();
					$dbADO->Execute('UPDATE Device_QuickStart SET SortOrder=? WHERE PK_Device_QuickStart=?',array($row['SortOrder'],$app));
					$dbADO->Execute('UPDATE Device_QuickStart SET SortOrder=? WHERE PK_Device_QuickStart=?',array($currentPos,$row['PK_Device_QuickStart']));
				}
			}

			// move down
			if((int)$app!=0 && isset($_POST['d_'.$app])){
				$res=$dbADO->Execute('SELECT * FROM Device_QuickStart WHERE FK_Device=? AND SortOrder>? ORDER BY SortOrder ASC LIMIT 0,1',array($mdID,$currentPos));
				if($res->RecordCount()>0){
					$row=$res->FetchRow();
					$dbADO->Execute('UPDATE Device_QuickStart SET SortOrder=? WHERE PK_Device_QuickStart=?',array($row['SortOrder'],$app));
					$dbADO->Execute('UPDATE Device_QuickStart SET SortOrder=? WHERE PK_Device_QuickStart=?',array($currentPos,$row['PK_Device_QuickStart']));
				}
			}
			
		}
		
		$sufix=(@$err!='')?'&error='.$err:'&msg=Computing Applications were updated';
		
		header("Location: index.php?section=editComputingApplications&mdID=$mdID".$sufix);		
	}

	$output->setScriptCalendar('null');
	$output->setBody($out);
	$output->setTitle(APPLICATION_NAME.' :: Edit Computing Applications');
	$output->output();
}


function getQuickStartTemplates($dbADO){
	$res=$dbADO->Execute('SELECT * FROM QuickStartTemplate');
	$qsTemplates=array();
	while($row=$res->FetchRow()){
		$qsTemplates[$row['FK_QuickStartCategory']][]=$row['PK_QuickStartTemplate'];
		
		$qsTemplates['description'][$row['PK_QuickStartTemplate']]=$row['Description'];
	}
	
	return $qsTemplates;
}

function getQuickStartTree($usedTemplates,$dbADO){
	$nodes=getNodesArray('QuickStartCategory','PK_QuickStartCategory','FK_QuickStartCategory_Parent',$dbADO);
	$quickStartTemplates=getQuickStartTemplates($dbADO);
	
	$out='
		<input type="hidden" name="qst" value="">
	<table cellpadding="3" cellspacing="0">
		<tr bgcolor="lightblue">
			<td colspan="2"><B>Quick start templates</B></td>
		</tr>';
	foreach ($nodes['root_node'] AS $rootNode){
		$out.='
		<tr>
			<td bgcolor="#EEEEEE" colspan="2"><B>'.$nodes['description'][$rootNode].'</B></td>
		</tr>';		
		if(count(@$quickStartTemplates[$rootNode])==0){
			$out.='
				<tr>
					<td style="padding-left:20px;" colspan="2">No quick start templates in this category.</td>
				</tr>
			';
		}
		foreach ($quickStartTemplates[$rootNode] AS $qsTemplate){
			if(!in_array($qsTemplate,$usedTemplates)){
			$out.='
				<tr>
					<td style="padding-left:20px;">'.$quickStartTemplates['description'][$qsTemplate].'</td>
					<td align="right"><a href="javascript:document.editComputingApplications.action.value=\'addFromTemplate\'; document.editComputingApplications.qst.value=\''.$qsTemplate.'\';document.editComputingApplications.submit();">[ Add ]</a></td>
				</tr>
			';
			}
		}
		$out.=getQuickStartChilds($nodes,$quickStartTemplates,$rootNode,$usedTemplates);

	}
	$out.='
		<tr>
			<td colspan="2"><a href="javascript:windowOpen(\'index.php?section=editQuickStartTemplates\',\'\')">Add other software</a> to permanently appear in this quick launch for all Pluto users</td>
		</tr>	
	</table>';
	
	return $out;
}

function getQuickStartChilds($nodes,$quickStartTemplates,$selectedCategory,$usedTemplates){
	$out='';
	foreach ($nodes[$selectedCategory] AS $childNode){
		$out.='
		<tr>
			<td bgcolor="#EEEEEE" style="padding-left:10px;" colspan="2"><B>'.$nodes['description'][$selectedCategory].' - '.$nodes['description'][$childNode].'</B></td>
		</tr>';		
		if(count(@$quickStartTemplates[$childNode])==0){
			$out.='
				<tr>
					<td style="padding-left:20px;" colspan="2">No quick start templates in this category.</td>
				</tr>
			';
		}
		foreach ($quickStartTemplates[$childNode] AS $qsTemplate){
			if(!in_array($qsTemplate,$usedTemplates)){
			$out.='
				<tr>
					<td style="padding-left:20px;">'.$quickStartTemplates['description'][$qsTemplate].'</td>
					<td align="right"><a href="javascript:document.editComputingApplications.action.value=\'addFromTemplate\'; document.editComputingApplications.qst.value=\''.$qsTemplate.'\';document.editComputingApplications.submit();">[ Add ]</a></td>
				</tr>
			';
			}
		}
		
		$out.=getQuickStartChilds($nodes,$quickStartTemplates,$childNode,$usedTemplates);		
	}
	
	return $out;
}
?>
