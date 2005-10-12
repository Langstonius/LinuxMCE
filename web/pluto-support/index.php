<?php
	session_name('Pluto');
	session_start();


	if($_SERVER['HTTP_HOST']=='www.plutohome.com'){
		header('Location: http://plutohome.com'.$_SERVER['PHP_SELF'].'?'.$_SERVER['QUERY_STRING']);
	}
	
	require('include/config/config.inc.php');
	require('include/utils.inc.php');
	require('include/masterusers.inc.php');
	require('include/template.class.inc.php');
	require('include/package.inc.php');

	// autologin check: if cookie is set grab the user's data from database
	if (@$_SESSION['userIsLogged']!="yes"){
		//print_r($_COOKIE);
		if(isset($_COOKIE['PlutoHomeAutoLogin'])){
			parse_str(base64_decode($_COOKIE['PlutoHomeAutoLogin']));
			$isMasterUsers=checkMasterUsers($username, $password,$checkMasterUserUrl,'&FirstAccount=&Email=&PlutoId=&Pin=');
			if($isMasterUsers[0]){
				parse_str($isMasterUsers[1]);
				$_SESSION['userID'] = $MasterUsersID;
				$_SESSION['PlutoId'] = $PlutoId;
				$_SESSION['Pin'] = $Pin;
				$_SESSION['username'] = $username;
				$_SESSION['userIsLogged']="yes";
				$_SESSION['categ']=$FirstAccount;
				$_SESSION['Email']=$Email;
				$_SESSION['extPassword']=trim($extPassword);
				$_SESSION['userLoggedIn'] = true;
			}
		}
	}
	// end autologin check	
	
	$section = @$_REQUEST['section'];
	$out='';
	$package=(isset($_SESSION['package']))?$_SESSION['package']:0;
	//echo 'pack : '.$package;
	
	switch ($section) {	
		case '':
			$leftFile='documents/leftMenu';
			$leftParams='';

			$rightFile='documents/documentDisplay';
			$rightParams='&docID=1';
		break;
		case 'home':
			$leftFile='operations/leftHome';
			$leftParams='';

			$rightFile='operations/home';
			$rightParams='&docID=main';
		break;
		case 'logout':
			$leftFile='operations/leftHome';
			$leftParams='';

			$rightFile='operations/logout';
			$rightParams='';
		break;	
		case 'forum':
			header("Location: operations/forum.php?".$_SERVER['QUERY_STRING']);
		break;	
		case 'platforms':
			$leftFile='operations/leftHome';
			$leftParams='&docID='.@$_REQUEST['docID'];

			$rightFile='operations/platforms';
			$rightParams='&pkid='.@$_REQUEST['pkid'];
		break;
		case 'versions':
			$leftFile='operations/leftVersions';
			$leftParams='';

			$rightFile='operations/versions';
			$rightParams='';
		break;
		case 'authors':
			$leftFile='operations/leftHome';
			$leftParams='';

			$rightFile='operations/authors';
			$rightParams='';
		break;
		case 'license':
			$leftFile='operations/leftHome';
			$leftParams='';

			$rightFile='operations/license';
			$rightParams='';
			break;
		case 'help':
			$leftFile='operations/leftHome';
			$leftParams='';

			$rightFile='operations/help';
			$rightParams='';
			break;
		case 'faq':
			$leftFile='operations/leftHome';
			$leftParams='';

			$rightFile='operations/faq';
			$rightParams='';
			break;
		case 'mail':
			$leftFile='operations/leftHome';
			$leftParams='';

			$rightFile='operations/mail';
			$rightParams='';
			break;
		case 'news':
			$leftFile='operations/leftHome';
			$leftParams='';

			$rightFile='operations/news';
			$rightParams='';
		break;
		case 'usersmanual':
			$queryUM = 'SELECT FK_Document_UsersManual
							FROM Package
							WHERE PK_Package = ?';
			$resUM = $dbADO->Execute($queryUM,$package);
			$rowUM = $resUM->FetchRow();
			$idUM = $rowUM['FK_Document_UsersManual'];
			
			if((int)$idUM!=0){
				$leftFile='documents/leftMenu';
				$leftParams='&docID='.$idUM;
				
				$rightFile='documents/documentDisplay';
				$rightParams='&docID='.$idUM;
			}else{
				$leftFile='operations/leftHome';
				$leftParams='';
				
				$rightFile='documents/error';
				$rightParams='&msg=No user manual yet.';
			}			
		break;
		case 'progguide':
			$queryUM = 'SELECT FK_Document_ProgrammersGuide
							FROM Package
							WHERE PK_Package = ?';
			$resUM = $dbADO->Execute($queryUM,$package);
			$rowUM = $resUM->FetchRow();
			$idUM = $rowUM['FK_Document_ProgrammersGuide'];

			if((int)$idUM!=0){
				$leftFile='documents/leftMenu';
				$leftParams='&docID='.$idUM;
				
				$rightFile='documents/documentDisplay';
				$rightParams='&docID='.$idUM;
			}else{
				$leftFile='operations/leftHome';
				$leftParams='';
				
				$rightFile='documents/error';
				$rightParams='&msg=No programmer\'s guide yet.';
			}
		break;
		case 'down_package':
			$leftFile='operations/leftHome';
			$leftParams='';

			$rightFile='operations/download';
			$rightParams='';
		break;
		case 'source_package':
			$leftFile='operations/leftHome';
			$leftParams='';

			$rightFile='operations/source';
			$rightParams='';
		break;
		case 'document':
			$leftFile='documents/leftMenu';
			$leftParams='&docID='.@$_REQUEST['docID'];
			
			$rightFile='documents/documentDisplay';
			$rightParams='&docID='.@$_REQUEST['docID'];
		break;
		case 'mainDownload':
			$leftFile='operations/leftHome';
			$leftParams='&docID='.@$_REQUEST['docID'];
			
			$rightFile='operations/mainDownload';
			$rightParams='';
		break;
		case 'packageDownload':
			$leftFile='operations/leftHome';
			$leftParams='&docID='.@$_REQUEST['docID'];
			
			$rightFile='operations/packageDownload';
			$rightParams='&pkid='.@$_REQUEST['pkid'];
		break;
		case 'modules':
			$leftFile='operations/leftHome';
			$leftParams='';

			$rightFile='operations/modules';
			$rightParams='';
		break;
		case 'pluto':
			$leftFile='operations/leftHome';
			$leftParams='';

			$rightFile='home';
			$rightParams='';
		break;
		case 'addDocument':
			$leftFile='documents/leftMenu';
			$leftParams='&docID='.$_REQUEST['parentID'].'&edit='.$_REQUEST['edit'];

			$rightFile='documents/addDocument';
			$rightParams='&parentID='.$_REQUEST['parentID'];
		break;
		
		// pluto admin pages
		case 'publicAdmin':
			$leftFile='operations/leftHome';
			$leftParams='';

			$rightFile='operations/publicAdmin';
			$rightParams='';
		break;				
		default:
			if(@file_exists('admin/'.$section.'.php')){
				$out='no frames';
				$popup='admin/'.$section.'.php';
			}else
				$out='{Invalid section}'.$section;	
		break;
	}
	
	
	
	// display frameset if no errors
	if($out==''){
	$out='
	<HTML>
		<HEAD>
		
		<meta http-equiv="Content-Language" content="en">        
		        <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
		        <meta http-equiv="Pragma" content="no-cache" />
		        <meta http-equiv="Cache-Control" content="no-cache" />
		        <meta http-equiv="Expires" content="Mon, 03 Jan 2000 00:00:01 GMT" />
		        <meta name="keywords" content="">
				<meta name="description" content="">
		        <meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1" />
		        
		<title>'.APPLICATION_NAME.'</title>		
			<FRAMESET cols="250,*" onResize="if (navigator.family == \'nn4\') window.location.reload()" border="1"> 
		  		<FRAME src="left.php?section='.$leftFile.$leftParams.'" name="treeframe" > 
		  		<FRAME SRC="right.php?section='.$rightFile.$rightParams.'" name="basefrm"> 
			</FRAMESET>	
		</HTML>
		
	';
	
	print $out;
	}else{
		require_once('include/plutoAdminUtils.inc.php');
			
		$section=$_REQUEST['section'];
		$output = new Template($dbADO);
		$output->setTemplateFileType('popup');
		$publicADO = &ADONewConnection('mysql'); 
  		$publicADO->NConnect($dbPlutoAdminServer,urlencode($dbPlutoAdminUser),urlencode($dbPlutoAdminPass),urlencode($dbPlutoAdminDatabase));
  		
		include_once($popup);
	}
?>