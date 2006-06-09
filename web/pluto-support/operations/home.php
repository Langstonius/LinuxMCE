<?
$package=(isset($_SESSION['package']))?$_SESSION['package']:0;
if($package!=0){
	
	$queryDocs = 'SELECT Contents,Comments FROM Package
					INNER JOIN Document ON FK_Document=PK_Document
					where PK_Package=?';
	$resDocs = $dbADO ->Execute($queryDocs,$package);
	$rowDocs = $resDocs->FetchRow();                             	
	
	$out = '<p><h1>**package**</h1></p>
		<p>'.$rowDocs['Comments'].'</p>';

	$documentation = "";
	if ( $rowDocs['Contents'] != "" )
	{
		$documentation = $rowDocs['Contents'];
	}
	else
	{
		$documentation ='
			<p>This is the main home page for the software module <b>**package**</b>. 
			The above menu has links to documentation, downloads, source code, forums, 
			mailing lists and FAQ\'s specific to <b>**package**</b>.</p>
			<p>Many of Pluto\'s <a href=index.php?section=document&docID=7>plug-ins</a> and 
			<a href=index.php?section=document&docID=6>DCE devices</a> have their own home pages.  
			For general help, visit the <a href=index.php?section=home&package=0>general support</a> page.</p>';
	}

	
	$queryDevices = 'SELECT PK_DeviceTemplate,Description FROM DeviceTemplate WHERE FK_Package=?';
	$resDevices = $dbADO->Execute($queryDevices,$package);

	$devices = "";
	$found = 0;
	while ($rowDevices = $resDevices->FetchRow()) 
	{
		if ($found == 1) {
			$out2=' - ';
		}
		$found = 1;
		$devices .= ' <b>'.$rowDevices['Description'].'</b>';
	}

	$out .= $documentation;
	if ( $found == 1 )
	{
		$out .= '<br>This module includes the following DCE Devices: ' . $devices . '.'; 
	}
	if($package!=159){ // Not PlutoVIP
		$out.='<p><h1>Download the software</h1></p>
			<p>There are hundreds of modules and many options in a Pluto Home system.  Normally you don\'t download modules separately.  
			To install a complete Pluto system, we created an Installation Wizard that asks a few questions about your hardware and software environment and builds a custom installer for you.  
			It even gets the source code, if you wish, and builds a script that will compile all the modules you chose.  
			Just register or login, go to the "My Pluto" home page and click "New Installation".</p>
			<p>If you already have Pluto, you can add extra modules modules in the Pluto Admin web site.  They will be downloaded and installed automatically--you don\'t have to do anything.</p>
			<p>If you want to download binaries or source code manually visit the <a href="http://plutohome.com/support/index.php?section=packageDownload&pkid='.$_SESSION['package'].'"><b>Download page for **package**</b></a></p>

			<p>For the brave of heart, you can also visit the main <a href="index.php?section=mainDownload&package=0">Download Page</a> that lists all the modules with 
			dependencies and compatibility.  Or you can <a href="'.$wikiHost.'index.php/'.wikiLink('Building from source').'" target="_blank">build from source</a>.</p>';
	}
}


$output->setBody($out);
$output->setTitle(APPLICATION_NAME);			
$output->output();  		
?>
