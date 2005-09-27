<?
if(!isset($_SESSION['userIsLogged'])){
	header('Location: index.php?section=login');
	exit();
}
function myPluto($output,$dbADO,$conn) {
	global $forumHost,$MantisHost;
	global $dbDealerServer, $dbDealerUser, $dbDealerPass, $dbDealerDatabase;
	global $dbManufacturerServer, $dbManufacturerUser, $dbManufacturerPass, $dbManufacturerDatabase;
	
	/* @var $dbADO ADOConnection */
	
	if ($_SESSION['userIsLogged']!="yes") {
		header("Location: index.php");
	}

	if(isset($_SESSION['setAutoCookie']) && $_SESSION['setAutoCookie']==1){
		$cookieStr='username='.$_SESSION['username'].'&password='.$_SESSION['Password'].'&extPassword='.$_SESSION['extPassword'];
		$cookieIsSet=setcookie("PlutoHomeAutoLogin",base64_encode($cookieStr),time()+3600*24*365,"/", false);
		unset($_SESSION['setAutoCookie']);
	}
	
	$FK_Users=(isset($_SESSION['userID']))?(int)$_SESSION['userID']:0;

	$queryInstallations='
			SELECT * FROM Installation_Users
				INNER JOIN Installation ON FK_Installation=PK_Installation
				INNER JOIN Users ON FK_Users=PK_Users
			WHERE Installation_Users.FK_Users=?';
	$res=$dbADO->Execute($queryInstallations,$FK_Users);
	if($res->RecordCount()==0)
		$installationsText='No Pluto installations.<br><br>
		<b>To get Pluto</b>, choose <a href="index.php?section=wizard&step=1&instid=0">Start wizard</a>.';
	else {
		$installationsText='You have the following installations:
		<table width="100%" border="0">';
		$isDealer=0;
		$isManufacturer=0;
		while($rowInstallations=$res->FetchRow()){
			if(@$rowInstallations['EK_Dealer']!='')
				$isDealer=1;
			if(@$rowInstallations['EK_Manufacturer']!='')
				$isManufacturer=1;
				
			$installationsText.='
			<tr>
				<td colspan="2">Installation no. <b>'.$rowInstallations['PK_Installation'].'</b></td>
			</tr>
			<tr bgcolor="#F5F8FF">
				<td>Description: <a href="index.php?section=wizard&step=8&instid='.$rowInstallations['PK_Installation'].'"><b>'.stripslashes($rowInstallations['Description']).'</b></a></td>
				<td align="right" width="100"><a href="javascript:if(confirm(\'Are you sure you want to delete the installation?\'))self.location=\'index.php?section=deleteInstallation&dID='.$rowInstallations['PK_Installation'].'\'">[ Delete ]</a></td>
			</tr>';
		}
		$installationsText.='</table><br><a href="index.php?section=wizard&step=1&instid=0"><b>Start wizard</b></a><br>';
	}

	if(isset($_GET['redirect'])){
		if($_GET['redirect']=='forum'){
			header("Location: support/phpbb2/login.php?username=".$_SESSION['username']."&password=".$_SESSION['extPassword']."&login=1");
			exit();
		}
		else{
			header("Location: ".$MantisHost."login.php?username=".$_SESSION['username']."&password=".$_SESSION['extPassword']."&login=1");
			exit();
		}
	}
	$msg=(isset($_REQUEST['msg']))?'<span class="confirm">You had deleted the installation no. '.$_REQUEST['msg'].'</span>':'';
	switch((int)@$_REQUEST['err']){
		case 1:
			$errMsg='<span class="err">Installation ID is not specified.</span>';
		break;
		case 2:
			$errMsg='<span class="err">You are allowed to delete only your own installations.</span>';
		break;
		default:
			$errMsg='';
	}
	
	// extract dealer, if any
	$dealerADO=dbConnection($dbDealerServer,$dbDealerUser,$dbDealerPass,$dbDealerDatabase);
	$userDealer=getFieldsAsArray('Dealer_Users','PK_Dealer,Name',$dealerADO,'INNER JOIN Dealer ON FK_Dealer=PK_Dealer WHERE EK_Users='.$FK_Users);
	if(count(@$userDealer['PK_Dealer'][0])==0){
		$hasDealerLink='';
	}else{
		$hasDealerLink='Dealer: <a href="index.php?section=dealer_area">'.@$userDealer['Name'][0].'</a>';
	}
	$dealerLink=(count($userDealer)==0)?'':$hasDealerLink;

	// extract manufacturer, if any
	$manufacturerADO=dbConnection($dbManufacturerServer,$dbManufacturerUser,$dbManufacturerPass,$dbManufacturerDatabase);
	$userManufacturer=getFieldsAsArray('Manufacturer_Users','EK_Manufacturer,EK_Manufacturer',$manufacturerADO,'WHERE EK_Users='.$FK_Users);
	$manufacturersArray=getAssocArray('Manufacturer','EK_Manufacturer','Name',$manufacturerADO);
	$manufacturersIDs=array_values($userManufacturer);
	if(count(@$userManufacturer['EK_Manufacturer'][0])==0){
		$hasManufacturerLink='';
	}else{
		$hasManufacturerLink='Manufacturer: <a href="index.php?section=manufacturer_area">'.@$manufacturersArray[$userManufacturer['EK_Manufacturer'][0]].'</a>';
	}
	
	$manufacturerLink=(count($userManufacturer)==0)?'':$hasManufacturerLink;


	
	$out = '
      		<table width="100%">
	      		<tr>
      				<td align="left" colspan="2" class="insidetable"><img src="images/titles/my_pluto.gif" border="0"></td>
				</tr>
				<tr bgcolor="#F5F8FF">
					<td class="normaltext">You are logged in as <b>'.$_SESSION['username'].'</b></td>
					<td class="normaltext" align="right"><a href="index.php?section=changePassword"><B>Change Password</B></a>&nbsp;<a href="index.php?section=changeEmail"><B>Change Email</B></a></td>
      			</tr>
				<tr>
					<td class="normaltext" colspan="2">&nbsp;</td>
      			</tr>	
				<tr>
					<td class="normaltext" colspan="2">Visit our <a href="support/index.php"><B>support site</B></a> to access <a href="support/index.php?section=home&package=0"><B>online documentation</B></a>, <a href="index.php?section=myPluto&redirect=forum"><B>forums</B></a>, <a href="index.php?section=myPluto&redirect=mantis" target="_blank"><B>bug reports</B></a>, <a href="support/index.php?section=mail"><B>mailing lists</B></a> and our <a href="support/index.php?section=document&docID=11"><B>quick start guide</B></a>.</td>
      			</tr>	
				<tr>
					<td class="normaltext">&nbsp;'.@$msg.@$errMsg.'</td>
					<td class="normaltext" align="right">&nbsp;</td>
      			</tr>	
      			<tr bgcolor="#DADDE4">
      				<td align="center" width="33%" class="normaltext"><b>Installations</b></td>
      				<td align="center" width="33%" class="normaltext"><b>Referrals</b></td>
      			</tr>
      	      	<tr>
      				<td align="left" valign="top" class="normaltext">'.$installationsText.'<br>					
					</td>
      				<td align="center">
						<table width="100%">
							<tr>
								<td colspan="2" align="left" class="normaltext">Pluto has unique and generous rewards for referring both open source and retail customers as well as
								 dealers and manufacturers who license the Pluto platform for their smarthome appliances.  Send us an email or fill out the contact form for details.<br><br>
								<hr>
								'.$dealerLink.'<br>
								'.$manufacturerLink.'
								 </td>
							</tr>
						</table>
					</td>
      			</tr>
	      		<tr>
      				<td align="center" width="33%" class="normaltext" colspan="3">&nbsp;</td>
      			</tr>	
	      		<tr bgcolor="#DADDE4">
					<td align="center" width="34%" class="normaltext"><b>'.((@$isDealer==1)?'Dealer�s corner':'Developer\'s corner').'</b></td>
      				<td align="center" width="33%" class="normaltext"><b>Status</b></td>
      			</tr>
	      		<tr>
      				<td align="left" valign="top" class="normaltext">';
					if(@$isDealer==1){
						$out.='
							<a href="index.php?section=updateProfile"><B>Update profile</B></a><br>
							<a href="index.php?section=requestInstallationAssistance"><B>Request Installation Assistance</B></a><br>
							<a href="index.php?section=dealer_users"><B>Add/delete users to dealer account</B></a><br>';
					}else{
						$out.='Pluto 2 has been written from the ground up to be a very comfortable development platform for open source programmers.<br><br>
      				We have developed class generators that will build a fully complete, ready-to-compile <a href="support/index.php?section=document&docID=51">DCE Devices</a> in minutes.
	     			They\'re standard C++, run on both Linux & Windows, ready talk to any other DCE Device on any platform.  See our <a href="http://plutohome.com/support/index.php?section=document&docID=15">Programmer\'s guide</a> for a quick intro.<br><br>';
					}
					if(@$isManufacturer==1){
						$out.='
						<table width="100%">
							<tr bgcolor="#DADDE4">
								<td align="center"><B>Manufacturer\'s corner</B></td>
							</tr>
							<tr>
								<td><a href="index.php?section=manufacturer_users"><B>Add/delete users to manufacturer account</B></a><br></td>
							</tr>
						</table>						
						';
					}					
					$out.='
      				</td>
      				<td align="left" width="33%" class="normaltext" valign="top">
      				Pluto has made available a preview release of the new Pluto.  This is targeted mainly for developers, since it is not yet ready for end users.
      				Click "New Installation" above to get a Kick-Start CD, or download the installer.
      				</td>
      			</tr>
				<tr bgcolor="#DADDE4">
      				<td align="center" width="33%" class="normaltext"><b>Support Site</b></td>
      				<td align="center" width="34%" class="normaltext"><b>Tools for developers</b></td>
				</tr>
				<tr>
      				<td align="left" width="33%" class="normaltext"  valign="top">
      				With the launch of this preview version, Pluto has put up it\'s new support web site at <a href="http://plutohome.com/support/index.php?section=home">www.plutohome.com/support</a>.<br><br>
      				There is online documentation, support forums, bug tracking and mailing lists.  Plus we have a live chat system so you can talk directly to one of our tech support team, or a developer.
      				</td>
      				<td align="left" width="34%" class="normaltext"  valign="top">
      				Pluto has released several general purpose classes that you may find interesting, such as <a href="http://plutohome.com/support/index.php?section=document&docID=42">sqlCVS</a>, which is basically a CVS for databases.  
      				It\'s what allows different programmers to all work on local copies of Pluto\'s master database and handles check-ins, updates, and remembers who owns each record.<br><br>
      				
      				There are many other libraries too, such as our <a href="http://plutohome.com/support/index.php?section=document&docID=108">Serialize Class</a>, 
      				<a href="http://plutohome.com/support/index.php?section=document&docID=109">sql2cpp</a> class generator,
      				<a href="http://plutohome.com/support/index.php?section=document&docID=41">Request/Action</a> client-server library</a>, 
      				<a href="http://plutohome.com/support/index.php?section=document&docID=39">Bi-Directional communications library</a>.  They\'re all GPL.<br><br>
        			</td>
				</tr>
      		</table>
      	';


	$output->setNavigationMenu(array("MyPluto"=>"index.php?section=myPluto"));

	$output->setScriptCalendar('null');
	$output->setScriptTRColor('null');
	$output->setBody($out);
	$output->setTitle(APPLICATION_NAME."::My Pluto");
	$output->output();
}
?>