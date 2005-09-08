<?
function deleteDeviceCategory($categoryID,$dbADO)
{
	getDeviceCategoryChildsArray($categoryID,$dbADO);

	$GLOBALS['childsDeviceCategoryArray']=cleanArray($GLOBALS['childsDeviceCategoryArray']);
	$GLOBALS['childsDeviceCategoryArray'][]=$categoryID;

	if ((int)$categoryID!=0) {
		$deleteObjFromDevice = 'DELETE FROM DeviceCategory WHERE PK_DeviceCategory in ('.join(",",$GLOBALS['childsDeviceCategoryArray']).')';
		$query = $dbADO->_Execute($deleteObjFromDevice);
		
		$deleteDT = 'DELETE FROM DeviceTemplate WHERE FK_DeviceCategory in ('.join(",",$GLOBALS['childsDeviceCategoryArray']).')';
		$query = $dbADO->_Execute($deleteDT);

		return 1;
	} else {
		return 0;
	}
}


function pulldownFromArray($valuesArray,$name,$selectedValue,$extra='',$valueKey='key',$zeroValueDescription='- Please select -',$highlightValue=-1)
{
//	if(count($valuesArray)==0)
//		return null;
	$out='<select name="'.$name.'" "'.$extra.'">';
	if($zeroValueDescription!='')
		$out.='<option value="0">'.$zeroValueDescription.'</option>';
	foreach ($valuesArray AS $key=>$value){
		$optionValue=($valueKey=='key')?$key:$value;
		$out.='<option value="'.$optionValue.'" '.(($optionValue==$selectedValue)?'selected':'').' '.((in_array($optionValue,explode(',',$highlightValue)))?'style="background:lightgreen;"':'').'>'.$value.'</option>';
	}
	$out.='</select>';
	return $out;
}

function generatePullDown($name,$tableName,$valueField,$labelField,$selectedValue,$dbADO,$filterTable='',$jsEvents='')
{
	$pullDown='
		<select name="'.$name.'" '.$jsEvents.'>	
			<option value="0">- Please select -</option>';
	$query="SELECT $tableName.$valueField,$tableName.$labelField FROM $tableName $filterTable ORDER BY $labelField ASC";
	$res=$dbADO->Execute($query);
	while($row=$res->FetchRow()){
		$pullDown.='<option value="'.$row[$valueField].'" '.(($row[$valueField]==$selectedValue)?'selected':'').'>'.$row[$labelField].'</option>';
	}
	$pullDown.='</select>';
	return $pullDown;
}

function getCheckedDeviceCommandGroup($dtID,$deviceCategory,$dbADO)
{
	$querySelCheckedCommandGroups = '
		SELECT 
		PK_DeviceCommandGroup,
		DeviceCommandGroup.Description,
		FK_DeviceTemplate,
		FK_Command,
		Command.Description AS Command
		FROM DeviceCommandGroup
		LEFT JOIN DeviceTemplate_DeviceCommandGroup ON DeviceTemplate_DeviceCommandGroup.FK_DeviceCommandGroup=PK_DeviceCommandGroup AND FK_DeviceTemplate=?
		LEFT JOIN DeviceCategory ON (FK_DeviceCategory=PK_DeviceCategory OR FK_DeviceCategory=FK_DeviceCategory_Parent)
		INNER JOIN DeviceCommandGroup_Command ON DeviceCommandGroup_Command.FK_DeviceCommandGroup=PK_DeviceCommandGroup
		INNER JOIN Command ON FK_Command=PK_Command
		WHERE PK_DeviceCategory=? OR FK_DeviceCategory IS NULL
		ORDER BY Description,Command ASC';
	$resSelCheckedCommandGroups = $dbADO->Execute($querySelCheckedCommandGroups,array($dtID,$deviceCategory));
	$dcgArray=array();
	while ($row= $resSelCheckedCommandGroups->FetchRow()) {
		$dcgArray[$row['PK_DeviceCommandGroup']]['Description']=$row['Description'];
		$dcgArray[$row['PK_DeviceCommandGroup']]['Checked']=(is_null($row['FK_DeviceTemplate']))?0:1;
		$dcgArray[$row['PK_DeviceCommandGroup']]['Commands'][]=$row['Command'];
		$dcgArray[$row['PK_DeviceCommandGroup']]['CommandIDs'][]=$row['FK_Command'];
	}
	
	return $dcgArray;
}

function DeviceCommandGroupTable($dtID,$deviceCategory,$dbADO){
	$dcgArray=getCheckedDeviceCommandGroup($dtID,$deviceCategory,$dbADO);
	$out='<table>';
	$oldCheckedDCG=array();
	foreach ($dcgArray AS $dcg=>$value){
		if($value['Checked']==1){
			$oldCheckedDCG[]=$dcg;
		}
		$out.='
		<tr>
			<td><input type="checkbox" name="dcg_'.$dcg.'" value="1" '.(($value['Checked']==1)?'checked':'').'></td>
			<td title="'.join(', ',$value['Commands']).'">'.$value['Description'].'</td>
		</tr>
		<input type="hidden" name="commands_'.$dcg.'" value="'.join(',',$value['CommandIDs']).'">
		';
	}
	$out.='</table>
		<input type="hidden" name="deviceCG" value="'.join(',',array_keys($dcgArray)).'">
		<input type="hidden" name="oldCheckedDCG" value="'.join(',',$oldCheckedDCG).'">
	';
	
	return $out;
}


function pickDeviceTemplate($categoryID, $boolManufacturer,$boolCategory,$boolDeviceTemplate,$returnValue,$defaultAll,$section,$firstColText,$dbADO,$useframes=0,$genericSerialDevicesOnly=0)
{
	$redirectUrl='index.php?section='.$section;

	global $dbPlutoMainDatabase;
	$out='
	<script>
	function windowOpen(locationA,attributes) {
		window.open(locationA,\'\',attributes);
	}
	</script>';
	
	$selectedManufacturer = (int)@$_REQUEST['manufacturers'];
	$selectedDeviceCateg= (int)@$_REQUEST['deviceCategSelected'];
	$selectedDevice= (int)@$_REQUEST['deviceSelected'];
	$selectedModel= (int)@$_REQUEST['model'];
	$justAddedNode = (int)@$_GET['justAddedNode'];

	$actionX = (isset($_REQUEST['actionX']) && cleanString($_REQUEST['actionX'])!='null')?cleanString($_REQUEST['actionX']):'form';
	
	$manufacturersArray=getAssocArray('Manufacturer','PK_Manufacturer','Description',$dbADO,'','ORDER BY Description ASC');
	$categsArray=getAssocArray('DeviceCategory','PK_DeviceCategory','Description',$dbADO,'WHERE PK_DeviceCategory='.$selectedDevice,'ORDER BY Description ASC');
	$selectedDeviceCategName=@$categsArray[$selectedDevice];
	$formName=str_replace('operations/','',$section);	
	
	if ($actionX=='form') {
	
	
		$jsTree = '';
		if(is_null($categoryID))
			$selectDeviceCategories = "SELECT * FROM DeviceCategory WHERE FK_DeviceCategory_Parent IS NULL ORDER BY Description";
		else 
			$selectDeviceCategories = "SELECT * FROM DeviceCategory WHERE FK_DeviceCategory_Parent ='$categoryID' ORDER BY Description";
		$rs = $dbADO->_Execute($selectDeviceCategories);
			while ($row = $rs->FetchRow()) {		
				$jsTree.='
				auxS'.$row['PK_DeviceCategory'].' = insFld(foldersTree, gFld("'.$row['Description'].' #'.$row['PK_DeviceCategory'].'", "javascript:setPicker('.$row['PK_DeviceCategory'].',\"'.htmlspecialchars($row['Description'],ENT_QUOTES).'\");"));
				auxS'.$row['PK_DeviceCategory'].'.xID = '.$row['PK_DeviceCategory'].';
				';
				$jsTree.=getChilds($row['PK_DeviceCategory'],$dbADO);
			}
		$rs->Close();
		
		
		$selectModels = '';
		$modelsJsArray = '
				modelsArray = new Array();
		';

		$where = " AND 1=1 ";
		$manufOrDevice = 0;
		if ($selectedManufacturer!=0) {	
			$where.=" AND FK_Manufacturer = '$selectedManufacturer'";
			$manufOrDevice = 1;
		}

		if ($selectedDevice!=0) {
			$where.=" AND FK_DeviceCategory = ".$selectedDevice;
		}
		
		$where.=(($genericSerialDevicesOnly==1)?' AND CommandLine=\''.$GLOBALS['GenericSerialDeviceCommandLine'].'\'':'');
		
		if ($manufOrDevice || $defaultAll==1)	{
		
			$queryModels = "SELECT DeviceTemplate.*
							FROM DeviceTemplate 
							WHERE 1=1 $where ORDER BY Description";	
			$rs = $dbADO->_Execute($queryModels);
			
			$arJsPos=0;
				$avDT=getAssocArray('DeviceTemplate','PK_DeviceTemplate','FK_CommMethod',$dbADO,'LEFT JOIN InfraredGroup ON FK_InfraredGroup=PK_InfraredGroup');
				while ($row = $rs->FetchRow()) {
					$irHighlight=(in_array($row['PK_DeviceTemplate'],array_keys($avDT)) && $avDT[$row['PK_DeviceTemplate']]==1)?'style="background-color:#FFDFDF;"':'';
					
					$selectModels.="<option ".($selectedModel==$row['PK_DeviceTemplate']?" selected ":"")." value='{$row['PK_DeviceTemplate']}' ".(($row['IsPlugAndPlay']==1)?'style="background-color:lightgreen;"':'')." $irHighlight>{$row['Description']}</option>";
					
					if ($row['PK_DeviceTemplate']>0) {
					$modelsJsArray.='
					
						model'.$row['PK_DeviceTemplate'].' = new Array();
					    model'.$row['PK_DeviceTemplate'].'.userID = \''.@$_SESSION['userID'].'\';
						modelsArray['.$row['PK_DeviceTemplate'].'] = model'.$row['PK_DeviceTemplate'].';			
						';
					}
					$arJsPos++;
				}
			$rs->Close();
		
		
		}
			
		
		$scriptInHead = "
		<style>
		a{
			text-decoration: none;
			color: #0166FF;
			font-size: 10px;
		}
		
		a:hover {
			text-decoration: none;
			color: #0F46AD;
			font-size: 10px;
		}
		</style>
		
		<!-- As in a client-side built tree, all the tree infrastructure is put in place
		     within the HEAD block, but the actual tree rendering is trigered within the
		     BODY -->
		
		<!-- Code for browser detection -->
		<script src=\"javascripts/treeview/treeview/ua.js\"></script>
		
		<!-- Infrastructure code for the tree -->
		<script src=\"javascripts/treeview/ftiens4.js\"></script>
		
		<!-- Execution of the code that actually builds the specific tree.
		     The variable foldersTree creates its structure with calls to
			 gFld, insFld, and insDoc -->
		<script>
		cssClass='normaltext'
		USETEXTLINKS = 1
		STARTALLOPEN = 0
		USEFRAMES = 0
		USEICONS = 0
		WRAPTEXT = 1
		PRESERVESTATE = 1
		ICONPATH = 'javascripts/treeview/' 
		HIGHLIGHT = 1
		
		";
		if(is_null($categoryID)){
			$scriptInHead.="
				foldersTree = gFld('<b>Device Category</b>', \"javascript:setPicker(0,'');\");";
		}else{
			$queryCategoryName='SELECT Description FROM DeviceCategory WHERE PK_DeviceCategory=?';
			$resCategoryName=$dbADO->Execute($queryCategoryName,$categoryID);
			$rowCategoryName=$resCategoryName->FetchRow();
			$scriptInHead.="
				foldersTree = gFld('<b>".$rowCategoryName['Description']."</b>', 'javascript:setPicker($categoryID,\"".htmlspecialchars($rowCategoryName['Description'],ENT_QUOTES)."\");');";
		}
		$scriptInHead.="
		foldersTree.xID = 1001635872;
		$jsTree
		
		foldersTree.treeID = 't2'
		
		function getObj(obj) {
				if (document.layers) {
					if (typeof obj == 'string') {
						return document.layers[obj];
					} else 	{
						return obj;
					}
				}
				if (document.all) {
					if (typeof obj == 'string') {
						return document.all(obj);
					} else {
						return obj;
					}
				}

				return null;	
			} 

			function showHideObject(obj) {
				obj = getObj(obj);					
				if(document.all || getObj) {  			
						if (obj.style.visibility == \"visible\") {
							obj.style.visibility = \"hidden\";
							obj.style.display = \"none\";
						} else {
						    obj.style.visibility = \"visible\";
							obj.style.display = \"block\";
						}
				} else if (document.layers) {
						if (obj.style.visibility == \"visible\") {
							obj.visibility = \"hide\";	
						} else {
						obj.visibility = \"show\";	
						}
				}
			} 
		
		function setPicker(val,descr){
			if(document.$formName.manufacturers.selectedIndex!=0 && document.$formName.manufacturers.selectedIndex!=-1){
				document.getElementById('manufInfo').innerHTML=document.$formName.manufacturers.options[document.$formName.manufacturers.selectedIndex].text;
			}
			if(val!='manufacturers'){
				document.getElementById('categInfo').innerHTML=descr;
				document.$formName.deviceSelected.value=val;
			}

			if(document.$formName.deviceSelected.value!=0 && document.$formName.manufacturers.selectedIndex!=0 && document.$formName.manufacturers.selectedIndex!=-1){
				document.$formName.actionX.value='form';
				document.$formName.submit();
			}

		}

		function addModel(){
			if(document.$formName.deviceSelected.value!=0 && document.$formName.manufacturers.selectedIndex!=0 && document.$formName.manufacturers.selectedIndex!=-1){
				self.location='index.php?section=addModel&mID='+document.$formName.manufacturers[document.$formName.manufacturers.selectedIndex].value+'&dcID='+document.$formName.deviceSelected.value+'&step=1';
			}
		}
		
		
		$modelsJsArray
		
		</script>
		";
		$out.=$scriptInHead;
		$out.='
		<div class="err">'.(isset($_GET['error'])&&$_GET['error']!='password'?strip_tags($_GET['error']):'').'</div>	
		<div class="confirm"><B>'.stripslashes(@$_GET['msg']).'</B></div>	
		
		<form action="index.php" method="POST" name="'.$formName.'">
		<input type="hidden" name="section" value="'.$section.'">
		<input type="hidden" name="actionX" value="add">
		<input type="hidden" name="deviceCategSelected" value="'.$selectedDeviceCateg.'">
		<input type="hidden" name="deviceSelected" value="'.$selectedDevice.'">
		<input type="hidden" name="modelSelected" value="'.$selectedModel.'">
		<input type="hidden" name="allowAdd" value="'.(int)@$_REQUEST['allowAdd'].'">
		
		<table cellpadding="5" cellspacing="0" border="0" align="center">
				<input type="hidden" name="selectedField" value="" />
					<tr>
						<th width="25%">Manufacturers</th>
						<th width="25%">Device Category</th>
						<th width="25%">
						Models
						
						</th>				
					</tr>
					<tr valign="top">
						<td width="25%" align="center"  valign="top">
							<span id="manufInfo" class="confirm">&nbsp;'.@$manufacturersArray[$selectedManufacturer].'</span><br>
							'.pulldownFromArray($manufacturersArray,'manufacturers',$selectedManufacturer,'multiple onchange="setPicker(\'manufacturers\',\'\');" size="10" style="z-index:1;"').'<br /><br />';
					if($boolManufacturer==1 && isset($_SESSION['userID'])){
						$out.='
							<input type="text" name="Manufacturer_Description" size="15" />
							<input type="submit" class="button" name="addManufacturer" value="Add"  />';
					}
					$out.=$firstColText.'
						</td>
						<td width="25%" align="left" valign="top">
							<span id="categInfo" class="confirm">&nbsp;'.@$selectedDeviceCategName.'</span><br>
							<span style="display:none;">
								<table border=0>
									<tr>
										<td><font size=-2><a style="font-size:7pt;text-decoration:none;color:#FFFFFF" href="http://www.treemenu.net/" target=_blank>JavaScript Tree Menu</a></font></td>
									</tr>
								</table>
							</span>
							<span>
							<script>
								initializeDocument();						
							</script>
							<noscript>
							A tree for site navigation will open here if you enable JavaScript in your browser.
							</noscript>
							</span>
							<br /><br />';
						if($boolCategory==1){
							$out.='
							<input type="text" name="DeviceCategory_Description" size="15" />
							<input type="submit" class="button" name="addDeviceCategory"   value="Add '.($selectedDevice==0?' Top Level Child':' Child').'" />
							'.($selectedDevice!=0?'<input type="button" class="button" name="editDeviceCategory" value="Edit" onClick="javascript: windowOpen(\'index.php?section=editDeviceCategory&from='.$section.'&manufacturers='.$selectedManufacturer.'&deviceCategSelected='.$selectedDeviceCateg.'&deviceSelected='.$selectedDevice.'&model='.$selectedModel.'\',\'status=0,resizable=1,width=600,height=250,toolbars=true\');" />':'<input   type="submit" class="button" name="editDeviceCategory" value="Edit" disabled="disabled" />').'';
							getDeviceCategoryChildsNo($selectedDevice,$dbADO);					
							$childsToDelete = $GLOBALS['childsDeviceCategoryNo'];
							$out.='
							&nbsp;&nbsp;'.($selectedDevice!=0?'<input type="button" class="button" name="deleteDeviceCategory" value="Delete" onClick="javascript: if (confirm(\'Are you sure you want to delete this device category?'.($childsToDelete==1?'There is 1 child to delete!':($childsToDelete>0?'There are '.$childsToDelete.' childs to delete!':'')).'\')) self.location=\'index.php?section='.$section.'&manufacturers='.$selectedManufacturer.'&deviceCategSelected='.$selectedDeviceCateg.'&deviceSelected='.$selectedDevice.'&model='.$selectedModel.'&actionX=del\';" />':'<input type="submit" class="button" name="deleteDeviceCategory"   value="Delete" disabled="disabled" />');
						}
						$out.='
						</td>
						<td width="25%" align="center"  valign="top" class="normaltext">
							<script>
								function checkEdit(targetSection) {
									if(document.'.$formName.'.model.selectedIndex!=0){							
										self.location=\'index.php?section=irCodes&dtID=\'+document.'.$formName.'.model[document.'.$formName.'.model.selectedIndex].value;
									}
								}
		
								function updateModelDetail(byItem) {
									objF=document.'.$formName.';
									eval("tmpFlag=objF."+byItem+".selectedIndex");
									eval("targetObj=objF."+byItem);
									eval("targetArray="+byItem+"sArray");
									if (tmpFlag!= "undefined") {
										document.getElementById(\'modelManuf\').innerHTML = \'<br />Manufacturer: \'+targetArray[targetObj[tmpFlag].value].manufacturerName;
										document.getElementById(\'modelDeviceDescription\').innerHTML = \'<br />Category: \'+targetArray[targetObj[tmpFlag].value].deviceDescription;
									} else {
										document.getElementById(\'modelManuf\').innerHTML = \'\';
										document.getElementById(\'modelDeviceDescription\').innerHTML = \'\';
									} 					
								}

							</script>
							<select name="model" id="model" size="10">
									<option value="" selected="selected">'.(($section=='deviceTemplatePicker')?'- Please select -':'All').'</option>
									'.$selectModels.'	
							</select>';
							if($returnValue==0){
								$out.='<br><input type="button" class="button" name="edit_DeviceTemplate" value="Show Model" onClick="javascript:checkEdit(\'model\');" />';
							}else{
								$out.='<br><input type="button" class="button" name="pickDT" value="Add device" onClick="opener.location=\'index.php?section='.$_SESSION['from'].'&deviceTemplate=\'+document.'.$section.'.model[document.'.$section.'.model.selectedIndex].value+\'&action=add&add=1\';self.close();" />';
							}
							$out.='
							<hr />
							<em>* Models in red use IR codes, in white use Pluto\'s GSD.  Models in green are plug and play.</em><br>
						<b><span id="modelManuf"></span><span id="modelDeviceDescription"></span></b><br />
							
							';
							
							if ($selectedManufacturer!=0 && $selectedDevice!=0) {
								$disabledAdd=' ';
							} else {
								$disabledAdd=' disabled="disabled" ';
							}
					
							$disabled_str = "disabled";
				
				
				$out.='
						</td>				
					</tr>';
				if($returnValue!=0){
					$out.='
					<tr>
						<td colspan="3">To add a device, choose the manufacturer and the category to see all models for you selection. Pick the model you want and click <B>"Add device"</B>.</td>
					</tr>';
				}
				if($boolDeviceTemplate==1 && $disabledAdd!=''){
					$out.='
					<tr>
						<td colspan="3" class="normaltext">If your model is not listed, pick the manufacturer and the category and click to <input type="button" class="button" name="addDeviceTemplate" value="Add"'. $disabledAdd .'  onClick="javascript:addModel();"/></td>
					</tr>';
				}
				if($categoryID==$GLOBALS['rootAVEquipment']){
					$out.='
					<tr>
						<td colspan="3" class="normaltext">After you add a new model you\'ll be able to choose the A/V properties and set I/R codes.</td>
					</tr>';
				}
				$out.='
				</table>
							
		</form>
		';
	}elseif($actionX=='del'){
		deleteDeviceCategory($selectedDevice,$dbADO);
		$out.="
			<script>
				self.location='$redirectUrl&manufacturers=$selectedManufacturer&deviceCategSelected=$selectedDeviceCateg&deviceSelected=$selectedDevice&model=$selectedModel&allowAdd=$boolDeviceTemplate&msg=Device category deleted.';
			</script>";
	} else {
		
		 $addDeviceCategory = @cleanString($_POST['addDeviceCategory']);
		 $deviceCategoryDesc = cleanString(@$_POST['DeviceCategory_Description']);
		 
		 $addDeviceTemplate = @cleanString($_POST['addDeviceTemplate']);
		 $DeviceTemplate_Desc = @cleanString($_POST['DeviceTemplate_Description']);
	
		 $addManufacturer = @cleanString($_POST['addManufacturer']);
		 $Manufacturer_Description = @cleanString($_POST['Manufacturer_Description']);	 
		
		 $deviceCategSelected = (int)$_POST['deviceCategSelected'];
		 $deviceSelected = (int)$_POST['deviceSelected'];
		 $manufacturerSelected = (int)$_POST['manufacturers'];
	
		 // process form only if user is logged
		 if (isset($_SESSION['userLoggedIn']) && $_SESSION['userLoggedIn']===true) {
	
			 // add new category
			 $justAddedNode = 0;
			 if (strstr($addDeviceCategory,'Add')) {
			 	if ($deviceCategoryDesc!='') {
			 		$queryInsertCategoryDesc = "insert into DeviceCategory(FK_DeviceCategory_Parent,Description,IsAV)
			 				values(?,?,0)";
			 		$deviceCategParent=($deviceSelected==0)?$categoryID:$deviceSelected;
			 		$dbADO->Execute($queryInsertCategoryDesc,array($deviceCategParent,$deviceCategoryDesc));	 			
			 		$justAddedNode = $deviceSelected;
			 		header("Location: $redirectUrl&manufacturers=$manufacturerSelected&deviceCategSelected=$selectedDeviceCateg&deviceSelected=$selectedDevice&model=$selectedModel&allowAdd=$boolDeviceTemplate&justAddedNode=$justAddedNode");	 			 		
				}
			 }	
			 	 
			 //add new master device list
			 if (strstr($addDeviceTemplate,'Add')) {
			 	
			 	if ($DeviceTemplate_Desc!='') {
			 		if ($deviceSelected!=0 && $manufacturerSelected!=0) {	 			
			 			$queryInsertMasterDevice = 'INSERT INTO DeviceTemplate (Description,FK_DeviceCategory,FK_Manufacturer) values(?,?,?)';
			 			$res = $dbADO->Execute($queryInsertMasterDevice,array($DeviceTemplate_Desc,$deviceSelected,$manufacturerSelected));	 			
			 			$dtID=$dbADO->Insert_ID();

			 			if(in_array($deviceSelected,$GLOBALS['TVdevicesArray'])){
							$openTunerConfig='windowOpen(\'index.php?section=tunerConfig&categoryID='.$deviceSelected.'&dtID='.$dtID.'\',\'width=600,height=400,toolbars=true,scrollbars=1,resizable=1\')';
						}else
							$openTunerConfig='';
			 			
		 				$out.='
							<script>
								'.$openTunerConfig.'
								self.location="index.php?section='.$section.'&manufacturers='.$manufacturerSelected.'&deviceCategSelected='.$selectedDeviceCateg.'&deviceSelected='.$selectedDevice.'&model='.$selectedModel.'&allowAdd='.$boolDeviceTemplate.'&justAddedNode='.$justAddedNode.'";
							</script>';
		 			
			 		}	 			 		
			 	}
			 }
			 
			 // add new manufacturer
			 if (strstr($addManufacturer,'Add')){
			 	if ($Manufacturer_Description!='') {
		 			$queryInsertManufacturer = 'INSERT INTO Manufacturer (Description) values(?)';
		 			$res = $dbADO->Execute($queryInsertManufacturer,array($Manufacturer_Description));	 			
		 			header("Location: $redirectUrl&manufacturers=$manufacturerSelected&deviceCategSelected=$selectedDeviceCateg&deviceSelected=$selectedDevice&model=$selectedModel&allowAdd=$boolDeviceTemplate&justAddedNode=$justAddedNode");
		 		}	 			 		
			 }
			$out.="
				<script>
					self.location='$redirectUrl&manufacturers=$manufacturerSelected&deviceCategSelected=$selectedDeviceCateg&deviceSelected=$selectedDevice&model=$selectedModel&allowAdd=$boolDeviceTemplate&justAddedNode=$justAddedNode';
				</script>";
		}else{
			$out.="
				<script>
					self.location='$redirectUrl&manufacturers=$manufacturerSelected&deviceCategSelected=$selectedDeviceCateg&deviceSelected=$selectedDevice&model=$selectedModel&allowAdd=$boolDeviceTemplate&error=Please login if you want to change device templates.';
				</script>";
		}
	}
	return $out;
}

function getChilds($parentID,$dbADO) {
	
	$selectDevicesFromCategories = "SELECT * FROM DeviceCategory WHERE FK_DeviceCategory_Parent = {$parentID} ORDER BY Description";
		$rs2 = $dbADO->_Execute($selectDevicesFromCategories);
		$jsTree='';
			while ($row2 = $rs2->FetchRow()) {		
				$jsTree.= "
					auxS".$row2['PK_DeviceCategory']." = insFld(auxS".$parentID.", gFld(\"".$row2["Description"]." #".$row2["PK_DeviceCategory"]."\", 'javascript:setPicker(".$row2["PK_DeviceCategory"].",\"".htmlspecialchars($row2["Description"],ENT_QUOTES)."\");'))
					auxS".$row2['PK_DeviceCategory'].".xID = ".$row2['PK_DeviceCategory'].";
				";
				$jsTree.=getChilds($row2['PK_DeviceCategory'],$dbADO);
			}

	return $jsTree;
}

function getMediaTypeCheckboxes($dtID,$publicADO)
{
	$mediaTypeCheckboxes='';
	$resMT=$publicADO->Execute('
		SELECT MediaType.Description, PK_MediaType, FK_DeviceTemplate 
		FROM MediaType 
		LEFT JOIN DeviceTemplate_MediaType ON FK_MediaType=PK_MediaType AND FK_DeviceTemplate=?
		WHERE DCEaware=0',$dtID);
	$displayedMT=array();
	$checkedMT=array();
	while($rowMT=$resMT->FetchRow()){
		$checked='';
		$displayedMT[]=$rowMT['PK_MediaType'];
		if($rowMT['FK_DeviceTemplate']!=''){
			$checked='checked';
			$checkedMT[]=$rowMT['PK_MediaType'];
		}
			
		$mediaTypeCheckboxes.='<input type="checkbox" name="mediaType_'.$rowMT['PK_MediaType'].'" '.@$checked.'>'.str_replace('np_','',$rowMT['Description']).' ';
		$mediaTypeCheckboxes.=((count($displayedMT))%5==0)?'<br>':'';
	}
	$mediaTypeCheckboxes.='
	<input type="hidden" name="displayedMT" value="'.join(',',$displayedMT).'">
	<input type="hidden" name="checkedMT" value="'.join(',',$checkedMT).'">';
	
	return $mediaTypeCheckboxes;
}

function reorderMultiPulldownJs()
{
	$jsFunctions='
	<script>
	function moveUp(srcList)
	{
		try{
			eval("selectedPos="+srcList+".selectedIndex");
			eval("selectedElement="+srcList+"["+srcList+".selectedIndex].value");
			eval("srcLen="+srcList+".length");
		}catch(e){
			alert("Please select the command to move.");
		}
	
	
		len=0;
		for( var i = 0; i < srcLen; i++ ) { 
			if(selectedPos==len){
				if(len>0){
					tmpText=eval(srcList+"["+(len-1)+"].text");
					tmpValue=eval(srcList+"["+(len-1)+"].value");
					
					eval(srcList+"["+(len-1)+"].text="+srcList+"["+len+"].text");
					eval(srcList+"["+(len-1)+"].value="+srcList+"["+len+"].value");

					eval(srcList+"["+len+"].text=\'"+tmpText+"\'");
					eval(srcList+"["+len+"].value=\""+tmpValue+"\"");
					
					eval(srcList+"["+len+"].selected=false");
					eval(srcList+"["+(len-1)+"].selected=true");
				}
			}
			len++;
		}	
	}

	function moveDown(srcList)
	{
		try{
			eval("selectedPos="+srcList+".selectedIndex");
			eval("selectedElement="+srcList+"["+srcList+".selectedIndex].value");
			eval("srcLen="+srcList+".length");
		}catch(e){
			alert("Please select the command to move.");
		}
	
	
		len=0;
		for( var i = 0; i < srcLen; i++ ) { 
			if(selectedPos==len){
				if(len!=(srcLen-1)){
					tmpText=eval(srcList+"["+(len+1)+"].text");
					tmpValue=eval(srcList+"["+(len+1)+"].value");
					
					eval(srcList+"["+(len+1)+"].text="+srcList+"["+len+"].text");
					eval(srcList+"["+(len+1)+"].value="+srcList+"["+len+"].value");

					eval(srcList+"["+len+"].text=\'"+tmpText+"\'");
					eval(srcList+"["+len+"].value=\""+tmpValue+"\"");
					
					eval(srcList+"["+len+"].selected=false");
					eval(srcList+"["+(len+1)+"].selected=true");
				}
			}
			len++;
		}	
	}
	
	function addToPulldown(srcList,newVal,newText){
		try{
			eval("srcLen="+srcList+".length");
		}catch(e){
			//alert("Invalid pulldown.");
		}
	
	

		alreadyInPulldown=0;
		for( var i = 0; i < srcLen; i++ ) { 
			tmpValue=eval(srcList+"["+i+"].value");
			if(tmpValue==newVal){
				alreadyInPulldown=1;
			}
		}	
		if(alreadyInPulldown==0){
			eval(srcList+".options["+srcLen+"] = new Option( \""+newText+"\", \""+newVal+"\")");
		}
	}
	
	function removeFromPulldown(srcList,delVal){
		try{
			eval("srcLen="+srcList+".length");
		}catch(e){
			//alert("Invalid pulldown.");
		}
	
	

		for( var i = 0; i < srcLen; i++ ) { 
			tmpValue=eval(srcList+"["+i+"].value");
			if(tmpValue==delVal){
				eval(srcList+".options["+i+"] = null");
			}
		}	
	}	
	</script>
	';
	
	return $jsFunctions;
}

// retrieve a matrix table with all commands and infrared groups
function getIrGroup_CommandsMatrix($dtID,$InfraredGroupsArray,$userID,$comMethod,$publicADO,$deviceID)
{
	$restrictedCommandsArray=array(194=>'Toggle power',192=>'On',193=>'Off',205=>'1',89=>'Vol Up',90=>'Vol down',63=>'Skip Fwd',64=>'Skip Back');
	$out='';
	if(count($InfraredGroupsArray)==0){
		return '';
	}
	$comMethodFilter=(!is_null($comMethod))?' AND FK_CommMethod='.$comMethod:'';	
	$codesData=getFieldsAsArray('InfraredGroup_Command','PK_InfraredGroup_Command,IRData,FK_InfraredGroup,FK_Command,InfraredGroup.Description AS IRG_Name',$publicADO,'INNER JOIN Command ON FK_Command=PK_Command INNER JOIN InfraredGroup ON FK_InfraredGroup=PK_InfraredGroup WHERE (FK_DeviceTemplate IS NULL OR FK_DeviceTemplate='.$dtID.') AND FK_InfraredGroup IN ('.join(',',$InfraredGroupsArray).') AND FK_Command IN ('.join(',',array_keys($restrictedCommandsArray)).')'.$comMethodFilter,'ORDER BY FK_InfraredGroup ASC,FK_Command ASC');

	$commandGrouped=array();
	$irgNames=array();
	for($i=0;$i<count(@$codesData['FK_InfraredGroup']);$i++){
		$commandGrouped[$codesData['FK_InfraredGroup'][$i]][$codesData['FK_Command'][$i]]=$codesData['PK_InfraredGroup_Command'][$i];
		$irgNames[$codesData['FK_InfraredGroup'][$i]]=$codesData['IRG_Name'][$i];
	}
	
	if(session_name()=='Pluto-admin'){
		if($deviceID>0){
			$deviceInfo=getFieldsAsArray('Device','FK_Device_ControlledVia,FK_DeviceCategory',$publicADO,'INNER JOIN DeviceTemplate ON Fk_DeviceTemplate=PK_DeviceTemplate WHERE PK_Device='.$deviceID);
		}
		$GLOBALS['DT_&_Room']=1;
		$controlledByRows='
		<tr>
			<td colspan="10" align="right">Send codes to device: '.controlledViaPullDown('controlledVia',$deviceID,$dtID,@$deviceInfo['FK_DeviceCategory'][0],@$deviceInfo['FK_Device_ControlledVia'][0],$publicADO,'0,- Please select -','onChange="document.addModel.action.value=\'changeParent\';document.addModel.submit();"').'</td>
		</tr>';
		$GLOBALS['DT_&_Room']=0;
	}
	
	// display table header
	$out='
	<table cellpadding="3" cellspacing="0" border="0">
		'.@$controlledByRows.'
		<tr class="normaltext" bgcolor="lightblue">
			<td align="center"><B>Infrared Group</B></td>';
	foreach ($restrictedCommandsArray AS $cmdID=>$cmdName){
		$out.='
			<td align="center"><B>'.$cmdName.'</B></td>';
	}
	$out.='
			<td><B>Action</B></td>
		</tr>';
	
	// display codes/commands
	$keysArray=array_keys($commandGrouped);
	for($i=0;$i<count($commandGrouped);$i++){
		$color=($i%2==0)?'#F0F3F8':'#FFFFFF';
		$out.='
		<tr class="normaltext" bgcolor="'.$color.'">
			<td><B><a href="index.php?section=irCodes&dtID='.$dtID.'&infraredGroupID='.$keysArray[$i].'&deviceID='.@$_REQUEST['deviceID'].'">'.$irgNames[$keysArray[$i]].'</a></B></td>';
		foreach ($restrictedCommandsArray AS $cmdID=>$cmdName){
			$pk_irgc=@$commandGrouped[$keysArray[$i]][$cmdID];
			$testCodeBtn=(session_name()=='Pluto-admin')?' <input type="button" class="button" name="testCode" value="T" onClick="frames[\'codeTester\'].location=\'index.php?section=testCode&irgcID='.$pk_irgc.'&deviceID='.@$_REQUEST['deviceID'].'\';">':'';
			$out.='<td align="center">'.((isset($commandGrouped[$keysArray[$i]][$cmdID]))?'<input type="button" class="button" name="copyCB" value="V" onClick="window.open(\'index.php?section=displayCode&irgcID='.$pk_irgc.'\',\'_blank\',\'\');">'.$testCodeBtn:'N/A').'</td>';
		}
		$out.='
			<td><input type="button" class="button" name="btn" onClick="self.location=\'index.php?section=irCodes&dtID='.$dtID.'&infraredGroupID='.$keysArray[$i].'&deviceID='.@$_REQUEST['deviceID'].'\';" value="This works"></td>
		</tr>';
	}
	
	$out.='
	</table>';
	
	
	return $out;
}

// create an embedded device template for selected device template 
function createEmbeddedDeviceTemplate($name,$manufacturer,$deviceCategory,$userID,$parentID,$commandID,$mediaType,$publicADO){
	$publicADO->Execute('
		INSERT INTO DeviceTemplate 
			(Description,FK_Manufacturer,FK_DeviceCategory,psc_user) 
		VALUES 
			(?,?,?,?)',
	array($name,$manufacturer,$deviceCategory,$userID));
	$embeddedID=$publicADO->Insert_ID();

	$publicADO->Execute('
		INSERT INTO DeviceTemplate_DeviceTemplate_ControlledVia 
			(FK_DeviceTemplate,FK_DeviceTemplate_ControlledVia,RerouteMessagesToParent,AutoCreateChildren)
		VALUES
			(?,?,1,1)',
	array($embeddedID,$parentID));
	$insertID=$publicADO->Insert_ID();

	$publicADO->Execute('
		INSERT INTO DeviceTemplate_DeviceTemplate_ControlledVia_Pipe
			(FK_DeviceTemplate_DeviceTemplate_ControlledVia,FK_Pipe,FK_Command_Input)
		VALUES 
			(?,?,?)',
	array($insertID,1,$commandID));
	
	$publicADO->Execute('
		INSERT INTO DeviceTemplate_DeviceTemplate_ControlledVia_Pipe
			(FK_DeviceTemplate_DeviceTemplate_ControlledVia,FK_Pipe,FK_Command_Input)
		VALUES 
			(?,?,?)',
	array($insertID,2,$commandID));
	
	$publicADO->Execute('
		INSERT INTO DeviceTemplate_Input 
			(FK_DeviceTemplate,FK_Command) 
		VALUES 
			(?,?)',
	array($embeddedID,$commandID));
	
	if(!is_null($mediaType)){
		$publicADO->Execute('
			INSERT INTO DeviceTemplate_MediaType 
				(FK_DeviceTemplate,FK_MediaType) 
			VALUES 
				(?,?)',
		array($embeddedID,$mediaType));
	}
}

// extract codes and put them in a multi dimensional array
function extractCodesTree($infraredGroupID,$dtID,$dbADO,$restriction=''){
	global $inputCommandsArray, $dspmodeCommandsArray,$powerCommands;
	$installationID=(int)@$_SESSION['installationID'];

	$userID=(int)@$_SESSION['userID'];
	$codesQuery='
			SELECT 
				PK_InfraredGroup_Command,
				FK_DeviceTemplate,
				FK_Device,
				InfraredGroup_Command.psc_user,
				IRData,
				Command.Description,
				IF(PK_Command=192 OR PK_Command=193 OR PK_Command=194,1, IF(FK_CommandCategory=22,2, IF(FK_CommandCategory=27,3, IF(FK_CommandCategory=21,4,5) ) ) ) AS GroupOrder,
				IF( InfraredGroup_Command.psc_user=?,2, IF( FK_DeviceTemplate IS NULL AND FK_Device IS NULL,1,3) ) As DefaultOrder,
				CommandCategory.Description AS CommandCategory,
				FK_Command 
			FROM InfraredGroup_Command 
			JOIN Command ON FK_Command=PK_Command JOIN CommandCategory ON FK_CommandCategory=PK_CommandCategory 
			WHERE '.(($infraredGroupID==0)?'FK_InfraredGroup IS NULL':'FK_InfraredGroup='.$infraredGroupID).' AND (FK_DeviceTemplate=? OR FK_DeviceTemplate IS NULL) '.$restriction.'
			ORDER BY GroupOrder,CommandCategory.Description,Description,DefaultOrder';

	$res=$dbADO->Execute($codesQuery,array($userID,$dtID));
	$codesArray=array();
	while($row=$res->FetchRow()){
		$categoryLabel=($row['CommandCategory']=='Generic')?'Power':$row['CommandCategory'];
		// move input select commands to inputs category
		$categoryLabel=($row['FK_Command']==$GLOBALS['inputSelectCommand'])?'Inputs':$categoryLabel;
		// move DSP mode commands to DSP mode
		$categoryLabel=($row['FK_Command']==$GLOBALS['DSPModeCommand'])?'DSP Modes':$categoryLabel;
		
		$codesArray[$categoryLabel]['FK_DeviceTemplate'][$row['PK_InfraredGroup_Command']]=$row['FK_DeviceTemplate'];
		$codesArray[$categoryLabel]['FK_Device'][$row['PK_InfraredGroup_Command']]=$row['FK_Device'];
		$codesArray[$categoryLabel]['psc_user'][$row['PK_InfraredGroup_Command']]=$row['psc_user'];
		$codesArray[$categoryLabel]['Description'][$row['PK_InfraredGroup_Command']]=$row['Description'];
		$codesArray[$categoryLabel]['CommandCategory'][$row['PK_InfraredGroup_Command']]=$categoryLabel;
		$codesArray[$categoryLabel]['FK_Command'][$row['PK_InfraredGroup_Command']]=$row['FK_Command'];
		$codesArray[$categoryLabel]['IRData'][$row['PK_InfraredGroup_Command']]=$row['IRData'];
		$codesArray[$categoryLabel]['DefaultOrder'][$row['PK_InfraredGroup_Command']]=$row['DefaultOrder'];
		$GLOBALS['displayedIRGC'][]=$row['PK_InfraredGroup_Command'];
		$GLOBALS['displayedCommands'][]=$row['FK_Command'];

		// remove input commands from input array if they are implemented
		if(in_array($row['FK_Command'],array_keys($inputCommandsArray))){
			unset ($inputCommandsArray[$row['FK_Command']]);
		}

		// remove DSP mode commands from input array if they are implemented
		if(in_array($row['FK_Command'],array_keys($dspmodeCommandsArray))){
			unset ($dspmodeCommandsArray[$row['FK_Command']]);
		}
		
		// remove power commands from power commands array if they are implemented
		if(in_array($row['FK_Command'],array_keys($powerCommands))){
			unset ($powerCommands[$row['FK_Command']]);
		}
	}
	$GLOBALS['igcPrefered']=getPreferredIRGC($installationID,$infraredGroupID,$dbADO);
	
	return $codesArray;
}

function getPreferredIRGC($installationID,$infraredGroupID,$dbADO){
	$preferred=getFieldsAsArray('InfraredGroup_Command_Preferred','FK_Command,FK_InfraredGroup_Command',$dbADO,' INNER JOIN InfraredGroup_Command ON FK_InfraredGroup_Command=PK_InfraredGroup_Command WHERE InfraredGroup_Command_Preferred.FK_Installation='.$installationID.(((int)$infraredGroupID!=0)?' AND FK_InfraredGroup='.$infraredGroupID:''));	
	$preferredByCategory=array();
	for($i=0;$i<count(@$preferred['FK_Command']);$i++){
		// only the preferred which I display are used
		if(in_array($preferred['FK_InfraredGroup_Command'][$i],$GLOBALS['displayedIRGC']))
			$preferredByCategory[$preferred['FK_Command'][$i]][]=$preferred['FK_InfraredGroup_Command'][$i];
	}
	
	return $preferredByCategory;
}

// parse the multidimensiona array with codes and return html code for table rows
function getCodesTableRows($section,$infraredGroupID,$dtID,$deviceID,$codesArray,$togglePower,$toggleInput,$toggleDSP){
	global $inputCommandsArray, $dspmodeCommandsArray,$powerCommands;
	//print_array($codesArray);
	$categNames=array_keys($codesArray);
	if(!in_array('Power',$categNames)){
		$categNames[]='Power';
	}	
	if(!in_array('Inputs',$categNames)){
		$categNames[]='Inputs';
	}
	if(!in_array('DSP Modes',$categNames)){
		$categNames[]='DSP Modes';
	}

	$newCodeSection=($section=='rubyCodes')?'newRubyCode':'newIRCode';

	$out='';
	for($i=0;$i<count($categNames);$i++){
		$out.='
			<tr>
				<td colspan="4" align="center">
					<fieldset style="width:98%">
						<legend><B>'.$categNames[$i].'</B></legend>';
		if($categNames[$i]=='Power'){
			if($togglePower==1){
				if(!@in_array(194,$codesArray['Power']['FK_Command'])){
					$out.='Toggle power not implemented: ';
					$out.='<b><a href="javascript:windowOpen(\'index.php?section='.$newCodeSection.'&deviceID='.$deviceID.'&dtID='.$dtID.'&infraredGroupID='.$infraredGroupID.'&commandID=194&action=sendCommand\',\'width=750,height=310,toolbars=true,scrollbars=1,resizable=1\');">Click to add</a>';
				}
			}elseif(count($powerCommands)>0){
				$out.='Power commands not implemented: ';
				foreach ($powerCommands AS $inputId=>$inputName){
					$powerCommands[$inputId]='<a href="javascript:windowOpen(\'index.php?section='.$newCodeSection.'&deviceID='.$deviceID.'&dtID='.$dtID.'&infraredGroupID='.$infraredGroupID.'&commandID='.$inputId.'&action=sendCommand\',\'width=750,height=310,toolbars=true,scrollbars=1,resizable=1\');">'.$inputName.'</a>';
				}
				$out.='<b>'.join(', ',$powerCommands).'</b> <em>(Click to add)</em>';
			}
		}
				
		// display input commands not implemented
		if($categNames[$i]=='Inputs'){
			if($toggleInput==1){
				if(@!in_array($GLOBALS['inputSelectCommand'],$codesArray['Inputs']['FK_Command'])){
					$out.='Input select not implemented: ';
					$out.='<b><a href="javascript:windowOpen(\'index.php?section='.$newCodeSection.'&deviceID='.$deviceID.'&dtID='.$dtID.'&infraredGroupID='.$infraredGroupID.'&commandID='.$GLOBALS['inputSelectCommand'].'&action=sendCommand\',\'width=750,height=310,toolbars=true,scrollbars=1,resizable=1\');">Click to add</a>';
				}
			}elseif(count($inputCommandsArray)>0){
				$out.='Input commands not implemented: ';
				foreach ($inputCommandsArray AS $inputId=>$inputName){
					$inputCommandsArray[$inputId]='<a href="javascript:windowOpen(\'index.php?section='.$newCodeSection.'&deviceID='.$deviceID.'&dtID='.$dtID.'&infraredGroupID='.$infraredGroupID.'&commandID='.$inputId.'&action=sendCommand\',\'width=750,height=310,toolbars=true,scrollbars=1,resizable=1\');">'.$inputName.'</a>';
				}
				$out.='<b>'.join(', ',$inputCommandsArray).'</b> <em>(Click to add)</em>';
			}
		}

		// display DSP modes commands not implemented
		if($categNames[$i]=='DSP Modes'){
			if($toggleDSP==1){
				if(@!in_array($GLOBALS['DSPModeCommand'],@$codesArray['DSP Modes']['FK_Command'])){
					$out.='DSP Mode not implemented: ';
					$out.='<b><a href="javascript:windowOpen(\'index.php?section='.$newCodeSection.'&deviceID='.$deviceID.'&dtID='.$dtID.'&infraredGroupID='.$infraredGroupID.'&commandID='.$GLOBALS['DSPModeCommand'].'&action=sendCommand\',\'width=750,height=310,toolbars=true,scrollbars=1,resizable=1\');">Click to add</a>';
				}
			}elseif(count($dspmodeCommandsArray)>0){
				$out.='DSP Mode commands not implemented: ';
				foreach ($dspmodeCommandsArray AS $dspmId=>$dspmName){
					$dspmodeCommandsArray[$dspmId]='<a href="javascript:windowOpen(\'index.php?section='.$newCodeSection.'&deviceID='.$deviceID.'&dtID='.$dtID.'&infraredGroupID='.$infraredGroupID.'&commandID='.$dspmId.'&action=sendCommand\',\'width=750,height=310,toolbars=true,scrollbars=1,resizable=1\');">'.$dspmName.'</a>';
				}
				$out.='<b>'.join(', ',$dspmodeCommandsArray).'</b> <em>(Click to add)</em>';
			}
		}

		if(isset($codesArray[$categNames[$i]]['FK_DeviceTemplate'])){
			for($pos=0;$pos<count($codesArray[$categNames[$i]]['FK_DeviceTemplate']);$pos++){
				$codeCommandsKeys=array_keys($codesArray[$categNames[$i]]['FK_DeviceTemplate']);
				$irg_c=$codeCommandsKeys[$pos];
				$out.=formatCode($section,$codesArray[$categNames[$i]],$irg_c,$infraredGroupID,$dtID,$deviceID);
			}
		}

		$out.='
					</fieldset>
				</td>
			</tr>	
			';
	}
	
	return $out;
}

?>
