<?
function coverArt($output,$mediadbADO) {
	global $mediaTypes;
	// include language files
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/common.lang.php');
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/coverArt.lang.php');
	
	/* @var $mediadbADO ADOConnection */
	/* @var $rs ADORecordSet */
	$mediaTypes=array(4=>'Music',3=>'DVD');
	
	$out='';
	$action = (isset($_REQUEST['action']) && $_REQUEST['action']!='')?cleanString($_REQUEST['action']):'form';
	$type=@$_REQUEST['type'];
	$mediaType=(isset($_REQUEST['mediaType']))?(int)$_REQUEST['mediaType']:4;
	$searchIndex=$mediaTypes[$mediaType];
	$ipp=((int)@$_REQUEST['ipp']<=0 || (int)@$_REQUEST['ipp']>100)?10:(int)@$_REQUEST['ipp'];
	
	if(automaticGrabStatus()){
		$automatic_label=$TEXT_STOP_CONST;
		$warning=$TEXT_WARNING_AUTOMATIC_GRAB_ACTIVE_CONST;
	}else{
		$automatic_label=$TEXT_START_CONST;
	}
	
	if($action=='form'){
		
		$out.='
		<script>
		function showType(val){
			self.location="index.php?section=coverArt&type="+val+"&ipp="+parseInt(document.coverArt.items_per_page.value)+"&mediaType="+document.coverArt.mediaType.value;
		}
		
		function reloadPage(){
			self.location="index.php?section=coverArt&type='.$type.'&ipp="+parseInt(document.coverArt.items_per_page.value)+"&mediaType="+document.coverArt.mediaType.value+"&filterPath="+document.coverArt.filterPath.value+"&filterFile="+document.coverArt.filterFile.value;
		}
		
		</script>
		
			<div align="center" class="err">'.@$_REQUEST['error'].'</div>
			<div align="center" class="confirm"><B>'.@$_REQUEST['msg'].'</B></div>

		<form action="index.php" method="POST" name="coverArt">
			<input type="hidden" name="section" value="coverArt">
			<input type="hidden" name="action" value="update">

		<table width="700">
			<tr>
				<td width="250">
					<a href="javascript:showType(\'allFiles\');">['.$TEXT_SHOW_ALL_FILES_CONST.']</a><br>
					<a href="javascript:showType(\'filesNoCover\');">['.$TEXT_SHOW_FILES_NO_COVERART_CONST.']</a><br>
					<a href="javascript:showType(\'allDiscs\');">['.$TEXT_SHOW_ALL_DISCS_CONST.']</a><br>
					<a href="javascript:showType(\'discsNoCover\');">['.$TEXT_SHOW_DISCS_WITH_NO_COVER_ART_CONST.']</a><br>
					<a href="javascript:showType(\'allAttributes\');">['.$TEXT_SHOW_ALL_ATTRIBUTES_CONST.']</a><br>
					<a href="javascript:showType(\'attributesNoCover\');">['.$TEXT_SHOW_ALL_ATTRIBUTES_WITH_NO_COVER_ART_CONST.']</a><br>
					<a href="index.php?section=matchCoverArt">['.$TEXT_MATCH_FILES_TO_COVER_ART_CONST.']</a><br>
				</td>
				<td align="right" valign="top" width="250">
					<table>
						<tr>
							<td>'.$TEXT_ITEMS_PER_PAGE_CONST.'</td>
							<td><input type="text" name="items_per_page" value="'.@$ipp.'" size="2"> <input type="button" class="button" name="go" value="'.$TEXT_GO_CONST.'" onClick="reloadPage();"></td>
						</tr>
						<tr>
							<td>'.$TEXT_MEDIA_TYPE_CONST.'</td>
							<td>'.pulldownFromArray($mediaTypes,'mediaType',$mediaType,'onChange=\'reloadPage();\'','key','').'</td>
						</tr>
						<tr>
							<td colspan="2" class="err">'.@$warning.'</td>
						</tr>						
					</table>
				</td>
				<td valign="top">Auto scan all media *&nbsp; <input type="submit" class="button" name="automatic" value="'.$automatic_label.'"><br><br>
				<em>* Automatic process will scan all media files, both audio and video.</em></td>
			</tr>		
		</table>	
		'.outputItemsToScan($type,$mediaType,$mediadbADO).'
		</form>';
	}else{
		if(isset($_POST['automatic'])){
			if(automaticGrabStatus()){
				$msg=automaticGrab('stop');
			}else{
				$msg=automaticGrab('start');
			}
			header('Location: index.php?section=coverArt&msg='.urlencode($msg));
			exit();
		}
		
		$ids=explode(',',$_POST['ids']);
		$itemName=$_POST['itemName'];
		$toScan='';
		foreach ($ids AS $id){
			if(isset($_POST['item_'.$id])){
				$searchParms='searchIndex='.$mediaTypes[$mediaType];
				// hardcoded first search parameter to "Keywords"
				$Keyword1Type='Keywords';
				$Keyword1Search=cleanString($_POST['Keyword1Search_'.$id]);
				$searchParms.=($Keyword1Search!='')?'&Keyword1Type='.$Keyword1Type.'&Keyword1Search='.urlencode($Keyword1Search):'';
				
				$Keyword2Type=$_POST['Keyword2Type_'.$id];
				$Keyword2Search=cleanString($_POST['Keyword2Search_'.$id]);
				$searchParms.=($Keyword2Type!='0' && $Keyword2Search!='')?'&Keyword2Type='.$Keyword2Type.'&Keyword2Search='.urlencode($Keyword2Search):'';

				$Keyword3Type=$_POST['Keyword3Type_'.$id];
				$Keyword3Search=cleanString($_POST['Keyword3Search_'.$id]);

				$searchParms.=($Keyword3Type!='0' && $Keyword3Search!='')?'&Keyword3Type='.$Keyword3Type.'&Keyword3Search='.urlencode($Keyword3Search):'';
				
				if($searchParms!=''){
					// add the record to the list of items to scan
					$scanUrl='coverArtExtractor.php?'.$itemName.'='.$id.'&'.$searchParms;
					$toScan.=$scanUrl."\n";
					
					// todo: remove after Mircea make his scripts
					$cmd='wget \'http://'.$_SERVER['HTTP_HOST'].'/pluto-admin/'.$scanUrl.'\' --header=\'User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.0.4) Gecko/20060508 Firefox/1.5.0.4\' -O -';
					//echo $cmd.'<br>';
					exec_batch_command($cmd);	
				}
			}
		}
		// processing: need to do something with those urls, put in a file or something

		//echo nl2br($toScan);
		//header('Location: index.php?section=coverArt&type='.$_POST['type'].'&ipp='.$_POST['ipp'].'&page='.$_POST['page'].'&msg='.$TEXT_ITEMS_ADDED_TO_SCAN_QUEUE_CONST);
		header('Location: index.php?section=matchCoverArt&msg='.$TEXT_ITEMS_SCANNED_CONST);
		exit();
	}

	$output->setMenuTitle($TEXT_FILES_AND_MEDIA_CONST.' |');
	$output->setPageTitle($TEXT_COVERART_CONST);
	$output->setNavigationMenu(array($TEXT_COVERART_CONST=>'index.php?section=coverArt'));
	$output->setBody($out);
	$output->setTitle(APPLICATION_NAME.' :: '.$TEXT_COVERART_CONST);
	$output->output();
}

function outputItemsToScan($type,$mediaType,$mediadbADO){
	global $mediaTypes;

	$searchIndex=$mediaTypes[$mediaType];
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/coverArt.lang.php');
		
	// if parameter is not specified, show files without cover arts
	$type=($type=='')?'filesNoCover':$type;
	$ipp=((int)@$_REQUEST['ipp']<=0 || (int)@$_REQUEST['ipp']>100)?10:(int)@$_REQUEST['ipp'];

	switch ($type){
		case 'allFiles':
			$extra=($mediaType==3)?' OR EK_MediaType=5':'';
			$pathSqlFilter=(@$_REQUEST['filterPath']!='')?' AND Path LIKE \'%'.$_REQUEST['filterPath'].'%\'':'';
			$fileSqlFilter=(@$_REQUEST['filterFile']!='')?' AND Filename LIKE \'%'.$_REQUEST['filterFile'].'%\'':'';
			
			$dataArray=getAssocArray('File','PK_File','CONCAT(Path,\'/\',Filename) AS Label',$mediadbADO,'LEFT JOIN CoverArtScan ON CoverArtScan.FK_File=PK_File WHERE (EK_MediaType ='.$mediaType.' '.$extra.') AND (Scanned IS NULL OR Scanned=0) AND Missing=0 '.$pathSqlFilter.$fileSqlFilter.' LIMIT 0,200');
			$itemName='fileID';
			$title=$TEXT_SHOW_ALL_FILES_CONST;
		break;
		case 'filesNoCover':
			/* Complete query:
			
			SELECT PK_File,CONCAT(Path,Filename) AS Filename
			FROM File
			LEFT JOIN Picture_File ON Picture_File.FK_File=PK_File
			LEFT JOIN CoverArtScan ON CoverArtScan.FK_File=PK_File
			WHERE EK_MediaType IN (3,4,5) AND (Scanned IS NULL OR Scanned=0)
			GROUP BY PK_File
			HAVING count(FK_Picture)=0 
			
			*/
			// todo: remove limit
			$extra=($mediaType==3)?' OR EK_MediaType=5':'';
			$pathSqlFilter=(@$_REQUEST['filterPath']!='')?' AND Path LIKE \'%'.$_REQUEST['filterPath'].'%\'':'';
			$fileSqlFilter=(@$_REQUEST['filterFile']!='')?' AND Filename LIKE \'%'.$_REQUEST['filterFile'].'%\'':'';
			$dataArray=getAssocArray('File','PK_File','CONCAT(Path,\'/\',Filename) AS Label',$mediadbADO,'LEFT JOIN Picture_File ON  Picture_File.FK_File=PK_File LEFT JOIN CoverArtScan ON CoverArtScan.FK_File=PK_File WHERE (EK_MediaType='.$mediaType.' '.$extra.') AND (Scanned IS NULL OR Scanned=0) AND Missing=0 '.$pathSqlFilter.$fileSqlFilter.' GROUP BY PK_File HAVING count(FK_Picture)=0 ORDER BY PK_File DESC LIMIT 0,200');

			$itemName='fileID';
			$title=$TEXT_SHOW_FILES_NO_COVERART_CONST;
		break;
		case 'allDiscs':
			$dataArray=getAssocArray('Disc','PK_Disc','Slot AS Label',$mediadbADO,'LEFT JOIN CoverArtScan ON CoverArtScan.FK_Disc=PK_Disc WHERE (EK_MediaType ='.$mediaType.' OR EK_MediaType=5) AND (Scanned IS NULL OR Scanned=0)');
			$itemName='discID';
			$title=$TEXT_SHOW_ALL_DISCS_CONST;
		break;
		case 'discsNoCover':
			$dataArray=getAssocArray('Disc','PK_Disc','Slot AS Label',$mediadbADO,'LEFT JOIN Picture_Disc ON  Picture_Disc.FK_Disc=PK_Disc LEFT JOIN CoverArtScan ON CoverArtScan.FK_Disc=PK_Disc WHERE EK_MediaType='.$mediaType.' AND (Scanned IS NULL OR Scanned=0) GROUP BY PK_Disc HAVING count(FK_Picture)=0');
			$itemName='discID';
			$title=$TEXT_SHOW_FILES_NO_COVERART_CONST;
		break;
		case 'allAttributes':
			$dataArray=getAssocArray('Attribute','PK_Attribute','Name AS Label',$mediadbADO,'LEFT JOIN CoverArtScan ON CoverArtScan.FK_Attribute=PK_Attribute WHERE (Scanned IS NULL OR Scanned=0)');
			$itemName='attributeID';
			$title=$TEXT_SHOW_ALL_ATTRIBUTES_CONST;
		break;
		case 'attributesNoCover':
			$dataArray=getAssocArray('Attribute','PK_Attribute','Name AS Label',$mediadbADO,'LEFT JOIN Picture_Attribute ON  Picture_Attribute.FK_Attribute=PK_Attribute LEFT JOIN CoverArtScan ON CoverArtScan.FK_Attribute=PK_Attribute WHERE (Scanned IS NULL OR Scanned=0) GROUP BY PK_Attribute HAVING count(FK_Picture)=0');
			$itemName='attributeID';
			$title=$TEXT_SHOW_ALL_ATTRIBUTES_WITH_NO_COVER_ART_CONST;
		break;
		default:
			return 'Invalid parameter';
		break;
	}
	

	
	$out='<div align="center"><b>'.$title.'</b></div><br><br>';
	$out.= multi_page_items($dataArray,$searchIndex,$ipp,$type,$itemName,$mediaType,$mediadbADO);
	return $out;
}

function multi_page_items($dataArray,$searchIndex,$ipp,$type,$itemName,$mediaType,$mediadbADO){
	
	/* @var $mediadbADO ADOConnection */
	/* @var $rs ADORecordSet */
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/common.lang.php');
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/coverArt.lang.php');

	$page=(isset($_REQUEST['page']))?(int)$_REQUEST['page']:1;
	$noPages=floor(count($dataArray)/$ipp);
	if($searchIndex=='Music'){
		// music options allowed as parameters for Amazon search index music
		$filterOptions=array(
			'Artist'=>'Artist',
			'Title'=>'Album'
		);
	}else{
		// music options allowed as parameters for Amazon search index DVD
		$filterOptions=array(
			'Actor'=>'Actor',
			'Director'=>'Director',
			'Publisher'=>'Publisher',
			'Title'=>'Title'
		);
	}	
	
	$links='';
	for($i=1;$i<=$noPages;$i++){
		$links.=($i==$page)?$i.'&nbsp; ':'<a href="index.php?section=coverArt&type='.$type.'&ipp='.$ipp.'&page='.$i.'&mediaType='.$mediaType.'">'.$i.'</a>&nbsp; ';
	}
	
	// set path and filename filters
	// filterPathPulldown('filterPath',@$_REQUEST['filterPath'],$mediadbADO) - removed due slow loading time
	$filterBox='<b>'.$TEXT_SEARCH_FOR_PATH_CONST.' <input type="text" name="filterPath" value="'.@$_REQUEST['filterPath'].'"> '.$TEXT_AND_OR_FILENAME_CONST.'</b> <input type="text" name="filterFile" value="'.@$_REQUEST['filterFile'].'">';
	$filterBox.=' <input type="button" class="button" name="searchBtn" value="'.$TEXT_GO_CONST.'" onClick="reloadPage();">';
	
	$out='
	<script>
	function selAllCheckboxes(val)
		{
		   for (i = 0; i < coverArt.elements.length; i++)
		   {
			tmpName=coverArt.elements[i].name;
		     if (coverArt.elements[i].type == "checkbox")
		     {
		         coverArt.elements[i].checked = val;
		     }
		   }
		}
	</script>	
	<table cellspacing="0" cellpadding="2">
		<tr>
			<td colspan="4" align="left">'.$filterBox.'</td>		
			<td colspan="3" align="right">'.$links.'</td>
		</tr>
		<tr class="alternate_back">
			<td align="left">&nbsp;</td>
			<td align="center"><B>'.$TEXT_ITEM_CONST.'</B></td>
			<td align="center"><B>'.$TEXT_KEYWORDS_CONST.'</B></td>
			<td align="center" colspan="4"><B>'.$TEXT_FILTERS_CONST.'</B></td>
		</tr>
	
					
	';
	
	//get title and performer for media
	$filters=array();	
	if(($type=='filesNoCover' || $type=='allFiles') && $mediaType==4 && count($dataArray)>0){
		$rs=$mediadbADO->Execute('SELECT * FROM File_Attribute INNER JOIN AttributeType ON FK_AttributeType=PK_AttributeType INNER JOIN Attribute ON FK_Attribute=PK_Attribute WHERE FK_AttributeType IN (2,3) AND FK_File IN ('.join(',',array_keys($dataArray)).')');
		while($row=$rs->FetchRow()){
			$filters[$row['FK_File']][$row['FK_AttributeType']]=$row['Name'];
		}
	}
	
	$pos=0;
	$ids=array();
	foreach ($dataArray AS $id=>$label){
		$pos++;
		if($pos>($ipp*($page-1)) && $pos<$ipp*$page){
			$ids[]=$id;
			$class=($pos%2!=0)?'':'alternate_back';
			$recordLabel=($type=='allFiles' || $type=='filesNoCover')?substr($label,strrpos($label,'/')+1):$label;
			$toSearch=substr($recordLabel,0,strrpos($recordLabel,'.'));
			$toSearch=str_replace(array('-','(',')'),' ',$toSearch);
			
			$filter2='';
			$filterSelected2='';
			$filter3='';
			$filterSelected3='';
			
			if(isset($filters[$id][2]) && isset($filters[$id][3]) && $mediaType==4){
				
				if(@trim($filters[$id][2])!=''){
					$filter2=@$filters[$id][2];	
					$filterSelected2='Artist';
				}
				if(@trim($filters[$id][3])!=''){
					$filter3=@$filters[$id][3];				
					$filterSelected3='Title';
				}
				$toSearch='';
			}

			$out.='
			<tr class="'.$class.'">
				<td align="center"><input type="checkbox" name="item_'.$id.'" value="'.$id.'" checked></td>
				<td title="'.$label.'" align="left">'.$recordLabel.'</td>
				<td align="left"><input type="text" name="Keyword1Search_'.$id.'" value="'.$toSearch.'"></td>
				<td align="left">'.pulldownFromArray($filterOptions,'Keyword2Type_'.$id,$filterSelected2).'</td>
				<td align="left"><input type="text" name="Keyword2Search_'.$id.'" value="'.@$filter2.'"></td>
				<td align="left">'.pulldownFromArray($filterOptions,'Keyword3Type_'.$id,$filterSelected3).'</td>
				<td align="left"><input type="text" name="Keyword3Search_'.$id.'" value="'.@$filter3.'"></td>
			</tr>';
		}
	}
	if(count($dataArray)>0){
	
		$out.='
		<tr class="alternate_back">
			<td colspan="6"><a href="javascript:selAllCheckboxes(true);">[ '.$TEXT_SELECT_ALL_CONST.' ]</a> <a href="javascript:selAllCheckboxes(false);">[ '.$TEXT_UNSELECT_ALL_CONST.' ]</a></td>
			<td align="right"><input type="submit" class="button" name="scan" value="'.$TEXT_SCAN_FOR_COVER_ARTS_CONST.'"></td>
		</tr>';
	}else{
		$out.='	
		<tr>
			<td colspan="7" align="center">'.$TEXT_NO_RECORDS_CONST.'</td>
		</tr>';
	}
	$out.='	
		<tr>
			<td colspan="7" align="right">'.$links.'</td>
		</tr>
	</table>
	<input type="hidden" name="itemName" value="'.$itemName.'">
	<input type="hidden" name="ids" value="'.join(',',$ids).'">
	<input type="hidden" name="page" value="'.$page.'">
	<input type="hidden" name="type" value="'.$type.'">
	<input type="hidden" name="ipp" value="'.$ipp.'">
	';
	
	return $out;
}

function automaticGrabStatus(){
	$cmd='sudo -u root /usr/pluto/bin/grabcoverart.sh status';
	$ret=exec_batch_command($cmd,1);
	
	if(ereg('Daemon allready running',$ret)){
		return true;		
	}
	return false;
}

function automaticGrab($action){
	$cmd='sudo -u root /usr/pluto/bin/grabcoverart.sh '.$action;
	$ret=exec_batch_command($cmd,1);
	
	return $ret;
}

function filterPathPulldown($name,$selected,$dbADO){
	$res=$dbADO->Execute('SELECT DISTINCT Path FROM File WHERE Missing=0  ORDER BY Path ASC');
	$pathsArray=array();
	while($row=$res->FetchRow()){
		$parts=explode('/',substr($row['Path'],1));

		$full='';
		$indent='';
		for($i=0;$i<count($parts);$i++){
			$node=$full.'/'.$parts[$i];
			if(!in_array($node,array_keys($pathsArray))){
				$pathsArray[$node]=$indent.$parts[$i];
			}
			$full=$node;
			$indent.='&nbsp;&nbsp;&nbsp;&nbsp;';
		}
	}
	
	return pulldownFromArray($pathsArray,$name,$selected);
}
?>