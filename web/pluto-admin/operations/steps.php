<?
function steps($output,$dbADO) {
	// include language files
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/common.lang.php');
	include(APPROOT.'/languages/'.$GLOBALS['lang'].'/steps.lang.php');
	
	/* @var $dbADO ADOConnection */

	$out='';
	$currentSection = @cleanString($_REQUEST['rightSection']);
	$installationID = (int)@$_SESSION['installationID'];
	$currentItem=(isset($_REQUEST['pageSetup']) && (int)$_REQUEST['pageSetup']>0)?(int)$_REQUEST['pageSetup']:$GLOBALS['InstallationPage'];

	if(isset($_REQUEST['senderID']) && (int)$_REQUEST['senderID']>0){
		$senderStep=(int)$_REQUEST['senderID'];
		$querySS='SELECT * FROM SetupStep WHERE FK_Installation=? AND FK_PageSetup=?';
		$resSS=$dbADO->Execute($querySS,array($installationID,$senderStep));
		if($resSS->RecordCount()==0){
			$insertSS='INSERT INTO SetupStep (FK_Installation, FK_PageSetup) VALUES (?,?)';
			$dbADO->Execute($insertSS,array($installationID,$senderStep));
		}
	}

$start_time=getmicrotime();		
	$scriptInHead ='
		<script>
		function setMenuItem(mainUrl,selfPage,sender)
		{
			if(mainUrl!=\'\'){
				if(top.basefrm.document.forms[0] && top.basefrm.document.forms[0].action){
					top.basefrm.document.forms[0].action.value=\'externalSubmit\';
					top.basefrm.document.forms[0].submit();
				}
				top.basefrm.location=mainUrl;
			}
			else{
				top.basefrm.location=\'index.php?section=installationSettings\';
			}
			self.location=\'index.php?section=wizard&pageSetup=\'+selfPage+\'&senderID=\'+sender;				
		}
		</script>
	';
	
	$GLOBALS['pagesArray']=array();
	$GLOBALS['descriptionArray']=array();
	$GLOBALS['linksArray']=array();
	$GLOBALS['levelArray']=array();
	$GLOBALS['isSync']=array();
	
	$out.='<table border="0" cellpading="2" cellspacing="0" width="100%">
			<tr>
				<td colspan="3" align="center">
					<a href="index.php?section=wizard"><img src="include/images/logo_pluto.jpg" border="0"></a>
				</td>
			</tr>
	';
	if (isset($_SESSION['userLoggedIn']) && $_SESSION['userLoggedIn']==true) {
		$out.='
				<tr>
					<td valign="top">'.($currentSection=='login'?'&raquo;':'').'<a href="index.php?section=login&action=logout" target="basefrm" >'.$TEXT_LOGOUT_CONST.'</a></td>					
				</tr>
		';
	} else {
		$out.='
				<tr>
					<td valign="top">'.($currentSection=='login'?'&raquo;':'').'<a href="index.php?section=login"  target="basefrm">'.$TEXT_LOGIN_CONST.'</a></td>
				</tr>
		';
	}	
	if(isset($_SESSION['installationID'])){
		$selectWizard = "
			SELECT DISTINCT PageSetup.*, SetupStep.FK_Installation
			FROM PageSetup 
			LEFT JOIN DeviceTemplate ON PageSetup.FK_Package=DeviceTemplate.FK_Package
			LEFT JOIN Device ON Device.FK_DeviceTemplate=PK_DeviceTemplate
			LEFT JOIN SetupStep ON FK_PageSetup=PK_PageSetup AND (SetupStep.FK_Installation IS NULL OR SetupStep.FK_Installation=?)
			WHERE FK_PageSetup_Parent = 1 AND showInTopMenu = 1 AND Website=1 AND (PageSetup.FK_Package IS NULL OR (PK_Device IS NOT NULL AND Device.FK_Installation=?))
			ORDER BY OrderNum ASC
			";
		$resWizard = $dbADO->Execute($selectWizard,array($installationID,$installationID));
		while ($rowNode = $resWizard->FetchRow()) {
			$GLOBALS['pagesArray'][]=$rowNode['PK_PageSetup'];
			$GLOBALS['linksArray'][]=$rowNode['pageURL'];
			$GLOBALS['descriptionArray'][]=$rowNode['Description'];
			$GLOBALS['levelArray'][]=0;
			$GLOBALS['isSync'][]=(($rowNode['FK_Installation']!='')?1:0);
			getChildPages($rowNode['PK_PageSetup'],1,$dbADO);
		}	
		$out.='
			<tr>
				<td colspan="3" align="center">
					<h3>'.$TEXT_WIZARD_CONST.'</h3>
				</td>
			</tr>';
		foreach ($GLOBALS['descriptionArray'] AS $pos=>$description){
			$fromPage=$GLOBALS['pagesArray'][$pos];
			$toPos=(isset($GLOBALS['pagesArray'][$pos+1]) && $GLOBALS['linksArray'][$pos+1]!='')?$pos+1:$pos+2;
			
			$toPage=isset($GLOBALS['pagesArray'][$toPos])?$GLOBALS['pagesArray'][$toPos]:$GLOBALS['pagesArray'][$pos];
			
			$wizardLink=($GLOBALS['linksArray'][$pos]!='')?'<a <a href="#" onClick="setMenuItem(\''.$GLOBALS['linksArray'][$pos].'\',\''.$fromPage.'\',\''.$fromPage.'\')">'.$description.'</a>':'<b>'.$description.'</b>';
			if($GLOBALS['isSync'][$pos]==1)
				$wizardLink.='<img src="include/images/sync.gif" align="middle">';
	
			$nextLink=($currentItem==@$GLOBALS['pagesArray'][$pos])?'<a <a href="#" onClick="setMenuItem(\''.@$GLOBALS['linksArray'][$toPos].'\',\''.$toPage.'\',\''.$fromPage.'\')">'.$TEXT_NEXT_CONST.'</a>':'&nbsp;';
			
			$out.='
				<tr bgcolor="'.(($currentItem==$GLOBALS['pagesArray'][$pos])?'#CCCCCC':'').'">
					<td></td>
					<td height="22">'.indent($GLOBALS['levelArray'][$pos]).$wizardLink.'</td>
					<td align="right">'.$nextLink.'</td>
				</tr>
				';	
		}
		$out.='
			<tr>
				<td colspan="2" align="center">&nbsp;</td>
			</tr>
			<tr>
				<td colspan="2" align="center"><a href="index.php?section=leftMenu"><B>Back to devices</B></a></td>
			</tr>';
	}
		$out.='
	</table>';
	$end_time=getmicrotime();			
	//$out.='<br><p class="normaltext">Page generated in '.round(($end_time-$start_time),3).' s.';
	
	$output->setScriptInHead($scriptInHead);
	$output->setScriptInBody('bgColor="#F0F3F8"');
	$output->setBody($out);
	$output->setTitle(APPLICATION_NAME);			
	$output->output();  
}

function getChildPages($parentNode,$level,$dbADO)
{
	$selectNodes = '
		SELECT DISTINCT PageSetup.*, SetupStep.FK_Installation
		FROM PageSetup 
		LEFT JOIN DeviceTemplate ON PageSetup.FK_Package=DeviceTemplate.FK_Package
		LEFT JOIN Device ON Device.FK_DeviceTemplate=PK_DeviceTemplate
		LEFT JOIN SetupStep ON FK_PageSetup=PK_PageSetup AND (SetupStep.FK_Installation IS NULL OR SetupStep.FK_Installation=?)
		WHERE FK_PageSetup_Parent = ? AND showInTopMenu = 1 AND Website=1 AND (PageSetup.FK_Package IS NULL OR (PK_Device IS NOT NULL AND Device.FK_Installation=?))		
		ORDER BY OrderNum ASC';
	$resNodes= $dbADO->Execute($selectNodes,array($_SESSION['installationID'],$parentNode,$_SESSION['installationID']));
	while($row=$resNodes->FetchRow()){
		$GLOBALS['pagesArray'][]=$row['PK_PageSetup'];
		$GLOBALS['linksArray'][]=$row['pageURL'];
		$GLOBALS['descriptionArray'][]=$row['Description'];
		$GLOBALS['levelArray'][]=$level;
		$GLOBALS['isSync'][]=(($row['FK_Installation']!='')?1:0);
		getChildPages($row['PK_PageSetup'],$level+1,$dbADO);
	}
}

function indent($level)
{
	$out='';
	for ($i=0;$i<$level;$i++){
		$out.='&nbsp;&nbsp;&nbsp;&nbsp;';
	}
	return $out;
}
?>